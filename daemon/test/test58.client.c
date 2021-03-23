#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	int i;

	setvbuf(stdout, NULL, _IOLBF, 0);

	for (i = 0; i < 5; ++i)
	{
		sleep(1);
		printf("%d\n", i);
	}

	return EXIT_SUCCESS;
}
