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
#include <boost/smart_ptr.hpp>
#include <memory>
#include <sstream>

#include "Reader.hh"
#include "BitStream.hh"
#include "UncompressorFacade.hh"
#include "Property.hh"
#include "Predictor.hh"
#include "Uncompressor.hh"
#include "hds/QVertexAttrib.hh"
#include "action/in/AttribRange.hh"
#include "action/in/ImmediateBitStream.hh"
#include "action/in/SymbolRange.hh"
#include "action/in/StitchRange.hh"
#include "ArCode/RangeCod.hh"
#include "ArCode/MoffatModel.hh"
#include "ArCode/AdaptiveModel.hh"

namespace VBE {

typedef RangeDec Decoder;

template <typename T>
static inline
void my_read(std::istream& is, T& t)
{
    is.read((char*)&t, sizeof(T));
}

struct attribute_t
{
    typedef VBE::InputProperty<QVertexAttrib<>, VBE::InputParallelogramPredictor> InputProperty;
    QVertexAttrib<> p;
    action::in::AttribRange<Decoder, MoffatModel> mop;
    InputProperty prop;

    attribute_t(const int num_ver, const int dim, const unsigned char q, const BoundingBox<>& bb):
	p(q, bb, num_ver, dim),
	mop(dim, q),
	prop(p, boost::ref(mop))
    {}
};

struct Reader::Impl
{
    boost::shared_ptr<HDS::Mesh> hds_mesh;
    boost::shared_ptr<ManifoldUncompressorFacade> mesh;
    typedef boost::shared_ptr<attribute_t> attrib_ptr;
    std::map<std::string, attrib_ptr> attr;
};

Reader::Reader():
    pimpl(new Impl)
{}

Reader::~Reader()
{
    delete pimpl;
}

void Reader::operator()(std::istream& is)
{
    std::istringstream immediate_is(std::ios::binary);
    Bit::IStream immediate_bit_stream(immediate_is);
    action::in::ImmediateBitStream iip(immediate_bit_stream);
    action::in::SymbolRange<Decoder, ArCode::AdaptiveModel>::Type ip(boost::ref(iip));
    action::in::StitchRange<Decoder, ArCode::AdaptiveModel> sip(iip);
    uint32_t plain_size;
    my_read(is, plain_size);
    std::vector<char> plain_buffer(plain_size);
#ifdef HAVE_ZLIB_H
    uint32_t compr_size;
    my_read(is, compr_size);
    std::vector<char> compr_buffer(compr_size);
    is.read(&compr_buffer[0], compr_size);
    uLong destLen = plain_size;
    ::uncompress((Bytef*)&plain_buffer[0], &destLen, (const Bytef*)&compr_buffer[0], compr_size);
#else
    is.read(&plain_buffer[0], plain_size);
#endif
    std::istringstream iis(std::ios::binary);
    iis.str(std::string(plain_buffer.begin(), plain_buffer.end()));
    uint32_t num_ver, num_tri;
    uint16_t num_attr;
    my_read(iis, num_ver);
    my_read(iis, num_tri);
    my_read(iis, num_attr);
    n_tri = num_tri;
    n_ver = num_ver;

    char tmp;
    my_read(iis, tmp);
    bool has_stitch = tmp == 0 ? false : true;
    pimpl->hds_mesh.reset(new HDS::Mesh);
    if (has_stitch == true)
	pimpl->mesh.reset(new UncompressorFacade(*pimpl->hds_mesh, num_ver, boost::ref(sip)));
    else
	pimpl->mesh.reset(new ManifoldUncompressorFacade(*pimpl->hds_mesh, num_ver));

    for (uint32_t i = 0; i < num_attr; ++i) {
	std::string descr;
	std::getline(iis, descr, '\0');
	uint8_t dim;
	my_read(iis, dim);
	unsigned char q;
	my_read(iis, q);
	BoundingBox<float> bb(dim);
	bb.unserialize(iis);
	std::pair<std::map<std::string, Impl::attrib_ptr>::iterator, bool> it = pimpl->attr.insert(std::make_pair(descr, Impl::attrib_ptr(new attribute_t(num_ver, dim, q, bb))));
	pimpl->mesh->addProperty(boost::ref(it.first->second->prop));
    }
    {
	size_t immediate_size = iis.str().size() - iis.tellg();
	std::vector<char> buffer(immediate_size);
	iis.read(&buffer[0], immediate_size);
	immediate_is.str(std::string(buffer.begin(), buffer.end()));
    }

    Decoder d(is);
    d.start();
    ip.setDecoder(d);
    sip.setDecoder(d);
    for (std::map<std::string, Impl::attrib_ptr>::iterator i = pimpl->attr.begin(); i != pimpl->attr.end(); ++i)
	i->second->mop.setDecoder(d);

    VBE::Uncompressor(*pimpl->mesh, boost::ref(ip)).run();
    d.done();
}

unsigned Reader::getNumAttr() const
{
    return pimpl->attr.size();
}

bool Reader::hasAttrib(const char* name) const
{
    return pimpl->attr.find(name) != pimpl->attr.end();
}

unsigned Reader::rows(const char* name) const
{
    return pimpl->attr[name]->p.rows();
}

unsigned* Reader::getTriangles() const
{
    unsigned* p = new unsigned[3 * getNumTri()];
    pimpl->hds_mesh->triangles(p);
    return p;
}

void Reader::getTriangles(unsigned* p) const
{
    pimpl->hds_mesh->triangles(p);
}

float* Reader::getAttrib(const char* name) const
{
    attribute_t& a = *pimpl->attr[name];
    float* p = new float[a.p.rows() * getNumVer()];
    a.p.dequantize(p);
    return p;
}

void Reader::getAttrib(const char* name, float* p) const
{
    pimpl->attr[name]->p.dequantize(p);
}

const char* Reader::getName(const unsigned i) const
{
    if (i > pimpl->attr.size() - 1)
	throw std::out_of_range("Invalid attrib index");
    std::map<std::string, Impl::attrib_ptr>::iterator it = pimpl->attr.begin();
    for (unsigned j = 0; j < i; ++j)
	++it;
    return it->first.c_str();
}

}
