#include "textlib.h"
#include "assembly.h"

#include <map>
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

int main () {
    using namespace TXTproc;
    std::ifstream input("input.txt");

    Text text;
    input >> text;
    std::vector<Token> terminals = {
        Token(" "), Token("\t"), Token("\n"), Token("\r"),
        Token("+"), Token("-"), Token("*"), Token("/"), 
        Token(":"), Token(";"), Token(","), Token("["), Token("]")
    };
    
    auto tokens = createTokens(text, terminals);
    
    auto bin = Assembly::compileFromTokens(tokens);
    size_t size = bin.size();

    FILE* output = fopen("prog.bin", "wb");
    fwrite(&size, sizeof(size_t), 1, output);
    fwrite(bin.data(), sizeof(uint8_t), size, output);
    fclose(output);

    return 0;
}