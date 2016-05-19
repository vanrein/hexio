/* llcio -- use a connection over LLC to access a remote stream
 *
 * From: Rick van Rein <rick@openfortress.nl>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <net/if_arp.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <linux/llc.h>

// TODO: What includes to use?
#ifndef PF_LLC
#  define PF_LLC 26
#endif

/*
 * LLC is a protocol that builds directly on top of the
 * ethernet MAC layer.  It can be connection-oriented or
 * connectionless, the latter with or without acknowledge.
 *
 * Given that it does not run on top of IPv4 or IPv6, it
 * will not route globally.  This is a disadvantage for
 * most uses, but it actually is an advantage to others.
 * As an example use of LLC, think of console I/O with a
 * networked device that works as soon as the networking
 * hardware is setup -- specifically, there is no need
 * for DHCP or any other protocol to obtain a higher level
 * of existence.  This is the kind of application that
 * this tool was built for, so it is connection-oriented.
 *
 * Linux supports LLC, and that is the basis of this tool.
 * A simple use of it would be to connect two instances
 * of this program by telling each what the MAC address and
 * SAP (service access point, a bit like ports in UDP/TCP)
 * of the other end is -- and end up with a chat tool that
 * runs over plain Ethernet.
 *
 * The same mechanism is a good replacement for serial
 * consoles on embedded devices -- a network makes it
 * easier to approach such a console.
 */


void parsemac (unsigned char mac [6], char *mactxt) {
	int todo = 6;
	long val;
	char *here = mactxt;
	unsigned char *mactodo = mac;
	while (todo-- > 0) {
		val = strtol (here, &here, 16);
		if ((val < 0x00) || (val > 0xff)) {
			fprintf (stderr, "MAC address contains bad byte: 0x%lx\n", val);
			exit (1);
		}
		*mactodo++ = val;
		if (*here++ != (todo? ':': '\0')) {
			fprintf (stderr, "MAC address not properly formatted: %s\n", mactxt);
			exit (1);
		}
	}
}


void parsesap (unsigned char *sap, char *saptxt) {
	char *endptr;
	long sapval = strtol (saptxt, &endptr, 10);
	if (*endptr != '\0') {
		fprintf (stderr, "Failure parsing SAP: %s\n", saptxt);
		exit (1);
	}
	if ((sapval < 0) || (sapval > 127)) {
		fprintf (stderr, "SAP value must be 7 bit, not %s\n", saptxt);
		exit (1);
	}
	if ((sapval & 0x03) || (sapval <= 0x03)) {
		fprintf (stderr, "SAP value is a reserved value: %s\n", saptxt);
		exit (1);
	}
	*sap = sapval;
}


int main (int argc, char *argv []) {
	/* Process parameters */
	if ((argc != 3) && (argc != 5)) {
		fprintf (stderr, "Usage: %s local_MAC local_SAP [remote_MAC remote_SAP]\n", argv [0]);
		exit (1);
	}
	/* Acquire LLC socket */
	int sox = socket (PF_LLC, SOCK_STREAM, 0);
	if (sox < 0) {
		perror ("Failed to acquire LLC socket");
		exit (1);
	}
	/* Bind to a local address */
	struct sockaddr_llc local, remot;
	bzero (&local, sizeof (local));
	bzero (&remot, sizeof (remot));
	local.sllc_family = remot.sllc_family = PF_LLC;
	local.sllc_arphrd = remot.sllc_arphrd = ARPHRD_ETHER;
	parsemac ( local.sllc_mac, argv [1]);
	parsesap (&local.sllc_sap, argv [2]);
	if (bind (sox, (struct sockaddr *) &local, sizeof (local)) == -1) {
		perror ("Failed to bind LLC address");
		exit (1);
	}
	/* Choose between client and server role */
	if (argc == 5) {
		parsemac ( remot.sllc_mac, argv [3]);
		parsesap (&remot.sllc_sap, argv [4]);
		if (connect (sox, (struct sockaddr *) &remot, sizeof (remot)) == -1) {
			perror ("Failed to connect to LLC peer");
			exit (1);
		}
	} else {
		if (listen (sox, 5) == -1) {
			perror ("Failed to listen for LLC peers");
			exit (1);
		}
		socklen_t remotlen = sizeof (remot);
		sox = accept (sox, (struct sockaddr *) &remot, &remotlen);
		if (sox == -1) {
			perror ("Failed to accept LLC");
			exit (1);
		}
	}
	printf ("Connected\n");
	int exitcode = 0;
	while (1) {
		fd_set inout;
		FD_ZERO (&inout);
		FD_SET (0, &inout);
		FD_SET (sox, &inout);
		if (select (sox + 1, &inout, NULL, NULL, NULL) == -1) {
			perror ("Failed while waiting for input");
			exitcode = 1;
			break;
		}
		char buf [80];
		int buflen;
		if (FD_ISSET (0, &inout)) {
			buflen = read (0, buf, 80);
			if (buflen > 0) {
				write (sox, buf, buflen);
			} else {
				if (buflen < 0) {
					exitcode = 1;
					perror ("Failed to read from stdin");
				}
				break;
			}
		}
		if (FD_ISSET (sox, &inout)) {
			buflen = read (sox, buf, 80);
			if (buflen > 0) {
				write (1, buf, buflen);
			} else {
				if (buflen < 0) {
					exitcode = 1;
					perror ("Failed to read LLC data");
				}
				break;
			}
		}
	}
	/* Cleanup */
	close (sox);
	exit (exitcode);
}

