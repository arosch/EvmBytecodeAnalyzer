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

        unsigned printBB(ofstream& ostrm,const unsigned first,const unsigned prev,map<unsigned,pair<unsigned,unsigned>>& bbFirstNode) const{
            unsigned next;
            unsigned last;

            auto search = bbFirstNode.find(index);
            if(search==bbFirstNode.end()){
                //bb is not yet written

                ostrm <<"\tsubgraph cluster"<<index<<" {\n";
                ostrm <<"\t\tlabel=\"bb"<<index<<"\";\n";

                unsigned i = first;
                for(const auto& instr:content){
                    ostrm <<"\t\t"<<i++<<"[label=\""<<instr.mnemonic<<"\"];\n";
                }
                ostrm <<"\t\t";
                if(prev!=0)
                    ostrm<<prev<<" -> ";
                for(unsigned j=first;j<i;j++){
                    ostrm<<j;
                    if(j!=i-1)
                        ostrm <<" -> ";
                    else
                        ostrm <<";";
                }
                ostrm <<'\n';
                ostrm <<"\t}\n\n";

                next=i;
                last=i-1;
                //if bb referenced again -> store first and last node value
                bbFirstNode.emplace(index,make_pair(first,i-1));
            } else {
                //bb was written previously
                next=search->second.first;
                last=search->second.second;

                ostrm <<'\t'<<prev<<" -> "<<next<<";\n";
            }

            if(nextFallthrough!=nullptr)
                next=nextFallthrough->printBB(ostrm,next,last,bbFirstNode);
            if(nextJump!=nullptr)
                next=nextJump->printBB(ostrm,next,last,bbFirstNode);
            return next;


        }

        void printBBdot(const string& fout) const{
            if (ofstream ostrm{fout, ios::binary}) {
                ostrm << "digraph G{\n";
                ostrm << "\tnode[shape=box];\n";

                if(content.empty()){
                    cerr<<"found empty bb!\n";
                    return;
                }

                map<unsigned,pair<unsigned,unsigned>> bbFirstNode;
                printBB(ostrm,0,0,bbFirstNode);
                ostrm << "}";
            }
        }
    };

    struct Norm1{
        vector<uint8_t> creation;
        vector<uint8_t> run;
        string auxdata;
    };

    struct Norm2{
        vector<Instruction> instr;
        unordered_map<unsigned,unsigned> jumptable;

        void print(){
            int j =0;
            for(const auto& i:instr){
                cout<<j++<<": "<<i.mnemonic<<'\n';
            }
        }
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
            {0x11,Instruction(0x11,"GT",2,1)},
            {0x12,Instruction(0x12,"SLT",2,1)},
            {0x13,Instruction(0x13,"SGT",2,1)},
            {0x14,Instruction(0x14,"EQ",2,1)},
            {0x15,Instruction(0x15,"ISZERO",1,1)},
            {0x16,Instruction(0x16,"AND",2,1)},
            {0x17,Instruction(0x17,"OR",2,1)},
            {0x18,Instruction(0x18,"XOR",2,1)},
            {0x19,Instruction(0x19,"NOT",1,1)},
            {0x1a,Instruction(0x1a,"BYTE",2,1)},
            {0x20,Instruction(0x20,"SHA3",2,1)},
            {0x30,Instruction(0x30,"ADDRESS",0,1)},
            {0x31,Instruction(0x31,"BALANCE",0,1)},
            {0x32,Instruction(0x32,"ORIGIN",0,1)},
            {0x33,Instruction(0x33,"CALLER",0,1)},
            {0x34,Instruction(0x34,"CALLVALUE",0,1)},
            {0x35,Instruction(0x35,"CALLDATALOAD",0,1)},
            {0x36,Instruction(0x36,"CALLDATASIZE",0,1)},
            {0x37,Instruction(0x37,"CALLDATACOPY",0,1)},
            {0x38,Instruction(0x38,"CODESIZE",0,1)},
            {0x39,Instruction(0x39,"CODECOPY",0,1)},
            {0x3a,Instruction(0x3a,"GASPRICE",0,1)},
            {0x3b,Instruction(0x3b,"EXTCODESIZE",0,1)},
            {0x3c,Instruction(0x3c,"EXTCODECOPY",0,1)},
            {0x3d,Instruction(0x3d,"RETURNDATASIZE",0,1)},
            {0x3e,Instruction(0x3e,"RETURNDATACOPY",0,1)},
            {0x40,Instruction(0x40,"BLOCKHASH",1,1)},
            {0x41,Instruction(0x41,"COINBASE",0,1)},
            {0x42,Instruction(0x42,"TIMESTAMP",0,1)},
            {0x43,Instruction(0x43,"NUMBER",0,1)},
            {0x44,Instruction(0x44,"DIFFICULTY",0,1)},
            {0x45,Instruction(0x45,"GASLIMIT",0,1)},
            {0x50,Instruction(0x50,"POP",1,0)},
            {0x51,Instruction(0x51,"MLOAD",1,1)},
            {0x52,Instruction(0x52,"MSTORE",2,0)},
            {0x53,Instruction(0x53,"MSTORE8",2,0)},
            {0x54,Instruction(0x54,"SLOAD",1,1)},
            {0x55,Instruction(0x55,"SSTORE",2,0)},
            {0x56,Instruction(0x56,"JUMP",1,0)},
            {0x57,Instruction(0x57,"JUMPI",2,0)},
            {0x58,Instruction(0x58,"PC",0,1)},
            {0x59,Instruction(0x59,"MSIZE",0,1)},
            {0x5a,Instruction(0x5a,"GAS",0,1)},
            {0x5b,Instruction(0x5b,"JUMPDEST",0,0)},
            {0x60,Instruction(0x60,"PUSH1",0,1)},
            {0x61,Instruction(0x61,"PUSH2",0,1)},
            {0x62,Instruction(0x62,"PUSH3",0,1)},
            {0x63,Instruction(0x63,"PUSH4",0,1)},
            {0x64,Instruction(0x64,"PUSH5",0,1)},
            {0x65,Instruction(0x65,"PUSH6",0,1)},
            {0x66,Instruction(0x66,"PUSH7",0,1)},
            {0x67,Instruction(0x67,"PUSH8",0,1)},
            {0x68,Instruction(0x68,"PUSH9",0,1)},
            {0x69,Instruction(0x69,"PUSH10",0,1)},
            {0x6a,Instruction(0x6a,"PUSH11",0,1)},
            {0x6b,Instruction(0x6b,"PUSH12",0,1)},
            {0x6c,Instruction(0x6c,"PUSH13",0,1)},
            {0x6d,Instruction(0x6d,"PUSH14",0,1)},
            {0x6e,Instruction(0x6e,"PUSH15",0,1)},
            {0x6f,Instruction(0x6f,"PUSH16",0,1)},
            {0x70,Instruction(0x70,"PUSH17",0,1)},
            {0x71,Instruction(0x71,"PUSH18",0,1)},
            {0x72,Instruction(0x72,"PUSH19",0,1)},
            {0x73,Instruction(0x73,"PUSH20",0,1)},
            {0x74,Instruction(0x74,"PUSH21",0,1)},
            {0x75,Instruction(0x75,"PUSH22",0,1)},
            {0x76,Instruction(0x76,"PUSH23",0,1)},
            {0x77,Instruction(0x77,"PUSH24",0,1)},
            {0x78,Instruction(0x78,"PUSH25",0,1)},
            {0x79,Instruction(0x79,"PUSH26",0,1)},
            {0x7a,Instruction(0x7a,"PUSH27",0,1)},
            {0x7b,Instruction(0x7b,"PUSH28",0,1)},
            {0x7c,Instruction(0x7c,"PUSH29",0,1)},
            {0x7d,Instruction(0x7d,"PUSH20",0,1)},
            {0x7e,Instruction(0x7e,"PUSH31",0,1)},
            {0x7f,Instruction(0x7f,"PUSH32",0,1)},
            {0x80,Instruction(0x80,"DUP1",1,2)},
            {0x81,Instruction(0x81,"DUP2",2,3)},
            {0x82,Instruction(0x82,"DUP3",3,4)},
            {0x83,Instruction(0x83,"DUP4",4,5)},
            {0x84,Instruction(0x84,"DUP5",5,6)},
            {0x85,Instruction(0x85,"DUP6",6,7)},
            {0x86,Instruction(0x86,"DUP7",7,8)},
            {0x87,Instruction(0x87,"DUP8",8,9)},
            {0x88,Instruction(0x88,"DUP9",9,10)},
            {0x89,Instruction(0x89,"DUP10",10,11)},
            {0x8a,Instruction(0x8a,"DUP11",11,12)},
            {0x8b,Instruction(0x8b,"DUP12",12,13)},
            {0x8c,Instruction(0x8c,"DUP13",13,14)},
            {0x8d,Instruction(0x8d,"DUP14",14,15)},
            {0x8e,Instruction(0x8e,"DUP15",15,16)},
            {0x8f,Instruction(0x8f,"DUP16",16,17)},
            {0x90,Instruction(0x90,"SWAP1",2,2)},
            {0x91,Instruction(0x91,"SWAP2",2,2)},
            {0x92,Instruction(0x92,"SWAP3",2,2)},
            {0x93,Instruction(0x93,"SWAP4",2,2)},
            {0x94,Instruction(0x94,"SWAP5",2,2)},
            {0x95,Instruction(0x95,"SWAP6",2,2)},
            {0x96,Instruction(0x96,"SWAP7",2,2)},
            {0x97,Instruction(0x97,"SWAP8",2,2)},
            {0x98,Instruction(0x98,"SWAP9",2,2)},
            {0x99,Instruction(0x99,"SWAP10",2,2)},
            {0x9a,Instruction(0x9a,"SWAP11",2,2)},
            {0x9b,Instruction(0x9b,"SWAP12",2,2)},
            {0x9c,Instruction(0x9c,"SWAP13",2,2)},
            {0x9d,Instruction(0x9d,"SWAP14",2,2)},
            {0x9e,Instruction(0x9e,"SWAP15",2,2)},
            {0x9f,Instruction(0x9f,"SWAP16",2,2)},
            {0xa0,Instruction(0xa0,"LOG0",2,0)},
            {0xa1,Instruction(0xa1,"LOG1",3,0)},
            {0xa2,Instruction(0xa2,"LOG2",4,0)},
            {0xa3,Instruction(0xa3,"LOG3",5,0)},
            {0xa4,Instruction(0xa4,"LOG4",6,2)},
            {0xf0,Instruction(0xf0,"CREATE",3,1)},
            {0xf1,Instruction(0xf1,"CALL",7,1)},
            {0xf2,Instruction(0xf2,"CALLCODE",7,1)},
            {0xf3,Instruction(0xf3,"RETURN",2,0)},
            {0xf4,Instruction(0xf4,"DELEGATECALL",6,1)},
            {0xfa,Instruction(0xfa,"STATICCALL",6,1)},
            {0xfd,Instruction(0xfd,"REVERT",2,0)},
            {0xfe,Instruction(0xfe,"INVALID",0,0)},
            {0xff,Instruction(0xff,"SELFDESTRUCT",1,0)}
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
            if(i.opcode == 0x56){
                //JUMP
                succ = new BasicBlock{bbidx++};

                curr->content.push_back(i);
                //curr->nextFallthrough = succ;

                //previous instr contains jump target & map curr bb to this jumptarget
                const auto& jumptarget = n.instr.at(instrIdx-1).getValue();
                jumpTo.emplace_back(curr,n.jumptable.at(jumptarget));

                //successor bb is the jump destination for key
                jumpDst.emplace(instrIdx+1,succ);

                //the new bb is now current
                curr = succ;

            } else if(i.opcode == 0x57) {
                //JUMPI
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
    const string fout = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/output/graph.gv";
    const string foutr = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/output/graph2.gv";

    Program p;
    Program::Norm1 n1 = p.normalize1(filename);
    if(!n1.creation.empty()){
        Program::Norm2 ncreate2 = p.normalize2(n1.creation);
        //ncreate2.print();
        auto start = p.normalize3(ncreate2);
        start->printBBdot(fout);
    }
    Program::Norm2 nrun2 = p.normalize2(n1.run);
    nrun2.print();
    auto startr = p.normalize3(nrun2);
    startr->printBBdot(foutr);


    return 0;
}