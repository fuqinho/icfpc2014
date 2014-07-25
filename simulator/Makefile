.PHONY: clean
CXXFLAGS=-O2 -std=c++11
CXX=g++

simulator: simulator.o
	$(CXX) -o $@ $^ -lglog

simulator.o: simulator.cc

clean:
	rm -rf *~ *.o simulator
