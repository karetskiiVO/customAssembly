#include "textlib.h"

#include "assembly.h"

#include <set>
#include <map>
#include <algorithm>

using namespace TXTproc;
using namespace Assembly;

class compilerError {
    Token failedToken;
    std::string message;
public:
    compilerError (const Token& failedToken, const std::string& message) 
        : failedToken(failedToken), message(message) {}

    const Token& getFailed () const {
        return failedToken;
    }

    const std::string& getMessage () const {
        return message;
    }
};

class linkerError {};

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

const std::map<std::string, Register> registers = setRegisters();
const std::map<std::string, Opcode>   opcodes   = setOpcodes();

static void getExpression (Argtype, Argument& argument, const std::vector<TXTproc::Token>& code, size_t& idx, bool useRegs) {
    bool startFlag = true;

    while (startFlag || code[idx] == "+" || code[idx] == "-") {
        int64_t multiplier = 1;
        startFlag = false;

        if (code[idx] == "+" || code[idx] == "-") {
            multiplier = (code[idx] == "+") ? 1 : -1;
            idx++;
        }

        try {
            argument.constantOffset.push_back(std::stoll(code[idx]));
            idx++;
        } catch (...) {
            auto registerIt = registers.find(code[idx]);
            if (registerIt != registers.end()) {
                if (!useRegs) throw compilerError(code[idx], "using of non-constant exprression is not allowed");

                switch (multiplier) {
                    case 1:
                    case 2:
                    case 4:
                    case 8:
                        argument.registers.push_back({multiplier, registerIt->second});
                        break;
                    default:
                        throw compilerError(code[idx], "incorrect register multiplier");
                }
            } else {
                argument.nonconstantOffset.push_back({multiplier, code[idx]});
            }

            idx++;
        }
    }
}

static void getArgument   (Argtype argtype, Argument& argument, const std::vector<TXTproc::Token>& code, size_t& idx) {
    if (argtype & ARG_REG) {
        auto it = registers.find(code[idx]);
        if (it != registers.end()) {
            idx++;
            argument.reg  = it->second;
            argument.type = ARG_REG;
            return;
        }
    }

    if (argtype & ARG_MEM) {
        if (code[idx] == "[") {
            idx++;
            argument.type = ARG_MEM;
            getExpression(argtype, argument, code, idx, true);
            if (code[idx] != "]") throw compilerError(code[idx], "\"]\" expected");
            idx++;
            return;
        }
    }

    if (argtype & ARG_CST) {
        argument.type = ARG_CST;
        getExpression(argtype, argument, code, idx, false);
        return;
    }

    throw compilerError(code[idx], "invalid argument");
}

static void tryLable     (NotLinkedModule& currentModule, const std::vector<TXTproc::Token>& code, size_t& idx) {
    if (idx + 2 > code.size()) return;
    if (code[idx + 1] != ":")  return;

    Command newPseudoCommand;
    newPseudoCommand.isPseudo = true;
    newPseudoCommand.commandid = PSEVDO_INSTR_ADD_LABLE;
    newPseudoCommand.pseudoArg.stringBuffer = code[idx];
    idx += 2;

    currentModule.code.push_back(newPseudoCommand);
}

static void tryCommand   (NotLinkedModule& currentModule, const std::vector<TXTproc::Token>& code, size_t& idx) {
    auto it = opcodes.find(code[idx]);
    if (it == opcodes.end()) return;
    
    idx++;
    const Opcode& currentOpcode = it->second;
    Command newCommand;
    newCommand.commandid = currentOpcode.commandid;

    // set size modifier to instruction
    if (currentOpcode.modifySize) {
        const std::map<std::string, uint8_t> sizeModifier = 
            {
                {"byte", 1}, {"word", 2}, {"dword", 4}, {"qword", 8}
            };
        
        auto it = sizeModifier.find(code[idx]);
        if (it != sizeModifier.end()) {
            newCommand.size = it->second;
            idx++;
        }
    } else {
        newCommand.size = 8;
    }

    bool isFirst = true;
    for (auto argType : currentOpcode.argumentsType) {
        if (!isFirst) {
            if (code[idx] != ",") throw compilerError(code[idx], "\",\" expected");
            idx++;
        }
        
        newCommand.args.push_back(Argument());
        getArgument(argType, newCommand.args.back(), code, idx);
        
        isFirst = false;
    } 

    currentModule.code.push_back(newCommand);
}

static void tryDirective (NotLinkedModule& currentModule, const std::vector<TXTproc::Token>& code, size_t& idx) {
    if (code[idx] == "global") {
        idx++;

        currentModule.code.push_back(Assembly::Command());
        currentModule.code.back().isPseudo  = true;
        currentModule.code.back().commandid = PSEVDO_INSTR_GLB_LABLE;
        currentModule.code.back().pseudoArg.stringBuffer = code[idx];
        
        idx++;
        return;
    }

    if (code[idx] == "extern") {
        idx++;

        currentModule.code.push_back(Assembly::Command());
        currentModule.code.back().isPseudo  = true;
        currentModule.code.back().commandid = PSEVDO_INSTR_EXT_LABLE;
        currentModule.code.back().pseudoArg.stringBuffer = code[idx];
        
        idx++;
        return;
    }
}

NotLinkedModule Assembly::translateModuleFromTokens (const std::vector<TXTproc::Token>& code)  {
    NotLinkedModule res;

    for (size_t idx = 0; idx < code.size();) {
        size_t previdx = idx;

        tryDirective(res, code, idx);
        tryLable(res, code, idx);
        tryCommand(res, code, idx);

        if (previdx == idx) throw compilerError(code[idx], "command or lable  is expected");
    }

    return res;
}

template <typename T>
static void addToUintArray (std::vector<uint8_t>& array, const T& arg) {
    for (size_t i = 0; i < sizeof(T); i++) {
        array.push_back(((uint8_t*)&arg)[i]);
    }
}

Binary Assembly::linkModules (const std::vector<NotLinkedModule>& modules) {
    Binary res;

    const uint16_t logSize[] = {0, 0, 1, 1, 
                                2, 2, 2, 2, 3};

    std::vector<std::map<std::string, uint64_t>> lableTable(modules.size());
    std::vector<std::vector<std::string>> externedLables(modules.size());
    std::map<std::string, uint64_t> globalLables;

    for (size_t moduleidx = 0; moduleidx < modules.size(); moduleidx++) {
        const auto& currentModule = modules[moduleidx];
        std::vector<std::string> newGlobalLables;

        for (const auto& command : currentModule.code) {
            if (command.isPseudo) {
                switch (command.commandid) {
                    case PSEVDO_INSTR_ADD_LABLE:
                        if (lableTable[moduleidx].find(command.pseudoArg.stringBuffer) != lableTable[moduleidx].end()) {
                            throw linkerError();
                        }
                        
                        lableTable[moduleidx][command.pseudoArg.stringBuffer] = res.bin.size();

                        break;
                    case PSEVDO_INSTR_EXT_LABLE:
                        externedLables[moduleidx].push_back(command.pseudoArg.stringBuffer);
                        break;
                    case PSEVDO_INSTR_GLB_LABLE:
                        newGlobalLables.push_back(command.pseudoArg.stringBuffer);
                        break;
                    default:
                        break;
                }

                continue;
            }

            addToUintArray(res.bin, (uint16_t)(command.commandid | (logSize[command.size] << 14)));

            for (const auto& arg : command.args) {
                switch (arg.type) {
                    case ARG_REG:
                        addToUintArray(res.bin, (uint8_t)(arg.reg.regid | COMMAND_ARG_REG));
                        break;
                    case ARG_MEM:
                        // check for maximum register counter
                        addToUintArray(res.bin, (uint8_t)(arg.registers.size() | COMMAND_ARG_MEM));
                        for (auto regPair : arg.registers) {
                            addToUintArray(res.bin, (uint8_t)(regPair.second.regid | (logSize[regPair.first] << 6)));
                        }
                        addToUintArray(res.bin,  (int64_t)0);
                        break;
                    case ARG_CST:
                        addToUintArray(res.bin, (uint8_t)COMMAND_ARG_CST);
                        addToUintArray(res.bin,  (int64_t)0);
                        break;
                    default:
                        break;
                }
            }
        }

        for (const auto& globalLable : newGlobalLables) {
            if (globalLables.find(globalLable) != globalLables.end()) throw linkerError(); // multi definition of global lables
            if (lableTable[moduleidx].find(globalLable) == lableTable[moduleidx].end()) throw linkerError(); // lable not defined

            globalLables[globalLable] = lableTable[moduleidx][globalLable];
        }
    }

    for (size_t moduleidx = 0; moduleidx < modules.size(); moduleidx++) {
        for (auto& externLable : externedLables[moduleidx]) {
            if (lableTable[moduleidx].find(externLable) != lableTable[moduleidx].end()) 
                throw linkerError();
            if (globalLables.find(externLable) == globalLables.end())
                throw linkerError();

            lableTable[moduleidx][externLable] = globalLables[externLable];
        }
    }

    res.bin.clear();

    for (size_t moduleidx = 0; moduleidx < modules.size(); moduleidx++) {
        const auto& currentModule = modules[moduleidx];
        for (const auto& command : currentModule.code) {
            if (command.isPseudo) continue;
            
            addToUintArray(res.bin, (uint16_t)(command.commandid | (logSize[command.size] << 14)));

            for (const auto& arg : command.args) {
                int64_t currentOffset = 0;
                switch (arg.type) {
                    case ARG_REG:
                        addToUintArray(res.bin, (uint8_t)(arg.reg.regid | COMMAND_ARG_REG));
                        break;
                    case ARG_MEM:
                        addToUintArray(res.bin, (uint8_t)(arg.registers.size() | COMMAND_ARG_MEM));
                        for (auto regPair : arg.registers) {
                            addToUintArray(res.bin, (uint8_t)(regPair.second.regid | (logSize[regPair.first] << 6)));
                        }

                        for (auto offset : arg.constantOffset) currentOffset += offset;
                        for (auto lable : arg.nonconstantOffset) {
                            if (lableTable[moduleidx].find(lable.second) == lableTable[moduleidx].end()) {
                                throw linkerError();
                            }

                            currentOffset += lable.first * lableTable[moduleidx][lable.second];
                        }

                        addToUintArray(res.bin, currentOffset);
                        break;
                    case ARG_CST:
                        addToUintArray(res.bin, (uint8_t)COMMAND_ARG_CST);
                        
                        for (auto offset : arg.constantOffset) currentOffset += offset;
                        for (auto lable : arg.nonconstantOffset) {
                            if (lableTable[moduleidx].find(lable.second) == lableTable[moduleidx].end()) {
                                throw linkerError();
                            }

                            currentOffset += lable.first * lableTable[moduleidx][lable.second];
                        }

                        addToUintArray(res.bin, currentOffset);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    auto startIt = globalLables.find("start");
    if (startIt == globalLables.end()) throw linkerError();
    res.start = startIt->second;

    return res;
}

Binary Assembly::compileNoErrors (const std::vector<TXTproc::Text>& sources) {
    std::vector<NotLinkedModule> modules;

    for (auto& sourceCode : sources) {
        const std::vector<Token> terminals = {
            Token(" "), Token("\t"), Token("\n"), Token("\r"),
            Token("+"), Token("-"), Token("*"), Token("/"), 
            Token(":"), Token(";"), Token(","), Token("["), Token("]")
        };

        auto code_ = createTokens(sourceCode, terminals);

        std::vector<TXTproc::Token> code;
        code = removeTokens(code_, {Token("\r")});
        code = removeComments(code);
        code = removeTokens(code, {Token(" "), Token("\t"), Token("\r"), Token("\n")});

        modules.push_back(translateModuleFromTokens(code));
    }

    return linkModules(modules);
}

Binary Assembly::compile (const std::vector<TXTproc::Text>& sources) {
    try {
        return compileNoErrors(sources);
    } catch (const compilerError& err) {
        // TODO: do good error presentation
        std::cout << err.getMessage() << err.getFailed().content() << "\n";
    }

    return Binary();
}
/*
std::vector<uint8_t> Assembly::compileFromTokens (const std::vector<TXTproc::Token>& code_) {
    std::vector<TXTproc::Token> code;
    code = removeTokens(code_, {Token("\r")});
    code = removeComments(code);
    code = removeTokens(code, {Token(" "), Token("\t"), Token("\r"), Token("\n")});

    try {
        NotLinkedModule buf = translateModuleFromTokens(code);
        return linkModules({buf});
    } catch (const compilerError& err) {
        size_t startTokenIdx = 0;
        for (; code_[startTokenIdx].getLine() < err.getFailed().getLine(); startTokenIdx++) {}

        std::string prefix = err.getFailed().filename() + "(" + std::to_string(err.getFailed().getLine()) 
                           + ", " + std::to_string(err.getFailed().getColumn()) + "): ";
        size_t messageOffset = prefix.length();
        std::cout << prefix;

        for (size_t i = startTokenIdx; code_[i].getLine() == err.getFailed().getLine() && i < code_.size(); i++) {
            std::cerr << code_[i].content(); 
        }

        for (size_t i = 0; i < messageOffset + err.getFailed().getColumn(); i++) std::cerr << "~";
        std::cerr << "^\n" << err.getMessage() << "\n";
    }

    return std::vector<uint8_t>();
}
*/