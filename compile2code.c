#include "regexp.h"

void insert_code(char *code, int at, int num, int *pc)
{
    memmove(code + at + num, code + at, *pc - at);
    *pc += num;
}

#define REL(at, to) (to - at - 2)

int size_code(const char *re)
{
    int pc = 5; // Save 0, Save 1, Match

    for (; *re; re++) {
        switch (*re) {
        case '\\':
            re++;
        default:
        case '?':
        case '+':
            pc += 2;
            break;
        case '.':
        case '^':
        case '$':
            pc++;
            break;
        case '*':
        case '|':
        case '(':
            pc += 4;
            break;
        case ')':
            break;
        }
    }

    return pc;
}

#define EMIT(at, byte) code[at] = byte

const char *_compile2code(const char *re, ByteProg *prog)
{
    char *code = prog->insts;
    int pc = prog->bytelen;
    int start = pc;
    int term = pc;
    int alt_label = 0;

    for (; *re && *re != ')'; re++) {
        switch (*re) {
        case '\\':
            re++;
        default:
            term = pc;
            EMIT(pc++, Char);
            EMIT(pc++, *re);
            prog->len++;
            break;
        case '.':
            term = pc;
            EMIT(pc++, Any);
            prog->len++;
            break;
        case '(':
            term = pc;

            EMIT(pc++, Save);
            EMIT(pc++, 2 * ++prog->sub);
            prog->len++;

            prog->bytelen = pc;
            re = _compile2code(re + 1, prog);
            pc = prog->bytelen;

            EMIT(pc++, Save);
            EMIT(pc++, 2 * prog->sub + 1);
            prog->len++;

            break;
        case '?':
            insert_code(code, term, 2, &pc);
            EMIT(term, Split);
            EMIT(term + 1, REL(term, pc));
            prog->len++;
            break;
        case '*':
            insert_code(code, term, 2, &pc);
            EMIT(pc, Jmp);
            EMIT(pc + 1, REL(pc, term));
            pc += 2;
            EMIT(term, Split);
            EMIT(term + 1, REL(term, pc));
            prog->len += 2;
            break;
        case '+':
            EMIT(pc, RSplit);
            EMIT(pc + 1, REL(pc, term));
            pc += 2;
            prog->len++;
            break;
        case '|':
            if (alt_label) {
                EMIT(alt_label, REL(alt_label, pc) + 1);
            }
            insert_code(code, start, 2, &pc);
            EMIT(pc++, Jmp);
            alt_label = pc++;
            EMIT(start, Split);
            EMIT(start + 1, REL(start, pc));
            prog->len += 2;
            break;
        case '^':
            EMIT(pc++, Bol);
            prog->len++;
            break;
        case '$':
            EMIT(pc++, Eol);
            prog->len++;
            break;
        }
    }

    if (alt_label) {
        EMIT(alt_label, REL(alt_label, pc) + 1);
    }
    prog->bytelen = pc;
    return re;
}

int compile2code(ByteProg *prog, const char *re)
{
    prog->len = 0;
    prog->bytelen = 0;
    prog->sub = 0;

    prog->insts[prog->bytelen++] = Save;
    prog->insts[prog->bytelen++] = 0;
    prog->len++;

    _compile2code(re, prog);

    prog->insts[prog->bytelen++] = Save;
    prog->insts[prog->bytelen++] = 1;
    prog->len++;

    prog->insts[prog->bytelen++] = Match;
    prog->len++;

    return 0;
}

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

#ifdef DEBUG
void dump_code(ByteProg *prog)
{
    int pc = 0;
    char *code = prog->insts;
    while (pc < prog->bytelen) {
                printf("%2d: ", pc);
                switch(code[pc++]) {
                default:
                        assert(0);
//                        fatal("printprog");
                case Split:
                        printf("split %d (%d)\n", pc + (signed char)code[pc] + 1, (signed char)code[pc]);
                        pc++;
                        break;
                case RSplit:
                        printf("rsplit %d (%d)\n", pc + (signed char)code[pc] + 1, (signed char)code[pc]);
                        pc++;
                        break;
                case Jmp:
                        printf("jmp %d (%d)\n", pc + (signed char)code[pc] + 1, (signed char)code[pc]);
                        pc++;
                        break;
                case Char:
                        printf("char %c\n", code[pc++]);
                        break;
                case Any:
                        printf("any\n");
                        break;
                case Match:
                        printf("match\n");
                        break;
                case Save:
                        printf("save %d\n", (unsigned char)code[pc++]);
                        break;
                case Bol:
                        printf("assert bol\n");
                        break;
                case Eol:
                        printf("assert eol\n");
                        break;
                }
    }
    printf("Bytes: %d, insts: %d\n", prog->bytelen, prog->len);
}
#endif

#if 0
int main(int argc, char *argv[])
{
    int pc = 0;
    ByteProg *code = compile2code(argv[1]);
    dump_code(code);
}
#endif
