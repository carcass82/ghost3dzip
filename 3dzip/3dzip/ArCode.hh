/*
    Copyright (C) 2009-2011 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of lib3dzip.

    lib3dzip is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    lib3dzip is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with lib3dzip.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef AR_CODE_HH
#define AR_CODE_HH

#include <climits>
#include <boost/cstdint.hpp>
#include "BitStream.hh"

namespace ArCode {

#ifndef ARCODE32
typedef uint64_t UINT_T;
#else
typedef uint32_t UINT_T;
#endif

static const UINT_T SIZE = sizeof(UINT_T) * CHAR_BIT;
static const UINT_T MSB = (UINT_T)1 << (SIZE - 1);
static const UINT_T FIRST_BIT = (UINT_T)1 << (SIZE - 2);
static const UINT_T SECOND_BIT = (UINT_T)1 << (SIZE - 3);

class Encoder
{
    Bit::OStream obs;
    UINT_T low, high;
    size_t underflow;

    void range(const unsigned low_count, const unsigned high_count, const unsigned scale);
    void flush();

public:
    Encoder(std::ostream& os);
    ~Encoder() {
	flush();
    }
    void start(char c, int l) {}
    void encode_freq(unsigned sy_f, unsigned lt_f, unsigned tot_f) {
	range(lt_f, lt_f + sy_f, tot_f);
    }
    void encode_shift(unsigned sy_f, unsigned lt_f, unsigned shift) {
	encode_freq(sy_f, lt_f, (1 << shift));
    }
    void done() {
	flush();
    }
};

class Decoder
{
    Bit::IStream ibs;
    UINT_T low, high, code;

    unsigned getCount(const unsigned range) const;
    void range(const unsigned low_count, const unsigned high_count, const unsigned scale);

public:
    Decoder(std::istream& is);
    void start() {}
    UINT_T decode_culfreq(unsigned tot_f) const {
	return getCount(tot_f);
    }
    UINT_T decode_culshift(unsigned shift) const {
	return decode_culfreq(1 << shift);
    }
    void update(unsigned sy_f, unsigned lt_f, unsigned tot_f) {
	range(lt_f, lt_f + sy_f, tot_f);
    }
    void update_shift(unsigned sy_f, unsigned lt_f, unsigned shift) {
	update(sy_f, lt_f, 1 << shift);
    }
    void done() {}
};

}

#endif
