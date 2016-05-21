#pragma once

#include "../../StitchSymbol.hh"
#include "../../log2upper.hh"
#include "ImmediateBitStream.hh"
#include "Arithmetic.hh"

namespace VBE {
namespace action {
namespace out {

class StitchArithmetic
{
    Arithmetic<unsigned> write;
    ImmediateBitStream& os;

public:
    StitchArithmetic(ArCode::Encoder& e, ImmediateBitStream& os):
	write(3, &e),
	os(os)
    {}
    void operator()(const StitchSymbol& s) {
	write(s.type);
	if (s.has_index() == true)
	    os(s.index, ::log2upper(s.size));
    }
};

}
}
}
