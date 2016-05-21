#include <iostream>
#include <iterator>
#include "HDS.hh"
#include "Edge.hh"
#include "Vertex.hh"
#include "HalfEdge.hh"

namespace HDS {

std::ostream &operator<<(std::ostream &os, const Mesh &m)
{
    os << "Ver count: " << m.stitch.size() << '\n';
    for (std::deque<int>::const_iterator i = m.stitch.begin(); i != m.stitch.end(); ++i)
	os << std::distance(m.stitch.begin(), i) << '\t' << *i << '\n';
    os << "\nTri count: " << m.he_tri.size();
    for (Mesh::Connectivity::const_iterator i = m.he_tri.begin(); i != m.he_tri.end(); ++i) {
	const Handle_t t = *i;
	os << "\n" << t->Lface() << "\t[" << t->Org() << ' ' << t->Lnext()->Org() << ' ' << t->Lprev()->Org() << "] [";
	Handle_t it = t;
	do {
	    if (it->Sym() == NULL)
		os << '-';
	    else
		os << it->Sym()->Lface();
	    os << ' ';
	    it = it->Lnext();
	} while (it != t);
	os << ']';
    }
    return os.flush();
}

}

std::ostream &operator<<(std::ostream &os, const Edge &e)
{
    return os << "(" << e.v1 << ", " << e.v2 << ")";
}

std::ostream &operator<<(std::ostream &os, const Vertex &v)
{
    os << "(";
    std::copy(v.star.begin(), v.star.end(), std::ostream_iterator<int>(os, " "));
    return os << ")";
}

std::ostream &operator<<(std::ostream &os, const HalfEdge &e)
{
    return os << "(" << e.v1 << " -> " << e.v2 << ")";
}
