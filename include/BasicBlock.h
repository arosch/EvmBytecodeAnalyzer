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
#include "Candidate.h"

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

    void setContent(vector<unique_ptr<T>>&& c);

    bool contentIsEmpty() const{ return content.empty(); }

    bool isAJumpOnlyBb() const;

    void setSuccessorLikeOther(BasicBlock<Operation>* other, vector<unique_ptr<BasicBlock<Instruction>>>& successors);

    void addInstruction(unique_ptr<T>&& instr);

    stack<bitset<256>> processStack(stack<bitset<256>> stack) const;

    stack<bitset<256>> processStackExceptLast(stack<bitset<256>> stack) const;

    uint64_t getTopUll(stack<bitset<256>>& stack) const;

    void adjustJumpPtr(stack<bitset<256>> stack, const map<uint64_t, BasicBlock *> &jumpDst,
                       const map<uint64_t, uint64_t> &jumptable);

    unsigned instantiate(stack<pair<unsigned, bitset<256>>> stack,
                         const int predecessor,
                         const unsigned varIndex,
                         vector<unique_ptr<BasicBlock<Instruction>>> &bbs,
                         map<unsigned, Candidate> &candidates,
                         map<unsigned, BasicBlock<Operation> *> &operations,
                         map<unsigned, unsigned> &indexMatch);

    void assignSuccessorToEligiblePredecessor(unsigned successorIndex,BasicBlock<Instruction>* newSuccessor, vector<vector<unsigned>>& predecessors, vector<unique_ptr<BasicBlock<Instruction>>>& bbs);

    unsigned printBbDot(ofstream& ostrm, const unsigned firstNodeId) const;
    void printBbDotDependencies(ofstream &ostrm, const vector<unsigned>& firstNodes, const vector<unsigned>& lastNodes) const;

    unsigned getStatistics() const;

    vector<unique_ptr<T>> content;
private:
    const unsigned index;
    BasicBlock* nextJump;
    BasicBlock* nextFallthrough;
};

} // namespace evmbca

#endif //INCLUDE_BASICBLOCK_H
