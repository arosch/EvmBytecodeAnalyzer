#ifndef EVAL_INSTRUCTION_H
#define EVAL_INSTRUCTION_H

#include <vector>
#include <bitset>
#include <stack>
#include <map>


using namespace std;

namespace instr {

    class Instruction{
    public:
        Instruction(const uint8_t o, const string m, const uint8_t d, const uint8_t a, const vector<pair<unsigned,bitset<256>>> p, const unsigned num):opcode(o),mnemonic(m),delta(d),alpha(a),params(p),variable(num) { }

        Instruction(const Instruction& other)=default;

        string toDotLabel(const unsigned bbJumpIndex) const;

        bool returnsVar() const{ return alpha==1; }

        bool isAJumpInstruction() { return opcode==0x56 || opcode==0x57; }

        bool operator==(const Instruction& other) const;

    private:
        /// the hex value of the instruction
        const uint8_t opcode;
        /// ...
        const string mnemonic;
        /// the number of elements popped from the stack
        const uint8_t delta;
        /// the number of elements pushed onto the stack
        const uint8_t alpha;
        const vector<pair<unsigned,bitset<256>>> params;
        const unsigned variable;
    };

}

#endif //EVAL_INSTRUCTION_H
