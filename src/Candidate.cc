#include "Candidate.h"

//using namespace contract;


vector<unique_ptr<evmbca::Instruction>> Candidate::getCopyOfContent() const{
    std::vector<std::unique_ptr<evmbca::Instruction>> to;
    to.reserve(content.size());

    for (const auto& e : content)
        to.push_back(std::make_unique<evmbca::Instruction>(*e));

    return to;
}

bool Candidate::operator==(const stack<pair<unsigned,bitset<256>>>& other) const{

    if(stackIncoming.size()!=other.size()) return false;

    auto* end = &other.top() +1;
    auto* begin = end - other.size();
    vector<pair<unsigned,bitset<256>>> contentsOther(begin,end);

    end = &stackIncoming.top() +1;
    begin = end - stackIncoming.size();
    vector<pair<unsigned,bitset<256>>> contents(begin,end);

    for(unsigned i=0;i<other.size();i++){
        auto equal = contents.at(i).first==contentsOther.at(i).first;
        equal &= contents.at(i).second==contentsOther.at(i).second;
        if(!equal) return false;
    }

    return true;
}