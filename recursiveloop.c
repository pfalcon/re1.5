// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int
recursiveloop(char *pc, char *sp, char *end, char **subp, int nsubp)
{
	char *old;
	int off;
	
	for(;;) {
		if(inst_is_consumer(*pc)) {
			// If we need to match a character, but there's none left, it's fail
			if(sp >= end)
				return 0;
		}
		switch(*pc++) {
		case Char:
			if(*sp != *pc++)
				return 0;
		case Any:
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
			if(recursiveloop(pc, sp, end, subp, nsubp))
				return 1;
			pc = pc + off;
			continue;
		case RSplit:
			off = (signed char)*pc++;
			if(recursiveloop(pc + off, sp, end, subp, nsubp))
				return 1;
			continue;
		case Save:
			off = (unsigned char)*pc++;
			if(off >= nsubp) {
				continue;
			}
			old = subp[off];
			subp[off] = sp;
			if(recursiveloop(pc, sp, end, subp, nsubp))
				return 1;
			subp[off] = old;
			return 0;
		}
		fatal("recursiveloop");
	}
}

int
recursiveloopprog(ByteProg *prog, char *input, char *end, char **subp, int nsubp)
{
	return recursiveloop(prog->start, input, end, subp, nsubp);
}
