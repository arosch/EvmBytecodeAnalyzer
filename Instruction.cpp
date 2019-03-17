//
// Created by alex on 16.03.19.
//

#include "Instruction.h"

using namespace std;
using namespace instr;

void Instruction::processStack(stack<bitset<256>>& stack) const{

    if(0x60<=opcode && opcode<=0x7f){
        stack.push(pushVal);
        return;
    }

    if(0x90<=opcode && opcode<=0x9f){
        vector<bitset<256>> swapItem;
        for(unsigned i=0;i<2;i++){
            swapItem.push_back(stack.top());
            stack.pop();
        }
        //swap first with last element
        iter_swap(swapItem.begin(),swapItem.rbegin());
        //push back onto the stack
        for(auto it=swapItem.rbegin();it!=swapItem.rend();it++){
            stack.push(*it);
        }
        return;
    }

    const unsigned pop = delta;
    const unsigned push = alpha;

    vector<bitset<256>> stackItem;

    //pop
    for(unsigned i=0;i<pop;i++){
        stackItem.push_back(stack.top());
        stack.pop();
    }

    //push
    for(unsigned i=0;i<push;i++){
        stack.emplace(0);
    }

}