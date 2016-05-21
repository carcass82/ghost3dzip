/*
    Copyright (C) 2009 Paolo Simone Gasparello <djgaspa@gmail.com>

    This file is part of libhds.

    libhds is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libhds is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libhds.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _VERTEX_HH
#define _VERTEX_HH

class Vertex
{
    friend std::ostream &operator<<(std::ostream &os, const Vertex &v);
    typedef std::vector<int> star_t;
    star_t star;
    int index;
    public:
    Vertex(const int i = 0): index(i)
    {}

    unsigned int operator()() const
    {
	return index;
    }

    void addStar(const int _t)
    {
	star.push_back(_t);
    }

    size_t size() const
    {
	return star.size();
    }

    int operator[](const int i) const
    {
	return star.at(i);
    }

    const star_t &getStar() const
    {
	return star;
    }

    bool operator< (const Vertex &v) const
    {
	return index > v.index;
    }
};

#endif
