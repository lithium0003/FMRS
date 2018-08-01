#!/bin/make
CXX = g++-8
CXXFLAGS = -O3 -Wall

SRC= main.cpp analyze.cpp wave.cpp
OBJ=$(SRC:.cpp=.o)
PROGRAM=fmrs


$(PROGRAM): $(OBJ)
	$(CXX) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

analyze.o wave.o main.o: fmrs.hpp

wave.o: wave.hpp

analyze.o: analyze.hpp

clean:
	$(RM) $(PROGRAM)
	$(RM) $(OBJ)
