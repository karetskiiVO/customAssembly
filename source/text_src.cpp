#include "textlib.h"

using namespace TXTproc;

Text::Text () {}

Text::Text (const std::vector<std::string>& _content) : content(_content) {}

std::string& Text::operator[] (size_t idx) {
    return content[idx];
}
const std::string& Text::operator[] (size_t idx) const {
    return content[idx];
}

void Text::clear () {
    content.clear();
}
size_t Text::size () const {
    return content.size();
}

void Text::addLine (const std::string& line) {
    content.push_back(line);
}

std::istream& TXTproc::operator>> (std::istream& stream, TXTproc::Text& text) {
    std::string bufString;
    text.clear();

    while (std::getline(stream, bufString)) {
        text.addLine(bufString + "\n");
    }

    return stream;
}
std::ostream& TXTproc::operator<< (std::ostream& stream, const TXTproc::Text& text) {
    for (size_t i = 0; i < text.size(); i++) {
        stream << text[i];
    }

    return stream;
}


