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

    INSTR_CALL = 0x10,
    INSTR_RET = 0x11,
    INSTR_JMP = 0x12,

    PSEVDO_INSTR_ADD_LABLE   = 0xF01,
    PSEVDO_INSTR_PLACE_BYTES = 0xF02,
    PSEVDO_INSTR_EXT_LABLE   = 0xF10,
    PSEVDO_INSTR_GLB_LABLE   = 0xF11,
};

const std::vector<Opcode> assemblyOpcodes = 
    {
        { "add", INSTR_ADD, {ARG_VAR, ARG_ANY}},
        { "mov", INSTR_MOV, {ARG_VAR, ARG_ANY}},

        {"call", INSTR_CALL, {ARG_ANY}, false},
        { "ret", INSTR_RET ,        {}, false},
        { "jmp", INSTR_JMP , {ARG_ANY}, false},

        { "syscall", INSTR_SYSCALL, {}, false},
    };
const std::vector<Register> assemblyRegisters = 
    {
        {"rax", RAX_CODE}, {"rbx", RBX_CODE}, 
        {"rcx", RCX_CODE}, {"rdx", RDX_CODE},
        {"rsp", RSP_CODE}, {"rip", RIP_CODE}
    };

struct Binary {
    size_t start = 0;
    std::vector<uint8_t> bin;
};

NotLinkedModule translateModuleFromTokens (const std::vector<TXTproc::Token>& code);
Binary linkModules (const std::vector<NotLinkedModule>& modules);
Binary compileNoErrors (const std::vector<TXTproc::Text>& source);
Binary compile (const std::vector<TXTproc::Text>& sources);
}
