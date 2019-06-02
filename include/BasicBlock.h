#ifndef INCLUDE_BASICBLOCK_H
#define INCLUDE_BASICBLOCK_H

#include <vector>
#include <bitset>
#include <stack>
#include <utility>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>
#include <set>

#include "Operation.h"
#include "Instruction.h"

using namespace std;

namespace evmbca {

template <class T>
class BasicBlock{
public:

    explicit BasicBlock(unsigned i):index(i),nextJump(nullptr),nextFallthrough(nullptr) { }

    BasicBlock(const BasicBlock&) = delete;

    void setFallthrough(BasicBlock* bb);

    void setJump(BasicBlock* bb);

    bool needsFallthrough() const;

    bool needsJump() const;

    bool hasFallthrough() const;

    bool hasSuccessorEligibleFallthrough() const;

    bool hasJump() const;

    bool hasSuccessorEligibleJump() const;

    unsigned getIndex() const {
        return index;
    }

    unsigned getFallthroughIndex() const{
        return nextFallthrough->getIndex();
    }

    unsigned getJumpIndex() const{
        if(!hasJump()) return 0;
        return nextJump->getIndex();
    }

    BasicBlock* getFallthrough(){
        return nextFallthrough;
    }

    BasicBlock* getJump(){
        return nextJump;
    }

    bool contentIsEmpty() const{ return content.empty(); }

    bool isAJumpOnlyBb() const;

    void setSuccessorLikeOther(BasicBlock<Operation>* other, vector<unique_ptr<BasicBlock<Instruction>>>& successors);

    void addPredecessor(BasicBlock<T>* p);

    void addInstruction(unique_ptr<T>&& instr);

    stack<bitset<256>> processStack(stack<bitset<256>> stack) const;

    stack<bitset<256>> processStackExceptLast(stack<bitset<256>> stack) const;

    uint64_t getTopUll(stack<bitset<256>>& stack) const;

    void adjustJumpPtr(stack<bitset<256>> stack, const map<uint64_t, BasicBlock *> &jumpDst,
                       const map<uint64_t, uint64_t> &jumptable);

    bool requiresMerge() const;

    void abstract(vector<unique_ptr<BasicBlock<Instruction>>>& bbis,
                  vector<bool>& abstractedinstantiated,
                  unsigned& kvar,
                  vector<Stack>& endStacks);

    //void assignSuccessorToEligiblePredecessor(unsigned successorIndex,BasicBlock<Instruction>* newSuccessor, vector<vector<unsigned>>& predecessors, vector<unique_ptr<BasicBlock<Instruction>>>& bbs);

    unsigned printBbDot(ofstream& ostrm, const unsigned firstNodeId) const;
    void printBbDotDependencies(ofstream &ostrm, const vector<unsigned>& firstNodes, const vector<unsigned>& lastNodes) const;

    unsigned getStatistics() const;

private:
    const unsigned index;
    vector<unique_ptr<T>> content;
    BasicBlock* nextJump;
    BasicBlock* nextFallthrough;

    vector<BasicBlock*> predecessors;
};

} // namespace evmbca

#endif //INCLUDE_BASICBLOCK_H
