#pragma once
#include <iosfwd>
#include "port.h"

typedef uint_4 code_value;
// Type of an rangecode value must accomodate 32 bits
// it is highly recommended that the total frequency count is less
// than 1 << 19 to minimize rounding effects.
// the total frequency count MUST be less than 1<<23

typedef uint_4 freq;

// make the following private in the arithcoder object in C++

class RangeCod {
protected:
	uint_4 low, range, help;
	unsigned char buffer;
	uint_4 bytecount;
};

class RangeEnc: RangeCod
{
	std::ostream& out;

	void enc_normalize();

public:

	RangeEnc(std::ostream& os):
	out(os)
	{}
	void start(char c, int initlength);


	// Encode a symbol using frequencies
	// rc is the range coder to be used
	// sy_f is the interval length (frequency of the symbol)
	// lt_f is the lower end (frequency sum of < symbols)
	// tot_f is the total interval length (total frequency sum)
	// or (a lot faster): tot_f = 1<<shift
	void encode_freq(freq sy_f, freq lt_f, freq tot_f );
	void encode_shift(freq sy_f, freq lt_f, freq shift );

	// Encode a byte/short without modelling
	// rc is the range coder to be used
	// b,s is the data to be encoded
	void encode_byte(unsigned char b)
	{
	encode_shift((freq)1, (freq)b, (freq)8);
	}

	void encode_short(unsigned short s)
	{
	encode_shift((freq)1, (freq)s, (freq)16);
	}

	// Finish encoding
	// rc is the range coder to be shut down
	// returns number of bytes written
	uint_4 done();
};

class RangeDec: RangeCod
{
	std::istream& in;

	void dec_normalize();

public:
	RangeDec(std::istream& in):
	in(in)
	{}

	// Start the decoder
	// rc is the range coder to be used
	// returns the char from start_encoding or EOF
	int start();

	// Calculate culmulative frequency for next symbol. Does NO update!
	// rc is the range coder to be used
	// tot_f is the total frequency
	// or: totf is 1<<shift
	// returns the <= culmulative frequency
	freq decode_culfreq(freq tot_f );
	freq decode_culshift(freq shift );

	// Update decoding state
	// rc is the range coder to be used
	// sy_f is the interval length (frequency of the symbol)
	// lt_f is the lower end (frequency sum of < symbols)
	// tot_f is the total interval length (total frequency sum)
	void update(freq sy_f, freq lt_f, freq tot_f);
	void update_shift(freq sy_f, freq lt_f, freq shift)
	{
	update(sy_f, lt_f, (freq) 1 << shift);
	}

	// Decode a byte/short without modelling
	// rc is the range coder to be used
	unsigned char decode_byte();
	unsigned short decode_short();

	// Finish decoding
	// rc is the range coder to be used
	void done();
};
