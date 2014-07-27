CXX = g++
CXXFLAGS = -g -Wall -O3 -std=c++11

OBJS = main.o
HEADERS = $(wildcard *.h)

ghcasm: $(OBJS) $(HEADERS)
	g++ -o $@ $(OBJS)

$(OBJS) : $(HEADERS)

.PHONY: all
all: ghcasm

.PHONY: clean
clean:
	rm $(OBJS)
