#!/usr/bin/env bash

make
#make clean

./EvmBcAnalyzer $1 ./output/graph.gv ./output/graph2.gv ./output/graph3.gv

dot -Tps output/graph.gv -o output/graph_creation.ps
dot -Tps output/graph2.gv -o output/graph2_runtime.ps
dot -Tps output/graph3.gv -o output/graph3_abstract.ps
