#pragma once
#include "Range.hh"

namespace VBE {
namespace action {
namespace out {

template <class Encoder, class Model>
class AttribRange
{
    Range<Encoder, Model> write;
    const int n;
    const unsigned shift;

public:
    AttribRange(const int n_, const unsigned bits = 12): write(1 << (bits + 1)), n(n_), shift((1 << (bits + 0)) - 1) {}
    void setEncoder(Encoder& e) {
	write.setEncoder(e);
    }
    template <typename T>
    void operator()(const T& s) {
	for (int i = 0; i < n; ++i) {
	    const unsigned sym = unsigned(s[i]) + shift;
	    write(sym);
	}
    }
};

}}}
