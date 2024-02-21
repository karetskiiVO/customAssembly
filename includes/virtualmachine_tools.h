#pragma once

#include "assembly.h"

#include <cstdint>

namespace VM {

#define REGISTER_NUM (1 << 6)

#define RAX (registers[Assembly::RAX_CODE])
#define RBX (registers[Assembly::RBX_CODE])
#define RCX (registers[Assembly::RCX_CODE])
#define RDX (registers[Assembly::RDX_CODE])

#define RSP (registers[Assembly::RSP_CODE])
#define RIP (registers[Assembly::RIP_CODE])

struct VirtualMachine {
    uint64_t registers[REGISTER_NUM];
    uint8_t* ram = nullptr;
    size_t ramsize = 0;
    
    VirtualMachine (size_t ramsize = 32768);
    ~VirtualMachine();

    void loadBinary (const std::vector<uint8_t>& bin, size_t start = 0);

    void run (); // may be frequency
};

}