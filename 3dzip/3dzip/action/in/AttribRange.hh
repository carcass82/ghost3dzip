#pragma once
#include "Range.hh"

namespace VBE {
namespace action {
namespace in {

template <class Decoder, class Model>
class AttribRange
{
    Range<Decoder, Model> read;
    const int n;
    const unsigned shift;

public:
    AttribRange(const int n_, const unsigned bits = 12): read(1 << (bits + 1)), n(n_), shift((1 << (bits + 0)) - 1) {}
    void setDecoder(Decoder& d) {
	read.setDecoder(d);
    }
    template <typename T>
    void operator()(T& s) {
	s = T(n);
	for (int i = 0; i < n; ++i) {
	    unsigned sym;
	    read(sym);
	    s[i] = sym - shift;
	}
    }
};

}}}
