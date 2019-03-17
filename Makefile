CFLAGS = -O
CC = g++

EvmBcAnalyzer: main.o Instruction.o BasicBlock.o
	$(CC) $(CFLAGS) -o EvmBcAnalyzer main.o Instruction.o BasicBlock.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

Instruction.o: Instruction.cpp Instruction.h
	$(CC) $(CFLAGS) -c Instruction.cpp

BasicBlock.o: BasicBlock.cpp BasicBlock.h
	$(CC) $(CFLAGS) -c BasicBlock.cpp

clean:
	rm -f core *.o
