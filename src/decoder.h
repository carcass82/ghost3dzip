#pragma once
#ifndef DECODER_H
#define DECODER_H

#include <iostream>
#include <vector>
#include <istream>
#include <stdexcept>
#include "hds/HDS.hh"
#include "hds/QVertexAttrib.hh"

#include "huffman.h"
#ifndef USE_HUFFMAN_CODER
 #include "3dzip/ArCode/MoffatModel.hh"
 #include "3dzip/ArCode/RangeCod.hh"
 #include "3dzip/action/in/Range.hh"
#endif

#include "eigensolver.h"
#include "utils.h"

#ifdef HAVE_METIS_H
 #include "metispart.h"
#endif

#ifdef USE_OPENMP
 #include <omp.h>
#endif

class Decoder
{
public:
	Decoder(unsigned int *f, unsigned int fsize, const int nVert);
	void Decode(std::istream &is);
	std::vector<float>& getVertices() { return m_Vertices; }

private:
	std::vector<float> m_Vertices;
	unsigned int *m_Faces;
	int m_VertCount;
	int m_FaceCount;
};

#endif // DECODER_H
