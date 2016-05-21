#ifndef HALFEDGE_INCLUDED // -*- C++ -*-
#define HALFEDGE_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  Half-Edge primitive for representing manifold subdivisions.

  Copyright (C) 2004 Michael Garland.

 ************************************************************************/

#include <vector>
#include <cassert>

template<class _Vertex, class _Face=int>
class Halfedge
{
public:
    typedef Halfedge *Handle;
    typedef _Vertex Vertex;
    typedef _Face Face;
    enum { NoFace=-1 };

private:
    Handle lnext, sym;
    Vertex org;
    Face lface;

public:
    Halfedge(Vertex v, Face f=NoFace)
	: lnext(NULL), sym(NULL), org(v), lface(f) {}

    ~Halfedge() { lnext=sym=NULL; }

    Handle Lnext() const { return lnext; }
    Handle Lprev() const { return Lnext()->Lnext(); }
    Handle Sym()   const { return sym; }

    Handle Onext() const { return Lprev()->Sym(); }
    Handle Oprev() const { return Sym()->Lnext(); }
    Handle Dnext() const { return Sym()->Lprev(); }
    Handle Dprev() const { return Lnext()->Sym(); }
    Handle Rnext() const { return Oprev()->Sym(); }
    Handle Rprev() const { return Dnext()->Sym(); }

    Vertex& Org()        { return org; }
    Vertex  Org() const  { return org; }
    //
    // we use Lnext here, rather than Sym because Lnext will
    // always be defined, whereas Sym may not be.
    Vertex& Dest()       { return Lnext()->Org(); }
    Vertex  Dest() const { return Lnext()->Org(); }

    // Halfedges are oriented from e[0] -> e[1]
    Vertex&       operator[](int i)       { return i ? Dest() : Org(); }
    Vertex        operator[](int i) const { return i ? Dest() : Org(); }

    // Each edge maintains an ID for the face on its left
    Face& Lface()       { return lface; }
    Face  Lface() const { return lface; }

public:

    template<class Polygon>
    static Handle create_face(Polygon& P, Face f=NoFace)
    {
	const int N = P.size();
	std::vector<Handle> edges(N);

	for (int i = 0; i < N; ++i)
	    edges[i] = new Halfedge(P[i], f);
	for (int i = 0; i < N - 1; ++i)
	    edges[i]->lnext = edges[i + 1];

	edges[N - 1]->lnext = edges[0];

	return edges[0];
    }

    static void delete_face(Handle e)
    {
	delete e->Lprev();
	delete e->Lnext();
	delete e;
    }

    static void paste(Handle e1, Handle e2)
    {
	assert(e1->Sym() == NULL);
	assert(e2->Sym() == NULL);
	assert(e1->Org() == e2->Dest());
	assert(e2->Org() == e1->Dest());

	e1->sym = e2;
	e2->sym = e1;
    }

    static void cut(Handle e)
    {
	if( e->sym )
	{
	    e->sym->sym = NULL;
	    e->sym = NULL;
	}
    }
};

#endif
