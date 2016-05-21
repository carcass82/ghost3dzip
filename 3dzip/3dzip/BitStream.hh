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

#pragma once
#include <iosfwd>

namespace Bit {

class StreamBase
{
    protected:
    static const int bufferSize = 32;
    char buffer[bufferSize];
    int bits;

    static int is_set(const char *buf, const int pos);
    static void bit_set(char *buf, const int pos);

    protected:
    StreamBase() : bits(0) {
	for (int i = 0; i < bufferSize; i++)
	    buffer[i] = 0;
    }
};

class IStream : public StreamBase
{
    std::istream &is;
    int offset;

    int refill();
    public:
    IStream(std::istream &is) : is(is), offset(0) {}
    bool read() {
	char c = 0;
	read(&c, 1);
	return c != 0;
    }    
    int read(char *buf, const int size);
};

class OStream : public StreamBase
{
    std::ostream &os;
    public:
    OStream(std::ostream &os) : os(os) {}
    int write(const bool bit) {
	char byte = (bit == true ? 0xff : 0x00);
	return write(&byte, 1);
    }
    int write(const char *buf, const int size);
    int flush();
};

OStream &operator << (OStream &bs, const int i);
IStream &operator >> (IStream &bs, int &i);
OStream &operator << (OStream &bs, const unsigned int i);
IStream &operator >> (IStream &bs, unsigned int &i);
OStream &operator << (OStream &bs, const short i);
IStream &operator >> (IStream &bs, short &i);
OStream &operator << (OStream &bs, const unsigned short i);
IStream &operator >> (IStream &bs, unsigned short &i);
OStream &operator << (OStream &bs, const unsigned char c);
IStream &operator >> (IStream &bs, unsigned char &c);

}
