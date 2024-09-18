#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	const char *arch;
	char prog[1024];

	arch = getenv("SRCARCH");
	if (!arch) {
		fprintf(stderr, "objtool: SRCARCH variable not defined\n");
		return 2;
	}

	snprintf(prog, sizeof(prog), "%s.real-%s", argv[0], arch);
	execv(prog, argv);

	fprintf(stderr, "objtool: Failed to execute %s\n", prog);
	return 1;
}
