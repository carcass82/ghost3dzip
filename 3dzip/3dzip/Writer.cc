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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_ZLIB_H
#  include <zlib.h>
#endif
#include <memory>
#include <map>

#include <hds/QVertexAttrib.hh>
#include "Writer.hh"
#include "Compressor.hh"
#include "Property.hh"
#include "Predictor.hh"
#include "BitStream.hh"
#include "CompressorFacade.hh"
#include "action/out/SymbolRange.hh"
#include "action/out/ImmediateBitStream.hh"
#include "action/out/AttribRange.hh"
#include "action/out/StitchRange.hh"
#include "ArCode/MoffatModel.hh"
#include "ArCode/RangeCod.hh"
#include "ArCode/AdaptiveModel.hh"

namespace VBE {

typedef RangeEnc Encoder;

template <typename T>
static inline
void my_write(std::ostream& os, const T& t)
{
    os.write((const char*)&t, sizeof(T));
}

struct attribute_t
{
    typedef VBE::OutputProperty<QVertexAttrib<>, VBE::OutputParallelogramPredictor> OutputProperty;
    QVertexAttrib<> p;
    action::out::AttribRange<Encoder, MoffatModel> mop;
    OutputProperty prop;

    attribute_t(const float* first, const float* last, const size_t dim, const unsigned char q):
	p(first, last, dim, q),
	mop(dim, q),
	prop(p, boost::ref(mop))
    {}
};

struct Writer::Impl
{
    typedef action::out::StitchRange<Encoder, ArCode::AdaptiveModel> StitchFunc;
    unsigned n_tri, n_ver;
    std::ostringstream immoss;
    Bit::OStream immbos;
    std::ostringstream oss;
    Encoder e;
    action::out::ImmediateBitStream biop;
    action::out::SymbolRange<Encoder, ArCode::AdaptiveModel>::Type sop;
    StitchFunc* stop;
    HDS::Mesh mesh;
    ManifoldCompressorFacade* m;
    std::map<std::string, attribute_t*> v_attr;

    Impl(const unsigned* first, const unsigned* last, const size_t num_ver, const bool correct_manifold = true);
    ~Impl();
};

Writer::Impl::Impl(const unsigned* first, const unsigned* last, const size_t num_ver, const bool correct_manifold):
    n_tri((last - first) / 3),
    n_ver(num_ver),
    immoss(std::ios::binary),
    immbos(immoss),
    oss(std::ios::binary), e(oss),
    biop(immbos),
    sop(boost::ref(biop)),
    stop(NULL),
    mesh(first, last, num_ver, correct_manifold)
{
    if (correct_manifold == true) {
	stop = new StitchFunc(e, boost::ref(biop));
	m = new CompressorFacade(mesh, boost::ref(*stop));
    }
    else
	m = new ManifoldCompressorFacade(mesh);
    sop.setEncoder(e);
}

Writer::Impl::~Impl()
{
    delete m;
    delete stop;
}

Writer::Writer(const unsigned* p, const size_t n, const size_t num_ver, const bool correct_manifold):
    pimpl(new Impl(p, p + 3 * n, num_ver, correct_manifold))
{}

Writer::~Writer()
{
    for (std::map<std::string, attribute_t*>::iterator i = pimpl->v_attr.begin(); i != pimpl->v_attr.end(); ++i)
	delete i->second;
    delete pimpl;
}

unsigned char Writer::suggestQ(const unsigned n)
{
    unsigned threshold[] = {80, 250, 800, 4000, 10000, 80000, 500000, 2000000};
    const unsigned char size = sizeof(threshold) / sizeof(unsigned);
    unsigned char q = 0;
    while (q < size && n > threshold[q])
	++q;
    q += 6;
    return q;
}	

void Writer::addAttrib(const float* p, const size_t n, const size_t dim, const char* descr, int q_bit)
{
    if (q_bit == 0)
	q_bit = suggestQ(n);
    if (q_bit < 6 || q_bit > 14)
	throw std::logic_error("Only quantization bit between 6 and 14 are allowed");
    std::pair<std::map<std::string, attribute_t*>::iterator, bool> i = pimpl->v_attr.insert(std::make_pair(descr, new attribute_t(p, p + dim * n, dim, q_bit)));
    attribute_t* attr = i.first->second;
    pimpl->m->addProperty(boost::ref(attr->prop));
    attr->mop.setEncoder(pimpl->e);
}

void Writer::operator()(std::ostream& os)
{
    pimpl->e.start(0, 0);
    VBE::Compressor(*pimpl->m, boost::ref(pimpl->sop)).run();
    pimpl->immbos.flush();
    pimpl->immoss.flush();
    pimpl->e.done();
    pimpl->oss.flush();
    std::ostringstream os2(std::ios::binary);
    {
	const uint32_t num_ver = pimpl->n_ver, num_tri = pimpl->n_tri;
	const uint16_t num_attr = pimpl->v_attr.size();
	my_write(os2, num_ver);
	my_write(os2, num_tri);
	my_write(os2, num_attr);
	char has_stitch = (pimpl->stop == NULL) ? 0 : 1;
	my_write(os2, has_stitch);
    }
    for (std::map<std::string, attribute_t*>::iterator i = pimpl->v_attr.begin(); i != pimpl->v_attr.end(); ++i) {
	os2 << i->first << '\0';
	attribute_t* attr = i->second;
	uint8_t dim = attr->p.rows();
	my_write(os2, dim);
	my_write(os2, attr->p.getQ());
	attr->p.getBoundingBox().serialize(os2);
    }
    os2.write(pimpl->immoss.str().data(), pimpl->immoss.str().size());
    const uint32_t plain_size = os2.str().size();
    my_write(os, plain_size);
#ifdef HAVE_ZLIB_H
    uLong destLen = compressBound(os2.str().size());
    std::vector<Byte> compr_buffer(destLen);
    ::compress2(&compr_buffer[0], &destLen, (const Bytef*)os2.str().data(), os2.str().size(), 6);
    const uint32_t compr_size = destLen;
    my_write(os, compr_size);
    os.write((const char*)&compr_buffer[0], destLen);
#else
    os.write(os2.str().data(), os2.str().size());
#endif
    os.write(pimpl->oss.str().data(), pimpl->oss.str().size());
}

struct VertexOrdering
{
    unsigned count;
    unsigned* data;
    VertexOrdering(unsigned* data):
	count(0), data(data)
    {}
    void operator()(const HDS::Mesh& m, const HDS::Handle_t v, const HDS::Handle_t active_edge) {
	if (m.isDummy(v) == true)
	    return;
	data[m.element(v)] = count++;
    }
};

unsigned* Writer::getVertexOrdering()
{
    unsigned* order = new unsigned[pimpl->n_ver];
    VertexOrdering vo(order);
    pimpl->m->addProperty(vo);
    return order;
}

struct TriangleOrdering
{
    unsigned count;
    unsigned* data;
    TriangleOrdering(unsigned* data):
	count(0), data(data)
    {}
    void operator()(const unsigned face_idx) {
	data[face_idx] = count++;
    }
};

unsigned* Writer::getFaceOrdering()
{
    unsigned* order = new unsigned[pimpl->n_tri];
    TriangleOrdering to(order);
    pimpl->m->setFaceCallback(to);
    return order;
}

}
