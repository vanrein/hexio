/* mbusio.c -- input/output through Modbus TCP */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/select.h>

#include "socket.h"


int get16 (char *msg) {
	return ((int) msg [0]) | (((int) msg [1]) << 8);
}

void set16 (char *msg, int val) {
	msg [0] =  val       & 0x00ff;
	msg [1] = (val >> 8) & 0x00ff;
}


int main (int argc, char *argv []) {
	fd_set sel;
	int busy=1;
	if ((argc < 1) || (argc > 3)) {
		fprintf (stderr, "Usage: %s [addr [port]]\n   where addr defaults to 127.0.0.1 and port defaults to 502\nSend and receive slave+PDU over Modbus TCP, so each message is formatted\nSLAVE/8,FUNCTION/8,DATA/n\nThe TCP headering is handled by this utility\n", argv [0]);
		exit (1);
	}
	char *addr = "127.0.0.1";
	if (argc >= 2) {
		addr = argv [1];
	}
	char *port = "502";
	if (argc >= 3) {
		port = argv [2];
	}
	struct sockaddr_storage ss;
	if (!socket_parse (addr, port, (struct sockaddr *) &ss)) {
		perror ("Cannot parse address and/or port");
		exit (1);
	}
	int sox;
	if (!socket_client ((struct sockaddr *) &ss, SOCK_STREAM, &sox)) {
		perror ("Cannot connect to Modbus TCP");
		exit (1);
	}
	int txnid_send = 1;
	int txnid_recv = 1;
	while (busy) {
		FD_ZERO (&sel);
		FD_SET (sox, &sel);
		FD_SET (0,  &sel);
		if (select (sox+1, &sel, NULL, NULL, NULL) < 0) {
			perror ("Select failed");
			busy = 0;
		} else {
			if (FD_ISSET (sox, &sel)) {
				char buf [6+256];
				int len = read (sox, buf, 6+256);
				if (len < 0) {
					perror ("Error reading");
					busy = 0;
				} else if (len < 7) {
					fprintf (stderr, "MBAP header too short\n");
					busy = 0;
				} else if (get16 (buf+0) != txnid_recv % 65536) {
					fprintf (stderr, "MBAP txnid is bad\n");
					busy = 0;
				} else if (get16 (buf+2) != 0) {
					fprintf (stderr, "MBAP protoid is bad\n");
					busy = 0;
				} else if (get16 (buf+4) != len - 7) {
					fprintf (stderr, "MBAP length is bad\n");
					busy = 0;
				} else if (txnid_recv >= txnid_send) {
					fprintf (stderr, "Reply without Query\n");
					busy = 0;
				} else {
					if (write (1, buf+6, len-6) < len-6) {
						perror ("Partial write");
						busy = 0;
					}
					txnid_recv++;
				}
			}
			if (FD_ISSET (0, &sel)) {
				char buf [6+256];
				int len = read (0, buf+6, 256);
				if (len < 0) {
					perror ("Error on stdin");
					busy = 0;
				} else {
					set16 (buf+0, txnid_send % 65536);
					set16 (buf+2, 0);
					set16 (buf+4, len);
					if (write (sox, buf, 6+len) < 6+len) {
						perror ("Partial output");
						busy = 0;
					}
					txnid_send++;
				}
			}
		}
	}
	close (sox);
	return 0;
}
