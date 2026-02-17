#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Metadata.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <optional>
#include <string_view>
#include <string>
#include <vector>
#include <filesystem>
#include <Nova/Core/core.h>

namespace Nova::Compiler {

    // ============================================================================
    // Project Management Types
    // ============================================================================

    enum class ProjectType {
        Executable,
        Library
    };

    enum class LibraryType {
        Static,
        Dynamic
    };

    struct Project {
        std::string name;
        std::vector<std::string> files;    // Source files for IR generation
        std::vector<std::string> headers;  // Headers for class organization and function definitions
        ProjectType type;
        std::optional<LibraryType> libType;
    };

    // ============================================================================
    // AST/Parser Types
    // ============================================================================

    struct FunctionDeclaration {
        std::string name;
        std::vector<std::string> args;
        std::string returnType;
        bool valid = true;
    };

    struct Function {
        std::string name;
        std::vector<std::string> args;
        std::string returnType;
        int offset = 0;  // Offset in source file (for error reporting)
    };


    struct ParseError {
        size_t line;
        size_t column;
        std::string message;
        std::string severity;

        std::string file;
        std::string snippet;
    };

    enum class TokenType {
        Identifier,
        Number,

        Plus, Minus, Star, Slash,
        Assign,

        Operator,

        Def, // Definition could bar var, func, ...

        LParen, RParen,
        LBrace, RBrace,
        Comma,
        End,
        Unknown
    };


    struct Token {
        std::string token;
        TokenType type;  
    };

    struct Assignment {
          std::vector<Token> tokens;   
    };

    struct ParseResult {
        std::vector<std::string> actions;
        bool valid = true;
        std::vector<ParseError> errors;
    };

    // ============================================================================
    // Compiler Class
    // ============================================================================

    class Compiler {
    public:
        // Constructor/Destructor
        Compiler();
        explicit Compiler(std::string_view configPath);
        ~Compiler();

        // Prevent copying
        Compiler(const Compiler&) = delete;
        Compiler& operator=(const Compiler&) = delete;

        // ========================================================================
        // Public API - Configuration
        // ========================================================================

        std::string findConfig();
        void parseConfig(std::string_view configPath);

        // ========================================================================
        // Public API - Compilation
        // ========================================================================

        void generateAll(std::string_view outputPath);
        void generateProject(const Project& project, std::string_view outputPath);
        std::vector<Assignment> codeParse(std::vector<Assignment> code);
        void generateCode(std::string code);
        
        std::string compileToIR(std::string_view filePath, std::string_view outputPath, llvm::Module* module = nullptr);

        // ========================================================================
        // Public API - Code Generation
        // ========================================================================

        void generateHeaders(std::string_view outputPath);
        void generateIR(llvm::Module* module, std::string_view sourcePath);

        // ========================================================================
        // Public API - Parsing (exposed for testing/debugging)
        // ========================================================================

        Function parseFunction(const std::vector<std::string>& lines, int funcLine, llvm::Module* module = nullptr);
        std::vector<std::string> tokenize(const std::string& line);

    private:
        // ========================================================================
        // Lexer/Parser Utilities
        // ========================================================================

        std::string trim(const std::string& str);
        std::vector<std::string> tokenize(const std::string& str) const;
        std::vector<std::string> splitStatements(const std::string& source);
        
        // ========================================================================
        // Function Parsing
        // ========================================================================

        std::vector<std::string> extractMultiLineBody(const std::vector<std::string>& lines, size_t startLine);
        FunctionDeclaration parseFunctionDeclaration(const std::string& decl);
        
        // ========================================================================
        // Type System
        // ========================================================================

        llvm::Type* novaTypeToLLVM(const std::string& novaType, llvm::LLVMContext& ctx);
        
        // ========================================================================
        // Code Generation
        // ========================================================================

        void generateFunctionBody(
            const std::vector<std::string>& code,
            llvm::Function* function,
            llvm::Type* returnType,
            const std::string& funcName,
            size_t funcLine,
            llvm::LLVMContext& ctx
        );
    public: // For now for testing
        std::vector<Assignment> splitCall(const std::string& line);

    private:
        // ========================================================================
        // Member Variables
        // ========================================================================

        std::vector<Project> _projects;
        llvm::LLVMContext context;
        
        NOVA_LOG_DEF("Compiler");
    };

} // namespace Nova::Compiler
