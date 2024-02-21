#include "virtualmachine_tools.h"

#include <cstdio>
#include <fstream>
#include <iostream>

int main () {
    using namespace VM;
    FILE* input = fopen("prog.bin", "rb");
    size_t programmSize, start;
    fread(&programmSize, sizeof(size_t), 1, input);
    fread(&start, sizeof(size_t), 1, input);

    std::vector<uint8_t> programm(programmSize);
    fread(programm.data(), sizeof(uint8_t), programmSize, input);
    fclose(input);    

    VirtualMachine vm;
    
    vm.loadBinary(programm, start);
    vm.run();

    return 0;   
}