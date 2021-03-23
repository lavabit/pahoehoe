#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void term(int sig)
{
	fprintf(stderr, "line 2 of 2\n");
	fflush(stderr);
	exit(0);
}

int main()
{
	signal(SIGTERM, term);
	fprintf(stderr, "line 1 of 2\n");
	sleep(100);
	return 0;
}
