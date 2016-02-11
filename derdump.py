#!/usr/bin/python

import sys

if len (sys.argv) != 2:
	print 'Usage: ' + sys.argv [0] + 'file.der'
	sys.exit (1)

der = open (sys.argv [1], 'r').read (65537)
ofs = 0

def eof ():
	global ofs, der
	return ofs >= len (der)

def read1 ():
	global ofs, der
	if eof ():
		print 'ATTEMPTED READ BEYOND EOF (RETURNING 0x00)'
		return 0
	else:
		ofs = ofs + 1
		return ord (der [ofs-1])

nesting = []

class2str = {
	0: 'Universal',
	1: 'Application',
	2: 'Contextual',
	3: 'Private'
}

pc2str = {
	0: 'Primitive',
	1: 'Constructed'
}

universal2str = {
	0: 'End-Of-Content',
	1: 'BOOLEAN',
	2: 'INTEGER',
	3: 'BITSTRING',
	4: 'OCTETSTRING',
	5: 'NULL',
	6: 'OID',
	7: 'Object-Descriptor',
	8: 'EXTERNAL',
	9: 'REAL',
	10: 'ENUMERATED',
	11: 'EMBEDDED PDV',
	12: 'UTF8String',
	13: 'RELATIVE-OID',
	14: '*****',
	15: '*****',
	16: 'SEQUENCE (OF)',
	17: 'SET (OF)',
	18: 'NumericString',
	19: 'PrintableString',
	20: 'T61String',
	21: 'VideotexString',
	22: 'IA5String',
	23: 'UTCTime',
	24: 'GeneralizedTime',
	25: 'GraphicString',
	26: 'VisibleString',
	27: 'GeneralString',
	28: 'UniversalString',
	29: 'CHARACTER STRING',
	30: 'BMPString',
	31: '*****'
}

while not eof ():

	while nesting != [] and ofs >= nesting [-1]:
		if ofs > nesting [-1]:
			print 'READ OFFSET %d EXCEEDS ENCAPSULATION %d (RETURNING)' % (ofs, nesting [-1])
		ofs = nesting.pop ()

	tag = read1 ()
	tag_class = (tag & 0xc0) >> 6
	tag_pc = (tag & 0x20) != 0
	tag_num = tag & 0x1f

	lenlen = read1 ()
	if lenlen & 0x80 == 0:
		leng = lenlen
		lenlen = 1
	else:
		lenlen = lenlen - 0x80 + 1
		leng = 0
		i = 1
		while i < lenlen:
			leng <<= 8
			leng = leng + read1 ()
			i = i + 1

	if tag_class == 0:
		meaning = universal2str [tag_num]
	elif tag_class == 1:
		meaning = '[APPLICATION ' + str (tag_num) + ']'
	elif tag_class == 2:
		meaning = '[' + str (tag_num) + ']'
	else:
		meaning = '[PRIVATE ' + str (tag_num) + ']'

	print '%s%s: tag 0x%02x %s%d @%d ^%d, %s, %s' % (
			'  ' * len (nesting),
			meaning, tag,
			'#' * lenlen, leng,
			ofs - lenlen - 1,
			len (nesting),
			class2str [tag_class],
			pc2str [tag_pc] )

	if tag_pc == 0 and leng > 0:
		print '  ' * ( len (nesting) + 1 ),
		cstr = '"'
		ival = 0
		ostr = ''
		oval = None
		while leng > 0:
			ch = read1 ()
			print '%02x' % ch,
			if 32 <= ch < 127:
				cstr = cstr + chr (ch)
			else:
				cstr = cstr + '.'
			ival = (ival << 8) | ch
			if oval is None:
				ostr = str (ch / 40) + '.' + str (ch % 40)
				oval = 0
			else:
				oval = (oval << 7) | (ch & 0x7f)
				if ch & 0x80 == 0:
					ostr = ostr + '.' + str (oval)
					oval = 0
			leng = leng - 1
		cstr = cstr + '"'
		if tag == 0x06:
			cstr = ostr
		elif tag == 0x02:
			cstr = str (ival)
		print '==', cstr

	if tag_pc != 0:
		# print 'Now at', ofs, 'adding', leng, 'pushing', ofs + leng
		nesting.append (ofs + leng)

while nesting != []:
	if ofs != nesting [-1]:
		print 'NESTING NOT ENDED CORRECTLY, OFFSET IS %d INSTEAD OF %d (CONTINUING)' % (ofs, nesting [-1])
	nesting.pop ()

