#pragma once
#include <iostream>
#include "../../Symbol.hh"

namespace VBE {
namespace action {
namespace out {

struct SymbolStream
{
    std::ostream& out;
    SymbolStream(std::ostream& out_ = std::cout): out(out_) {}
    void operator()(const Symbol& s) {
	out << s << std::endl;
    }
};

}
}
}
