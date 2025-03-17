# use : make pdincludepath=/path/to/pure-data/src/
#
# The following command will build the external and install the distributable
# files into a subdirectory called build/moonlib :
#
# make install pdincludepath=../pure-data/src/ objectsdir=./build

PONGLIB = ../fw

cflags = -I. -I$(PONGLIB) -I$(PONGLIB)/hw -I$(PONGLIB)/game -g -Wno-unused-parameter
lib.name = pongaremul

pongaremul.class.sources = pongaremul.cpp audiolayer.cpp $(PONGLIB)/game/game.cpp $(PONGLIB)/game/players.cpp $(PONGLIB)/sound/osc.cpp $(PONGLIB)/gfx/gfx.cpp

PDLIBBUILDER_DIR ?= .
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

