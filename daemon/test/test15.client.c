#include <unistd.h>
#include <termios.h>

/* Set EOF to be Control-F then do cat */

int main()
{
	struct termios attr[1];
	char c;

	if (tcgetattr(STDIN_FILENO, attr) == -1)
		return 1;

	attr->c_cc[VEOF] = CTRL('f');

	if (tcsetattr(STDIN_FILENO, TCSANOW, attr) == -1)
		return 1;

	while (read(STDIN_FILENO, &c, 1) == 1)
		write(STDOUT_FILENO, &c, 1);

	return 0;
}
