#include "virtualmachine_tools.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <functional>

using namespace VM;

#define MAX_ARGC 16

static void* getArgument (VirtualMachine& vm, uint64_t& ip) {
    uint8_t argDt = vm.ram[ip];
    ip += sizeof(uint8_t);

    void* res = nullptr;
    switch (argDt & (0b11 << 6)) {
        case Assembly::COMMAND_ARG_CST:
            res = (uint8_t*)vm.ram + ip;
            ip += sizeof(uint64_t);
            break;
        case Assembly::COMMAND_ARG_MEM:
            res = vm.ram;
            for (uint8_t regCnt = 0; regCnt < (argDt & (~(0b11 << 6))); regCnt++) {
                uint8_t regid = *((uint8_t*)vm.ram + ip) & (~(0b11 << 6));
                uint8_t mult  = 1 << (*((uint8_t*)vm.ram + ip) >> 6);
                res = (uint8_t*)res + vm.registers[regid] * mult;
                ip += sizeof(uint8_t);
            }
            res = (uint8_t*)res + *(uint64_t*)((uint8_t*)vm.ram + ip);
            ip += sizeof(uint64_t);
            break;
        case Assembly::COMMAND_ARG_REG:
            res = vm.registers + (argDt & (~(0b11 << 6)));
            break;
        default:
            break;
    }

    return res;
}

static std::vector<size_t> instructionArgc () {
    std::vector<size_t> res(0xFFFF);
    for (auto opcode : Assembly::assemblyOpcodes) {
        res[opcode.commandid] = opcode.argumentsType.size();
    }

    return res;
}

VirtualMachine::VirtualMachine (size_t ramsize) : ramsize(ramsize) {
    ram = new uint8_t[ramsize];
}

VirtualMachine::~VirtualMachine () {
    delete[] ram;
}

void VirtualMachine::loadBinary (const std::vector<uint8_t>& bin, size_t start) {
    if (bin.size() > ramsize) throw std::length_error("Not enought space to load programm");

    RIP = start; // start lable in future
    memcpy(ram, bin.data(), sizeof(uint8_t) * bin.size());
    RSP = ramsize; // rsp also must be allocated in bin
} 

//extern "C" uint64_t getFlags();

template <typename T>
static void updateFlags (VirtualMachine& vm, T val) {
    auto& flags = vm.flags;

    SF = !!(val & ((T)1 << (T)(sizeof(T) - 1)));
    ZF = !!val;
    PF = !(val & (T)0b01);
}

static void add (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) += *((uint8_t*)args[1]);
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) += *((uint16_t*)args[1]);
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) += *((uint32_t*)args[1]);
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) += *((uint64_t*)args[1]);
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void sub (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) -= *((uint8_t*)args[1]);
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) -= *((uint16_t*)args[1]);
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) -= *((uint32_t*)args[1]);
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) -= *((uint64_t*)args[1]);
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void mul (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) *= *((uint8_t*)args[1]);
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) *= *((uint16_t*)args[1]);
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) *= *((uint32_t*)args[1]);
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) *= *((uint64_t*)args[1]);
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void div_ (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) /= *((uint8_t*)args[1]);
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) /= *((uint16_t*)args[1]);
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) /= *((uint32_t*)args[1]);
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) /= *((uint64_t*)args[1]);
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void inc (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) += 1;
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) += 1;
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) += 1;
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) += 1;
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void dec (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            *((uint8_t*)args[0]) -= 1;
            updateFlags(vm, *((uint8_t*)args[0]));
            break;
        case 2:
            *((uint16_t*)args[0]) -= 1;
            updateFlags(vm, *((uint16_t*)args[0]));
            break;
        case 4:
            *((uint32_t*)args[0]) -= 1;
            updateFlags(vm, *((uint32_t*)args[0]));
            break;
        case 8:
            *((uint64_t*)args[0]) -= 1;
            updateFlags(vm, *((uint64_t*)args[0]));
            break;
        default:
            break;
    }
}

static void cmp (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    switch (size) {
        case 1:
            updateFlags(vm, *((uint8_t*)args[0]) - *((uint8_t*)args[1]));
            break;
        case 2:
            updateFlags(vm, *((uint16_t*)args[0]) - *((uint16_t*)args[1]));
            break;
        case 4:
            updateFlags(vm, *((uint32_t*)args[0]) - *((uint32_t*)args[1]));
            break;
        case 8: 
            updateFlags(vm, *((uint64_t*)args[0]) - *((uint64_t*)args[1]));
            break;
        default:
            break;
    }
}

static void mov (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    if (args[0] == args[1]) return;
    memcpy(args[0], args[1], size);
}

static void push (VirtualMachine& vm, void** args, uint8_t, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;
    memcpy(vm.ram + RSP - sizeof(uint64_t), args[0], sizeof(uint64_t));
    RSP -= sizeof(uint64_t);
}

static void pop (VirtualMachine& vm, void** args, uint8_t, uint64_t nextrip) {
    auto& registers = vm.registers;
    RIP = nextrip;

    memcpy(args[0], vm.ram + RSP, sizeof(uint64_t));
    RSP += sizeof(uint64_t);
}

static void jmp (VirtualMachine& vm, void** args, uint8_t size, uint64_t) {
    auto& registers = vm.registers;
    memcpy(&RIP, args[0], size);
}

static void call (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    memcpy(&RIP, args[0], size);
    memcpy(vm.ram + RSP - sizeof(uint64_t), &nextrip, sizeof(uint64_t));
    RSP -= sizeof(uint64_t);
}

static void ret (VirtualMachine& vm, void**, uint8_t, uint64_t) {
    auto& registers = vm.registers;
    memcpy(&RIP, vm.ram + RSP, sizeof(uint64_t));
    RSP += sizeof(uint64_t);
}

static void je (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    auto& flags = vm.flags;

    if (!ZF) memcpy(&RIP, args[0], size);
    else RIP = nextrip;
}

static void jne (VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip) {
    auto& registers = vm.registers;
    auto& flags = vm.flags;

    if (ZF) memcpy(&RIP, args[0], size);
    else RIP = nextrip;
}

static void syscall (VirtualMachine& vm, void**, uint8_t, uint64_t nextrip) {
    auto& registers = vm.registers;
    
    if (RAX == 1) {
        if (RBX == 1) {
            std::cout << RDX;
        }
    }
    if (RAX == 0) exit(0);

    RIP = nextrip;
}

void VirtualMachine::run () {
    std::map<uint16_t, std::function<void(VirtualMachine& vm, void** args, uint8_t size, uint64_t nextrip)>> instructions = 
        {
            {Assembly::INSTR_ADD, add}, 
            {Assembly::INSTR_SUB, sub},
            {Assembly::INSTR_MUL, mul},
            {Assembly::INSTR_DIV, div_},
            {Assembly::INSTR_INC, inc},
            {Assembly::INSTR_DEC, dec},
            {Assembly::INSTR_CMP, cmp},

            {Assembly::INSTR_MOV, mov},
            {Assembly::INSTR_PUSH, push},
            {Assembly::INSTR_POP, pop},
            
            {Assembly::INSTR_JMP, jmp},

            {Assembly::INSTR_JE , je},
            {Assembly::INSTR_JNE, jne},


            {Assembly::INSTR_RET , ret},
            {Assembly::INSTR_CALL, call},

            {Assembly::INSTR_SYSCALL, syscall}
        };
    
    auto instrArgc = instructionArgc();

    while (true) {
        uint16_t command    = (*(uint16_t*)(ram + RIP)) & ((1 << 14) - 1);
        uint8_t commandsize = 1 << ((*(uint16_t*)(ram + RIP)) >> 14);

        size_t bufferip = RIP + sizeof(uint16_t);
        void* args[MAX_ARGC];

        for (size_t i = 0; i < instrArgc[command]; i++) {
            args[i] = getArgument(*this, bufferip);
        }

        instructions[command](*this, args, commandsize, bufferip);
    }      
};
