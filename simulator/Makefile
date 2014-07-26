.PHONY: clean
CXXFLAGS=-O3 -std=c++11 -g
CC=g++

simulator: simulator.o lambdaman.o ghost.o
	$(CC) -o $@ $^ -lglog

simulator.o: simulator.cc
lambdaman.o: lambdaman.cc
ghost.o: ghost.cc

clean:
	rm -rf *~ *.o simulator
