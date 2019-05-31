#include "Contract.h"
#include "Candidate.h"

using namespace contract;

Contract::Contract(const string& filename) {
    auto bytes = readBytecode(filename);

    Norm2 n2 = normalize1(move(bytes));
    creationHead = normalize2(move(n2.instrsCreation), n2.jumptableCreation);
    runtimeHead = normalize2(move(n2.instrsRun), n2.jumptableRun);
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

Contract::Norm2 Contract::normalize1(const vector<uint8_t> bytes){
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

    cout<<"Bytes in creation code : "<<creationCount<<'\n';
    cout<<"Bytes in runtime code : "<<bytes.size()-creationCount<<'\n';

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

            auto prev = bbs.back().get();

            //create the new bb
            bbs.emplace_back(make_unique<BasicBlock<Operation>>(bbIdx++));
            //only if JUMPDEST is it a viable jump destination
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
        const vector<unique_ptr<BasicBlock<Operation>>> &oldBbs) const{

    map<unsigned,BasicBlock<Operation>*> operations;
    map<unsigned,unsigned> indexMatch;
    for(const auto& bb:oldBbs){
        operations.emplace(bb->getIndex(),bb.get());
        indexMatch.emplace(bb->getIndex(),bb->getIndex());
    }

    vector<unique_ptr<BasicBlock<Instruction>>> bbs;

    //create empty bbs again as bbs with instructions
    for(auto i=0u;i<oldBbs.size();i++){
        bbs.emplace_back(make_unique<BasicBlock<Instruction>>(i));
    }

    //set jump and fallthrough before recursive call, because of merging paths. results in multiple evaluation cycles
    for(auto i=0u;i<oldBbs.size();i++){
        bbs.at(i)->setSuccessorLikeOther(oldBbs.at(i).get(),bbs);
    }

    stack<pair<unsigned,bitset<256>>> stack;
    map<unsigned,Candidate> candidates;

    bbs.front()->instantiate(stack, 0, 1, bbs, candidates, operations, indexMatch);

    //get predecessors
    vector<vector<unsigned>> predecessors;
    predecessors.resize(bbs.size());
    for(auto i=0u;i<bbs.size();i++){
        auto bb = bbs.at(i).get();
        if(bb->hasJump()) predecessors.at(bb->getJumpIndex()).emplace_back(i);
        if(bb->hasFallthrough()) predecessors.at(bb->getFallthroughIndex()).emplace_back(i);
    }

    //remove empty bbs && jumps
    //due to chaining only adjust the ptr of
    for(auto i=0u;i<bbs.size();i++){
        auto bb = bbs.at(i).get();
        BasicBlock<Instruction>* newSuccessor;
        if(bb->contentIsEmpty() && bb->hasSuccessorEligibleFallthrough()){
            newSuccessor = bb->getFallthrough();
        } else if(bb->isAJumpOnlyBb() && bb->hasSuccessorEligibleJump()){
            newSuccessor = bb->getJump();
        } else {
            //nothing to remove..
            continue;
        }

        //remove this bb by assigning successor to eligible predecessor
        for(auto& p:predecessors.at(i)){
            bbs.at(p)->assignSuccessorToEligiblePredecessor(bb->getIndex(), newSuccessor, predecessors,bbs);
        }

        //actually remove it from vector?
    }

    return bbs;
}
