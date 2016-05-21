#pragma once
#include "SymbolGeneric.hh"
#include "Range.hh"

namespace VBE {
namespace action {
namespace in {

template <class Decoder, class Model>
struct SymbolRange
{
    typedef SymbolGeneric<Range<Decoder, Model> > Type;
};

}}}
