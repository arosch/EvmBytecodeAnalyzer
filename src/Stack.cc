#include "Stack.h"

using namespace evmbca;

void Stack::pop(){
    stack.pop();
}

void Stack::push(Instruction* value){
    stack.push(value);
}

size_t Stack::size(){
    return stack.size();
}


bool Stack::operator==(const Stack other) const{
    //TODO
    (void)other;
    return false;
}

void Stack::merge(Stack other){
    if(stack.size()!=other.stack.size()) throw std::logic_error("Merging with different stack heights.");

    //comparing the elements of the stacks for equality
    auto* end = &other.stack.top() +1;
    auto* begin = end - other.stack.size();
    std::vector<Instruction*> ostack(begin,end);

    end = &stack.top() +1;
    begin = end - stack.size();
    std::vector<Instruction*> tstack(begin,end);

    for(auto i=0u;i<stack.size();i++){
        const auto tvar = tstack.at(i)->getVariable();
        auto oInstr = ostack.at(i);

        if(tvar!=oInstr->getVariable()){
            oInstr->setVariable(tvar);
        }
    }
}

std::vector<Instruction*> Stack::retrieveN(const unsigned n){
    if(stack.size()<n) throw logic_error("retrieving more elements than there are on the stack");

    std::vector<Instruction*> items;
    items.reserve(n);
    for(auto i=0u;i<n;i++){
        items.push_back(stack.top());
        stack.pop();
    }
    return items;
}

void Stack::pushN(std::vector<Instruction*> items){

    for(auto& i:items){
        stack.push(i);
    }
}

