all: inputter outputter devio llcio pcscio

PCSCFLAGS=-D_THREAD_SAFE -I/usr/local/include/PCSC/ -L/usr/local/lib -L/usr/local/lib/pth -ggdb3 -pthread

inputter: inputter.c
	$(CC) $(CFLAGS) -o inputter inputter.c

outputter: outputter.c
	$(CC) $(CFLAGS) -o outputter outputter.c

devio: devio.c
	$(CC) $(CFLAGS) -o devio devio.c

llcio: llcio.c
	$(CC) $(CFLAGS) -o llcio llcio.c

pcscio: pcscio.c
	$(CC) $(CFLAGS) $(PCSCFLAGS) -o pcscio pcscio.c -lpcsclite

clean:

very:
	rm inputter outputter

veryclean: very clean

anew: very clean all

