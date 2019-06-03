#!/usr/bin/env bash

# directory of this file
dir="${0%/*}"

# make the executable
(cd $dir/build && make)

# execution
$dir/build/evmBcAnalyzer $1 $2 $dir/output/graph.gv $dir/output/graph2.gv $dir/output/graph3.gv

# generate the ps cfgs
dot -Tps $dir/output/graph.gv -o $dir/output/graph_creation.ps
dot -Tps $dir/output/graph2.gv -o $dir/output/graph2_runtime.ps
dot -Tps $dir/output/graph3.gv -o $dir/output/graph3_abstract.ps
