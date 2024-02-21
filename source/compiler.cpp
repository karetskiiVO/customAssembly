#include "textlib.h"
#include "assembly.h"

#include <map>
#include <cstdio>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>

int main (int argc, char* argv[]) {
    using namespace TXTproc;
    
    std::vector<Text> src;
    // rework with good lib
    for (int i = 1; i < argc; i++) {
        std::ifstream input(argv[i]);
        
        Text text;
        input >> text;
        src.push_back(text);
    }
    
    auto prog = Assembly::compile(src);
    size_t size = prog.bin.size();

    FILE* output = fopen("prog.bin", "wb");
    fwrite(&size, sizeof(size_t), 1, output);
    fwrite(&prog.start, sizeof(size_t), 1, output);
    fwrite(prog.bin.data(), sizeof(uint8_t), size, output);
    fclose(output);

    return 0;
}