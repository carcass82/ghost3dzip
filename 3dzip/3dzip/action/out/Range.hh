#pragma once

namespace VBE {
namespace action {
namespace out {

template <typename Encoder, typename Model>
class Range
{
    Model m;
    Encoder* e;

public:
    Range(const unsigned size, Encoder* e = NULL): m(size), e(e) {}
    void setEncoder(Encoder& e) {
	this->e = &e;
    }
    typedef void result_type;
    void operator()(unsigned s) {
	e->encode_freq(m.freq(s), m.low(s), m.range());
	m.count(s);
    }
    unsigned size() const {
	return m.size();
    }
};

}}}
