# EVM Bytecode Analyzer

## Author
Alexander Roschlaub

## Build Instructions

```sh
mkdir build
cd build
cmake -G "Unix Makefiles" ..
```

## Run Instructions
1 for statistics.
0 without.
```sh
./run.sh <1|0> path/bytecode/file
```


## Compile Examples
```sh
solc --evm-version byzantium --bin <path/example> > <path/ouput>
```