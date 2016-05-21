/*
  rangecod.c     range encoding

  (c) Michael Schindler
  1997, 1998, 1999, 2000
  http://www.compressconsult.com/
  michael@compressconsult.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.  It may be that this
  program violates local patents in your country, however it is
  belived (NO WARRANTY!) to be patent-free. Glen Langdon also
  confirmed my poinion that IBM UK did not protect that method.


  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA.

  Range encoding is based on an article by G.N.N. Martin, submitted
  March 1979 and presented on the Video & Data Recording Conference,
  Southampton, July 24-27, 1979. If anyone can name the original
  copyright holder of that article please contact me; this might
  allow me to make that article available on the net for general
  public.

  Range coding is closely related to arithmetic coding, except that
  it does renormalisation in larger units than bits and is thus
  faster. An earlier version of this code was distributed as byte
  oriented arithmetic coding, but then I had no knowledge of Martin's
  paper from 1997.

  The input and output is done by the inbyte and outbyte macros
  defined in the .c file; change them as needed; the first parameter
  passed to them is a pointer to the rangecoder structure; extend that
  structure as needed (and don't forget to initialize the values in
  start_encoding resp. start_decoding). This distribution writes to
  stdout and reads from stdin.

  There are no global or static var's, so if the IO is thread save the
  whole rangecoder is - unless GLOBALRANGECODER in rangecod.h is defined.

  For error recovery the last 3 bytes written contain the total number
  of bytes written since starting the encoder. This can be used to
  locate the beginning of a block if you have only the end. The header
  size you pass to initrangecoder is included in that count.

  There is a supplementary file called renorm95.c available at the
  website (www.compressconsult.com/rangecoder/) that changes the range
  coder to an arithmetic coder for speed comparisons.

  define RENORM95 if you want the arithmetic coder type renormalisation.
  Requires renorm95.c
  Note that the old version does not write out the bytes since init.
  you should not define GLOBALRANGECODER then. This Flag is provided
  only for spped comparisons between both renormalizations, see my
  data compression conference article 1998 for details.
*/
/* #define RENORM95 */

/*
  define NOWARN if you do not expect more than 2^32 outstanding bytes
  since I recommend restarting the coder in intervals of less than
  2^23 symbols for error tolerance this is not expected
*/
#define NOWARN

/*
  define EXTRAFAST for increased speed; you loose compression and
  compatibility in exchange.
*/
/* #define EXTRAFAST */

#include "ArCode/RangeCod.hh"

/* SIZE OF RANGE ENCODING CODE VALUES. */

#define CODE_BITS 32
#define Top_value ((code_value)1 << (CODE_BITS-1))


/* all IO is done by these macros - change them if you want to */
/* no checking is done - do it here if you want it             */
/* cod is a pointer to the used rangecoder                     */

#define SHIFT_BITS (CODE_BITS - 9)
#define EXTRA_BITS ((CODE_BITS-2) % 8 + 1)
#define Bottom_value (Top_value >> 8)

#include <iostream>
#define M_outbyte(a) out.put(a)
#define M_inbyte in.get()


/* rc is the range coder to be used                            */
/* c is written as first byte in the datastream                */
/* one could do without c, but then you have an additional if  */
/* per outputbyte.                                             */
void RangeEnc::start(char c, int initlength)
{
	low = 0;                /* Full code range */
	range = Top_value;
	buffer = c;
	help = 0;               /* No bytes to follow */
	bytecount = initlength;
}


/* Encode a symbol using frequencies                         */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
/* or (faster): tot_f = (code_value)1<<shift                             */
void RangeEnc::encode_freq(freq sy_f, freq lt_f, freq tot_f)
{	code_value r, tmp;
	enc_normalize();
	r = range / tot_f;
	tmp = r * lt_f;
	low += tmp;
#ifdef EXTRAFAST
	range = r * sy_f;
#else
	if (lt_f+sy_f < tot_f)
		range = r * sy_f;
	else
		range -= tmp;
#endif
}

/* I do the normalization before I need a defined state instead of */
/* after messing it up. This simplifies starting and ending.       */
inline void RangeEnc::enc_normalize()
{   while(range <= Bottom_value)     /* do we need renormalisation?  */
	{   if (low < (code_value)0xff<<SHIFT_BITS)  /* no carry possible --> output */
		{   M_outbyte(buffer);
			for(; help; help--)
				M_outbyte(0xff);
			buffer = (unsigned char)(low >> SHIFT_BITS);
		} else if (low & Top_value) /* carry now, no future carry */
		{   M_outbyte(buffer+1);
			for(; help; help--)
				M_outbyte(0);
			buffer = (unsigned char)(low >> SHIFT_BITS);
		} else                           /* passes on a potential carry */
#ifdef NOWARN
			help++;
#else
			if (bytestofollow++ == 0xffffffffL)
			{   fprintf(stderr,"Too many bytes outstanding - File too large\n");
				exit(1);
			}
#endif
		range <<= 8;
		low = (low<<8) & (Top_value-1);
		bytecount++;
	}
}

void RangeEnc::encode_shift(freq sy_f, freq lt_f, freq shift)
{	code_value r, tmp;
	enc_normalize();
	r = range >> shift;
	tmp = r * lt_f;
	low += tmp;
#ifdef EXTRAFAST
	range = r * sy_f;
#else
	if ((lt_f+sy_f) >> shift)
		range -= tmp;
	else
		range = r * sy_f;
#endif
}


#ifndef RENORM95
/* Finish encoding                                           */
/* rc is the range coder to be used                          */
/* actually not that many bytes need to be output, but who   */
/* cares. I output them because decode will read them :)     */
/* the return value is the number of bytes written           */
uint_4 RangeEnc::done()
{
	uint tmp;
	enc_normalize();     /* now we have a normalized state */
	bytecount += 5;
	if ((low & (Bottom_value - 1)) < ((bytecount & 0xffffffL) >> 1))
	   tmp = low >> SHIFT_BITS;
	else
	   tmp = (low >> SHIFT_BITS) + 1;
	if (tmp > 0xff) { /* we have a carry */
	M_outbyte(buffer+1);
		for(; help; help--)
			M_outbyte(0);
	} else { /* no carry */
	M_outbyte(buffer);
		for(; help; help--)
			M_outbyte(0xff);
	}
	M_outbyte(tmp & 0xff);
	M_outbyte((bytecount>>16) & 0xff);
	M_outbyte((bytecount>>8) & 0xff);
	M_outbyte(bytecount & 0xff);
	return bytecount;
}


/* Start the decoder                                         */
/* rc is the range coder to be used                          */
/* returns the char from start_encoding or EOF               */
int RangeDec::start()
{
	int c = M_inbyte;
	if (!in.good())
		return -1;
	buffer = M_inbyte;
	low = buffer >> (8 - EXTRA_BITS);
	range = (code_value) 1 << EXTRA_BITS;
	return c;
}


inline void RangeDec::dec_normalize()
{
	while (range <= Bottom_value) {
	low = (low << 8) | ((buffer << EXTRA_BITS) & 0xff);
		buffer = M_inbyte;
		low |= buffer >> (8 - EXTRA_BITS);
		range <<= 8;
	}
}
#endif


/* Calculate culmulative frequency for next symbol. Does NO update!*/
/* rc is the range coder to be used                          */
/* tot_f is the total frequency                              */
/* or: totf is (code_value)1<<shift                                      */
/* returns the culmulative frequency                         */
freq RangeDec::decode_culfreq(freq tot_f)
{   freq tmp;
	dec_normalize();
	help = range / tot_f;
	tmp = low / help;
#ifdef EXTRAFAST
	return tmp;
#else
	return (tmp >= tot_f ? tot_f - 1 : tmp);
#endif
}

freq RangeDec::decode_culshift(freq shift)
{   freq tmp;
	dec_normalize();
	help = range>>shift;
	tmp = low/help;
#ifdef EXTRAFAST
	return tmp;
#else
	return (tmp >> shift ? ((code_value) 1 << shift) - 1 : tmp);
#endif
}


/* Update decoding state                                     */
/* rc is the range coder to be used                          */
/* sy_f is the interval length (frequency of the symbol)     */
/* lt_f is the lower end (frequency sum of < symbols)        */
/* tot_f is the total interval length (total frequency sum)  */
void RangeDec::update(freq sy_f, freq lt_f, freq tot_f)
{
	code_value tmp;
	tmp = help * lt_f;
	low -= tmp;
#ifdef EXTRAFAST
	range = help * sy_f;
#else
	if (lt_f + sy_f < tot_f)
		range = help * sy_f;
	else
		range -= tmp;
#endif
}


/* Decode a byte/short without modelling                     */
/* rc is the range coder to be used                          */
unsigned char RangeDec::decode_byte()
{
	unsigned char tmp = decode_culshift(8);
	update(1, tmp, (freq) 1 << 8);
	return tmp;
}

unsigned short RangeDec::decode_short()
{   unsigned short tmp = decode_culshift(16);
	update(1, tmp, (freq) 1 << 16);
	return tmp;
}


/* Finish decoding                                           */
/* rc is the range coder to be used                          */
void RangeDec::done()
{
	dec_normalize();      /* normalize to use up all bytes */
}
