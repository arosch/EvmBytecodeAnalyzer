#ifndef INCLUDE_CONTRACT_H
#define INCLUDE_CONTRACT_H

#include <limits>
#include <vector>
#include <bitset>
#include <stack>
#include <array>
#include <unordered_map>
#include <stdexcept>
#include <set>
#include <memory>
#include <map>

#include "BasicBlock.h"
#include "Operation.h"
#include "Instruction.h"

using namespace std;

namespace evmbca {

class Contract {

    struct Norm2{
        vector<unique_ptr<Operation>> instrsCreation;
        map<uint64_t,uint64_t> jumptableCreation;

        vector<unique_ptr<Operation>> instrsRun;
        map<uint64_t,uint64_t> jumptableRun;
    };

public:
    explicit Contract(const string& filename);

    void retrieveCfgCreation(const string& fout) const{
        printToDotFile(fout,bbos_creation);
    }

    void retrieveCfgRuntime(const string& fout) const{
        printToDotFile(fout,bbos_runtime);
    }

    void retrieveCfgOptimizedRuntime(const string& fout) const{
        printToDotFile(fout,bbis_runtime);
    }

    void writeStatistics() const{
        if(!bbos_creation.empty()) writeStatstics("Creation",bbos_creation);
        if(!bbos_runtime.empty()) writeStatstics("Runtime",bbos_runtime);
        if(!bbis_runtime.empty()) writeStatstics("Abstract",bbis_runtime);
    }

private:

    template<typename T>
    void printToDotFile(const string& fout, const vector<unique_ptr<BasicBlock<T>>>& bbs) const;

    template<typename T>
    void writeStatstics(const string& label, const vector<unique_ptr<BasicBlock<T>>>& bbs) const;

    ///ignores the first three lines of given ifstream
    bool isCreationAndPrep(ifstream &istrm) const;

    vector<uint8_t> extractAuxdata(vector<uint8_t>& bytes);

    ///reads the given evm bytecode, and converts the hex chars to respective int value,
    vector<uint8_t> readBytecode(const string& filename);

    ///remove push bytes and create jump table
    Norm2 normalize1(const vector<uint8_t>& bytes);

    ///create BasicBlocks with respective fallthrough and jump children
    vector<unique_ptr<BasicBlock<Operation>>> normalize2(vector<unique_ptr<Operation>> &&instrs,
                                                 map<uint64_t, uint64_t> &jumptable);

    vector<unique_ptr<BasicBlock<Instruction>>> abstractStack(const vector<unique_ptr<BasicBlock<Operation>>>& bbos) const;

    bool creation;
    vector<unique_ptr<BasicBlock<Operation>>> bbos_creation;
    vector<unique_ptr<BasicBlock<Operation>>> bbos_runtime;
    vector<unique_ptr<BasicBlock<Instruction>>> bbis_runtime;
    vector<uint8_t> auxdata;
};

/*------------------------- Implementation ----------------------------*/

template<typename T>
void Contract::writeStatstics(const string& label, const vector<unique_ptr<BasicBlock<T>>>& bbs) const{
    unsigned countInstr = 0;
    unsigned countBb = 0;
    for(const auto& bb:bbs){
        countInstr += bb->getStatistics();
        if(!bb->contentIsEmpty() || !bb->isAJumpOnlyBb()) countBb++;
    }
    cout<<label <<" \t| Number of instructions: "<<countInstr<< " \t| Number of basic blocks: "<<countBb<<'\n';
}

template<typename T>
void Contract::printToDotFile(const string& fout, const vector<unique_ptr<BasicBlock<T>>>& bbs) const{
    //cout<<"Printing cfg to: "<<fout<<"...\n";

    if (ofstream ostrm{fout, ios::binary}) {
        ostrm << "digraph G{\n";
        ostrm << "\tnode[shape=box];\n\n";

        vector<unsigned> firstNode;
        vector<unsigned> lastNode;
        unsigned nodeId = 0;

        for(const auto& bb:bbs){
            firstNode.push_back(nodeId);
            nodeId = bb->printBbDot(ostrm,nodeId);
            lastNode.push_back(nodeId-1);
        }

        for(const auto& bb:bbs){
            bb->printBbDotDependencies(ostrm,firstNode,lastNode);
        }

        ostrm << "}";
    } else
        throw invalid_argument("Couldn't write to "+fout);
}

} // namespace evmbca

#endif //INCLUDE_CONTRACT_H
