// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "regexp.h"

struct {
	char *name;
	int (*fn)(ByteProg*, char*, char**, int);
} tab[] = {
	{"recursive", recursiveprog},
	{"recursiveloop", recursiveloopprog},
	{"backtrack", backtrack},
	{"thompson", thompsonvm},
	{"pike", pikevm},
};

void
usage(void)
{
	fprintf(stderr, "usage: re regexp string...\n");
	exit(2);
}

int
main(int argc, char **argv)
{
	int i, j, k, l;
	Regexp *re;
	Prog *prog;

	if(argc < 2)
		usage();
	
	re = parse(argv[1]);
	printre(re);
	printf("\n");

	prog = compile(re);
	printprog(prog);
	printf("=============\n");
	ByteProg *code = compile2code(argv[1]);
	#ifdef DEBUG
	dump_code(code);
	#endif

	int sub_els = (code->sub + 1) * 2;
	char *sub[sub_els];
	for(i=2; i<argc; i++) {
		printf("#%d %s\n", i, argv[i]);
		for(j=0; j<nelem(tab); j++) {
			printf("%s ", tab[j].name);
			memset(sub, 0, sub_els * sizeof sub[0]);
			if(!tab[j].fn(code, argv[i], sub, sub_els)) {
				printf("-no match-\n");
				continue;
			}
			printf("match");
			for(k=sub_els; k>0; k--)
				if(sub[k-1])
					break;
			for(l=0; l<k; l+=2) {
				printf(" (");
				if(sub[l] == nil)
					printf("?");
				else
					printf("%d", (int)(sub[l] - argv[i]));
				printf(",");
				if(sub[l+1] == nil)
					printf("?");
				else
					printf("%d", (int)(sub[l+1] - argv[i]));
				printf(")");
			}
			printf("\n");
		}
	}
	return 0;
}
