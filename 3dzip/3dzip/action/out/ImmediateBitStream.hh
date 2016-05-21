#pragma once
#include "../../BitStream.hh"

namespace VBE {
namespace action {
namespace out {

class ImmediateBitStream
{
    Bit::OStream& out;

public:
    ImmediateBitStream(Bit::OStream& out_): out(out_) {}
    void operator()(const int val, const unsigned size) {
	out.write((const char*) &val, size);
    }
};

}}}
