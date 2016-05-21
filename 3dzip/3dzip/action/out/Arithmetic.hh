#pragma once
#include "../../ArCode.hh"
#include "../../ArCode/AdaptiveModel.hh"

namespace VBE {
namespace action {
namespace out {

template <typename T>
class Arithmetic
{
    ArCode::AdaptiveModel m;
    ArCode::Encoder* e;

public:
    Arithmetic(const unsigned size, ArCode::Encoder* e = NULL): m(size), e(e) {}
    void setEncoder(ArCode::Encoder& e) {
	this->e = &e;
    }
    template <typename V>
    void count(const V& v) {
	m.count(v);
    }
    typedef void result_type;
    void operator()(const T& s) {
	unsigned c = s;
	e->ad_encode(m, c);
    }
    unsigned size() const {
	return m.size();
    }
};

}}}
