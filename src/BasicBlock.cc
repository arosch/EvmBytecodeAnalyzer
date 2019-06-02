#include <stdexcept>
#include <algorithm>

#include "BasicBlock.h"
#include "Stack.h"

using namespace evmbca;

template<>
void BasicBlock<Operation>::setFallthrough(BasicBlock* bb) {
    if(nextFallthrough!=nullptr) throw logic_error("Falsely attempting to assign fallthrough bb");
    nextFallthrough=bb;
    nextFallthrough->addPredecessor(this);
}

template<>
void BasicBlock<Instruction>::setFallthrough(BasicBlock* bb) {
    //if(nextFallthrough!=nullptr) throw logic_error("Falsely attempting to assign fallthrough bb");
    nextFallthrough=bb;
    nextFallthrough->addPredecessor(this);
}

template<>
void BasicBlock<Operation>::setJump(BasicBlock* bb) {
    if(nextJump!=nullptr) throw logic_error("Falsely attempting to assign jump bb");
    nextJump=bb;
    nextJump->addPredecessor(this);
}

template<>
void BasicBlock<Instruction>::setJump(BasicBlock* bb) {
    //if(nextJump!=nullptr) throw logic_error("Falsely attempting to assign jump bb");
    nextJump=bb;
    nextJump->addPredecessor(this);
}

template<>
bool BasicBlock<Operation>::needsFallthrough() const{
    static const set<Operation::Opcode> noFallthroughInstrs = {Operation::Opcode::STOP,Operation::Opcode::JUMP,Operation::Opcode::RETURN,Operation::Opcode::REVERT,Operation::Opcode::INVALID};
    return noFallthroughInstrs.find(static_cast<Operation::Opcode>(content.back()->getOpcode()))==noFallthroughInstrs.end();
}

template<>
bool BasicBlock<Operation>::needsJump() const{
    const auto& opc = content.back()->getOpcode();
    return (opc == Operation::Opcode::JUMP || opc == Operation::Opcode::JUMPI) && nextJump == nullptr;
}

template<typename T>
bool BasicBlock<T>::hasFallthrough() const{
    return nextFallthrough!=nullptr;
}
template<>
bool BasicBlock<Instruction>::hasSuccessorEligibleFallthrough() const{
    return hasFallthrough() && !(nextFallthrough->isAJumpOnlyBb() || nextFallthrough->contentIsEmpty());
}

template<typename T>
bool BasicBlock<T>::hasJump() const{
    return nextJump!=nullptr;
}

template<>
bool BasicBlock<Instruction>::hasSuccessorEligibleJump() const{
    return hasJump() && !(nextJump->isAJumpOnlyBb() || nextJump->contentIsEmpty());
}

template<typename T>
bool BasicBlock<T>::isAJumpOnlyBb() const { return content.size()==1 && content.front()->isAJumpInstruction(); }

template <>
void BasicBlock<Instruction>::setSuccessorLikeOther(BasicBlock<Operation>* other, vector<unique_ptr<BasicBlock<Instruction>>>& successors) {
    if(other->hasJump())
        setJump(successors.at(other->getJumpIndex()).get());
    if(other->hasFallthrough())
        setFallthrough(successors.at(other->getFallthroughIndex()).get());
}

template<typename T>
void BasicBlock<T>::addPredecessor(BasicBlock<T>* p){
    predecessors.emplace_back(p);
}

template<typename T>
void BasicBlock<T>::addInstruction(unique_ptr<T>&& instr){
    content.push_back(move(instr));
}

template<>
stack<bitset<256>> BasicBlock<Operation>::processStack(stack<bitset<256>> stack) const{
    const auto num = content.size();
    for(unsigned i=0;i<num;i++){
        content[i]->processStack(stack);
    }
    return stack;
}

template<>
stack<bitset<256>> BasicBlock<Operation>::processStackExceptLast(stack<bitset<256>> stack) const{
    const auto num = content.size()-1;
    for(unsigned i=0;i<num;i++){
        content[i]->processStack(stack);
    }
    return stack;
}

template<>
uint64_t BasicBlock<Operation>::getTopUll(stack<bitset<256>>& stack) const {
    try {
        return stack.top().to_ullong();
    } catch (const std::overflow_error& e) {
        cerr<<"request for unsigned long long value from: "<<stack.top()<<"...\n";
        return 0;
    }
}

template<>
void BasicBlock<Operation>::adjustJumpPtr(stack<bitset<256>> stack, const map<uint64_t, BasicBlock *> &jumpDst,
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

template<>
bool BasicBlock<Operation>::requiresMerge() const{
    return content.back()->getOpcode()!=0xfd;
}

template<>
void BasicBlock<Operation>::abstract(
        vector<unique_ptr<BasicBlock<Instruction>>>& bbis,
        vector<bool>& abstracted,
        unsigned& kvar,
        vector<Stack>& endStacks){

    const auto idx = index;

    //maybe empty stack check?
    if(abstracted.at(idx)) return;

    //retrieve the intial state of the stack by merging the stacks of all predecessors
    Stack stack;

    //TODO proper implementation of cheaty workaround for reverts
    if(!predecessors.empty() && requiresMerge()){

        for(const auto& pred:predecessors){
            pred->abstract(bbis,abstracted,kvar,endStacks);
        }

        stack = endStacks.at(predecessors.at(0)->index);

        if(predecessors.size()>1){
            for(auto i=1u;i<predecessors.size();i++){
                stack.merge(endStacks.at(predecessors.at(i)->index));
            }
        }
    }

    for(const auto& op:content){
        //transform an operation into an instruction
        if(auto instr = op->toInstruction(stack,kvar)) {
            bbis.at(idx)->addInstruction(std::move(instr));
        }
    }

    endStacks.at(idx) = stack;
    abstracted.at(idx) = true;
}

/*template<>
void BasicBlock<Instruction>::assignSuccessorToEligiblePredecessor(unsigned successorIndex, BasicBlock<Instruction>* newSuccessor, vector<vector<unsigned>>& predecessors, vector<unique_ptr<BasicBlock<Instruction>>>& bbs){
    if(contentIsEmpty() || isAJumpOnlyBb()){
        for(auto& p:predecessors.at(index)) {
            bbs.at(p)->assignSuccessorToEligiblePredecessor(index, newSuccessor, predecessors, bbs);
        }
    }
    if(hasJump() && getJumpIndex() == successorIndex){
        setJump(newSuccessor);
    }
    if(hasFallthrough() && getFallthroughIndex() == successorIndex){
        setFallthrough(newSuccessor);
    }
}*/

template<typename T>
unsigned BasicBlock<T>::printBbDot(ofstream &ostrm, const unsigned firstNodeId) const {

    if(content.empty()){
        return firstNodeId;
    }

    ostrm <<"\tsubgraph cluster"<<index<<" {\n";
    ostrm <<"\t\tlabel=\"bb"<<index<<"\";\n";

    unsigned nodeId = firstNodeId;
    for(const auto& instr:content){
        ostrm <<"\t\t"<<nodeId++<<"[label=\""<<instr->toString()<<"\"];\n";
    }

    //dependency chain within the basic block
    ostrm <<"\t\t";
    for(unsigned i=firstNodeId;i<nodeId;i++){
        ostrm<<i;
        if(i!=nodeId-1)
            ostrm <<" -> ";
        else
            ostrm <<";";
    }
    ostrm <<'\n';
    ostrm <<"\t}\n\n";

    return nodeId;

}

template<typename T>
void BasicBlock<T>::printBbDotDependencies(ofstream &ostrm, const vector<unsigned>& firstNodes, const vector<unsigned>& lastNodes) const {
    if(contentIsEmpty()) return;

    if(hasJump()){
        ostrm<<'\t'<<lastNodes.at(index)<<" -> "<<firstNodes.at(getJumpIndex())<<";\n";
    }
    if(hasFallthrough()){
        ostrm<<'\t'<<lastNodes.at(index)<<" -> "<<firstNodes.at(getFallthroughIndex())<<";\n";
    }
}
template<typename T>
unsigned BasicBlock<T>::getStatistics() const{
    if(isAJumpOnlyBb()) return 0;
    return content.size();
}

template class evmbca::BasicBlock<Operation>;
template class evmbca::BasicBlock<Instruction>;