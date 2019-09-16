#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int
main (int argc, char *argv[], char *env[])
{
	int x;

	printf ("Program Arguments:\n");

	for (x = 0; x < argc; x++) {
		printf ("argument %d = %s\n", x, argv[x]);
	}

	printf ("\n\nEnvironment Strings\n");

	for (x = 0; env[x] != NULL; x++) {
		printf ("environment %d = %s\n", x, env[x]);
	}

	return 0;
}

