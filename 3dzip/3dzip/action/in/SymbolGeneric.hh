#pragma once
#include <boost/function.hpp>
#include "../../Symbol.hh"

namespace VBE {
namespace action {
namespace in {

template <class T>
class SymbolGeneric
{
    T type, idx, skip;
    typedef boost::function<const int(const unsigned)> ImmediateFun;
    ImmediateFun imm_read;
    static const int idx_size = 32;
    static const int skip_size = 8;

public:
    SymbolGeneric(const ImmediateFun& f): type(32), idx(idx_size), skip(skip_size), imm_read(f) {}
    void operator()(Symbol& s) {
	type(s);
	if (s.has_v1() == true)
	    if (s.type == ADD)
		s.val1 = imm_read(16);
	    else {
		idx(s.val1);
		if (s.val1 == idx_size - 1)
		    s.val1 = imm_read(16) + idx_size - 1;
	    }
	if (s.has_v2() == true) {
	    skip(s.val2);
	    if (s.val2 == skip_size - 1)
		s.val2 = imm_read(16) + skip_size - 1;
	}
	if (s.has_v3() == true) {
	    skip(s.val3);
	    if (s.val3 == 7)
		s.val3 = imm_read(16);
	}
    }
    template <typename D>
    void setDecoder(D& dec) {
	type.setDecoder(dec);
	idx.setDecoder(dec);
	skip.setDecoder(dec);
    }
};

}}}
