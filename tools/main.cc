#include <string>

#include "BasicBlock.h"
#include "Operation.h"
#include "Contract.h"

int main(int argc, char *argv[]) {

    if(argc!=5){
        cerr<<"Requires input filename, out creation,  out runtime, out abstract\n";
        return EXIT_FAILURE;
    }
    const std::string filename = argv[1];
    const std::string fout = argv[2];
    const std::string foutr = argv[3];
    const std::string fouta = argv[4];
    try{
        evmbca::Contract contract(filename);
        contract.retrieveCfgCreation(fout);
        contract.retrieveCfgRuntime(foutr);
        contract.retrieveCfgOptimizedRuntime(fouta);

        return EXIT_SUCCESS;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
}
