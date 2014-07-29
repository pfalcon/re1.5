// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

int
recursive(char *pc, char *sp, char *end, char **subp, int nsubp)
{
	char *old;
	int off;

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
		return recursive(pc, sp+1, end, subp, nsubp);
	case Match:
		return 1;
	case Jmp:
		off = (signed char)*pc++;
		return recursive(pc + off, sp, end, subp, nsubp);
	case Split:
		off = (signed char)*pc++;
		if(recursive(pc, sp, end, subp, nsubp))
			return 1;
		return recursive(pc + off, sp, end, subp, nsubp);
	case RSplit:
		off = (signed char)*pc++;
		if(recursive(pc + off, sp, end, subp, nsubp))
			return 1;
		return recursive(pc, sp, end, subp, nsubp);
	case Save:
		off = (unsigned char)*pc++;
		if(off >= nsubp)
			return recursive(pc, sp, end, subp, nsubp);
		old = subp[off];
		subp[off] = sp;
		if(recursive(pc, sp, end, subp, nsubp))
			return 1;
		subp[off] = old;
		return 0;
	}
	fatal("recursive");
	return -1;
}

int
recursiveprog(ByteProg *prog, char *input, char *end, char **subp, int nsubp)
{
	return recursive(prog->start, input, end, subp, nsubp);
}
