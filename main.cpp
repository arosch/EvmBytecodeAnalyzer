#include <string>

#include "BasicBlock.h"
#include "Operation.h"
#include "Contract.h"

using namespace std;
using namespace contract;



int main(int argc, char *argv[]) {

    if(argc!=5){
        cerr<<"Requires input filename, out creation,  out runtime, out abstract\n";
        return EXIT_FAILURE;
    }
    const string filename = argv[1];
    const string fout = argv[2];
    const string foutr = argv[3];
    const string fouta = argv[4];
    try{
        Contract contract(filename);
        contract.retrieveCfgCreation(fout);
        contract.retrieveCfgRuntime(foutr);
        contract.retrieveCfgOptimizedRuntime(fouta);

        return EXIT_SUCCESS;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
}