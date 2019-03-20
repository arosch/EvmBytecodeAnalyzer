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
        vector<uint8_t> creation;
        vector<uint8_t> run;
        string auxdata;
    };

    struct Norm2{
        vector<unique_ptr<Instruction>> instrs;
        map<uint64_t,uint64_t> jumptable;

        void print(){
            unsigned i=0;
            for(const auto& e:instrs){
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
        //size of "Binary:" = 7
        return !s.compare("Binary: ");

    }

    ///read
    void extractSubProgram(ifstream& istrm, vector<uint8_t>& bytes){
        string s(2,'\0');
        while(istrm.read(&s[0], 2)) {
            const auto opc = static_cast<uint8_t >(stoi(s, nullptr, 16));
            if(opc == Instruction::Opcode::INVALID)
                return;
            bytes.push_back(opc);
        }
    }

    void extractAuxdata(ifstream& istrm, string& auxdata){
        constexpr unsigned length = 43 << 1;
        string aux(length,'\0');
        istrm.read(&aux[0],length);
        if(aux.substr(0,2)!="a1")
            throw invalid_argument("no a1 auxdata found");
        auxdata = move(aux);
    }

    ///reads the given evm bytecode, and converts the hex chars to respective int value,
    Norm1 normalize1(const string& filename){
        cout<<"Reading binary code from: "<<filename<<"...\n";
        if (ifstream istrm{filename, ios::binary}) {

            Norm1 n1;
            if(isCreationAndPrep(istrm)){
                extractSubProgram(istrm,n1.creation);
            }
            extractSubProgram(istrm,n1.run);
            extractAuxdata(istrm,n1.auxdata);
            return n1;

        } throw invalid_argument("Could not read from "+filename);
    }

    ///remove push bytes and create jump table
    Norm2 normalize2(const vector<uint8_t>& bytes){
        Norm2 n2;
        //the number of bytes that are "removed" from the bytes vector, because they belong to a push instruction
        unsigned pushCount =0;

        for(unsigned idx=0;idx<bytes.size();idx++){
            const auto& opc = bytes.at(idx);

            if(Instruction::Opcode::PUSH1<=opc && opc<=Instruction::Opcode::PUSH32){

                const uint8_t num = opc - (Instruction::Opcode::PUSH1-1);
                bitset<256> t(0);
                for(unsigned j=1;j<=num;j++){
                    t = (t<<8) | bitset<256>(bytes.at(idx+j));
                }

                n2.instrs.push_back(make_unique<Push>(opc,t));

                idx+=num;
                pushCount+=num;
            } else if(Instruction::Opcode::DUP1<=opc && opc<=Instruction::Opcode::DUP16){

                n2.instrs.push_back(make_unique<Dup>(opc));

            } else if(Instruction::Opcode::SWAP1<=opc && opc<=Instruction::Opcode::SWAP16) {

                n2.instrs.push_back(make_unique<Swap>(opc));

            } else {
                n2.instrs.push_back(make_unique<Instruction>(opc));

                if(opc==Instruction::Opcode::JUMPDEST){
                    n2.jumptable.emplace(idx,idx-pushCount);
                }
            }
        }
        return n2;
    }



    ///end of bb without fallthrough:   revert/stop/return/jump
    ///end of bb with fallthrough:      jumpi
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
                                         Instruction::Opcode::REVERT};

        for(auto it=n.instrs.begin();it!=n.instrs.end();it++){
            const auto opc = (*it)->getOpcode();
            curr->addInstruction(move(*it));

            //to skip creation of empty bb
            if(++instrIdx==n.instrs.size()) continue;

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
        head->adjustJumpPtr(stack, jumpDst, n.jumptable);

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
        /*if(!n1.creation.empty()){
            Program::Norm2 ncreate2 = p.normalize2(n1.creation);
            //ncreate2.print();
            auto start = p.normalize3(ncreate2);
            start->printBBdot(fout);
        }*/
        Program::Norm2 ncreate2 = p.normalize2(n1.creation);
        Program::Norm2 nrun2 = p.normalize2(n1.run);
        //nrun2.print();

        auto ele = p.normalize3(nrun2);
        ele->printBBdot(foutr);

        return EXIT_SUCCESS;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
}