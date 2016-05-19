/* devio.c -- input/output through a file (char device, socket, ...) */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/select.h>


int main (int argc, char *argv []) {
	int fd;
	fd_set sel;
	int busy=1;
	if (argc != 2) {
		fprintf (stderr, "Usage: %s /dev/filename\n", argv [0]);
		exit (1);
	}
	if ((fd = open (argv [1], O_RDWR)) < 0) {
		perror ("Failed to open device");
		exit (1);
	}
	while (busy) {
		FD_ZERO (&sel);
		FD_SET (fd, &sel);
		FD_SET (0,  &sel);
		if (select (fd+1, &sel, NULL, NULL, NULL) < 0) {
			perror ("Select failed");
			busy = 0;
		} else {
			if (FD_ISSET (fd, &sel)) {
				char buf [100];
				int len = read (fd, buf, 100);
				if (len < 0) {
					perror ("Error reading");
					busy = 0;
				} else {
					if (write (1, buf, len) < len) {
						perror ("Partial write");
						busy = 0;
					}
				}
			}
			if (FD_ISSET (0, &sel)) {
				char buf [100];
				int len = read (0, buf, 100);
				if (len < 0) {
					perror ("Error on stdin");
					busy = 0;
				} else {
					if (write (fd, buf, len) < len) {
						perror ("Partial output");
						busy = 0;
					}
				}
			}
		}
		
	}
	close (fd);
	return 0;
}
