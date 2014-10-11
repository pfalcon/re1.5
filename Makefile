# Copyright 2007-2009 Russ Cox.  All Rights Reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

CC=gcc
# Add -DDEBUG to CFLAGS when developing/testing
CFLAGS=-ggdb -Wall -Os

TARG=re
OFILES=\
	backtrack.o\
	compile.o\
	main.o\
	pike.o\
	recursive.o\
	recursiveloop.o\
	sub.o\
	thompson.o\
	compilecode.o\
	dumpcode.o\
	util.o\
	y.tab.o\

HFILES=\
	regexp.h\

re: $(OFILES)
	$(CC) -o re $(OFILES)

%.o: %.c $(HFILES)
	$(CC) -c $(CFLAGS) $*.c

y.tab.h y.tab.c: parse.y
	bison -v -y parse.y

clean:
	rm -f *.o core re y.tab.[ch] y.output
