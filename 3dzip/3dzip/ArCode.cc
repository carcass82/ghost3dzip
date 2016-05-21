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

#include "ArCode.hh"

namespace ArCode {

Encoder::Encoder(std::ostream& os) :
    obs(os), low(0), high(~MSB), underflow(0)
{
}

void Encoder::range(const unsigned low_count, const unsigned high_count, const unsigned scale)
{
    UINT_T r = (high - low + 1) / scale;
    high = low + r * high_count - 1;
    low += r * low_count;

    for (;;) {
	if ((high & FIRST_BIT) == (low & FIRST_BIT)) {
	    obs.write(high & FIRST_BIT);
	    while (underflow > 0) {
		obs.write(~high & FIRST_BIT);
		--underflow;
	    }
	}
	else if ((low & SECOND_BIT) && !(high & SECOND_BIT)) {
	    ++underflow;
	    low ^= SECOND_BIT;
	    high ^= SECOND_BIT;
	}
	else
	    return;
	low <<= 1;
	high <<= 1;
	high |= 1;
	low &= ~MSB;
	high &= ~MSB;
    }
}

void Encoder::flush()
{
    obs.write(low & SECOND_BIT);
    ++underflow;
    while (underflow-- > 0)
	obs.write(~low & SECOND_BIT);
    obs.flush();
}

Decoder::Decoder(std::istream& is) :
    ibs(is), low(0), high(~MSB), code(0)
{
    for (unsigned i = 0; i < SIZE - 1; ++i) {
	code <<= 1;
	if (ibs.read() == true)
	    ++code;
    }
}

unsigned Decoder::getCount(const unsigned range) const
{
    UINT_T r = (high - low + 1) / range;
    return (code - low) / r;
}

void Decoder::range(const unsigned low_count, const unsigned high_count, const unsigned scale)
{
    UINT_T r = (high - low + 1) / scale;
    high = low + r * high_count - 1;
    low += r * low_count;

    for (;;) {
	if ((high & FIRST_BIT) == (low & FIRST_BIT)) {
	}
	else if ((low & SECOND_BIT) != 0 && (high & SECOND_BIT) == 0) {
	    code ^= SECOND_BIT;
	    low ^= SECOND_BIT;
	    high ^= SECOND_BIT;
	}
	else
	    break;
	low <<= 1;
	high <<= 1;
	high |= 1;
	code <<= 1;
	if (ibs.read() == true)
	    ++code;
	low &= ~MSB;
	high &= ~MSB;
	code &= ~MSB;
    }
}

}
