#pragma once

#include "compiler.h"
#include "logger.h"
#include <lsp/connection.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/io/standardio.h>
#include <lsp/types.h>


namespace Nova::Compiler {
    class LSP {
        private:
            lsp::MessageHandler& handler;
            Nova::Compiler::Compiler compiler;


        public:
            LSP(lsp::MessageHandler& h): handler(h), compiler() {
                registerHandlers();
            }
            ~LSP() {}

        private:
            void registerHandlers() {
                handler.add<lsp::requests::Initialize>(
                    [](lsp::requests::Initialize::Params&& params) {
                        return lsp::requests::Initialize::Result{
                            .capabilities = {
                                .positionEncoding = lsp::PositionEncodingKind::UTF8,
                                .textDocumentSync = lsp::TextDocumentSyncOptions{
                                    .openClose = true,
                                    .change = lsp::TextDocumentSyncKind::Full
                                }
                            },
                            .serverInfo = lsp::InitializeResultServerInfo{
                                .name    = "Nova Language Server",
                                .version = "0.1.0"
                            }
                        };
                    }  
                );

                handler.add<lsp::notifications::Initialized>(
                    [this](lsp::notifications::Initialized::Params&& params) {
                        NCINFO("Client has initialized.");
                    }
                );
            };

    };
}