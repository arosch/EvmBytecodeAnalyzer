#ifndef EVMBCA_STACK_H
#define EVMBCA_STACK_H

#include <stack>
#include <stdexcept>
#include <vector>
#include <bitset>
#include "Operation.h"
#include "Instruction.h"

namespace evmbca {

//template <class T,class E>
class Stack {

public:
    Stack()=default;

    void pop();
    void push(Instruction* value);
    size_t size();

    template <class... Args>
    decltype(auto) emplace(Args&&... args){
        return stack.emplace(std::forward<Args>(args)...);
    }

    bool operator==(const Stack other) const;

    void merge(Stack other);

    std::vector<Instruction*> retrieveN(const unsigned n);
    void pushN(std::vector<Instruction*> items);

public:
    std::stack<Instruction*> stack;
};

} // namespace evmbca

#endif //EVMBCA_STACK_H
