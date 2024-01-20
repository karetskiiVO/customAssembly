#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

namespace Assembly {

using regid_t = uint8_t;
using commandid_t = uint64_t; 

enum Argtype {
    ARG_REG = 0b001,
    ARG_CST = 0b010,
    ARG_MEM = 0b100,
    ARG_VAR = ARG_REG | ARG_MEM,
    ARG_ANY = ARG_VAR | ARG_CST,
};

struct Opcode {
    std::string name;
    commandid_t commandid;
    std::vector<Argtype> argumentsType;

    bool modifySize = true;
};

struct Register {
    std::string name;
    regid_t regid;
};

struct Argument {
    Argtype type;

    std::vector<std::pair<int8_t, Register>> registers;
    std::vector<int64_t> constantOffset;
    std::vector<std::pair<int64_t, std::string>> nonconstantOffset;
    
    Register reg;
};

struct PseudoArgument {
    std::string stringBuffer;
};

struct Command {
    commandid_t commandid;
    
    uint8_t size = 8;
    std::vector<Argument> args;

    bool isPseudo = false;
    PseudoArgument pseudoArg;
};

struct NotLinkedAddress {

};

struct NotLinkedModule {
    std::vector<Command> code;
};

}