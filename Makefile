CFLAGS = -O -std=c++17
CC = g++

EvmBcAnalyzer: main.o Operation.o Instruction.o BasicBlock.o Contract.o Candidate.o
	$(CC) $(CFLAGS) -o EvmBcAnalyzer main.o Operation.o Instruction.o BasicBlock.o Contract.o Candidate.o

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

Operation.o: Operation.cpp Operation.h
	$(CC) $(CFLAGS) -c Operation.cpp

Instruction.o: Instruction.cpp Instruction.h
	$(CC) $(CFLAGS) -c Instruction.cpp

BasicBlock.o: BasicBlock.cpp BasicBlock.h
	$(CC) $(CFLAGS) -c BasicBlock.cpp

Contract.o: Contract.cpp Contract.h
	$(CC) $(CFLAGS) -c Contract.cpp

Candidate.o: Candidate.cpp Candidate.h
	$(CC) $(CFLAGS) -c Candidate.cpp

clean:
	rm -f core *.o
