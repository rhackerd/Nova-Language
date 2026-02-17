#include "CLI/CLI.hpp"
#include "compiler.h"
#include <cstdlib>
#include <fmt/format.h>
#include <lsp/connection.h>
#include <lsp/io/standardio.h>
#include <stdio.h>
#include <dlfcn.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <core.h>
#include <Nova/Core/core.h>
#include <CLI/CLI.hpp>
#include <logger.h>
#include <lsp.h>
#include <vector>

struct MyArgs {
    bool compiler {false};
    bool help {false};



    std::string configPath {};
    bool generateAll {false};

    bool compileAll {true};
    bool lsp{false};
};

NOVA_LOG_DEF("Main");


int main(int argc, char** argv) {

    MyArgs args{};

    NCINFO("Welcome to Nova Language!");
    CLI::App app{"Nova Language Compiler"};
    argv = app.ensure_utf8(argv);

    auto compiler = app.add_subcommand("compiler", "Manual usage of the Nova Compiler")->callback([&args](){
        args.compiler = true;
    });

    // LSP
    auto lsp = app.add_subcommand("lsp", "Manual usage of the Nova Language Server")->callback([&args]() {
        args.lsp = true;
    });

    compiler->add_option("-c, --config", args.configPath, "Path to configuration file")->check(CLI::ExistingFile);
    compiler->add_flag("--parse", args.compileAll, "Compile all projects specified in the configuration file");
    compiler->add_flag("--compile", args.compileAll, "Compile all projects specified in the configuration file");


    CLI11_PARSE(app, argc, argv);

    if (args.compiler) NCINFO("Compiler usage was requested.");
    if (args.compiler) {
        Nova::Compiler::Compiler compiler;

        if (args.generateAll) compiler.generateAll("./");
        if (args.compileAll) {
            compiler.generateAll("./");
            // Then compile
        }

        // std::string test = "var int x = 2 + 2;ret 0;x = test();";

        // NCINFO("Splitting: {}", test);
        // std::vector<Nova::Compiler::Assignment> ass = compiler.splitCall(test);
        // for (const auto& as : ass) {
        //     NCINFO("Assign: ");
        //     for (const auto& a : as.tokens) {
        //         NCINFO("'{}'", a.token);
        //     }
        // }

    }else if (args.lsp) {
        NCINFO("LSP usage was requested.");
        
        auto connection = lsp::Connection(lsp::io::standardIO());
        auto messageHandler = lsp::MessageHandler(connection);

        auto lsp = Nova::Compiler::LSP(messageHandler);

        bool running = true;
        while(running) {
            messageHandler.processIncomingMessages();
        }
    }


    NCINFO("Goodbye.");
    return EXIT_SUCCESS;
}
