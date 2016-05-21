/*
    Copyright (C) 2009 Paolo Simone Gasparello <djgaspa@gmail.com>

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

#ifndef AR_CODE_STATIC_MODEL_HH
#define AR_CODE_STATIC_MODEL_HH

#include <vector>
#include <iostream>

namespace Bit {
    class IStream;
    class OStream;
}

namespace ArCode {

class StaticModel
{
    std::vector<unsigned int> frequency;
    std::vector<unsigned int> totals;
    void scale_frequencies(std::vector<unsigned short> &sf) const;
    static int log2(const int x);
    public:
    StaticModel(const int range) : frequency(range, 0), totals(range + 1, 0)
    {}
    StaticModel(Bit::IStream &ibs);
    StaticModel(std::istream& is);
    void count(const unsigned int s) {
	frequency.at(s)++;
    }
    void count(const unsigned int s, const unsigned int n) {
	frequency.at(s) += n;
    }
    void updateTotals();
    void writeTable(Bit::OStream &obs) const;
    void writeTable(std::ostream &os) const;
    int size() const {
	return log2(frequency.size());
    }
    unsigned low(unsigned s) const {
	return totals.at(s);
    }
    unsigned high(unsigned s) const {
	return totals.at(s + 1);
    }
    unsigned range() const {
	return totals.back();
    }
    unsigned find(const unsigned code) const {
	unsigned int i = frequency.size();
	while (code < totals[i])
	    --i;
	return i;
    }

    friend std::ostream &operator<<(std::ostream &os, const StaticModel &sm);
};

inline
std::ostream &operator<<(std::ostream &os, const StaticModel &sm)
{
    os << "Frequencies:\n";
    for (unsigned i = 0; i < sm.frequency.size(); i++)
	os << i << '\t' << sm.frequency[i] << '\n';
    os << "\nTotals:\n";
    for (unsigned i = 0; i < sm.totals.size(); i++)
	os << i << '\t' << sm.totals[i] << '\n';
    return os << std::flush;
}

}

#endif
