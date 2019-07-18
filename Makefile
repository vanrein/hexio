PREFIX ?= /usr/local
DESTDIR ?=

TARGETS=hexin hexout devio mbusio
# Dropped llcio, since LLC is not commonly used
# TARGETS += llcio
# Dropped pcscio, as it requires uncommon headers
# TARGETS += pcscio
LITERAL=derdump

all: $(TARGETS)

CC=gcc -Wall

PCSCFLAGS=-D_THREAD_SAFE -I/usr/local/include/PCSC/ -L/usr/local/lib -L/usr/local/lib/pth -ggdb3 -pthread

hexin: hexin.c
	$(CC) $(CFLAGS) -o hexin hexin.c

hexout: hexout.c
	$(CC) $(CFLAGS) -o hexout hexout.c

devio: devio.c
	$(CC) $(CFLAGS) -o devio devio.c

llcio: llcio.c
	$(CC) $(CFLAGS) -o llcio llcio.c

pcscio: pcscio.c
	$(CC) -I /usr/include/PCSC $(CFLAGS) $(PCSCFLAGS) -o pcscio pcscio.c -lpcsclite

mbusio: mbusio.c socket.c
	$(CC) $(CFLAGS) -o mbusio mbusio.c socket.c

install: all
	install $(TARGETS) $(LITERAL) "$(DESTDIR)$(PREFIX)/sbin"

uninstall:
	cd "$(DESTDIR)$(PREFIX)/sbin" ; rm -f $(TARGETS)

clean:
	rm -f $(TARGETS)

very:

veryclean: very clean

anew: very clean all

