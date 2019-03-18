//
// Created by alex on 16.03.19.
//

#ifndef EVAL_BASICBLOCK_H
#define EVAL_BASICBLOCK_H

#include <vector>
#include <bitset>
#include <stack>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>
#include <set>

#include "Instruction.h"

using namespace std;
using namespace instr;

namespace bb {

    class BasicBlock{
    public:

        explicit BasicBlock(unsigned i):index(i),nextJump(nullptr),nextFallthrough(nullptr) { }

        BasicBlock(const BasicBlock&) = delete;

        void setFallthrough(BasicBlock* bb);

        void setJump(BasicBlock* bb);

        ///only JUMPI has a fallthrough
        bool needsFallthrough() const;

        ///tests if BasicBlock needs a next Jump
        bool needsJump() const;

        bool hasFallthrough() const;

        bool hasJump() const;

        //TODO && vs no &!
        void addInstruction(unique_ptr<Instruction>&& instr);

        ///Process the stack for all instructions, but the last
        stack<bitset<256>> processStackExceptLast(stack<bitset<256>> stack) const;

        uint64_t getTopUll(stack<bitset<256>>& stack) const;

        void adjustJumpPtr(stack<bitset<256>> stack, const map<uint64_t, BasicBlock *> &jumpDst,
                           const map<uint64_t, uint64_t> &jumptable);

        unsigned printBB(ofstream& ostrm,const unsigned first,const unsigned prev,map<unsigned,pair<unsigned,unsigned>>& bbFirstNode) const;

        void printBBdot(const string& fout) const;

    private:
        const unsigned index;
        vector<unique_ptr<Instruction>> content;
        BasicBlock* nextJump;
        BasicBlock* nextFallthrough;

        static const set<Instruction::Opcode> noFallthroughInstrs;
    };
}

#endif //EVAL_BASICBLOCK_H
