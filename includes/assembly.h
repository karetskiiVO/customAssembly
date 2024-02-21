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

    RSI_CODE = 0x0C,
    RDI_CODE = 0x0D,

    RSP_CODE = 0x3E,
    RIP_CODE = 0x3F,
};

enum InstructionID {
    INSTR_ADD = 0x00,
    INSTR_SUB = 0x01,
    INSTR_MUL = 0x02,
    INSTR_DIV = 0x03,
    
    INSTR_INC = 0x0D, 
    INSTR_DEC = 0x0E,
    INSTR_CMP = 0x0F,
    
    INSTR_MOV  = 0x30,
    INSTR_PUSH = 0x31,
    INSTR_POP  = 0x32,

    INSTR_SYSCALL = 0xFF,

    INSTR_CALL = 0x10,
    INSTR_RET = 0x11,
    INSTR_JMP = 0x12,
    INSTR_JE  = 0x14,
    INSTR_JNE = 0x15,

    PSEVDO_INSTR_ADD_LABLE   = 0xF01,
    PSEVDO_INSTR_PLACE_BYTES = 0xF02,
    PSEVDO_INSTR_EXT_LABLE   = 0xF10,
    PSEVDO_INSTR_GLB_LABLE   = 0xF11,
};

const std::vector<Opcode> assemblyOpcodes = 
    {
        { "add", INSTR_ADD, {ARG_VAR, ARG_ANY}},
        { "sub", INSTR_DIV, {ARG_VAR, ARG_ANY}},
        { "mul", INSTR_MUL, {ARG_VAR, ARG_ANY}},
        { "div", INSTR_MUL, {ARG_VAR, ARG_ANY}},
        { "inc", INSTR_INC, {ARG_VAR}},
        { "dec", INSTR_DEC, {ARG_VAR}},
        { "cmp", INSTR_CMP, {ARG_ANY, ARG_ANY}},

        { "mov", INSTR_MOV , {ARG_VAR, ARG_ANY}},
        {"push", INSTR_PUSH, {ARG_NMEM}, false},
        { "pop", INSTR_POP , {ARG_NMEM}, false},

        {"call", INSTR_CALL, {ARG_ANY}, false},
        { "ret", INSTR_RET ,        {}, false},
        { "jmp", INSTR_JMP , {ARG_ANY}, false},

        {  "je", INSTR_JE  , {ARG_ANY}, false},
        { "jne", INSTR_JNE , {ARG_ANY}, false},

        { "syscall", INSTR_SYSCALL, {}, false},
    };
const std::vector<Register> assemblyRegisters = 
    {
        {"rax", RAX_CODE}, {"rbx", RBX_CODE}, 
        {"rcx", RCX_CODE}, {"rdx", RDX_CODE},
        {"rdi", RDI_CODE}, {"rsi", RSI_CODE},
        {"rsp", RSP_CODE}, {"rip", RIP_CODE},
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
