/* pcscio.c -- input/output through PCSClite */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/select.h>

#include <pthread.h>
#include <winscard.h>


/*
int             pthread_detach __P((pthread_t pth)) {
	fprintf (stderr, "ERROR: pthread_detach called\n");
}
*/


int main (int argc, char *argv []) {
	int busy=1;
	SCARDCONTEXT ctx;
	SCARDHANDLE card;
	LONG err;
	DWORD actproto;

	if (SCardEstablishContext (SCARD_SCOPE_SYSTEM, NULL, NULL, &ctx) != SCARD_S_SUCCESS) {
		fprintf (stderr, "Failed to contact pcscd\n");
		exit (1);
	}
	if (argc != 2) {
		DWORD listsz=0;
		LPTSTR readers=NULL, reader=NULL;
		fprintf (stderr, "Usage: %s readername\n", argv [0]);
		if ((SCardListReaders (ctx, NULL, NULL, &listsz) == SCARD_S_SUCCESS)
		 && (readers=malloc (listsz))
		 && (SCardListReaders (ctx, NULL, readers, &listsz) == SCARD_S_SUCCESS)) {
			fprintf (stderr, "Currently attached readers:\n");
			reader = readers;
			while (*reader) {
				fprintf (stderr, " - %s\n", reader);
				reader = reader + strlen (reader);
				reader++;
			}
		}
		if (readers) {
			free (readers);
		}
		SCardReleaseContext (ctx);
		exit (1);
	}
	err = SCardConnect (ctx, argv [1], SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T1, &card, &actproto);
	if (err != SCARD_S_SUCCESS) {
		fprintf (stderr, "Failed to access the smart card\n");
		busy = 0;
	}
	fprintf (stderr, "Yippy!\n");
	while (busy) {
		BYTE txbuf [275], rxbuf [275];
		DWORD rxlen;
		SCARD_IO_REQUEST req;
		int txlen = read (0, txbuf, sizeof (txbuf));
		if (txlen < 0) {
			perror ("Error on stdin");
			busy = 0;
		} else if (txlen == 0) {
			/* EOF encountered */
			busy = 0;
		} else {
			req.dwProtocol = actproto;
			req.cbPciLength = sizeof (req);
			rxlen = sizeof (rxbuf);
			err = SCardTransmit (card, &req, txbuf, txlen, &req, rxbuf, &rxlen);
			if (err != SCARD_S_SUCCESS) {
				fprintf (stderr, "Error writing to smart card\n");
				busy = 0;
			} else {
				if (write (1, txbuf, rxlen) < rxlen) {
					perror ("Partial output");
					busy = 0;
				}
			}
		}
	}
	SCardReleaseContext (ctx);
}
