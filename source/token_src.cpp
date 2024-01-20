#include "textlib.h"

#include <map>
#include <set>
#include <algorithm>
#include <functional>

using namespace TXTproc;

Token::Token (const std::string& content, size_t line, size_t column) :
    content_(content), line(line), column(column) {}
Token::Token (const char* ptr2content, size_t line, size_t column) :
    content_(ptr2content), line(line), column(column) {}

Token::operator std::string () {
    return content_;
}
Token::operator const std::string () const {
    return content_;
}

std::string& Token::content () {
    return content_;
}
const std::string& Token::content () const {
    return content_;
}

size_t Token::getLine   () const { return   line; }
size_t Token::getColumn () const { return column; }

bool Token::operator== (const Token& rhv) const {
    return content_ == rhv.content_;
}
bool Token::operator!= (const Token& rhv) const {
    return !(*this == rhv);
}
bool Token::operator<  (const Token& rhv) const {
    for (size_t i = 0; i < std::min(rhv.content_.size(), content_.size()); i++) {
        if (content_[i] != rhv.content_[i]) {
            return content_[i] < rhv.content_[i];
        }
    }

    return content_.size() >= rhv.content_.size();
}
bool Token::operator<= (const Token& rhv) const {
    return (*this == rhv) || (*this < rhv);
}
bool Token::operator>  (const Token& rhv) const {
    return rhv < *this;
}
bool Token::operator>= (const Token& rhv) const {
    return (*this == rhv) || (*this > rhv);
}

static bool isPrefix (const std::string& base, size_t idx, const std::string& prefix) {
    if (base.size() < prefix.size() + idx) return false;

    for (size_t i = 0; i < prefix.size(); i++) {
        if (base[i + idx] != prefix[i]) return false;
    }

    return true;
}

std::vector<Token> TXTproc::createTokens (const Text& text, const std::vector<Token>& terminalTokens) {
    std::vector<Token> terminals(terminalTokens.begin(), terminalTokens.end());
    std::sort(terminals.begin(), terminals.end());

    std::vector<Token> tokens;
    bool startNewToken = true;

    for (size_t lineNo = 0; lineNo < text.size(); lineNo++) {
        for (size_t colNo = 0; colNo < text[lineNo].length();) {
            bool wasFixedToken = false;

            for (const auto& terminal : terminals) {
                if (isPrefix(text[lineNo], colNo, terminal)) {
                    tokens.push_back(Token(terminal, lineNo, colNo));
                    colNo += terminal.content().length();

                    startNewToken = true;
                    wasFixedToken = true;
                }
            }

            if (wasFixedToken) continue;
            if (startNewToken) {
                startNewToken = false;

                tokens.push_back(Token("", lineNo, colNo));
            }

            tokens.back().content().push_back(text[lineNo][colNo]);
            colNo++;
        }
    }

    return tokens;
}


