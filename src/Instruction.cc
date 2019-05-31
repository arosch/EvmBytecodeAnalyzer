#include <sstream>
#include "Instruction.h"
#include "Operation.h"

using namespace evmbca;

string Instruction::toDotLabel(const unsigned bbJumpIndex) const{
    stringstream ss;

    if(alpha==1){
        ss<<"v"<<variable<<" := ";
    }

    ss<<mnemonic<<"(";

    for(const auto& p:params){
        //0 indicates a push value
        if(p.first == 0){
            if(opcode == 0x56|| opcode == 0x57){
                //push value indicates a bb
                ss<<"bb"<<bbJumpIndex;
            } else {
                try{
                    auto v = p.second.to_ullong();
                    ss<<v;
                } catch(const overflow_error& e){
                    ss<<"WORD";

                    //ss<<p.second.to_string();
                }
            }
        } else {
            ss<<"v"<<p.first;
        }
        ss<<",";
    }

    auto s = ss.str();
    if(!params.empty()) s.pop_back();
    s.append(")");
    return s;
}

bool Instruction::operator==(const Instruction& other) const {
    auto equal = (opcode==other.opcode)
                 && (mnemonic==other.mnemonic)
                 && (delta==other.delta)
                 && (alpha==other.alpha)
                 && (variable==other.variable);
    unsigned index=0;
    for(const auto& p:params){
        equal &= p.first==other.params.at(index).first;
        equal &= p.second==other.params.at(index).second;
        index++;
    }
    return equal;
}