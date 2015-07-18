// Copyright 2014 Paul Sokolovsky.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "re1.5.h"

void
cleanmarks(ByteProg *prog)
{
    char *pc = prog->insts;
    char *end = pc + prog->bytelen;
    while (pc < end) {
        *pc &= 0x7f;
        switch (*pc) {
        case Jmp:
        case Split:
        case RSplit:
        case Save:
        case Char:
                pc++;
        }
        pc++;
    }
}
