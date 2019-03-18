//
// Created by alex on 16.03.19.
//

#include <stdexcept>
#include "BasicBlock.h"

using namespace std;
using namespace bb;

void BasicBlock::setFallthrough(BasicBlock* bb) {
    if(nextFallthrough!=nullptr) throw logic_error("Falsely attempting to assign fallthrough bb");
    nextFallthrough=bb;
}

void BasicBlock::setJump(BasicBlock* bb) {
    if(nextJump!=nullptr) throw logic_error("Falsely attempting to assign jump bb");
    nextJump=bb;
}

bool BasicBlock::needsFallthrough() const{
    return noFallthroughInstrs.find(static_cast<Instruction::Opcode>(content.back()->getOpcode()))==noFallthroughInstrs.end();
}

bool BasicBlock::needsJump() const{
    const auto& opc = content.back()->getOpcode();
    return opc == Instruction::Opcode::JUMP || opc == Instruction::Opcode::JUMPI;
}

bool BasicBlock::hasFallthrough() const{
    return nextFallthrough!=nullptr;
}

bool BasicBlock::hasJump() const{
    return nextJump!=nullptr;
}

void BasicBlock::addInstruction(unique_ptr<Instruction>&& instr){
    content.push_back(move(instr));
}

stack<bitset<256>> BasicBlock::processStackExceptLast(stack<bitset<256>> stack) const{
    const auto num = content.size()-1;
    for(unsigned i=0;i<num;i++){
        content[i]->processStack(stack);
    }
    return stack;
}

uint64_t BasicBlock::getTopUll(stack<bitset<256>>& stack) const {
    try {
        return stack.top().to_ullong();
    } catch (const std::overflow_error& e) {
        cerr<<"request for unsigned long long value from: "<<stack.top()<<"...\n";
        return 0;
    }
}

void BasicBlock::adjustJumpPtr(stack<bitset<256>> stack, const map<uint64_t, BasicBlock *> &jumpDst,
                   const map<uint64_t, uint64_t> &jumptable){
    //fallthrough also processes stack, because of jumpi
    if(!needsJump()) return;

    stack = processStackExceptLast(stack);

    //jumptarget is the topmost element of the stack
    const uint64_t jumptarget = [&]{
        uint64_t oldTarget=0;
        try{
            oldTarget = getTopUll(stack);
            return jumptable.at(oldTarget);
        } catch(const out_of_range& e) {
            cerr << "Could not find Jumptable value for stack value "<<oldTarget <<" in BB"<<index<<'\n';
            throw e;
        }
    }();

    setJump([&]{
        try{
            return jumpDst.at(jumptarget);
        } catch(const out_of_range& e) {
            cerr << "Could not find a JUMPDEST at: "<<jumptarget<<" for BB"<<index<<'\n';
            throw e;
        }
    }());

    content.back()->processStack(stack);

    nextJump->adjustJumpPtr(stack, jumpDst, jumptable);

    if(hasFallthrough()){
        nextFallthrough->adjustJumpPtr(stack, jumpDst, jumptable);
    }
}

unsigned BasicBlock::printBB(ofstream& ostrm,const unsigned first,const unsigned prev,map<unsigned,pair<unsigned,unsigned>>& bbFirstNode) const{
    unsigned next;
    unsigned last;

    auto search = bbFirstNode.find(index);
    if(search==bbFirstNode.end()){
        //bb is not yet written

        ostrm <<"\tsubgraph cluster"<<index<<" {\n";
        ostrm <<"\t\tlabel=\"bb"<<index<<"\";\n";

        unsigned i = first;
        for(const auto& instr:content){
            ostrm <<"\t\t"<<i++<<"[label=\""<<instr->getMnemonic()<<"\"];\n";
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

    if(hasFallthrough())
        next=nextFallthrough->printBB(ostrm,next,last,bbFirstNode);
    if(hasJump())
        next=nextJump->printBB(ostrm,next,last,bbFirstNode);
    return next;
}

void BasicBlock::printBBdot(const string& fout) const{
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

const set<Instruction::Opcode> BasicBlock::noFallthroughInstrs = {Instruction::Opcode::STOP,Instruction::Opcode::JUMP,Instruction::Opcode::RETURN,Instruction::Opcode::REVERT};