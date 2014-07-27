// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int
recursive(char *pc, char *sp, char **subp, int nsubp)
{
	char *old;
	int off;

	switch(*pc++) {
	case Char:
		if(*sp != *pc++)
			return 0;
	case Any:
		if(*sp == '\0')
			return 0;
		return recursive(pc, sp+1, subp, nsubp);
	case Match:
		return 1;
	case Jmp:
		off = (signed char)*pc++;
		return recursive(pc + off, sp, subp, nsubp);
	case Split:
		off = (signed char)*pc++;
		if(recursive(pc, sp, subp, nsubp))
			return 1;
		return recursive(pc + off, sp, subp, nsubp);
	case RSplit:
		off = (signed char)*pc++;
		if(recursive(pc + off, sp, subp, nsubp))
			return 1;
		return recursive(pc, sp, subp, nsubp);
	case Save:
		off = (unsigned char)*pc++;
		if(off >= nsubp)
			return recursive(pc, sp, subp, nsubp);
		old = subp[off];
		subp[off] = sp;
		if(recursive(pc, sp, subp, nsubp))
			return 1;
		subp[off] = old;
		return 0;
	}
	fatal("recursive");
	return -1;
}

int
recursiveprog(ByteProg *prog, char *input, char **subp, int nsubp)
{
	return recursive(prog->start, input, subp, nsubp);
}

int
recursiveloop(char *pc, char *sp, char **subp, int nsubp)
{
	char *old;
	int off;
	
	for(;;) {
		switch(*pc++) {
		case Char:
			if(*sp != *pc++)
				return 0;
		case Any:
			if(*sp == 0)
				return 0;
			sp++;
			continue;
		case Match:
			return 1;
		case Jmp:
			off = (signed char)*pc++;
			pc = pc + off;
			continue;
		case Split:
			off = (signed char)*pc++;
			if(recursiveloop(pc, sp, subp, nsubp))
				return 1;
			pc = pc + off;
			continue;
		case RSplit:
			off = (signed char)*pc++;
			if(recursiveloop(pc + off, sp, subp, nsubp))
				return 1;
			continue;
		case Save:
			off = (unsigned char)*pc++;
			if(off >= nsubp) {
				continue;
			}
			old = subp[off];
			subp[off] = sp;
			if(recursiveloop(pc, sp, subp, nsubp))
				return 1;
			subp[off] = old;
			return 0;
		}
		fatal("recursiveloop");
	}
}

int
recursiveloopprog(ByteProg *prog, char *input, char **subp, int nsubp)
{
	return recursiveloop(prog->start, input, subp, nsubp);
}
