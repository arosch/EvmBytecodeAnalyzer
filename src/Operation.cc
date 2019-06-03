#include <stdexcept>
#include "Operation.h"

using namespace evmbca;

//----------------------------------------------------------------------------------------------------------------------

Operation::Operation(uint8_t opc):Operation(opc, [&]{
    static const map<uint8_t,tuple<string,uint8_t,uint8_t>> instrMap = {
            {0x00,{"STOP",0,0}},
            {0x01,{"ADD",2,1}},
            {0x02,{"MUL",2,1}},
            {0x03,{"SUB",2,1}},
            {0x04,{"DIV",2,1}},
            {0x05,{"SDIV",2,1}},
            {0x06,{"MOD",2,1}},
            {0x07,{"SMOD",2,1}},
            {0x08,{"ADDMOD",3,1}},
            {0x09,{"MULMOD",3,1}},
            {0x0a,{"EXP",2,1}},
            {0x0b,{"SIGNEXTEND",2,1}},
            {0x10,{"LT",2,1}},
            {0x11,{"GT",2,1}},
            {0x12,{"SLT",2,1}},
            {0x13,{"SGT",2,1}},
            {0x14,{"EQ",2,1}},
            {0x15,{"ISZERO",1,1}},
            {0x16,{"AND",2,1}},
            {0x17,{"OR",2,1}},
            {0x18,{"XOR",2,1}},
            {0x19,{"NOT",1,1}},
            {0x1a,{"BYTE",2,1}},
            {0x20,{"SHA3",2,1}},
            {0x30,{"ADDRESS",0,1}},
            {0x31,{"BALANCE",0,1}},
            {0x32,{"ORIGIN",0,1}},
            {0x33,{"CALLER",0,1}},
            {0x34,{"CALLVALUE",0,1}},
            {0x35,{"CALLDATALOAD",1,1}},
            {0x36,{"CALLDATASIZE",0,1}},
            {0x37,{"CALLDATACOPY",0,1}},
            {0x38,{"CODESIZE",0,1}},
            {0x39,{"CODECOPY",0,1}},
            {0x3a,{"GASPRICE",0,1}},
            {0x3b,{"EXTCODESIZE",0,1}},
            {0x3c,{"EXTCODECOPY",0,1}},
            {0x3d,{"RETURNDATASIZE",0,1}},
            {0x3e,{"RETURNDATACOPY",0,1}},
            {0x40,{"BLOCKHASH",1,1}},
            {0x41,{"COINBASE",0,1}},
            {0x42,{"TIMESTAMP",0,1}},
            {0x43,{"NUMBER",0,1}},
            {0x44,{"DIFFICULTY",0,1}},
            {0x45,{"GASLIMIT",0,1}},
            {0x50,{"POP",1,0}},
            {0x51,{"MLOAD",1,1}},
            {0x52,{"MSTORE",2,0}},
            {0x53,{"MSTORE8",2,0}},
            {0x54,{"SLOAD",1,1}},
            {0x55,{"SSTORE",2,0}},
            {0x56,{"JUMP",1,0}},
            {0x57,{"JUMPI",2,0}},
            {0x58,{"PC",0,1}},
            {0x59,{"MSIZE",0,1}},
            {0x5a,{"GAS",0,1}},
            {0x5b,{"JUMPDEST",0,0}},
            {0x60,{"PUSH1",0,1}},
            {0x61,{"PUSH2",0,1}},
            {0x62,{"PUSH3",0,1}},
            {0x63,{"PUSH4",0,1}},
            {0x64,{"PUSH5",0,1}},
            {0x65,{"PUSH6",0,1}},
            {0x66,{"PUSH7",0,1}},
            {0x67,{"PUSH8",0,1}},
            {0x68,{"PUSH9",0,1}},
            {0x69,{"PUSH10",0,1}},
            {0x6a,{"PUSH11",0,1}},
            {0x6b,{"PUSH12",0,1}},
            {0x6c,{"PUSH13",0,1}},
            {0x6d,{"PUSH14",0,1}},
            {0x6e,{"PUSH15",0,1}},
            {0x6f,{"PUSH16",0,1}},
            {0x70,{"PUSH17",0,1}},
            {0x71,{"PUSH18",0,1}},
            {0x72,{"PUSH19",0,1}},
            {0x73,{"PUSH20",0,1}},
            {0x74,{"PUSH21",0,1}},
            {0x75,{"PUSH22",0,1}},
            {0x76,{"PUSH23",0,1}},
            {0x77,{"PUSH24",0,1}},
            {0x78,{"PUSH25",0,1}},
            {0x79,{"PUSH26",0,1}},
            {0x7a,{"PUSH27",0,1}},
            {0x7b,{"PUSH28",0,1}},
            {0x7c,{"PUSH29",0,1}},
            {0x7d,{"PUSH20",0,1}},
            {0x7e,{"PUSH31",0,1}},
            {0x7f,{"PUSH32",0,1}},
            {0x80,{"DUP1",1,2}},
            {0x81,{"DUP2",2,3}},
            {0x82,{"DUP3",3,4}},
            {0x83,{"DUP4",4,5}},
            {0x84,{"DUP5",5,6}},
            {0x85,{"DUP6",6,7}},
            {0x86,{"DUP7",7,8}},
            {0x87,{"DUP8",8,9}},
            {0x88,{"DUP9",9,10}},
            {0x89,{"DUP10",10,11}},
            {0x8a,{"DUP11",11,12}},
            {0x8b,{"DUP12",12,13}},
            {0x8c,{"DUP13",13,14}},
            {0x8d,{"DUP14",14,15}},
            {0x8e,{"DUP15",15,16}},
            {0x8f,{"DUP16",16,17}},
            {0x90,{"SWAP1",2,2}},
            {0x91,{"SWAP2",2,2}},
            {0x92,{"SWAP3",2,2}},
            {0x93,{"SWAP4",2,2}},
            {0x94,{"SWAP5",2,2}},
            {0x95,{"SWAP6",2,2}},
            {0x96,{"SWAP7",2,2}},
            {0x97,{"SWAP8",2,2}},
            {0x98,{"SWAP9",2,2}},
            {0x99,{"SWAP10",2,2}},
            {0x9a,{"SWAP11",2,2}},
            {0x9b,{"SWAP12",2,2}},
            {0x9c,{"SWAP13",2,2}},
            {0x9d,{"SWAP14",2,2}},
            {0x9e,{"SWAP15",2,2}},
            {0x9f,{"SWAP16",2,2}},
            {0xa0,{"LOG0",2,0}},
            {0xa1,{"LOG1",3,0}},
            {0xa2,{"LOG2",4,0}},
            {0xa3,{"LOG3",5,0}},
            {0xa4,{"LOG4",6,2}},
            {0xf0,{"CREATE",3,1}},
            {0xf1,{"CALL",7,1}},
            {0xf2,{"CALLCODE",7,1}},
            {0xf3,{"RETURN",2,0}},
            {0xf4,{"DELEGATECALL",6,1}},
            {0xfa,{"STATICCALL",6,1}},
            {0xfd,{"REVERT",2,0}},
            {0xfe,{"INVALID",0,0}},
            {0xff,{"SELFDESTRUCT",1,0}}
    };
    try{
        return instrMap.at(opc);
    } catch(const out_of_range& e) {
        throw out_of_range("Couldn't create an instruction with the specified opcode " +to_string(opc));
    }
}()) { }

bitset<256> Operation::getPushValue() const{
    throw logic_error("Trying to get a push value from a non push instruction");
}

void Operation::processStack(stack<bitset<256>>& stack) const {

    const unsigned pop = delta;
    const unsigned push = alpha;

    vector<bitset<256>> stackItem;

    //pop
    for(unsigned i=0;i<pop;i++){
        stackItem.push_back(stack.top());
        stack.pop();
    }

    //push
    if(push==1)
        stack.emplace(0);

}

std::unique_ptr<Instruction> Operation::toInstruction(Stack& stack, unsigned& kvar) const {
    const auto items = stack.retrieveN(delta);

    //TODO single if & processStack revamp?
    unsigned var ;
    if(alpha==1) var = kvar++;
    else var = 0;

    auto instr = make_unique<Instruction>(opcode,mnemonic,delta,alpha,items,var);

    if(alpha==1) stack.emplace(instr.get());

    //filter specific instructions
    if(opcode == Opcode::POP)
        return std::unique_ptr<Instruction>(nullptr);
    else if(opcode == Opcode::JUMPDEST)
        return std::unique_ptr<Instruction>(nullptr);

    return instr;
}

//----------------------------------------------------------------------------------------------------------------------

bitset<256> Push::getPushValue() const{
    return pushValue;
}

std::unique_ptr<Instruction> Push::toInstruction(Stack& stack, unsigned& kvar) const {
    auto instr = make_unique<Instruction>(getOpcode(),getMnemonic(),getDelta(),getAlpha(),kvar,getPushValue());
    stack.emplace(instr.get());
    kvar++;
    return instr;
}

string Push::toString() const{
    try{
        return getMnemonic()+": "+to_string(getPushValue().to_ullong());
    } catch(const overflow_error& e){
        return getMnemonic();
    }
}

void Push::processStack(stack<bitset<256>>& stack) const{
    stack.push(pushValue);
}

//----------------------------------------------------------------------------------------------------------------------

void Swap::processStack(stack<bitset<256>>& stack) const{
    const unsigned amount = getOpcode() - 0x8e;
    vector<bitset<256>> swapItem;
    swapItem.reserve(amount);

    for(unsigned i=0;i<amount;i++){
        swapItem.push_back(stack.top());
        stack.pop();
    }
    //swap first with last element
    iter_swap(swapItem.begin(),swapItem.rbegin());
    //push back onto the stack
    for(auto it=swapItem.rbegin();it!=swapItem.rend();it++){
        stack.push(*it);
    }
}

std::unique_ptr<Instruction> Swap::toInstruction(Stack& stack, unsigned& kvar) const {
    (void)kvar;

    const unsigned amount = getOpcode() - 0x8e;
    auto items = stack.retrieveN(amount);

    //swap first with last element
    iter_swap(items.begin(),items.rbegin());

    //push back onto the stack
    for(auto it=items.rbegin();it!=items.rend();it++){
        stack.push(*it);
    }

    return unique_ptr<Instruction>(nullptr);
}

//----------------------------------------------------------------------------------------------------------------------

void Dup::processStack(stack<bitset<256>>& stack) const{
    const unsigned amount = getOpcode() - 0x7f;
    vector<bitset<256>> dupItem;
    //+1 for dup item
    dupItem.reserve(amount+1);
    //dummy holder for dup item
    dupItem.emplace_back(0);

    for(unsigned i=0;i<amount;i++){
        dupItem.push_back(stack.top());
        stack.pop();
    }
    //duplicate last item to the front
    dupItem.at(0)=dupItem.back();
    //push back onto the stack
    for(auto it=dupItem.rbegin();it!=dupItem.rend();it++){
        stack.push(*it);
    }
}

std::unique_ptr<Instruction> Dup::toInstruction(Stack& stack, unsigned& kvar) const {
    (void)kvar;
    const unsigned amount = getOpcode() - 0x7f;
    auto items = stack.retrieveN(amount);

    //duplicate last item to the front
    items.insert(items.begin(),items.back());

    //push back onto the stack
    stack.pushN(items);

    return unique_ptr<Instruction>(nullptr);
}
