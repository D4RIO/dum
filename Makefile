# Makefile for dum  -----------------------------------------------------------
#               Copyright (C) 2010,2016 Dario A. Rodriguez, all rights reserved
# This software is free under a BSD-like license. See LICENSE file for details
# -----------------------------------------------------------------------------
#
#  This is a very basic makefile for basic 'make' programs
#  This should work on almost every UNIX system
#
# -----------------------------------------------------------------------------



# BEGIN OBJECT HANDLING _________________
include makefileConfig

APIS_OBJ=\
	messages.o

#MAIN BINARY
BINARY_OBJ=\
	dum.o

OBJECTS=$(APIS_OBJ) $(BINARY_OBJ) 

.PHONY:all clean

all::dum

dum.o: dum.c
	$(CCMESG) dum.o
	$(CC) -c dum.c

messages.o: messages.c dum.h
	$(CCMESG) messages.o
	$(CC) -c messages.c

dum: $(OBJECTS)
	$(CCMESG) dum
	$(CC) -o dum $(OBJECTS)

clean::
	$(RMMESG) *.o dum *~ *.bak *.bkp \#*
	$(RM) *.o dum *~ *.bak *.bkp \#*
