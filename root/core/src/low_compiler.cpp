#include "compiler.h"
#include "logger.h"
#include <algorithm>
#include <sstream>
#include <sys/select.h>
#include <unordered_set>
#include <vector>

namespace Nova::Compiler {

// Helper function to trim whitespace from strings
std::string Compiler::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// Tokenize a string by whitespace
std::vector<std::string> Compiler::tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Split statements by semicolon, trimming whitespace
std::vector<std::string> Compiler::splitStatements(const std::string& source) {
    std::vector<std::string> statements;
    size_t start = 0;
    size_t pos;
    
    while ((pos = source.find(";", start)) != std::string::npos) {
        std::string stmt = source.substr(start, pos - start);
        std::string trimmed = trim(stmt);
        if (!trimmed.empty()) {
            statements.push_back(trimmed);
        }
        start = pos + 1;
    }
    
    // Handle remaining content after last semicolon
    if (start < source.size()) {
        std::string stmt = source.substr(start);
        std::string trimmed = trim(stmt);
        if (!trimmed.empty()) {
            statements.push_back(trimmed);
        }
    }
    
    return statements;
}

// Extract function body code from multi-line function
std::vector<std::string> Compiler::extractMultiLineBody(const std::vector<std::string>& lines, size_t startLine) {
    std::vector<std::string> code;
    uint32_t braceDepth = 0;
    
    for (size_t i = startLine; i < lines.size(); i++) {
        const auto& line = lines[i];
        
        // Track brace depth
        if (line.find('{') != std::string::npos) {
            braceDepth++;
        }
        if (line.find('}') != std::string::npos) {
            if (braceDepth == 0) break; // Found closing brace of function
            braceDepth--;
        }
        
        // Remove semicolons and add statements
        std::string cleanLine = line;
        size_t semiPos = line.find(';');
        if (semiPos != std::string::npos) {
            cleanLine = line.substr(0, semiPos);
        }
        
        std::string trimmed = trim(cleanLine);
        if (!trimmed.empty()) {
            code.push_back(trimmed);
        }
    }
    
    return code;
}

// Parse function declaration to extract name, args, and return type
FunctionDeclaration Compiler::parseFunctionDeclaration(const std::string& decl) {
    FunctionDeclaration result;
    
    auto tokens = tokenize(decl);
    if (tokens.size() < 2 || tokens[0] != "func") {
        NCERROR("Invalid function definition");
        result.valid = false;
        return result;
    }
    
    // Extract function name
    size_t funcPos = decl.find("func");
    size_t parenOpen = decl.find("(", funcPos);
    if (parenOpen == std::string::npos) {
        NCERROR("Missing opening parenthesis in function declaration");
        result.valid = false;
        return result;
    }
    
    result.name = trim(decl.substr(funcPos + 4, parenOpen - (funcPos + 4)));
    
    // Extract arguments
    size_t parenClose = decl.find(")", parenOpen);
    if (parenClose == std::string::npos) {
        NCERROR("Missing closing parenthesis in function declaration");
        result.valid = false;
        return result;
    }
    
    std::string argsStr = decl.substr(parenOpen + 1, parenClose - (parenOpen + 1));
    if (!trim(argsStr).empty()) {
        std::stringstream ss(argsStr);
        std::string arg;
        while (std::getline(ss, arg, ',')) {
            std::string trimmedArg = trim(arg);
            if (!trimmedArg.empty()) {
                result.args.push_back(trimmedArg);
            }
        }
    }
    
    // Extract return type (default to "int" if not specified)
    size_t arrowPos = decl.find("->", parenClose);
    if (arrowPos != std::string::npos) {
        size_t bracePos = decl.find('{');
        std::string retType;
        if (bracePos != std::string::npos) {
            retType = decl.substr(arrowPos + 2, bracePos - (arrowPos + 2));
        } else {
            retType = decl.substr(arrowPos + 2);
        }
        result.returnType = trim(retType);
    } else {
        result.returnType = "int";
    }
    
    return result;
}

// Convert Nova type to LLVM type
llvm::Type* Compiler::novaTypeToLLVM(const std::string& novaType, llvm::LLVMContext& ctx) {
    if (novaType == "int") return llvm::Type::getInt64Ty(ctx);
    if (novaType == "void") return llvm::Type::getVoidTy(ctx);
    if (novaType == "i8") return llvm::Type::getInt8Ty(ctx);
    if (novaType == "i16") return llvm::Type::getInt16Ty(ctx);
    if (novaType == "i32") return llvm::Type::getInt32Ty(ctx);
    if (novaType == "i64") return llvm::Type::getInt64Ty(ctx);
    if (novaType == "float") return llvm::Type::getFloatTy(ctx);
    if (novaType == "double") return llvm::Type::getDoubleTy(ctx);
    return nullptr; // Unknown type
}

// Generate LLVM IR for function body
void Compiler::generateFunctionBody(
    const std::vector<std::string>& code,
    llvm::Function* function,
    llvm::Type* returnType,
    const std::string& funcName,
    size_t funcLine,
    llvm::LLVMContext& ctx
) {
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", function);
    llvm::IRBuilder<> builder(entry);
    
    bool hasReturn = false;


    std::vector<Assignment> assignments;

    for (const auto& line : code) {
        std::vector<Assignment> lineAssignments = splitCall(line);
        assignments.insert(assignments.end(), lineAssignments.begin(), lineAssignments.end());
    }
    
    for (const auto& Token : assignments) {
        
        for (const auto& token : Token.tokens) {

            

        };

        // Parse function call

        
        // Skip closing braces
        
        // std::vector<std::string> line = splitCall(Iline);
        // auto closeBracket = std::find(line.begin(), line.end(), "{");

        // if (closeBracket != line.end()) {
        //    std::size_t index = std::distance(line.begin(), closeBracket);
            
        // }
        
        // Parse return statement
        // if (line.find("ret") != std::string::npos) {
        //     size_t retPos = line.find("ret");
        //     std::string retValue = trim(line.substr(retPos + 3));
            
        //     if (returnType->isIntegerTy()) {
        //         int64_t value = 0;
        //         if (!retValue.empty()) {
        //             value = std::stoll(retValue);
        //         }
        //         builder.CreateRet(llvm::ConstantInt::get(returnType, value));
        //     } else if (returnType->isVoidTy()) {
        //         builder.CreateRetVoid();
        //     } else if (returnType->isFloatingPointTy()) {
        //         double value = 0.0;
        //         if (!retValue.empty()) {
        //             value = std::stod(retValue);
        //         }
        //         builder.CreateRet(llvm::ConstantFP::get(returnType, value));
        //     }
            
        //     hasReturn = true;
        // }
    }
    
    // Add default return if missing
    if (!hasReturn) {
        _ncwarn();
        std::cout << termcolor::grey << termcolor::bold 
                  << fmt::format("      [func {}:{}] ", funcName, funcLine + 1) 
                  << termcolor::reset 
                  << "Function has no return statement (default 'int' has been written)" 
                  << std::endl;
        
        if (returnType->isVoidTy()) {
            builder.CreateRetVoid();
        } else if (returnType->isIntegerTy()) {
            builder.CreateRet(llvm::ConstantInt::get(returnType, 0));
        } else if (returnType->isFloatingPointTy()) {
            builder.CreateRet(llvm::ConstantFP::get(returnType, 0.0));
        }
    }

    


}




// This function gets the string (line) and then tokenizes it.
// std::string {"x = test()"} -> std::vector<Token> [{Ident, "x"}, {Assign, "="}, {Ident, test}]
// Ident is just a definition in a way, ident on itself could be a variable, class, namespace or function

// This works only inside functions
std::vector<Assignment> Compiler::splitCall(const std::string& line) {
    std::vector<Assignment> assignments;
    Assignment current{};
    std::string buffer;

    // Set of keywords
    const std::unordered_set<std::string> keywords = {"var", "int", "void", "ret", "const"};

    auto flushBuffer = [&]() {
        if (buffer.empty()) return;

        // Check if buffer is a keyword
        TokenType type;
        if (keywords.contains(buffer)) {
            type = TokenType::Def;
        } else if (std::isdigit(buffer[0])) {
            type = TokenType::Number;
        } else {
            type = TokenType::Identifier;
        }

        current.tokens.push_back(Token{.token = buffer, .type = type});
        buffer.clear();
    };

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        char next = (i + 1 < line.size()) ? line[i + 1] : '\0';

        switch (c) {
            case '+': flushBuffer(); current.tokens.push_back(Token{.token = "+", .type = TokenType::Plus}); break;
            case '-': flushBuffer(); current.tokens.push_back(Token{.token = "-", .type = TokenType::Minus}); break;
            case '*': flushBuffer(); current.tokens.push_back(Token{.token = "*", .type = TokenType::Star}); break;
            case '/': flushBuffer(); current.tokens.push_back(Token{.token = "/", .type = TokenType::Slash}); break;

            case '=':
                flushBuffer();
                if (next == '=') {
                    current.tokens.push_back(Token{.token = "==", .type = TokenType::Operator});
                    ++i;
                } else {
                    current.tokens.push_back(Token{.token = "=", .type = TokenType::Assign});
                }
                break;

            case '(' : flushBuffer(); current.tokens.push_back(Token{.token = "(", .type = TokenType::LParen}); break;
            case ')' : flushBuffer(); current.tokens.push_back(Token{.token = ")", .type = TokenType::RParen}); break;
            case '{' : flushBuffer(); current.tokens.push_back(Token{.token = "{", .type = TokenType::LBrace}); break;
            case '}' : flushBuffer(); current.tokens.push_back(Token{.token = "}", .type = TokenType::RBrace}); break;
            case ',' : flushBuffer(); current.tokens.push_back(Token{.token = ",", .type = TokenType::Comma}); break;
            case ';' :
                flushBuffer();
                assignments.push_back(current);
                current = Assignment{};
                break;

            case ' ':
            case '\t':
            case '\n':
                flushBuffer();
                break;

            default:
                buffer += c;
                break;
        }
    }

    flushBuffer();
    if (!current.tokens.empty()) assignments.push_back(current);
    return assignments;
}



std::vector<Assignment> Compiler::codeParse(std::vector<Assignment> code) {

    int assID = 0; // Assignment Index
    int TokenID = 0; // Token Index

    for (Assignment& assignment : code) {
        for (Token& token : assignment.tokens) {
            
            TokenID++;
        }
        assID++;
    };


    return code;
}



} // namespace Nova::Compilerb
