CXX = g++
CXXFLAGS = -g -O3 -std=c++11

OBJS = main.o parse.o compile.o link.o
HEADERS = $(wildcard *.h)

compiler: $(OBJS) $(HEADERS)
	g++ -o $@ $(OBJS)

.PHONY: all
all: compiler

.PHONY: clean
clean:
	rm $(OBJS)
