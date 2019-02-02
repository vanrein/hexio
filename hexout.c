#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>


/* Outputter reads input from stdin and prints it in hex on stdout.
 * It will collect lines of one character plus anything that arrives
 * within a second after that first character, limited to a maximum
 * number of characters that just fits on a 80-char wide display.
 */


#define BYTES_PER_LINE 24

unsigned long timersofar = 123;

void tick (int signal) {
	timersofar++;
}

struct itimerval tickival = { { 0, 0 } , { 1, 0 } };
struct itimerval stopival = { { 0, 0 } , { 0, 0 } };

int main (int argc, char *argv []) {
	unsigned char buf [BYTES_PER_LINE];
/*
	unsigned long timer;
*/
	size_t len;
	size_t offset = 0;
	int prompt;
	char comma [80];

	if (signal (SIGALRM, tick) == SIG_ERR) {
		perror ("Failed to install interval handler");
		exit (1);
	}
	system ("stty raw -echo");
	printf ("*** outputter starts -- in hex mode ***\r\n");
	fflush (stdout);
/*
	while (timersofar = timer,
			len = read (0, buf, BYTES_PER_LINE),
			(len > 0) || ( errno == SIGALRM ) ) {
*/
	prompt = isatty (1);
	while (len = read (0, buf, 1), len > 0) {
		size_t i;
		int len2;
		/* Set a timeout before reading more; ignore problems */
		setitimer (ITIMER_REAL, &tickival, NULL);
		len2 = read (0, buf + 1, BYTES_PER_LINE - 1);
		setitimer (ITIMER_REAL, &stopival, NULL);
		if ((len2 == -1) && (errno == EINTR)) {
			len2 = 0;
		}
		if (len2 >= 0) {
			len += len2;
		}
		if (prompt) {
			snprintf (comma, sizeof(comma)-1, "%08lx", offset);
		} else {
			comma [0] = '\0';
		}
		offset += len;
		for (i=0; i<len; i++) {
			printf ("%s%02x", comma, buf [i]);
			comma [0] = ' ';
			comma [1] = '\0';
		}
		printf ("\r\n");
		fflush (stdout);
		if (len2 < 0) {
			len = -1;
			break;
		}
	}
	if (len < 0) {
		perror ("read(2) failed");
	} else {
		printf ("*** outputter ends ***\r\n");
	}
	system ("stty cooked echo");
	return 0;
}
