// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#define nil ((void*)0)
#define nelem(x) (sizeof(x)/sizeof((x)[0]))

typedef struct Regexp Regexp;
typedef struct Prog Prog;
typedef struct ByteProg ByteProg;
typedef struct Inst Inst;

struct Regexp
{
	int type;
	int n;
	int ch;
	Regexp *left;
	Regexp *right;
};

enum	/* Regexp.type */
{
	Alt = 1,
	Cat,
	Lit,
	Dot,
	Paren,
	Quest,
	Star,
	Plus,
};

Regexp *parse(char*);
Regexp *reg(int type, Regexp *left, Regexp *right);
void printre(Regexp*);
void fatal(char*, ...);
void *mal(int);

struct Prog
{
	Inst *start;
	int len;
};

struct ByteProg
{
	char *start;
	int bytelen;
	int len;
	int sub;
};

struct Inst
{
	int opcode;
	int c;
	int n;
	Inst *x;
	Inst *y;
	int gen;	// global state, oooh!
};

enum	/* Inst.opcode */
{
	// Instructions which consume input bytes (and thus fail if none left)
	CONSUMERS = 1,
	Char = CONSUMERS,
	Any,
	// Instructions which take relative offset as arg
	JUMPS = 0x60,
	Jmp = JUMPS,
	Split,
	RSplit,
	// Other (special) instructions
	Save = 0x7e,
	Match = 0x7f,
};

#define inst_is_consumer(inst) ((inst) < JUMPS)
#define inst_is_jump(inst) ((inst) & 0x70 == JUMPS)

Prog *compile(Regexp*);
void printprog(Prog*);

extern int gen;

enum {
	MAXSUB = 20
};

typedef struct Sub Sub;

struct Sub
{
	int ref;
	int nsub;
	char *sub[MAXSUB];
};

Sub *newsub(int n);
Sub *incref(Sub*);
Sub *copy(Sub*);
Sub *update(Sub*, int, char*);
void decref(Sub*);

int backtrack(ByteProg*, char*, char**, int);
int pikevm(ByteProg*, char*, char**, int);
int recursiveloopprog(ByteProg*, char*, char**, int);
int recursiveprog(ByteProg*, char*, char *, char**, int);
int thompsonvm(ByteProg*, char*, char**, int);

ByteProg *compile2code(char *re);
void dump_code(ByteProg *prog);
void cleanmarks(ByteProg *prog);
