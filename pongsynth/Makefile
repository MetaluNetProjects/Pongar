# use : make pdincludepath=/path/to/pure-data/src/
#
# The following command will build the external and install the distributable
# files into a subdirectory called build/moonlib :
#
# make install pdincludepath=../pure-data/src/ objectsdir=./build

PONGLIB = ../fw/sound

cflags = -I$(PONGLIB) -I. -g
lib.name = pongsynth~

pongsynth~.class.sources = pongsynth~.cpp $(PONGLIB)/osc.cpp

PDLIBBUILDER_DIR ?= .
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

