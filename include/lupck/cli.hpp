#pragma once
#include <string>
#include <vector>

namespace Lupck::CLI {
    struct CLIOptions {
        bool valid;
        std::string command;
        std::string appFile;
        std::vector<std::string> extraArgs;
    };

    CLIOptions ParseCLI(int argc, char* argv[]);
}
