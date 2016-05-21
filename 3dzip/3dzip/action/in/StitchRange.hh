#pragma once

#include "../../StitchSymbol.hh"
#include "../../log2upper.hh"
#include "ImmediateBitStream.hh"
#include "Range.hh"

namespace VBE {
namespace action {
namespace in {

template <class Decoder, class Model>
class StitchRange
{
    Range<Decoder, Model> read;
    ImmediateBitStream& os;

public:
    StitchRange(ImmediateBitStream& os):
	read(3),
	os(os)
    {}
    void operator()(StitchSymbol& s) {
	read(s.type);
	if (s.has_index() == true)
	    s.index = os(::log2upper(s.size));
    }
    void setDecoder(Decoder& d) {
	read.setDecoder(d);
    }
};

}
}
}
