#ifndef INCLUDE_INSTRUCTION_H
#define INCLUDE_INSTRUCTION_H

#include <vector>
#include <bitset>
#include <stack>
#include <map>

namespace evmbca {

class Instruction{

public:
    Instruction(const uint8_t o, const std::string m, const uint8_t d, const uint8_t a, const std::vector<Instruction*> p, const unsigned var)
        :opcode(o),mnemonic(m),delta(d),alpha(a),params(p),variable(var) { }

    /// specific constructor for Push
    Instruction(const uint8_t o, const std::string m, const uint8_t d, const uint8_t a, const unsigned var, std::bitset<256> val)
            :opcode(o),mnemonic(m),delta(d),alpha(a),variable(var),value(val) { }

    Instruction(const Instruction& other)=delete;

    bool isAJumpInstruction() { return opcode==0x56 || opcode==0x57; }

    void setVariable(const unsigned var);
    unsigned getVariable() const;

    std::string toString() const;

private:
    /// the hex value of the instruction
    const uint8_t opcode;
    /// ...
    const std::string mnemonic;
    /// the number of elements popped from the stack
    const uint8_t delta;
    /// the number of elements pushed onto the stack
    const uint8_t alpha;

    std::vector<Instruction*> params;

    /// can be changed for merging path problem
    unsigned variable;
    std::bitset<256> value;
};

} // namespace evmbca

#endif //INCLUDE_INSTRUCTION_H
