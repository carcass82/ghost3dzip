#pragma once
#include "SymbolGeneric.hh"
#include "Range.hh"

namespace VBE {
namespace action {
namespace out {

template <class Encoder, class Model>
struct SymbolRange
{
    typedef SymbolGeneric<Range<Encoder, Model> > Type;
};

}}}
