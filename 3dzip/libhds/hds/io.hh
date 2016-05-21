#ifndef HDS_IO_HH
#define HDS_IO_HH

#include <iostream>
#include "QVertexAttrib.hh"

#ifdef HALFEDGE_INCLUDED
template<class T>
inline std::ostream& operator<<(std::ostream& out, const Halfedge<T>& e)
{
    return out << e.Org() << " -> " << e.Dest();
}
#endif

#ifdef VERTEX_ATTRIB_HH
template <typename U>
std::ostream& operator<<(std::ostream& os, const VertexAttrib<U>& p)
{
    for (typename VertexAttrib<U>::size_type i = 0; i < p.size(); ++i) {
	os << i << ":\t[" << p[i].transpose() << "]\n";
    }
    return os << std::flush;
}
#endif

#ifdef QVERTEX_ATTRIB_HH
template <typename T>
inline
std::ostream& operator<<(std::ostream& os, const BoundingBox<T>& bb)
{
    return os << "Offset: [" << bb.offset.transpose() << "]\nScale: [" << bb.scale.transpose() << ']';
}

template <typename T, typename S>
inline
std::ostream& operator<<(std::ostream& os, const QVertexAttrib<T, S>& v)
{
    return os << (VertexAttrib<T>&)v << '\n' << v.getBoundingBox();
}
#endif

#endif
