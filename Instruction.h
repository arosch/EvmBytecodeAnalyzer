//
// Created by alex on 16.03.19.
//

#ifndef EVAL_INSTRUCTION_H
#define EVAL_INSTRUCTION_H

#include <vector>
#include <bitset>
#include <stack>

using namespace std;

namespace instr {
    class Instruction{
    public:
        /// the hex value of the instruction
        const uint8_t opcode;
        /// ...
        const string mnemonic;
        /// the number of elements popped from the stack
        const uint8_t delta;
        /// the number of elements pushed onto the stack
        const uint8_t alpha;

        /// ...
        bitset<256> pushVal;

        Instruction(uint8_t val, string m, uint8_t d, uint8_t a):opcode(val),mnemonic(m),delta(d),alpha(a),pushVal(bitset<256>(0)) { }

        Instruction(const Instruction&) = default;

        void processStack(stack<bitset<256>>& stack) const;

    };
}

#endif //EVAL_INSTRUCTION_H
