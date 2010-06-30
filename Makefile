#Makefile for dum  ---------------------------------------------------------------------------------
#                           Copyright (C) 2010 Dario A. Rodriguez
#---------------------------------------------------------------------------------------------------
#
# This is a very basic and stupid makefile for stupid 'make' programs, this will work even on AIX
#
#---------------------------------------------------------------------------------------------------



# BEGIN OBJECT HANDLING _________________
include makefileConfig


#API DSTRING DOC IN DOCS/API-DSTRING.TXT (If any)
APIS_OBJ=\
	dstring.o \
	messages.o
#MAIN BINARY
BINARY_OBJ=\
	dum.o



OBJECTS=$(APIS_OBJ) $(BINARY_OBJ) 



.PHONY:all clean

all::dum

dstring.o: dstring.c
	$(CCMESG) dstring.o
	$(CC) -c dstring.c

dum.o:dum.c dstring.h
	$(CCMESG) dum.o
	$(CC) -c dum.c

messages.o: messages.c dum.h dstring.h
	$(CCMESG) messages.o
	$(CC) -c messages.c

dum: $(OBJECTS)
	$(CCMESG) dum
	$(CC) -o dum $(OBJECTS)

clean::
	$(RMMESG) *.o dum *~ *.bak *.bkp
	$(RM) *.o dum *~ *.bak *.bkp
