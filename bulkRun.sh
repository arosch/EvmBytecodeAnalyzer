#!/usr/bin/env bash

# directory of this file
dir="${0%/*}"

# make the executable
(cd $dir/build && make)

# execution
for filename in $dir/input/*.bin; do
    $dir/build/evmBcAnalyzer $1 $filename $dir/output/graph.gv $dir/output/graph2.gv $dir/output/graph3.gv
    echo "-------------------------------------------------------------"
done
