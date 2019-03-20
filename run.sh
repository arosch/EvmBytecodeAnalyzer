#!/usr/bin/env bash

make
#make clean

./EvmBcAnalyzer $1 ./output/graph.gv ./output/graph2.gv

dot -Tps output/graph.gv -o output/graph.ps
dot -Tps output/graph2.gv -o output/graph2.ps
