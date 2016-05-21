#pragma once
#ifndef ENCODER_H
#define ENCODER_H

#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "hds/HDS.hh"
#include "hds/QVertexAttrib.hh"

#include "huffman.h"
#ifndef USE_HUFFMAN_CODER
 #include "3dzip/ArCode/MoffatModel.hh"
 #include "3dzip/ArCode/RangeCod.hh"
 #include "3dzip/action/out/Range.hh"
#endif

#include "eigensolver.h"
#include "utils.h"

#ifdef HAVE_METIS_H
 #include "metispart.h"
#endif

#ifdef USE_OPENMP
 #include <omp.h>
#endif

#include "huffman.h"


class Encoder
{
public:
	Encoder(float *v, unsigned *f, int vsize, int fsize);
	void Encode(std::ostream &os, float coeffratio,  unsigned short splitThreshold = 0, unsigned char q = 16);

private:
	float *m_Vertices;
	unsigned int *m_Faces;
	int m_VertCount;
	int m_FaceCount;
};

#endif // ENCODER_H
