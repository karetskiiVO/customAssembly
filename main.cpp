#include "textlib.h"

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

std::string convertToken (const TXTproc::Token& token) {
    static const std::map<std::string, std::string> convert = {
        {"\n", "\\n"}, {"\t", "\\t"}
    };
    const std::string& str = static_cast<const std::string&>(token);

    auto it = convert.find(str);
    return (it == convert.end()) ? str : it->second;
}

int main () {
    using namespace TXTproc;
    std::ifstream input("input.txt");

    Text text;
    input >> text;
    std::vector<Token> terminals = {
        Token(" "), Token("\n"), Token("\r\n"), 
        Token("("), Token(")"), Token("["), Token("]"),
        Token("+"), Token("-"), Token("/"), Token("*"), Token("^"),
        Token(","), Token(";")
    };
    auto tokens = createTokens(text, terminals);
    
    for (auto token : tokens) {
        if (token.content() == " " || token.content() == "\n" || token.content() == "\r\n") continue;

        std::cout << "(" << token.getLine() + 1 << ",\t" << token.getColumn() + 1 
             << "):\t\"" << convertToken(token) << "\"\n";
    }

    return 0;
}