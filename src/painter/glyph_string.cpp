#include <cppurses/painter/glyph.hpp>
#include <cppurses/painter/glyph_string.hpp>

#include <algorithm>
#include <iterator>
#include <ostream>
#include <string>

namespace cppurses {

Glyph_string::operator std::string() const {  // NOLINT
    return this->str();
}

std::string Glyph_string::str() const {
    std::string string_;
    for (const Glyph& g : *this) {
        string_.append(g.c_str());
    }
    return string_;
}

Glyph_string::size_type Glyph_string::length() const {
    return this->size();
}

Glyph_string& Glyph_string::operator+=(const Glyph& glyph) {
    return this->append(glyph);
}

Glyph_string Glyph_string::operator+(const Glyph_string& gs) const {
    Glyph_string result{*this};
    result.append(gs);
    return result;
}

void Glyph_string::remove_attribute(Attribute attr) {
    for (Glyph& glyph : *this) {
        glyph.brush().remove_attribute(attr);
    }
}

bool operator==(const Glyph_string& x, const Glyph_string& y) {
    return std::equal(std::begin(x), std::end(x), std::begin(y), std::end(y));
}

bool operator!=(const Glyph_string& x, const Glyph_string& y) {
    return !(x == y);
}

std::ostream& operator<<(std::ostream& os, const Glyph_string& gs) {
    return os << static_cast<std::string>(gs);
}

}  // namespace cppurses
