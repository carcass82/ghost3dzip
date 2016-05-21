#pragma once

#include "../../StitchSymbol.hh"
#include "../../log2upper.hh"
#include "ImmediateBitStream.hh"
#include "Arithmetic.hh"

namespace VBE {
namespace action {
namespace in {

class StitchArithmetic
{
    Arithmetic<unsigned char> read;
    ImmediateBitStream& os;

public:
    StitchArithmetic(ImmediateBitStream& os):
	read(3),
	os(os)
    {}
    void operator()(StitchSymbol& s) {
	read(s.type);
	if (s.has_index() == true)
	    s.index = os(::log2upper(s.size));
    }
    void setDecoder(ArCode::Decoder& d) {
	read.setDecoder(d);
    }
};

}
}
}
