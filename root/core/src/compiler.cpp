#include "compiler.h"
#include "core.h"
#include "logger.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <filesystem>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/TargetParser/Triple.h>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tao/config.hpp>
#include <termcolor/termcolor.hpp>
#include <vector>
#include <fstream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>


namespace Nova::Compiler {

    Compiler::~Compiler() {

    };

    Compiler::Compiler(std::string_view configPath) {
        const auto config = tao::config::from_file(configPath);
        NCINFO("Custom config file loading not yet supported");
    }

    Compiler::Compiler() {
        const auto configPath = findConfig();
        if (configPath.empty()) {
            NCINFO("No configuration file found - using defaults");
            return;
        }

        parseConfig(configPath);
    }

    std::string Compiler::findConfig() {
        const auto& cwd = std::filesystem::current_path();
        if (std::filesystem::exists(cwd / WORKING_DIR)) {
            NCINFO("Found working directory: {}", (cwd / WORKING_DIR).string());
            if (std::filesystem::exists(cwd / WORKING_DIR / "nc.conf")) {
                NCINFO("Loading config: {}", (cwd / WORKING_DIR / "nc.conf").string());
                return (cwd / WORKING_DIR / "nc.conf").string();
            }else {
                NWARN("No nc.conf found in working directory");
                return "";
            }
        }else {
            NWARN("Working directory {} does not exist", (cwd / WORKING_DIR).string());
            return "";
        }

        return "";
    }

    void Compiler::parseConfig(std::string_view configPath) {
        const auto config = tao::config::from_file(configPath);

        const auto& projects = config.at("projects");

        const std::string projectDir = config.get_object().at("projectDir").get_string();
        const auto absoluteProjectDir = std::filesystem::absolute(projectDir);

        NCINFO("Project root: {}", absoluteProjectDir.string());

        for (const auto& [projectName, projectConfig] : projects.get_object()) {
            Project project;

            NCINFO("◁ ─┬─Project: {}───▷", projectName);
            project.name = projectName;

            const auto typeStr = projectConfig.at("type").get_string();
            if (typeStr == "exec") {
                project.type = ProjectType::Executable;
                NCINFO("  ├▶ Type: Executable");
            }else if (typeStr == "lib") {
                project.type = ProjectType::Library;
                NCINFO("  ├▶ Type: Library");
                const auto libTypeStr = projectConfig.at("library_type").get_string();
                if (libTypeStr == "static") {
                    project.libType = LibraryType::Static;
                    NCINFO("  ├▶ Linkage: Static");
                }else if (libTypeStr == "dynamic") {
                    project.libType = LibraryType::Dynamic;
                    NCINFO("  ├▶ Linkage: Dynamic");
                }else {
                    NWARN("  ├▶ Unknown linkage '{}' - defaulting to static", libTypeStr);
                    project.libType = LibraryType::Static;
                }
            }

            const auto sourceDir = absoluteProjectDir / projectConfig.at("sourceDir").get_string();
            
            if (!std::filesystem::exists(sourceDir)) {
                NERROR("  ├▶ Source directory does not exist: {}", sourceDir.string());
                continue;
            }

            NCINFO("  ├▶ Source directory: {}", sourceDir.string());

            std::string sourceFileExt = ".nl";
            if (projectConfig.at("sourceFiles").is_string()) {
                sourceFileExt = projectConfig.at("sourceFiles").get_string();
            }else{
                NCINFO("  ├▶ Extension filter: .nl (default)");
            }

            NCINFO("  └─┐Source Files:");
            int size = 0;
            for (const auto& file : std::filesystem::directory_iterator(sourceDir)) {
                if (file.path().extension() == sourceFileExt) {
                    project.files.push_back(file.path().string());
                    if (size + 1 == project.files.size()) {
                        NCINFO("    └─➤ {}", file.path().filename().string());
                    }else {
                        NCINFO("    ├─➤ {}", file.path().filename().string());
                    };
                    size++;
                }
                
            }

            NCINFO("  ┌▶ Header Files:");
            //for (const auto& header : projectConfig.at("headers").get_array()) {
            //    project.headers.push_back(header.get_string());
            //    NCINFO("    + {}", header.get_string())
            //}


            NCINFO("  └▶ Done");

            _projects.push_back(project);
            
        }
    }

    void Compiler::generateAll(std::string_view outputPath) {
        for (const auto& project : _projects) {
            generateProject(project, outputPath);
        }
    }

    void Compiler::generateProject(const Project& project, std::string_view outputPath) {
        NCINFO("◁ ─┬─Compiling: {}───▷", project.name);
        int x = 0;
        for (const auto& file : project.files){
            std::string log;
            if (project.files.size() != (x+1)) {log = fmt::format("   ├─➤ {}", std::filesystem::path(file).filename().string());}
            else {log = fmt::format("   └─➤ {}", std::filesystem::path(file).filename().string());}
            NCINFO("{}", log);
            auto module = std::make_unique<llvm::Module>(project.name, context);

            // compileToIR(file, outputPath, module.get());

            // Write the IR to a file
            std::filesystem::path irPath = std::filesystem::path(outputPath) / (std::filesystem::path(file).stem().string() + ".ll");
            std::ofstream irFile(irPath);
            if (irFile.is_open()) {
                irFile << compileToIR(file, outputPath, module.get());
                irFile.close();
            }else {
                NERROR("  Failed to write IR file: {}", irPath.string());
            }

            if (llvm::verifyModule(*module, &llvm::errs())) {
                NERROR("  Module verification failed aborting");
                return;
                assert(false);
            }else {
                // printf("\033[%dA\r\033[%zuC   \033[1;97;42m ✔ DONE \033[0m\n", logLinesPrinted, offset);
                // NCINFO("\033[{}A\r\033[{}C   \033[1;97;42m ✔ DONE \033[0m\n", logLinesPrinted, offset);
            }
            x++;
        }
        NCINFO("◁ ───Finished compiling: {}───▷", project.name);
    }

    std::string Compiler::compileToIR(std::string_view filePath, std::string_view outputPath, llvm::Module* module) {

    
        llvm::IRBuilder<> builder(context);

        llvm::Triple triple("aarch64-unknown-linux-gnu");
        module->setTargetTriple(triple);
        module->setDataLayout("e-m:e-i64:64-i128:128-n32:64-S128");
        module->setSourceFileName(std::filesystem::path(filePath).filename().string());




        generateIR(module, filePath);

        std::string ir;
        llvm::raw_string_ostream rso(ir);
        module->print(rso, nullptr);
        rso.flush();

        return ir;
    }


    void Compiler::generateHeaders(std::string_view outputPath) {
        NCINFO("Generating headers to {}", outputPath);
    }

    void Compiler::generateIR(llvm::Module* module, std::string_view sourcePath) {

        // open source file

        std::string filename = std::filesystem::path(sourcePath).string();

        std::ifstream file((std::filesystem::path(sourcePath)));
        if (!file.is_open()) {
            NERROR("Failed to open source file: {}", sourcePath);
            return;
        }

        std::string ir;

        std::string line;
        int lineNumber = 0;
        std::vector<std::string> lines;
        // push lines into lines vecto
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        for (const auto& line : lines) {

            if (line.find("func ") != std::string::npos) {
                auto func = parseFunction(lines, lineNumber, module);
                // ir += func.ir;
                
            }

            lineNumber++;
        }

        file.close();

        return;
    }


    Function Compiler::parseFunction(const std::vector<std::string>& lines, int funcLine, llvm::Module* module) {
        Function func;
        const auto& funcDefLine = lines[funcLine];
        
        // Split declaration from body
        size_t bracePos = funcDefLine.find('{');
        size_t closeBrace = funcDefLine.rfind('}');
        
        std::string decl;
        std::string parsedLine;
        std::vector<std::string> code;
        
        // Extract declaration and inline code
        if (bracePos != std::string::npos) {
            decl = funcDefLine.substr(0, bracePos);
            
            // Check if function body is on same line
            if (closeBrace != std::string::npos && closeBrace > bracePos) {
                parsedLine = funcDefLine.substr(bracePos + 1, closeBrace - bracePos - 1);
            }
        } else {
            decl = funcDefLine;
        }
        
        
        // If no inline code, extract multi-line body
        if (trim(parsedLine).empty()) {
            code = extractMultiLineBody(lines, funcLine + 1);
        } else {
            // Parse inline code by splitting on semicolons
            code = splitStatements(parsedLine);
        }
        
        // Parse function declaration
        FunctionDeclaration decl_info = parseFunctionDeclaration(decl);
        if (!decl_info.valid) {
            return func;
        }
        
        func.name = decl_info.name;
        func.args = decl_info.args;
        func.returnType = decl_info.returnType;
        
        // Convert to LLVM type
        llvm::Type* llvmReturnType = novaTypeToLLVM(func.returnType, context);
        if (llvmReturnType == nullptr) {
            NCERROR("Unknown return type: {}", func.returnType);
            return func;
        }
        
        // Create LLVM function
        llvm::FunctionType* fnType = llvm::FunctionType::get(llvmReturnType, false);
        
        llvm::Function* function = llvm::Function::Create(
            fnType,
            llvm::Function::ExternalLinkage,
            func.name,
            module
        );
        
        // Generate function body IR
        generateFunctionBody(code, function, llvmReturnType, func.name, funcLine, context);
        
        return func;
    }
        
};