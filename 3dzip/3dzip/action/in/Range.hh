#pragma once

namespace VBE {
namespace action {
namespace in {

template <typename Decoder, typename Model>
class Range
{
    Model m;
    Decoder* d;

public:
    Range(const unsigned size, Decoder* d = NULL): m(size), d(d) {}
    void setDecoder(Decoder& d) {
	this->d = &d;
    }
    typedef void result_type;
    template <typename T>
    void operator()(T& s) {
	int syfreq, ltfreq;
	ltfreq = d->decode_culfreq(m.range());
	unsigned c = m.find(ltfreq);
	d->update(m.freq(c), m.low(c), m.range());
	m.count(c);
	s = T(c);
    }
    unsigned size() const {
	return m.size();
    }
};

}}}
