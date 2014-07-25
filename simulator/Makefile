.PHONY: clean
CXXFLAGS=-lglog -O2 -std=c++11
CXX=g++

simulator: simulator.o
	$(CXX) -o $@ $+

simulator.o: simulator.cc

clean:
	rm -rf *~ *.o simulator
