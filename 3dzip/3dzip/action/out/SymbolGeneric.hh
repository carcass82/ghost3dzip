#pragma once
#include <boost/function.hpp>
#include "../../Symbol.hh"

namespace VBE {
namespace action {
namespace out {

template <class T>
class SymbolGeneric
{
    T type, idx, skip;
    typedef boost::function<void(const int, const unsigned)> ImmediateFun;
    ImmediateFun imm_write;
    static const int idx_size = 32;
    static const int skip_size = 8;

public:
    SymbolGeneric(const ImmediateFun& f): type(32), idx(idx_size), skip(skip_size), imm_write(f) {}
    void operator()(const Symbol& s) {
	type(s);
	if (s.has_v1() == true)
	    if (s.type == ADD)
		imm_write(s.val1, 16);
	    else {
		if (s.val1 < idx_size - 1)
		    idx(s.val1);
		else {
		    idx(idx_size - 1);
		    imm_write(s.val1 - idx_size + 1, 16);
		}
	    }
	if (s.has_v2() == true)
	    if (s.val2 < skip_size - 1)
		skip(s.val2);
	    else {
		skip(skip_size - 1);
		imm_write(s.val2 - skip_size + 1, 16);
	    }
	if (s.has_v3() == true)
	    if (s.val3 < 7)
		skip(s.val3);
	    else {
		skip(7);
		imm_write(s.val3, 16);
	    }
    }
    template <typename E>
    void setEncoder(E& enc) {
	type.setEncoder(enc);
	idx.setEncoder(enc);
	skip.setEncoder(enc);
    }
};

}}}
