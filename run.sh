#!/usr/bin/env bash

g++ main.cpp
./a.out

dot -Tps output/graph.gv -o output/graph.ps
dot -Tps output/graph2.gv -o output/graph2.ps
