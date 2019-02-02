#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define BYTES_PER_LINE 35

enum status { normal, endline, error, endfile };
enum status status = normal;

void ch2hex (unsigned char *output, int ch) {
	if ((ch >= '0') && (ch <= '9')) {
		*output <<= 4;
		*output += ch - '0';
	} else {
		ch = toupper (ch);
		if ((ch < 'A') || (ch > 'F')) {
			fprintf (stderr, "Illegal char '%c'\n", ch);
			status = error;
		} else {
			*output <<= 4;
			*output += ch - 'A' + 10;
		}
	}
}

int getbyte (unsigned char *output) {
	int ch;

	*output = 0;

	if (status == error) {
		return 0;
	}

	do {
		if ((ch = getchar ()) < 0) {
			status = endfile;
			return 0;
		}
		if (ch == '\n') {
			status = endline;
			return 0;
		}
	} while ((ch == ' ') || (ch == ':'));

	if (status == normal) {
		ch2hex (output, ch);
		ch = getchar ();
		ch2hex (output, ch);
	}

	return (status == normal);
}

int main (int argc, char *argv []) {
	size_t offset = 0;
	size_t len;
	unsigned char buf [BYTES_PER_LINE];
	char ch;
	int prompt;

	prompt = isatty (0);
	while (status != endfile) {
		if (status == endline) {
			status = normal;
		}
		if (prompt) {
			usleep (1000000L); // Yield to others -- better prompt printing
			fprintf (stderr, "%08lx>", offset);
			fflush (stderr);
		}
		len = 0;
		while ((len < BYTES_PER_LINE) && (status == normal)) {
			if (getbyte (&buf [len])) {
				len++;
				offset++;
			}
		}
		write (1, buf, len);
		if (status == error) {
			while (ch = getchar (), (ch >=0) && (ch!='\n')) {
				;
			}
			fprintf (stderr, "*** skipped remainder of line ***\n");
			status = normal;
		}
	}
	return 0;
}
