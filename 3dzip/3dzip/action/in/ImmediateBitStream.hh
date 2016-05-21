#pragma once
#include "../../BitStream.hh"

namespace VBE {
namespace action {
namespace in {

class ImmediateBitStream
{
    Bit::IStream& in;

public:
    ImmediateBitStream(Bit::IStream& in_): in(in_) {}
    const int operator()(const unsigned size) {
	int value = 0;
	in.read((char*)&value, size);
	return value;
    }
};

}}}
