#ifndef EVAL_CANDIDATE_H
#define EVAL_CANDIDATE_H

#include <vector>
#include <utility>
#include <memory>
#include <cmath>

#include "Instruction.h"

using namespace std;

class Candidate{
public:

    Candidate(const unsigned pred, stack<pair<unsigned,bitset<256>>> s):predecessor(pred),stackIncoming(s) {}

    Candidate(const Candidate& other)=delete;

    Candidate(Candidate&& other):predecessor(other.predecessor),stackIncoming(move(other.stackIncoming)){
        for(auto& i:other.content)
            content.emplace_back(move(i));
    }

    int getPredecessor() const{ return abs(predecessor); }

    bool predecessorIsJump() { return predecessor<0; }
    bool predecessorIsFallthrough() { return predecessor>0; }

    vector<unique_ptr<instr::Instruction>> getCopyOfContent() const;

    void addToContent(unique_ptr<instr::Instruction>&& c){
        content.push_back(move(c));
    }

    bool operator==(const stack<pair<unsigned,bitset<256>>>& other) const;

private:
    const int predecessor;
    vector<unique_ptr<instr::Instruction>> content;
    const stack<pair<unsigned,bitset<256>>> stackIncoming;

};

#endif //EVAL_CANDIDATE_H