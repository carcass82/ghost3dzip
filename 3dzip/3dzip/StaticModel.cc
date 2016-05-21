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

#include <stdexcept>
#include <climits>
#include <algorithm>
#include "ArCode/StaticModel.hh"
#include "BitStream.hh"

namespace ArCode {

StaticModel::StaticModel(Bit::IStream &ibs)
{
    short range;
    ibs >> range;
    frequency.resize(range);
    totals.resize(range + 1, 0);
    for (int i = 0; i < range; i++) {
	unsigned short n;
	ibs >> n;
	frequency[i] = n;
    }
}

StaticModel::StaticModel(std::istream& is)
{
    short range;
    is.read((char*)&range, sizeof(range));
    frequency.resize(range);
    totals.resize(range + 1, 0);
    for (int i = 0; i < range; i++) {
	unsigned short n;
	is.read((char*)&n, sizeof(n));
	frequency[i] = n;
    }
}

int StaticModel::log2(const int x)
{
    if (x < 1)
	throw std::out_of_range("Log argument must be a positive number");
    int ret = 0;
    for (unsigned i = 0; i < sizeof(x) * CHAR_BIT; i++)
	if ((x & (1 << i)) != 0)
	    ret = i;
   return ret; 
}

void StaticModel::scale_frequencies(std::vector<unsigned short> &sf) const
{
    const int max_freq = *std::max_element(frequency.begin(), frequency.end());
    const int scale = max_freq / (1 << (sizeof(unsigned short) * 8)) + 1;
    sf.resize(frequency.size());
    for (unsigned i = 0; i < sf.size(); i++) {
	sf[i] = frequency[i] / scale;
	if (sf[i] == 0 && frequency[i] != 0)
	    sf[i] = 1;
    }
}

void StaticModel::updateTotals()
{
    std::vector<unsigned short> scaled_freq;
    scale_frequencies(scaled_freq);
    totals[0] = 0;
    for (unsigned i = 0; i < scaled_freq.size(); i++)
	totals[i + 1] = totals[i] + scaled_freq[i];
}

void StaticModel::writeTable(Bit::OStream &obs) const
{
    std::vector<unsigned short> scaled_freq;
    scale_frequencies(scaled_freq);
    const short range = scaled_freq.size();
    obs << range;
    for (int i = 0; i < range; i++)
	obs << scaled_freq[i];
}

void StaticModel::writeTable(std::ostream &os) const
{
    typedef std::vector<unsigned short> scaled_freq_t;
    scaled_freq_t scaled_freq;
    scale_frequencies(scaled_freq);
    const short range = scaled_freq.size();
    os.write((const char*)&range, sizeof(range));
    for (int i = 0; i < range; i++)
	os.write((const char*)&scaled_freq[i], sizeof(scaled_freq_t::value_type));
}

}
