#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <ply.hpp>

namespace ply {

class reader
{
    ply::ply_parser p;
    std::vector<float>& coords;
    std::vector<unsigned>& index;

    void append_coord(ply::float32 x)
    {
	coords.push_back(x);
    }

    static void void_f(ply::float32) {}

    boost::function<void(ply::float32)> cb(const std::string& element_name, const std::string& property_name)
    {
	if (element_name == "vertex" && (property_name == "x" ||
		    property_name == "y" ||
		    property_name == "z"))
	    return boost::bind(&reader::append_coord, this, _1);
	return &reader::void_f;
    }

    static void list_size(ply::uint8 size)
    {
	if (size != 3)
	    throw std::logic_error("Only triangles are admitted");
    }

    void list_index(ply::int32 i)
    {
	index.push_back(i);
    }

    ply::ply_parser::element_callbacks_type edc(const std::string& element_name, std::size_t size)
    {
	if (element_name == "vertex")
	    coords.reserve(size * 3);
	else if (element_name == "face")
	    index.reserve(size * 3);
	return ply::ply_parser::element_callbacks_type(0, 0);
    }

    typedef std::tr1::tuple<std::tr1::function<void(ply::uint8)>, std::tr1::function<void(ply::int32)>, std::tr1::function<void()> > CallbackReturnType;
    CallbackReturnType lcb(const std::string& element_name, const std::string& property_name)
    {
	if (element_name == "face" && property_name == "vertex_indices")
	    return CallbackReturnType(&reader::list_size, boost::bind(&reader::list_index, this, _1), 0);
	return  CallbackReturnType(0, 0, 0);
    }

public:
    reader(std::vector<float>& c, std::vector<unsigned>& i):
	coords(c), index(i)
    {
	ply::ply_parser::scalar_property_definition_callbacks_type cbs;
	ply::at<ply::float32>(cbs) = boost::bind(&reader::cb, this, _1, _2);
	p.scalar_property_definition_callbacks(cbs);
	ply::ply_parser::list_property_definition_callbacks_type lcbs;
	ply::at<ply::uint8, ply::int32>(lcbs) = boost::bind(&reader::lcb, this, _1, _2);
	p.list_property_definition_callbacks(lcbs);
	p.element_definition_callback(boost::bind(&reader::edc, this, _1, _2));
    }

    void operator()(const std::string& filename)
    {
	p.parse(filename);
    }
};

}
