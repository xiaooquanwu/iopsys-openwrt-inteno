#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <config.h>
#include <getopt.h>
#include "log.h"

int debug_level = 0;

void print_usage(char *prg_name) {
        printf("Usage: %s -h -f\n", prg_name);
        printf("  Options: \n");
        printf("      -f, --foreground\tDon't fork off as a daemon.\n");
        printf("      -d, --debug=NUM\tSet debug level. Higher = more output\n");
        printf("      -h\t\tShow this help screen.\n");
        printf("\n");
}

int main(int argc, char **argv)
{
	int ch;
	int daemon = 1;
	pid_t pid, sid;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
                        {"foreground", no_argument, 0, 'f'},
                        {"verbose",    no_argument, 0, 'v'},
                        {"debug",required_argument, 0, 'd'},
                        {0, 0, 0, 0}
                };

                ch = getopt_long(argc, argv, "vfhd:",
                                long_options, &option_index);

		if (ch == -1)
                        break;

                switch (ch) {
                case 'f':
                        daemon = 0;
                        break;
                case 'd':
                        debug_level = strtol(optarg, 0, 0);
                        break;

		case 'h':
                default:
			print_usage(argv[0]);
			exit(0);
                }
 	}

	if (optind < argc) {
		printf("Extra arguments discarded: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
	}


	/* Setup logging */
	if (daemon) {
		setlogmask(LOG_UPTO(LOG_INFO));
		openlog(PACKAGE, LOG_CONS, LOG_USER);

		syslog(LOG_INFO, "%s daemon starting up", PACKAGE);
	} else {
		setlogmask(LOG_UPTO(LOG_DEBUG));
		openlog(PACKAGE, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);

		syslog(LOG_INFO, "%s program starting up", PACKAGE);
	}

	/* daemonize */
	if (daemon) {
		syslog(LOG_INFO, "starting the daemonizing process");

		/* Fork off the parent process */
		pid = fork();
		if (pid < 0) {
			exit(EXIT_FAILURE);
		}
		/* If we got a good PID, then
		   we can exit the parent process. */
		if (pid > 0) {
			exit(EXIT_SUCCESS);
		}

		/* Change the file mode mask */
		umask(0);

		/* Create a new SID for the child process */
		sid = setsid();
		if (sid < 0) {
			/* Log the failure */
			exit(EXIT_FAILURE);
		}

		/* Change the current working directory */
		if ((chdir("/")) < 0) {
			/* Log the failure */
			exit(EXIT_FAILURE);
		}
		/* Close out the standard file descriptors */
		//close(STDIN_FILENO);
		//close(STDOUT_FILENO);
		//close(STDERR_FILENO);
	}

	DBG(1,"testing\n");
	syslog(LOG_INFO, "%s exiting", PACKAGE);
	exit(0);

	return 0;
}
