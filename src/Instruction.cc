#include <sstream>
#include "Instruction.h"

using namespace evmbca;

void Instruction::setVariable(const unsigned var){
    variable = var;
}

unsigned Instruction::getVariable() const{
    return variable;
}

std::string Instruction::toString() const {
    std::stringstream ss;

    if(alpha==1){
        ss<<"v"<<variable<<" := ";
    }

    ss<<mnemonic<<"(";

    if(opcode>=0x60 && opcode<=0x7f){
        try{
            ss<<value.to_ullong();
        } catch(const std::overflow_error& e){
            ss<<"BIGWORD";
        }
    } else {
        for(const auto& p:params){
            ss<<"v"<<p->getVariable()<<",";
        }
    }

    auto s = ss.str();
    if(!params.empty()) s.pop_back();
    s.append(")");
    return s;
}