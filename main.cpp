#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <vector>
#include <bitset>
#include <stack>
#include <array>
#include <unordered_map>
#include <stdexcept>
#include <set>
#include <memory>
#include <map>

#include "BasicBlock.h"
#include "Instruction.h"

using namespace std;
using namespace instr;
using namespace bb;

struct Program {

    struct Norm1{
        vector<uint8_t> bytes;
        vector<uint8_t> auxdata;
        bool isCreation;
    };

    struct Norm2{
        vector<unique_ptr<Instruction>> instrsCreation;
        map<uint64_t,uint64_t> jumptableCreation;

        vector<unique_ptr<Instruction>> instrsRun;
        map<uint64_t,uint64_t> jumptableRun;

        void print(){
            unsigned i=0;
            for(const auto& e:instrsRun){
                cout<<i++<<" "<<e->toString()<<'\n';
            }
        }
    };

    Program() = default;

    ///ignores the first three lines of given ifstream
    bool isCreationAndPrep(ifstream &istrm) const {
        constexpr long max = numeric_limits<streamsize>::max();
        istrm.ignore(max,'\n');
        istrm.ignore(max,'\n');
        string s;
        getline(istrm,s);
        return s =="Binary: " || s == "Binary:";

    }

    vector<uint8_t> extractAuxdata(vector<uint8_t>& bytes){
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

    ///reads the given evm bytecode, and converts the hex chars to respective int value,
    Norm1 normalize1(const string& filename){
        cout<<"Reading binary code from: "<<filename<<"...\n";
        if (ifstream istrm{filename, ios::binary}) {

            Norm1 n1;
            n1.isCreation = isCreationAndPrep(istrm);
            string s(2,'\0');
            while(istrm.read(&s[0], 2)) {
                n1.bytes.push_back(static_cast<uint8_t >(stoi(s, nullptr, 16)));
            }
            n1.auxdata = extractAuxdata(n1.bytes);

            return n1;

        } throw invalid_argument("Could not read from "+filename);
    }

    ///remove push bytes and create jump table
    Norm2 normalize2(const Norm1 n1){
        Norm2 n2;

        //init instruction vector and jumptable -> creation or run
        bool isCreation = n1.isCreation;
        vector<unique_ptr<Instruction>>* instrs;
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
        const auto* bytes = &n1.bytes;
        for(unsigned idx=0;idx<bytes->size();idx++){
            const auto& opc = bytes->at(idx);

            if(Instruction::Opcode::PUSH1<=opc && opc<=Instruction::Opcode::PUSH32){

                const uint8_t num = opc - (Instruction::Opcode::PUSH1-1);
                bitset<256> t(0);
                for(unsigned j=1;j<=num;j++){
                    t = (t<<8) | bitset<256>(bytes->at(idx+j));
                }

                instrs->push_back(make_unique<Push>(opc,t));

                idx+=num;
                pushCount+=num;
            } else if(Instruction::Opcode::DUP1<=opc && opc<=Instruction::Opcode::DUP16){

                instrs->push_back(make_unique<Dup>(opc));

            } else if(Instruction::Opcode::SWAP1<=opc && opc<=Instruction::Opcode::SWAP16) {

                instrs->push_back(make_unique<Swap>(opc));

            } else if(isCreation && opc==Instruction::Opcode::INVALID) {
                instrs->push_back(make_unique<Instruction>(opc));
                //reset pushCount
                pushCount=0;
                creationCount=idx+1;
                //not looking for the end of creation code anymore
                isCreation=false;
                instrs = &n2.instrsRun;
                jumpTable = &n2.jumptableRun;

            } else {
                instrs->push_back(make_unique<Instruction>(opc));

                if(opc==Instruction::Opcode::JUMPDEST){
                    jumpTable->emplace(idx-creationCount,idx-creationCount-pushCount);
                }
            }
        }
        return n2;
    }

    ///create BasicBlocks with respective fallthrough and jump children
    unique_ptr<BasicBlock> normalize3(Norm2 &n){

        //the bb to jump for specified key(=original byte position)
        map<uint64_t,BasicBlock*> jumpDst;
        //index counter for the basic blocks; inc according to occurrence in bytecode
        unsigned bbIdx=0;
        //index counter for the instructions; inc according to occurrence in bytecode
        uint64_t instrIdx=0;

        auto head = make_unique<BasicBlock>(bbIdx++);
        auto curr = head.get();
        jumpDst.emplace(instrIdx,curr);

        static const set<uint8_t> bbendInstr = {Instruction::Opcode::STOP,
                                         Instruction::Opcode::JUMP,
                                         Instruction::Opcode::JUMPI,
                                         Instruction::Opcode::RETURN,
                                         Instruction::Opcode::REVERT,
                                         Instruction::Opcode::INVALID};

        for(auto it=n.instrsRun.begin();it!=n.instrsRun.end();it++){
            const auto opc = (*it)->getOpcode();
            curr->addInstruction(move(*it));

            //to skip creation of empty bb
            if(++instrIdx==n.instrsRun.size()) continue;

            if(bbendInstr.find(opc)!=bbendInstr.end() || ((*next(it,1))->getOpcode()==Instruction::Opcode::JUMPDEST)){
                //found end of bb: next instr is a new bb
                auto succ = new BasicBlock(bbIdx++);
                jumpDst.emplace(instrIdx,succ);
                if(curr->needsFallthrough())
                    curr->setFallthrough(succ);
                curr = succ;
            }
        }

        stack<bitset<256>> stack;
        head->adjustJumpPtr(stack, jumpDst, n.jumptableRun);

        return head;
    }
};

int main(int argc, char *argv[]) {

    if(argc!=4){
        cerr<<"Requires input filename, out creation and out runtime\n";
        return EXIT_FAILURE;
    }
    const string filename = argv[1];
    const string fout = argv[2];
    const string foutr = argv[3];
    try{
        Program p;
        Program::Norm1 n1 = p.normalize1(filename);
        Program::Norm2 n2 = p.normalize2(n1);
        //nrun2.print();

        auto ele = p.normalize3(n2);
        ele->printBBdot(foutr);

        return EXIT_SUCCESS;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
}