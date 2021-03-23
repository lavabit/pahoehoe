#include <slack/lib.h>
#include <sys/time.h>

extern char **environ;

/* Test --pty=noecho */

int main(int ac, char **av)
{
	pid_t pid;
	int in, out, err;
	char *client_name;
	char *daemon_path;
	char *config_path;
	char *client_path;
	char buf[BUFSIZ];
	ssize_t bytes;
	char *argv[] = { "daemon", "-C", null, "-n", null, null, null };

	fd_set save_readfds[1];
	fd_set readfds[1];
	int maxfd;

	prog_init();
	prog_set_name("test13.daemon");
	prog_set_syntax("name \"`pwd`\" client");

	if (ac != 4)
		prog_usage_msg("Invalid arguments");

	daemon_path = "../daemon";

	client_name = av[1];
	config_path = av[2];
	client_path = av[3];

	argv[2] = config_path;
	argv[4] = client_name;
	argv[5] = client_path;

	if ((pid = coproc_open(&in, &out, &err, daemon_path, argv, environ, null, null)) == -1)
		fatalsys("coproc_open failed");

	FD_ZERO(save_readfds);
	FD_SET(STDIN_FILENO, save_readfds);
	FD_SET(out, save_readfds);
	FD_SET(err, save_readfds);
	maxfd = (err > out) ? err : out;

	for (*readfds = *save_readfds; (FD_ISSET(STDIN_FILENO, readfds) || FD_ISSET(out, readfds) || FD_ISSET(err, readfds)) && select(maxfd + 1, readfds, null, null, null) != -1; *readfds = *save_readfds)
	{
		if (FD_ISSET(STDIN_FILENO, readfds))
		{
			switch (bytes = read(STDIN_FILENO, buf, BUFSIZ))
			{
				case -1:
					errorsys("read(STDIN_FILENO) failed");
					FD_CLR(STDIN_FILENO, save_readfds);
					break;

				case 0:
					close(in);
					FD_CLR(STDIN_FILENO, save_readfds);
					break;

				default:
					write(in, buf, bytes);
					break;
			}
		}

		if (FD_ISSET(out, readfds))
		{
			switch (bytes = read(out, buf, BUFSIZ))
			{
				case -1:
					errorsys("read(out) failed");
					FD_CLR(out, save_readfds);
					break;

				case 0:
					FD_CLR(out, save_readfds);
					break;

				default:
					write(STDOUT_FILENO, buf, bytes);
					break;
			}
		}

		if (FD_ISSET(err, readfds))
		{
			switch (bytes = read(err, buf, BUFSIZ))
			{
				case -1:
					errorsys("read(err) = %d", bytes);
					FD_CLR(err, save_readfds);
					break;

				case 0:
					FD_CLR(err, save_readfds);
					break;

				default:
					write(STDERR_FILENO, buf, bytes);
					break;
			}
		}
	}

	if (coproc_close(pid, &in, &out, &err) == -1)
		fatalsys("coproc_close failed");

	return EXIT_SUCCESS;
}
