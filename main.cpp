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
        vector<Instruction> instrs;
        map<uint64_t,uint64_t> jumptable;
    };

    unordered_map<uint8_t,Instruction> instrMap = {
            {0x00,{0x00,"STOP",0,0}},
            {0x01,{0x01,"ADD",2,1}},
            {0x02,{0x02,"MUL",2,1}},
            {0x03,{0x03,"SUB",2,1}},
            {0x04,{0x04,"DIV",2,1}},
            {0x05,{0x05,"SDIV",2,1}},
            {0x06,{0x06,"MOD",2,1}},
            {0x07,{0x07,"SMOD",2,1}},
            {0x08,{0x08,"ADDMOD",3,1}},
            {0x09,{0x09,"MULMOD",3,1}},
            {0x0a,{0x0a,"EXP",2,1}},
            {0x0b,{0x0b,"SIGNEXTEND",2,1}},
            {0x10,{0x10,"LT",2,1}},
            {0x11,{0x11,"GT",2,1}},
            {0x12,{0x12,"SLT",2,1}},
            {0x13,{0x13,"SGT",2,1}},
            {0x14,{0x14,"EQ",2,1}},
            {0x15,{0x15,"ISZERO",1,1}},
            {0x16,{0x16,"AND",2,1}},
            {0x17,{0x17,"OR",2,1}},
            {0x18,{0x18,"XOR",2,1}},
            {0x19,{0x19,"NOT",1,1}},
            {0x1a,{0x1a,"BYTE",2,1}},
            {0x20,{0x20,"SHA3",2,1}},
            {0x30,{0x30,"ADDRESS",0,1}},
            {0x31,{0x31,"BALANCE",0,1}},
            {0x32,{0x32,"ORIGIN",0,1}},
            {0x33,{0x33,"CALLER",0,1}},
            {0x34,{0x34,"CALLVALUE",0,1}},
            {0x35,{0x35,"CALLDATALOAD",0,1}},
            {0x36,{0x36,"CALLDATASIZE",0,1}},
            {0x37,{0x37,"CALLDATACOPY",0,1}},
            {0x38,{0x38,"CODESIZE",0,1}},
            {0x39,{0x39,"CODECOPY",0,1}},
            {0x3a,{0x3a,"GASPRICE",0,1}},
            {0x3b,{0x3b,"EXTCODESIZE",0,1}},
            {0x3c,{0x3c,"EXTCODECOPY",0,1}},
            {0x3d,{0x3d,"RETURNDATASIZE",0,1}},
            {0x3e,{0x3e,"RETURNDATACOPY",0,1}},
            {0x40,{0x40,"BLOCKHASH",1,1}},
            {0x41,{0x41,"COINBASE",0,1}},
            {0x42,{0x42,"TIMESTAMP",0,1}},
            {0x43,{0x43,"NUMBER",0,1}},
            {0x44,{0x44,"DIFFICULTY",0,1}},
            {0x45,{0x45,"GASLIMIT",0,1}},
            {0x50,{0x50,"POP",1,0}},
            {0x51,{0x51,"MLOAD",1,1}},
            {0x52,{0x52,"MSTORE",2,0}},
            {0x53,{0x53,"MSTORE8",2,0}},
            {0x54,{0x54,"SLOAD",1,1}},
            {0x55,{0x55,"SSTORE",2,0}},
            {0x56,{0x56,"JUMP",1,0}},
            {0x57,{0x57,"JUMPI",2,0}},
            {0x58,{0x58,"PC",0,1}},
            {0x59,{0x59,"MSIZE",0,1}},
            {0x5a,{0x5a,"GAS",0,1}},
            {0x5b,{0x5b,"JUMPDEST",0,0}},
            {0x60,{0x60,"PUSH1",0,1}},
            {0x61,{0x61,"PUSH2",0,1}},
            {0x62,{0x62,"PUSH3",0,1}},
            {0x63,{0x63,"PUSH4",0,1}},
            {0x64,{0x64,"PUSH5",0,1}},
            {0x65,{0x65,"PUSH6",0,1}},
            {0x66,{0x66,"PUSH7",0,1}},
            {0x67,{0x67,"PUSH8",0,1}},
            {0x68,{0x68,"PUSH9",0,1}},
            {0x69,{0x69,"PUSH10",0,1}},
            {0x6a,{0x6a,"PUSH11",0,1}},
            {0x6b,{0x6b,"PUSH12",0,1}},
            {0x6c,{0x6c,"PUSH13",0,1}},
            {0x6d,{0x6d,"PUSH14",0,1}},
            {0x6e,{0x6e,"PUSH15",0,1}},
            {0x6f,{0x6f,"PUSH16",0,1}},
            {0x70,{0x70,"PUSH17",0,1}},
            {0x71,{0x71,"PUSH18",0,1}},
            {0x72,{0x72,"PUSH19",0,1}},
            {0x73,{0x73,"PUSH20",0,1}},
            {0x74,{0x74,"PUSH21",0,1}},
            {0x75,{0x75,"PUSH22",0,1}},
            {0x76,{0x76,"PUSH23",0,1}},
            {0x77,{0x77,"PUSH24",0,1}},
            {0x78,{0x78,"PUSH25",0,1}},
            {0x79,{0x79,"PUSH26",0,1}},
            {0x7a,{0x7a,"PUSH27",0,1}},
            {0x7b,{0x7b,"PUSH28",0,1}},
            {0x7c,{0x7c,"PUSH29",0,1}},
            {0x7d,{0x7d,"PUSH20",0,1}},
            {0x7e,{0x7e,"PUSH31",0,1}},
            {0x7f,{0x7f,"PUSH32",0,1}},
            {0x80,{0x80,"DUP1",1,2}},
            {0x81,{0x81,"DUP2",2,3}},
            {0x82,{0x82,"DUP3",3,4}},
            {0x83,{0x83,"DUP4",4,5}},
            {0x84,{0x84,"DUP5",5,6}},
            {0x85,{0x85,"DUP6",6,7}},
            {0x86,{0x86,"DUP7",7,8}},
            {0x87,{0x87,"DUP8",8,9}},
            {0x88,{0x88,"DUP9",9,10}},
            {0x89,{0x89,"DUP10",10,11}},
            {0x8a,{0x8a,"DUP11",11,12}},
            {0x8b,{0x8b,"DUP12",12,13}},
            {0x8c,{0x8c,"DUP13",13,14}},
            {0x8d,{0x8d,"DUP14",14,15}},
            {0x8e,{0x8e,"DUP15",15,16}},
            {0x8f,{0x8f,"DUP16",16,17}},
            {0x90,{0x90,"SWAP1",2,2}},
            {0x91,{0x91,"SWAP2",2,2}},
            {0x92,{0x92,"SWAP3",2,2}},
            {0x93,{0x93,"SWAP4",2,2}},
            {0x94,{0x94,"SWAP5",2,2}},
            {0x95,{0x95,"SWAP6",2,2}},
            {0x96,{0x96,"SWAP7",2,2}},
            {0x97,{0x97,"SWAP8",2,2}},
            {0x98,{0x98,"SWAP9",2,2}},
            {0x99,{0x99,"SWAP10",2,2}},
            {0x9a,{0x9a,"SWAP11",2,2}},
            {0x9b,{0x9b,"SWAP12",2,2}},
            {0x9c,{0x9c,"SWAP13",2,2}},
            {0x9d,{0x9d,"SWAP14",2,2}},
            {0x9e,{0x9e,"SWAP15",2,2}},
            {0x9f,{0x9f,"SWAP16",2,2}},
            {0xa0,{0xa0,"LOG0",2,0}},
            {0xa1,{0xa1,"LOG1",3,0}},
            {0xa2,{0xa2,"LOG2",4,0}},
            {0xa3,{0xa3,"LOG3",5,0}},
            {0xa4,{0xa4,"LOG4",6,2}},
            {0xf0,{0xf0,"CREATE",3,1}},
            {0xf1,{0xf1,"CALL",7,1}},
            {0xf2,{0xf2,"CALLCODE",7,1}},
            {0xf3,{0xf3,"RETURN",2,0}},
            {0xf4,{0xf4,"DELEGATECALL",6,1}},
            {0xfa,{0xfa,"STATICCALL",6,1}},
            {0xfd,{0xfd,"REVERT",2,0}},
            {0xfe,{0xfe,"INVALID",0,0}},
            {0xff,{0xff,"SELFDESTRUCT",1,0}}
    };

    Program() = default;

    ///ignores the first three lines of given ifstream
    bool isCreationAndPrep(ifstream &istrm) const {
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
            if(isCreationAndPrep(istrm)){
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
        unsigned pushCount =0;

        for(unsigned idx=0;idx<bytes.size();idx++){
            const auto& opc = bytes.at(idx);
            Instruction instr = [&]{
                try{
                    return instrMap.at(opc);
                } catch(const out_of_range& e) {
                    cerr << e.what() <<'\n';
                    throw e;
                }
            }();
            if(0x60<=opc && opc<=0x7f){
                //PUSH1 = 0x60 .. PUSH32 = 0x7f
                const uint8_t num = opc-0x5f;
                for(unsigned j=1;j<=num;j++){
                    instr.pushVal = (instr.pushVal<<8) | bitset<256>(bytes.at(idx+j));
                }
                idx+=num;
                pushCount+=num;
            } else if(opc==0x5b){
                //JUMPDEST = 0x5b
                n2.jumptable.emplace(idx,idx-pushCount);
            }
            n2.instrs.push_back(instr);
        }
        return n2;
    }



    ///end of bb without fallthrough:   revert/stop/return/jump
    ///end of bb with fallthrough:      jumpi
    unique_ptr<BasicBlock> normalize3(const Norm2 &n){

        //the bb to jump for specified key(=original byte position)
        map<uint64_t,BasicBlock*> jumpDst;
        //index counter for the basic blocks; inc according to occurrence in bytecode
        unsigned bbIdx=0;
        //index counter for the instructions; inc according to occurrence in bytecode
        uint64_t instrIdx=0;

        auto head = new BasicBlock(bbIdx++);
        auto curr = head;
        jumpDst.emplace(instrIdx,curr);

        const set<uint8_t> bbendInstr = {0x00,0x56,0x57,0xf3,0xfd};

        //splits into basic blocks
        for(const auto& instr:n.instrs){
            curr->addInstruction(instr);
            instrIdx++;

            if(bbendInstr.find(instr.opcode)!=bbendInstr.end() && instrIdx<n.instrs.size()){
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

        return make_unique<BasicBlock>(*head);
    }
};

int main() {
    const string filename = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/input/test.bin";
    const string fout = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/output/graph.gv";
    const string foutr = "/home/alex/CLionProjects/EvmBytecodeAnalyzer/output/graph2.gv";
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

        auto ele = p.normalize3(nrun2);
        ele->printBBdot(foutr);

        cout<<"hi!!!!"<<endl;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }

    return 0;
}