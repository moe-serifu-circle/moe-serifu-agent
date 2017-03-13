# Example makefile
#
# This serves as an example for compiling C++ plugins for Moe Serifu.
# This Makefile is not called from any recipe in the main project
# Makefile, but its instructions may be replicated


# Set up flags
#
# The vital flags in CXXFLAGS are -fPIC (or not if you don't mind not being
# supported by older architectures) and -I../../src. The include should be
# replaced with the location of the MSA headers on your system.
#
# LDFLAGS must also contain -shared, otherwise the .so won't get linked as
# shared objects.
CXX ?= g++
CXXFLAGS ?= -I../../src -fPIC -std=c++11 -Wall -Wextra -Wpedantic
LDFLAGS ?= -shared

.PHONY: clean

example.so: example.o
	$(CXX) -o $@ example.o $(LDFLAGS)
	
example.o: example.cpp
	$(CXX) -c -o $@ example.cpp $(CXXFLAGS)

clean:
	rm -f *.o
	rm -f *.so

