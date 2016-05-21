#pragma once

#include "../../StitchSymbol.hh"
#include "../../log2upper.hh"
#include "ImmediateBitStream.hh"
#include "Range.hh"
#include "../../ArCode/AdaptiveModel.hh"

namespace VBE {
namespace action {
namespace out {

template <class Encoder, class Model>
class StitchRange
{
    Range<Encoder, Model> write;
    ImmediateBitStream& os;

public:
    StitchRange(Encoder& e, ImmediateBitStream& os):
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
