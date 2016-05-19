# Package Description: hexio

This is a very simple package of developer utilities.  They are targeted
at binary/hexadecimal interaction with low-down parts of a system, so
ideal for various reverse engineering tasks.  In spite of their utter
simplicity, they can be true life savers.

  * **hexin** reads hexadecimal input on `stdin` and outputs it in binary
    on `stdout`; it will prompt with an address (or input offset) printed
    in hexadecimal on the tty
  * **hexout** reads binary input on `stdin` and outputs it in hexadecimal
    form on `stdout`
  * **devio** reads from `stdin` and passes what it finds to a given device;
    anything returned by the device is passed on to `stdout`
  * **llcio** reads from `stdin` and passes what it finds to a networked
    device over LLC; anything returned over that LLC channel is passed on
    to `stdout`
  * **pcscio** uses the PC/SC smart card interface to pass what it finds
    on `stdin` to a smart card as PDU codes; anything returned by that
    smart card is passed on to `stdout`
  * **derdump** parses DER structures (distinguished ASN.1 encoding) from
    a file and shows the structure in great detail on `stdout`.  Special
    about this utility is that it will work hard to print input that is
    full of errors; the utility will complain loudly, but continue instead
    of failing fatally; this makes it a very useful developer tool.

