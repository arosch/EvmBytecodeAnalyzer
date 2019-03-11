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

using namespace std;

struct Program {

    struct Instruction{
        /// the hex value of the instruction
        const uint8_t opcode;
        /// ...
        const string mnemonic;
        /// the number of elements popped from the stack
        const uint8_t delta;
        /// the number of elements pushed onto the stack
        const uint8_t alpha;

        /// if payload !emtpy then it is a push instruction
        vector<uint8_t> payload;

        Instruction(uint8_t val, string m, uint8_t d, uint8_t a):opcode(val),mnemonic(m),delta(d),alpha(a) { }

        uint8_t getValue() const {
            //TODO proper getValue not just from first byte
            return payload.at(0);
        }

        void print() const {
            cout << mnemonic << '\n';
        }
    };

    struct BasicBlock{
        const unsigned index;
        vector<Instruction> content;
        BasicBlock* nextJump;
        BasicBlock* nextFallthrough;

        explicit BasicBlock(unsigned i):index(i),nextJump(nullptr),nextFallthrough(nullptr) { }
    };

    struct Norm1{
        vector<uint8_t> creation;
        vector<uint8_t> run;
        string auxdata;
    };

    struct Norm2{
        vector<Instruction> instr;
        unordered_map<unsigned,unsigned> jumptable;
    };

    unordered_map<uint8_t,Instruction> instrs = {
            {0x00,Instruction(0x00,"STOP",0,0)},
            {0x01,Instruction(0x01,"ADD",2,1)},
            {0x02,Instruction(0x02,"MUL",2,1)},
            {0x03,Instruction(0x03,"SUB",2,1)},
            {0x04,Instruction(0x04,"DIV",2,1)},
            {0x05,Instruction(0x05,"SDIV",2,1)},
            {0x06,Instruction(0x06,"MOD",2,1)},
            {0x07,Instruction(0x07,"SMOD",2,1)},
            {0x08,Instruction(0x08,"ADDMOD",3,1)},
            {0x09,Instruction(0x09,"MULMOD",3,1)},
            {0x0a,Instruction(0x0a,"EXP",2,1)},
            {0x0b,Instruction(0x0b,"SIGNEXTEND",2,1)},
            {0x10,Instruction(0x10,"LT",2,1)},
            {0x52,Instruction(0x52,"MSTORE",2,0)},
            {0x56,Instruction(0x56,"JUMP",1,0)},
            {0x5b,Instruction(0x5b,"JUMPDEST",0,0)},
            {0x60,Instruction(0x60,"PUSH1",0,1)},
            {0xfd,Instruction(0xfd,"REVERT",2,0)},
            {0xfe,Instruction(0xfe,"INVALID",0,0)}
    };

    Program() = default;

    ///ignores the first three lines of given ifstream
    bool isCreation(ifstream &istrm) const {
        const long max = numeric_limits<streamsize>::max();
        istrm.ignore(max,'\n');
        istrm.ignore(max,'\n');
        string s;
        getline(istrm,s);
        //size of "Binary:" = 7
        return s.length() == 7;

    }

    ///read
    void extractSubProgram(ifstream& istrm, vector<uint8_t>& bytes){
        string s(2,'\0');
        while(istrm.read(&s[0], 2)) {
            const auto opc = (uint8_t) stoi(s, nullptr, 16);
            if(opc == 0xfe)
                return;
            bytes.push_back(opc);
        }
    }

    void extractAuxdata(ifstream& istrm, string& auxdata){
        const unsigned length = 43 << 1;
        string aux(length,'\0');
        istrm.read(&aux[0],length);
        if(aux.substr(0,2)!="a1")
            throw invalid_argument("no a1 auxdata found");
        auxdata = move(aux);
    }

    ///reads the given evm bytecode, and converts the hex chars to respective int value,
    Norm1 normalize1(const string& filename){
        if (ifstream istrm{filename, ios::binary}) {

            Norm1 n1;
            if(isCreation(istrm)){
                extractSubProgram(istrm,n1.creation);
            }
            extractSubProgram(istrm,n1.run);
            extractAuxdata(istrm,n1.auxdata);
            return n1;

        } throw invalid_argument("istrm failure");
    }

    ///remove push bytes and create jump table
    Norm2 normalize2(const vector<uint8_t>& bytes){
        Norm2 n2;
        //the number of bytes that are "removed" from the bytes vector, because they belong to a push instruction
        unsigned pc =0;

        for(unsigned idx=0;idx<bytes.size();idx++){
            const auto& opc = bytes.at(idx);
            Instruction instr = instrs.at(opc);
            if(0x60<=opc && opc<=0x7f){
                //PUSH1 = 0x60 .. PUSH32 = 0x7f
                const uint8_t num = opc-0x5f;
                for(unsigned j=1;j<=num;j++)
                    instr.payload.emplace_back(bytes.at(idx+j));
                idx+=num;
                pc+=num;
            } else if(opc==0x5b){
                //JUMPDEST = 0x5b
                n2.jumptable.emplace(idx,idx-pc);
            }
            n2.instr.push_back(instr);
        }
        return n2;
    }

    ///build BB by finding JUMP/JUMPI/REVERT/RETURN/STOP
    unique_ptr<BasicBlock> normalize3(const Norm2& n){
        vector<pair<BasicBlock*,unsigned>> jumpTo;
        unordered_map<unsigned,BasicBlock*> jumpDst;

        unsigned bbidx=0;
        unsigned instrIdx=0;

        BasicBlock head(bbidx++);

        BasicBlock* curr = &head;
        BasicBlock* succ;

        const set<uint8_t> bbendInstr = {0x00,0xf3,0xfd};

        for(const auto& i:n.instr){
            if(i.opcode == 0x56 || i.opcode == 0x57){
                //JUMP | JUMPI
                succ = new BasicBlock{bbidx++};

                curr->content.push_back(i);
                curr->nextFallthrough = succ;

                //previous instr contains jump target & map curr bb to this jumptarget
                const auto& jumptarget = n.instr.at(instrIdx-1).getValue();
                jumpTo.emplace_back(curr,n.jumptable.at(jumptarget));

                //successor bb is the jump destination for key
                jumpDst.emplace(instrIdx+1,succ);

                //the new bb is now current
                curr = succ;

            } else if(bbendInstr.find(i.opcode)!=bbendInstr.end()){
                //Stopping instruction found
                succ = new BasicBlock{bbidx++};

                curr->content.push_back(i);
                //successor bb is the jump destination for key
                jumpDst.emplace(instrIdx+1,succ);

                //the new bb is now current
                curr = succ;

            } else
                curr->content.push_back(i);
            instrIdx++;
        }

        for(auto& bbjump:jumpTo){
            auto search = jumpDst.find(bbjump.second);
            if(search!=jumpDst.end())
                bbjump.first->nextJump = search->second;
            else
                cerr<<"jumptarget not found"<<'\n';
        }
        return make_unique<BasicBlock>(head);
    }

    ///print the bytecode
    void printBytecodeCreation(vector<uint8_t>& bc){
        for(int i=0;i<bc.size();i++){
            cout << dec << i <<": ";
            cout << hex << (bc.at(i)&0xff) << '\n';
        }
    }
};

int main() {
    const string filename = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/input/test.bin";
    const string fout = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/graph.gv";

    Program p;
    Program::Norm1 n1 = p.normalize1(filename);
    if(!n1.creation.empty()){
        Program::Norm2 ncreate2 = p.normalize2(n1.creation);
        auto start = p.normalize3(ncreate2);
    }

    return 0;
}