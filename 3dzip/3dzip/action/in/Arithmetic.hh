#pragma once
#include "../../ArCode.hh"
#include "../../ArCode/AdaptiveModel.hh"

namespace VBE {
namespace action {
namespace in {

template <typename T>
class Arithmetic
{
    ArCode::AdaptiveModel m;
    ArCode::Decoder* d;

public:
    Arithmetic(const unsigned size, ArCode::Decoder* d = NULL): m(size), d(d) {}
    void setDecoder(ArCode::Decoder& d) {
	this->d = &d;
    }
    typedef void result_type;
    void operator()(T& s) {
	s = T(d->ad_decode(m));
    }
    unsigned size() const {
	return m.size();
    }
};

}}}
