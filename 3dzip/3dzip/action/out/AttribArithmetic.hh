#pragma once
#include "Arithmetic.hh"

namespace VBE {
namespace action {
namespace out {

class AttribArithmetic
{
    Arithmetic<unsigned> write;
    const int n;
    const unsigned shift;

public:
    AttribArithmetic(const int n_, const unsigned bits = 12): write(1 << (bits + 1)), n(n_), shift((1 << (bits + 0)) - 1) {}
    void setEncoder(ArCode::Encoder& e) {
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
