// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

typedef struct Thread Thread;
struct Thread
{
	char *pc;
};

typedef struct ThreadList ThreadList;
struct ThreadList
{
	int n;
	Thread t[1];
};

static Thread
thread(char *pc)
{
	Thread t = {pc};
	return t;
}

static ThreadList*
threadlist(int n)
{
	return mal(sizeof(ThreadList)+n*sizeof(Thread));
}

static void
addthread(ThreadList *l, Thread t)
{
	int off;
	if(*t.pc & 0x80)
		return;	// already on list

	*t.pc |= 0x80;
	l->t[l->n] = t;
	l->n++;
	
	switch(*t.pc & 0x7f) {
	case Jmp:
		off = (signed char)t.pc[1];
		t.pc += 2;
		addthread(l, thread(t.pc + off));
		break;
	case Split:
		off = (signed char)t.pc[1];
		t.pc += 2;
		addthread(l, thread(t.pc));
		addthread(l, thread(t.pc + off));
		break;
	case RSplit:
		off = (signed char)t.pc[1];
		t.pc += 2;
		addthread(l, thread(t.pc + off));
		addthread(l, thread(t.pc));
		break;
	case Save:
		off = (unsigned char)t.pc[1];
		t.pc += 2;
		addthread(l, thread(t.pc));
		break;
	}
}

int
thompsonvm(ByteProg *prog, char *input, char *end, char **subp, int nsubp)
{
	int i, len, matched;
	ThreadList *clist, *nlist, *tmp;
	char *pc;
	char *sp;

	for(i=0; i<nsubp; i++)
		subp[i] = nil;

	len = prog->len;
	clist = threadlist(len);
	nlist = threadlist(len);
	
	if(nsubp >= 1)
		subp[0] = input;
	cleanmarks(prog);
	addthread(clist, thread(prog->start));
	matched = 0;
	for(sp=input;; sp++) {
		if(clist->n == 0)
			break;
		// printf("%d(%02x).", (int)(sp - input), *sp & 0xFF);
		cleanmarks(prog);
		for(i=0; i<clist->n; i++) {
			pc = clist->t[i].pc;
			// printf(" %d", (int)(pc - prog->start));
			if (inst_is_consumer(*pc & 0x7f)) {
				// If we need to match a character, but there's none left,
				// it's fail (we don't schedule current thread for continuation)
				if(sp >= end)
					continue;
			}
			switch(*pc++ & 0x7f) {
			case Char:
				if(*sp != *pc++)
					break;
			case Any:
				addthread(nlist, thread(pc));
				break;
			case Match:
				if(nsubp >= 2)
					subp[1] = sp;
				matched = 1;
				goto BreakFor;
			// Jmp, Split, Save handled in addthread, so that
			// machine execution matches what a backtracker would do.
			// This is discussed (but not shown as code) in
			// Regular Expression Matching: the Virtual Machine Approach.
			}
		}
	BreakFor:
		// printf("\n");
		tmp = clist;
		clist = nlist;
		nlist = tmp;
		nlist->n = 0;
		if(*sp == '\0')
			break;
	}
	return matched;
}
