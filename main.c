// Copyright 2007-2009 Russ Cox.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "re1.5.h"

struct {
	char *name;
	int (*fn)(ByteProg*, Subject*, const char**, int, int);
} tab[] = {
	{"recursive", re1_5_recursiveprog},
	{"recursiveloop", re1_5_recursiveloopprog},
	{"backtrack", re1_5_backtrack},
	{"thompson", re1_5_thompsonvm},
	{"pike", re1_5_pikevm},
};

void
usage(void)
{
	fprintf(stderr, "usage: re search|match <regexp> <string>...\n");
	exit(2);
}

int
main(int argc, char **argv)
{
	int i, j, k, l;
	int is_anchored = 0;

	if(argc < 3)
		usage();

	if (*argv[1] == 'm')
		is_anchored = 1;

	#ifdef ODEBUG
	Regexp *re = parse(argv[2]);
	printre(re);
	printf("\n");

	Prog *prog = compile(re);
	printprog(prog);
	printf("=============\n");
	#endif
	int sz = re1_5_sizecode(argv[2]);
	#ifdef DEBUG
	printf("Precalculated size: %d\n", sz);
	#endif
	if (sz == -1) {
		re1_5_fatal("Error in regexp");
	}

	ByteProg *code = malloc(sizeof(ByteProg) + sz);
	int ret = re1_5_compilecode(code, argv[2]);
        if (ret != 0) {
		re1_5_fatal("Error in regexp");
	}
	#ifdef DEBUG
	re1_5_dumpcode(code);
	#endif

	int sub_els = (code->sub + 1) * 2;
	const char *sub[sub_els];
	for(i=3; i<argc; i++) {
		printf("#%d %s\n", i, argv[i]);
		for(j=0; j<nelem(tab); j++) {
			Subject subj = {argv[i], argv[i] + strlen(argv[i])};
			printf("%s ", tab[j].name);
			memset(sub, 0, sub_els * sizeof sub[0]);
			if(!tab[j].fn(code, &subj, sub, sub_els, is_anchored)) {
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
