# Copyright 2007-2009 Russ Cox.  All Rights Reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

CC=gcc
TARG=re
# Set to 1 for debug compilation by default
DEBUG ?= 0
.DEFAULT_GOAL:=$(TARG)

# "Production" compilation flags
CFLAGS=-g3 -Wall -Os

# Compilation flags for development
# For -fsanitize=undefined, you may need to install libubsan
DEBUG_CFLAGS=-DDEBUG -g3 -Wall -O0 -fsanitize=address,undefined -fno-omit-frame-pointer

ifeq (1, $(DEBUG))
	CFLAGS=$(DEBUG_CFLAGS)
endif
gdb: CFLAGS=$(DEBUG_CFLAGS)

ASAN_SETUP=ASAN_SYMBOLIZER_PATH=/usr/bin/llvm-symbolizer ASAN_OPTIONS=abort_on_error=1,symbolize=1


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
	charclass.o\
	util.o\
	y.tab.o\

HFILES=\
	re1.5.h\

$(TARG): $(OFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OFILES)

%.o: %.c $(HFILES)
	$(CC) -c $(CFLAGS) $*.c

y.tab.h y.tab.c: parse.y
	bison -v -y parse.y

gdb: $(TARG)
	$(ASAN_SETUP) gdb ./$<

# For now, LSAN (detect_leaks) is disabled because currently
# allocated memory in the program not is freed.
# Else its non-zero exit status causes the tests to fail.
test: $(TARG)
	$(ASAN_SETUP),detect_leaks=0 ./run-tests $(TFLAGS)

clean:
	rm -f *.o core $(TARG) y.tab.[ch] y.output
