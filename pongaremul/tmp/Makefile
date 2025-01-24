
PONGLIB = ../../fw/

all: test

test: test.cpp $(PONGLIB)/players.cpp
	g++ -o test -I. -I$(PONGLIB) -g -Wno-unused-parameter test.cpp $(PONGLIB)/players.cpp


