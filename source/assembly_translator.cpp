#include "textlib.h"

#include "assembly.h"

#include <set>
#include <map>
#include <algorithm>

using namespace TXTproc;
using namespace Assembly;

class compilerError {};
class failedAttempt {};

static std::vector<TXTproc::Token> removeComments (const std::vector<TXTproc::Token>& code) {
    bool inComment = false;
    std::vector<TXTproc::Token> res;

    for (const auto& token : code) {
        if (inComment) {
            if (static_cast<std::string>(token) == "\n") {
                inComment = false;
                res.push_back(token);
            }
        } else {
            if (static_cast<std::string>(token) == ";") {
                inComment = true;
                continue;
            }

            res.push_back(token);
        }
    }

    return res;
}

static std::vector<TXTproc::Token> removeTokens (const std::vector<TXTproc::Token>& tokens, 
                                                 const std::vector<TXTproc::Token>& removable) {
    std::vector<Token> res;

    for (auto token : tokens) {
        if (std::find(removable.begin(), removable.end(), token) != removable.end()) continue;   
        res.push_back(token);
    }

    return res;
}

static std::map<std::string, Opcode> setOpcodes () {
    std::map<std::string, Opcode> res;
    for (auto opcode : assemblyOpcodes) res[opcode.name] = opcode;
    return res;
}
static std::map<std::string, Register> setRegisters () {
    std::map<std::string, Register> res;
    for (auto reg : assemblyRegisters) res[reg.name] = reg;
    return res;
}

const std::map<std::string, Register> registers = setRegisters ();
const std::map<std::string, Opcode>   opcodes   = setOpcodes();

static void getArgument (Argtype argtype, Argument& argument, const std::vector<TXTproc::Token>& code, size_t& idx) {
    if (argtype & ARG_REG) {
        auto it = registers.find(code[idx]);
        if (it != registers.end()) {
            idx++;
            argument.reg  = it->second;
            argument.type = ARG_REG;
            return;
        }
    }

    if (argtype & ARG_CST) {
        try {
            argument.constant = std::stoll(code[idx]);
            idx++;
            
            argument.type = ARG_CST;
            return;
        } catch (...) {}
    }

    throw compilerError();
}

static void tryCommand (NotLinkedModule& currentModule, const std::vector<TXTproc::Token>& code, size_t& idx) {
    auto it = opcodes.find(code[idx]);
    if (it == opcodes.end()) return;
    
    idx++;
    const Opcode& currentOpcode = it->second;
    Command newCommand;
    newCommand.commandid = currentOpcode.commandid;

    bool isFirst = true;

    for (auto argType : currentOpcode.argumentsType) {
        if (!isFirst) {
            if (static_cast<std::string>(code[idx]) != ",") throw compilerError();
            idx++;
        }
        
        newCommand.args.push_back(Argument());
        getArgument(argType, newCommand.args.back(), code, idx);
        
        isFirst = false;
    } 

    currentModule.code.push_back(newCommand);
}

NotLinkedModule Assembly::translateModuleFromTokens (const std::vector<TXTproc::Token>& code)  {
    NotLinkedModule res;

    for (size_t idx = 0; idx < code.size();) {
        size_t previdx = idx;

        tryCommand(res, code, idx);

        if (previdx == idx) throw compilerError();
    }

    return res;
}

template <typename T>
static void addToUintArray (std::vector<uint8_t>& array, const T& arg) {
    for (size_t i = 0; i < sizeof(T); i++) {
        array.push_back(((uint8_t*)&arg)[i]);
    }
}

std::vector<uint8_t> Assembly::linkModules (const std::vector<NotLinkedModule>& modules) {
    std::vector<uint8_t> res;

    const uint16_t logSize[] = {0, 0, 1, 1, 
                                2, 2, 2, 2, 3};

    for (const auto& currentModule : modules) {
        for (const auto& command : currentModule.code) {
            addToUintArray(res, (uint16_t)(command.commandid | (logSize[command.size] << 14)));

            for (const auto& arg : command.args) {
                switch (arg.type) {
                    case ARG_REG:
                        addToUintArray(res, (uint8_t)(arg.reg.regid | COMMAND_ARG_REG));
                        break;
                    case ARG_MEM:
                        break;
                    case ARG_CST:
                        addToUintArray(res, (uint8_t)COMMAND_ARG_CST);
                        addToUintArray(res, arg.constant);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    return res;
}

std::vector<uint8_t> Assembly::compileFromTokens (const std::vector<TXTproc::Token>& code_) {
    std::vector<TXTproc::Token> code;
    code = removeTokens(code_, {Token("\r")});
    code = removeComments(code);
    code = removeTokens(code, {Token(" "), Token("\t"), Token("\r"), Token("\n")});

    return linkModules({translateModuleFromTokens(code)});
}
