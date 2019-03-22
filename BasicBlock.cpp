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
    static const set<Instruction::Opcode> noFallthroughInstrs = {Instruction::Opcode::STOP,Instruction::Opcode::JUMP,Instruction::Opcode::RETURN,Instruction::Opcode::REVERT,Instruction::Opcode::INVALID};
    return noFallthroughInstrs.find(static_cast<Instruction::Opcode>(content.back()->getOpcode()))==noFallthroughInstrs.end();
}

bool BasicBlock::needsJump() const{
    const auto& opc = content.back()->getOpcode();
    return (opc == Instruction::Opcode::JUMP || opc == Instruction::Opcode::JUMPI) && nextJump == nullptr;
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

stack<bitset<256>> BasicBlock::processStack(stack<bitset<256>> stack) const{
    const auto num = content.size();
    for(unsigned i=0;i<num;i++){
        content[i]->processStack(stack);
    }
    return stack;
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

    if(needsJump()){
        stack = processStackExceptLast(stack);
        //jumptarget is the topmost element of the stack
        const uint64_t jumptarget = [&]{
            uint64_t oldTarget=0;
            try{
                oldTarget = getTopUll(stack);
                return jumptable.at(oldTarget);
            } catch(const out_of_range& e) {
                throw out_of_range("Could not find Jumptable value for stack value: "+to_string(oldTarget)+ " for BB"+to_string(index));
            }
        }();

        setJump([&]{
            try{
                return jumpDst.at(jumptarget);
            } catch(const out_of_range& e) {
                throw out_of_range("Could not find a JUMPDEST at: "+to_string(jumptarget)+ " for BB"+to_string(index));
            }
        }());

        content.back()->processStack(stack);

        nextJump->adjustJumpPtr(stack, jumpDst, jumptable);

        if(hasFallthrough()){
            nextFallthrough->adjustJumpPtr(stack, jumpDst, jumptable);
        }
    } else {
        if(hasFallthrough()){
            nextFallthrough->adjustJumpPtr(processStack(stack), jumpDst, jumptable);
        }
    }


}

unsigned BasicBlock::printBB(ofstream& ostrm, const unsigned first, map<unsigned,unsigned>& bbFirstNode, vector<pair<unsigned,unsigned>>& dependencies) const{

    unsigned next;

    auto search = bbFirstNode.find(index);
    if(search!=bbFirstNode.end()) return first;

    //bb is not yet written...

    ostrm <<"\tsubgraph cluster"<<index<<" {\n";
    ostrm <<"\t\tlabel=\"bb"<<index<<"\";\n";

    unsigned i = first;
    for(const auto& instr:content){
        ostrm <<"\t\t"<<i++<<"[label=\""<<instr->getMnemonic()<<"\"];\n";
    }
    ostrm <<"\t\t";

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
    //if bb referenced again -> store first and last node value
    bbFirstNode.emplace(index,first);

    const unsigned last=i-1;
    if(hasFallthrough()){
        dependencies.emplace_back(last,nextFallthrough->index);
        next=nextFallthrough->printBB(ostrm,next,bbFirstNode,dependencies);
    }
    if(hasJump()){
        dependencies.emplace_back(last,nextJump->index);
        next=nextJump->printBB(ostrm,next,bbFirstNode,dependencies);
    }

    return next;
}

unsigned BasicBlock::printBBDependencies(ofstream& ostrm, map<unsigned,unsigned>& bbFirstNode, vector<pair<unsigned,unsigned>>& dependencies) const{
    for(const auto& d:dependencies){
        ostrm<<'\t'<<d.first<<" -> "<<bbFirstNode.at(d.second)<<";\n";
    }
}

void BasicBlock::printBBdot(const string& fout) const{
    cout<<"Printing cfg to: "<<fout<<"...\n";
    if (ofstream ostrm{fout, ios::binary}) {
        ostrm << "digraph G{\n";
        ostrm << "\tnode[shape=box];\n";

        if(content.empty()){
            cerr<<"found empty bb!\n";
            return;
        }

        map<unsigned,unsigned> bbFirstNode;
        vector<pair<unsigned,unsigned>> dependencies;
        printBB(ostrm,0,bbFirstNode,dependencies);
        printBBDependencies(ostrm,bbFirstNode,dependencies);
        ostrm << "}";
    } else
        throw invalid_argument("Couldn't write to "+fout);
}