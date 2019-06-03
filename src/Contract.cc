#include <algorithm>
#include "Contract.h"

using namespace evmbca;

Contract::Contract(const string& filename) {
    auto bytes = readBytecode(filename);
    Norm2 n2 = normalize1(move(bytes));
    bbos_creation = normalize2(move(n2.instrsCreation), n2.jumptableCreation);
    bbos_runtime = normalize2(move(n2.instrsRun), n2.jumptableRun);
    bbis_runtime = abstractStack(bbos_runtime);
};

bool Contract::isCreationAndPrep(ifstream &istrm) const {
    constexpr long max = numeric_limits<streamsize>::max();
    istrm.ignore(max,'\n');
    istrm.ignore(max,'\n');
    string s;
    getline(istrm,s);
    return s =="Binary: " || s == "Binary:";

}

vector<uint8_t> Contract::extractAuxdata(vector<uint8_t>& bytes){
    const unsigned length = 43;
    const auto it_end = bytes.begin()+bytes.size();
    const auto it_start = it_end-length;

    vector<uint8_t> auxdata(it_start,it_end);
    bytes.erase(it_start,it_end);

    if(auxdata.front()!=0xa1){
        throw invalid_argument("no a1 auxdata found, but: ");
    }

    return auxdata;
}

vector<uint8_t> Contract::readBytecode(const string& filename){
    cout<<"Reading binary code from: "<<filename<<"...\n";
    if (ifstream istrm{filename, ios::binary}) {

        if(istrm.peek() == std::ifstream::traits_type::eof()){
            throw invalid_argument("File is empty...");
        }

        this->creation = isCreationAndPrep(istrm);
        vector<uint8_t> bytes;
        string s(2,'\0');
        while(istrm.read(&s[0], 2)) {
            bytes.push_back(static_cast<uint8_t >(stoi(s, nullptr, 16)));
        }
        this->auxdata = extractAuxdata(bytes);

        return bytes;

    } throw invalid_argument("Could not read from "+filename);
}

Contract::Norm2 Contract::normalize1(const vector<uint8_t>& bytes){
    Norm2 n2;

    //init instruction vector and jumptable -> creation or run
    bool isCreation = this->creation;
    vector<unique_ptr<Operation>>* instrs;
    map<uint64_t,uint64_t>* jumpTable;
    if(isCreation){
        instrs = &n2.instrsCreation;
        jumpTable = &n2.jumptableCreation;
    } else {
        instrs = &n2.instrsRun;
        jumpTable = &n2.jumptableRun;
    }

    //the number of bytes that are "removed" from the bytes vector, because they belong to a push instruction
    unsigned pushCount =0;
    unsigned creationCount=0;

    for(auto idx=0u;idx<bytes.size();idx++){
        const auto& opc = bytes.at(idx);

        if(Operation::Opcode::PUSH1<=opc && opc<=Operation::Opcode::PUSH32){

            const uint8_t num = opc - (Operation::Opcode::PUSH1-1);
            bitset<256> t(0);
            for(auto j=1;j<=num;j++){
                t = (t<<8) | bitset<256>(bytes.at(idx+j));
            }

            instrs->push_back(make_unique<Push>(opc,t));

            idx+=num;
            pushCount+=num;
        } else if(Operation::Opcode::DUP1<=opc && opc<=Operation::Opcode::DUP16){

            instrs->push_back(make_unique<Dup>(opc));

        } else if(Operation::Opcode::SWAP1<=opc && opc<=Operation::Opcode::SWAP16) {

            instrs->push_back(make_unique<Swap>(opc));

        } else if(isCreation && opc==Operation::Opcode::INVALID) {
            //reset pushCount
            pushCount=0;
            creationCount=idx+1;
            //not looking for the end of creation code anymore
            isCreation=false;
            instrs = &n2.instrsRun;
            jumpTable = &n2.jumptableRun;

        } else {
            instrs->push_back(make_unique<Operation>(opc));

            if(opc==Operation::Opcode::JUMPDEST){
                jumpTable->emplace(idx-creationCount,idx-creationCount-pushCount);
            }
        }
    }

    //Put in statistics?
    //cout<<"Bytes in creation code : "<<creationCount<<'\n';
    //cout<<"Bytes in runtime code : "<<bytes.size()-creationCount<<'\n';

    //remove invalid if last (and unreachable?)
    if(instrs->back()->getOpcode()==Operation::Opcode::INVALID){
        instrs->pop_back();
    }

    return n2;
}

vector<unique_ptr<BasicBlock<Operation>>> Contract::normalize2(vector<unique_ptr<Operation>> &&instrs,
                                                       map<uint64_t, uint64_t> &jumptable){

    //the bb to jump for specified key(=original byte position)
    map<uint64_t,BasicBlock<Operation>*> jumpDst;
    //index counter for the basic blocks; inc according to occurrence in bytecode
    unsigned bbIdx=0;
    //index counter for the instructions; inc according to occurrence in bytecode
    uint64_t instrIdx=0;

    vector<unique_ptr<BasicBlock<Operation>>> bbs;
    bbs.emplace_back(make_unique<BasicBlock<Operation>>(bbIdx++));

    jumpDst.emplace(instrIdx,bbs.back().get());

    static const set<uint8_t> bbendInstr = {Operation::Opcode::STOP,
                                            Operation::Opcode::JUMP,
                                            Operation::Opcode::JUMPI,
                                            Operation::Opcode::RETURN,
                                            Operation::Opcode::REVERT,
                                            Operation::Opcode::INVALID};

    for(auto it=instrs.begin();it!=instrs.end();it++){
        const auto opc = (*it)->getOpcode();
        bbs.back()->addInstruction(move(*it));

        //to skip creation of empty bb
        if(++instrIdx==instrs.size()) continue;

        if(bbendInstr.find(opc)!=bbendInstr.end() || ((*next(it,1))->getOpcode()==Operation::Opcode::JUMPDEST)){
            //found end of bb: next op is a new bb

            const auto prev = bbs.back().get();

            //create the new bb
            bbs.emplace_back(make_unique<BasicBlock<Operation>>(bbIdx++));
            //only if JUMPDEST is a viable jump destination
            jumpDst.emplace(instrIdx,bbs.back().get());

            if(prev->needsFallthrough())
                prev->setFallthrough(bbs.back().get());
        }
    }

    stack<bitset<256>> stack;
    bbs.front()->adjustJumpPtr(stack, jumpDst, jumptable);

    return bbs;
}

vector<unique_ptr<BasicBlock<Instruction>>> Contract::abstractStack(
        const vector<unique_ptr<BasicBlock<Operation>>>& bbos) const{

    const auto nbbs = bbos.size();

    //create empty bbs again as bbs with instructions
    vector<unique_ptr<BasicBlock<Instruction>>> bbis;
    bbis.reserve(nbbs);

    //indicates, whether each bb already instantiated(processed)
    vector<bool> abstracted;
    abstracted.reserve(nbbs);

    //the state of the stack after the respective bb
    vector<Stack> endStacks(nbbs);

    //initialize
    for(auto i=0u;i<nbbs;i++){
        bbis.emplace_back(make_unique<BasicBlock<Instruction>>(i));
        abstracted.emplace_back(false);
    }

    for(auto i=0u;i<nbbs;i++){
        bbis.at(i)->setSuccessorLikeOther(bbos.at(i).get(),bbis);
    }

    unsigned kvar = 1;
    for(const auto& bbo:bbos){
        bbo->abstract(bbis,abstracted,kvar,endStacks);
    }

    return bbis;
}