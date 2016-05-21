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

#ifndef AR_CODE_ADAPTIVE_MODEL_HH
#define AR_CODE_ADAPTIVE_MODEL_HH

#include <vector>
#include <ostream>

namespace ArCode {

class AdaptiveModel
{
    std::vector<unsigned> sym;
    std::vector<unsigned> pos;
    std::vector<unsigned> totals;
public:
    AdaptiveModel(const unsigned range):
	sym(range), pos(range), totals(range + 1)
    {
	for (unsigned i = 0; i < sym.size(); ++i) {
	    sym[i] = i;
	    pos[i] = i;
	    totals[i + 1] = i + 1;
	}
	totals[0] = 0;
    }
    unsigned freq(const unsigned s) const {
	unsigned p = pos.at(s);
	return totals[p + 1] - totals[p];
    }
    void count(const unsigned s) {
	unsigned p = pos.at(s);
	unsigned f = freq(s);
	while (p < sym.size() - 1 && totals[p + 2] == totals[p + 1] + f)
	    ++p;
	if (p > pos[s]) {
	    unsigned sym_a = sym[p], sym_b = s;
	    unsigned pos_a = p, pos_b = pos[s];
	    pos[sym_a] = pos_b;
	    pos[sym_b] = pos_a;
	    sym[pos_a] = sym_b;
	    sym[pos_b] = sym_a;
	}
	for (unsigned i = pos[s] + 1; i < totals.size(); ++i)
	    ++totals[i];
    }
    unsigned size() const {
	return totals.size() - 1;
    }
    unsigned low(unsigned s) const {
	return totals[pos.at(s)];
    }
    unsigned high(unsigned s) const {
	return totals[pos.at(s) + 1];
    }
    unsigned range() const {
	return totals.back();
    }
    unsigned find(const unsigned code) const {
	unsigned int i = size();
	while (code < totals[i])
	    --i;
	return sym[i];
    }
    friend std::ostream &operator<<(std::ostream &os, const AdaptiveModel &am);
};

inline
std::ostream &operator<<(std::ostream &os, const AdaptiveModel &am)
{
    for (unsigned i = 0; i < am.size(); ++i)
	os << i << ": [" << am.pos[i] << ':' << am.freq(i) << ':' << am.high(i) << "] [" << am.sym[i] << ':' << am.totals[i + 1] - am.totals[i] << ':' << am.totals[i + 1] << ']' << std::endl;
    return os;
}

}

#endif
