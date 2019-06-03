#include <string>

#include "Contract.h"

int main(int argc, char *argv[]) {

    if(argc!=6){
        cerr<<"Usage: <stats> <input filename> <out creation>  <out runtime> <out abstract>\n";
        return EXIT_FAILURE;
    }
    const unsigned stats = std::atoi(argv[1]);
    const std::string filename = argv[2];
    const std::string fout = argv[3];
    const std::string foutr = argv[4];
    const std::string fouta = argv[5];
    try{
        evmbca::Contract contract(filename);
        contract.retrieveCfgCreation(fout);
        contract.retrieveCfgRuntime(foutr);
        contract.retrieveCfgOptimizedRuntime(fouta);

        if(stats==1){
            contract.writeStatistics();
        }

        return EXIT_SUCCESS;

    } catch(const exception& e){
        cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }
}
