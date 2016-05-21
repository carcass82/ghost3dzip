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

#include <istream>
#include <ostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include "BitStream.hh"

namespace Bit {

int
StreamBase::is_set (const char *buf, const int pos)
{
    const int ofs_bytes = pos >> 3;
    return buf[ofs_bytes] & 1 << (pos & ((1 << 3) - 1));
}

void
StreamBase::bit_set (char *buf, const int pos)
{
    std::div_t res = std::div(pos, 8);
    buf[res.quot] |= 1 << res.rem;
}

int
IStream::refill()
{
    is.read(buffer, bufferSize);
    if (is.bad() == true)
	throw std::logic_error("Error reading input file");
    offset = 0;
    bits = is.gcount() * 8;
    return bits;
}

int
IStream::read(char *buf, const int size)
{
    int i;
    std::memset(buf, 0, (size + 7) / 8);
    for (i = 0; i < size; ++i, ++offset) {
	if (offset >= bits)
	    if (refill() == 0)
		break;		// break and continue if EOF
	if (is_set(buffer, offset))
	    bit_set(buf, i);
    }
    return i;
}

int
OStream::flush()
{
    if (bits == 0)
	return 0;
    int count = (bits + 7) / 8;
    os.write(buffer, count);
    bits = 0;
    return (os.bad() == true) ? -1 : count;
}

int
OStream::write(const char *buf, const int size)
{
    for (int i = 0; i < size; ++i, ++bits) {
	if (bits >= bufferSize * 8) {
	    if (flush() == -1)
		return 0;	// return an error if bit_flush fails
	    memset(buffer, 0, bufferSize);	// sets all bits 0
	    bits = 0;
	}
	if (is_set(buf, i))	// set only if one
	    bit_set(buffer, bits);
    }
    return size;
}

OStream &operator << (OStream &bs, const int i)
{
    bs.write((char*) &i, sizeof(int) * 8);
    return bs;
}

IStream &operator >> (IStream &bs, int &i)
{
    bs.read((char*) &i, sizeof(int) * 8);
    return bs;
}

OStream &operator << (OStream &bs, const unsigned int i)
{
    bs.write((char*) &i, sizeof(unsigned int) * 8);
    return bs;
}

IStream &operator >> (IStream &bs, unsigned int &i)
{
    bs.read((char*) &i, sizeof(unsigned int) * 8);
    return bs;
}

OStream &operator << (OStream &bs, const short i)
{
    bs.write((char*) &i, sizeof(short) * 8);
    return bs;
}

IStream &operator >> (IStream &bs, short &i)
{
    bs.read((char*) &i, sizeof(short) * 8);
    return bs;
}

OStream &operator << (OStream &bs, const unsigned short i)
{
    bs.write((char*) &i, sizeof(unsigned short) * 8);
    return bs;
}

IStream &operator >> (IStream &bs, unsigned short &i)
{
    bs.read((char*) &i, sizeof(short) * 8);
    return bs;
}

OStream &operator << (OStream &bs, const unsigned char c)
{
    bs.write((char*) &c, sizeof(unsigned char) * 8);
    return bs;
}

IStream &operator >> (IStream &bs, unsigned char &c)
{
    bs.read((char*) &c, sizeof(unsigned char) * 8);
    return bs;
}

}
