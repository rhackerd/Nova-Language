#pragma once
#include <string_view>

// Used for managing projects
namespace Nova::Compiler {
    class Manager {
    public:
        Manager(std::string_view projectPath);
        ~Manager();
    public:
        void newProject(std::string_view projectName, std::string_view projectPath);
    };
};