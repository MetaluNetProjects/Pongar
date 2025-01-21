# use : make pdincludepath=/path/to/pure-data/src/
#
# The following command will build the external and install the distributable
# files into a subdirectory called build/moonlib :
#
# make install pdincludepath=../pure-data/src/ objectsdir=./build

PONGLIB = ../fw/

cflags = -I. -I$(PONGLIB) -I$(PONGLIB)/hw -g -Wno-unused-parameter
lib.name = ponggame

ponggame.class.sources = ponggame.cpp $(PONGLIB)/game.cpp

PDLIBBUILDER_DIR ?= .
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

