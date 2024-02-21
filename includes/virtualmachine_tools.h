#pragma once

#include "assembly.h"

#include <cstdint>

namespace VM {

#define REGISTER_NUM (1 << 6)
#define FLAGS_NUM 5

#define RAX (registers[Assembly::RAX_CODE])
#define RBX (registers[Assembly::RBX_CODE])
#define RCX (registers[Assembly::RCX_CODE])
#define RDX (registers[Assembly::RDX_CODE])
#define RDI (registers[Assembly::RDI_CODE])
#define RSI (registers[Assembly::RSI_CODE])

#define RSP (registers[Assembly::RSP_CODE])
#define RIP (registers[Assembly::RIP_CODE])

#define CF (flags[0])
#define AF (flags[1])
#define PF (flags[2])
#define ZF (flags[3])
#define SF (flags[4])

struct VirtualMachine {
    uint64_t registers[REGISTER_NUM];
    uint8_t* ram = nullptr;
    size_t ramsize = 0;
    
    uint8_t flags[FLAGS_NUM];

    VirtualMachine (size_t ramsize = 32768);
    ~VirtualMachine();

    void loadBinary (const std::vector<uint8_t>& bin, size_t start = 0);

    void run (); // may be frequency
};

}