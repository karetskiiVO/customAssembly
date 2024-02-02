#pragma once

#include <string>
#include <vector>
#include <iostream>

namespace TXTproc {

using Line = std::string;

class Text {
    std::vector<std::string> content;
public:
    Text ();
    Text (const std::vector<std::string>& _content);
    
    std::string& operator[] (size_t idx);
    const std::string& operator[] (size_t idx) const;

    size_t size () const;

    void clear ();

    void addLine (const std::string& line);

    friend std::istream& operator>> (std::istream& stream, TXTproc::Text& text);
    friend std::ostream& operator<< (std::ostream& stream, const TXTproc::Text& text);
};

class Token {
    std::string content_;
    size_t line, column;
    std::string filename_;
public:
    static constexpr size_t UnknownPosition () { return (size_t)-1; }

    Token (const std::string& content, size_t line = UnknownPosition(), 
           size_t column = UnknownPosition(), const std::string& filename = "");
    Token (const char* ptr2content, size_t line = UnknownPosition(), 
           size_t column = UnknownPosition(), const std::string& filename = "");

    operator std::string ();
    operator const std::string () const;

    size_t getLine   () const;
    size_t getColumn () const;

    std::string& content ();
    const std::string& content () const;

    std::string& filename ();
    const std::string& filename () const; 

    bool operator== (const Token& rhv) const;
    bool operator!= (const Token& rhv) const;
    bool operator<  (const Token& rhv) const;
    bool operator<= (const Token& rhv) const;
    bool operator>  (const Token& rhv) const;
    bool operator>= (const Token& rhv) const;
};

std::vector<Token> createTokens (const Text& text, const std::vector<Token>& terminalTokens);

std::istream& operator>> (std::istream& stream, TXTproc::Text& text);
std::ostream& operator<< (std::ostream& stream, const TXTproc::Text& text);
}
