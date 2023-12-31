#pragma once

#include "assemblytypes.h"
#include "textlib.h"

namespace Assembly {
enum CommandArgType {
    COMMAND_ARG_REG = 0b00 << 6,
    COMMAND_ARG_CST = 0b01 << 6,
    COMMAND_ARG_MEM = 0b10 << 6,
};

enum RegisterCodes {
    RAX_CODE = 0x00,
    RBX_CODE = 0x01,
    RCX_CODE = 0x02,
    RDX_CODE = 0x03,

    RSP_CODE = 0x3E,
    RIP_CODE = 0x3F,
};

enum InstructionID {
    INSTR_ADD = 0x00,
    INSTR_MOV = 0x30,
    INSTR_SYSCALL = 0xFF,
};

const std::vector<Opcode> assemblyOpcodes = 
    {
        { "add", INSTR_ADD, {ARG_VAR, ARG_ANY}},

        { "mov", INSTR_MOV, {ARG_VAR, ARG_ANY}},
        { "syscall", INSTR_SYSCALL, {}},
    };
const std::vector<Register> assemblyRegisters = 
    {
        {"rax", RAX_CODE}, {"rbx", RBX_CODE}, 
        {"rcx", RCX_CODE}, {"rdx", RDX_CODE},
        {"rsp", RSP_CODE}, {"rip", RIP_CODE}
    };


NotLinkedModule translateModuleFromTokens (const std::vector<TXTproc::Token>& code);
std::vector<uint8_t> linkModules (const std::vector<NotLinkedModule>& modules);
std::vector<uint8_t> compileFromTokens (const std::vector<TXTproc::Token>& code_);
}
