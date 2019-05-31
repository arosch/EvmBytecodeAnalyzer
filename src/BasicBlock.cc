#include <stdexcept>
#include "BasicBlock.h"

using namespace evmbca;

template<typename T>
void BasicBlock<T>::setFallthrough(BasicBlock* bb) {
    //if(nextFallthrough!=nullptr) throw logic_error("Falsely attempting to assign fallthrough bb");
    nextFallthrough=bb;
}

template<typename T>
void BasicBlock<T>::setJump(BasicBlock* bb) {
    //if(nextJump!=nullptr) throw logic_error("Falsely attempting to assign jump bb");
    nextJump=bb;
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
void BasicBlock<T>::setContent(vector<unique_ptr<T>>&& c){
    content = {make_move_iterator(c.begin()),make_move_iterator(c.end())};
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
unsigned BasicBlock<Instruction>::instantiate(stack<pair<unsigned,bitset<256>>> stack,
                const int predecessor,
                const unsigned varIndex,
                vector<unique_ptr<BasicBlock<Instruction>>>& bbs,
               map<unsigned,Candidate>& candidates,
                map<unsigned,BasicBlock<Operation>*>& operations,
                map<unsigned,unsigned>& indexMatch
                ){

    //the BB we process
    BasicBlock* bb;

    //if incoming stack differs to existing candidates stacks
    auto search = candidates.find(index);
    if(search!=candidates.end()){
        if(search->second==stack){
            return /*?*/ varIndex;
        }
        //bb already instantiated -> new bb
        bbs.emplace_back(make_unique<BasicBlock<Instruction>>(bbs.size()));
        bb = bbs.back().get();

        indexMatch.emplace(bb->getIndex(),indexMatch.at(index));

        //set the successor of the predecessor to the newly created bb
        auto preBb = bbs.at(predecessor).get();
        if(preBb->hasJump() && preBb->getJumpIndex() == index){
            preBb->setJump(bb);
        } else if(preBb->hasFallthrough() && preBb->getFallthroughIndex() == index){
            preBb->setFallthrough(bb);
        } else {
            throw logic_error("predecessor of newly created bb is not actually a predecessor");
        }

        //set the successors of the new basic block
        if(hasJump()) bb->setJump(nextJump);
        if(hasFallthrough()) bb->setFallthrough(nextFallthrough);

    } else {
        //bb is not instantiated -> use this bb
        bb = this;
    }

    //the index of the BB we process
    auto bbIndex = bb->getIndex();

    //new candidate
    candidates.emplace(bbIndex,Candidate(predecessor,stack));

    //lookup the associated old index for this index
    unsigned oldIndex;
    try {
        oldIndex = indexMatch.at(bbIndex);
    } catch(const out_of_range& e){
        throw logic_error("Could not find an oldIndex for index: "+to_string(bbIndex));
    }

    auto varIdx = varIndex;
    BasicBlock<Operation>* old;
    try {
        old = operations.at(oldIndex);
    } catch(const out_of_range& e){
        throw logic_error("Could not find an BasicBlock<Operation> for index: "+to_string(oldIndex));
    }
    for(const auto& op:old->content){

        //transform operation to instruction, if not an instruction it adjusts the stack only
        if(auto h = op->toInstruction(stack,varIdx)){
            //in case a instruction that returns a variable
            if(h.value().returnsVar()) varIdx++;
            bb->addInstruction(move(make_unique<Instruction>(move(h.value()))));
        }
    }

    if(hasJump()){ varIdx = nextJump->instantiate(stack, bb->getIndex(), varIdx, bbs, candidates, operations,
                                                  indexMatch); }

    if(hasFallthrough()){ varIdx = nextFallthrough->instantiate(stack, bb->getIndex(), varIdx, bbs, candidates,
                                                                operations, indexMatch); }

    return varIdx;
}

template<>
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
}

template<typename T>
unsigned BasicBlock<T>::printBbDot(ofstream &ostrm, const unsigned firstNodeId) const {

    if(content.empty() || isAJumpOnlyBb()){
        return firstNodeId;
    }

    ostrm <<"\tsubgraph cluster"<<index<<" {\n";
    ostrm <<"\t\tlabel=\"bb"<<index<<"\";\n";

    unsigned nodeId = firstNodeId;
    for(const auto& instr:content){
        ostrm <<"\t\t"<<nodeId++<<"[label=\""<<instr->toDotLabel(getJumpIndex())<<"\"];\n";
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
    if(isAJumpOnlyBb() || contentIsEmpty()) return;

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