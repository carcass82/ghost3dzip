#pragma once
#include <iostream>

namespace VBE {
namespace action {
namespace out {

class AttribStream
{
    const int n;
    std::ostream& out;

public:
    AttribStream(const int n_, std::ostream& out_ = std::cout): n(n_), out(out_) {}
    template <typename T>
    void operator()(const T& s) {
	for (int i = 0; i < n; ++i)
	    out << s[i] << ' ';
	std::endl;
    }
};

}}}
