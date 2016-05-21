#pragma once
#include <iostream>

namespace VBE {
namespace action {
namespace out {

class ImmediateStream
{
    std::ostream& out;

public:
    ImmediateStream(std::ostream& out_ = sd::cout): out(out_) {}
    void operator()(const int val, const unsigned) {
	out << val << std::endl;
    }
};

}}}
