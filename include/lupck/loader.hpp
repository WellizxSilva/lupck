#pragma once
#include "cli.hpp"

namespace Lupck::Loader {
    bool Run(const Lupck::CLI::CLIOptions& opts);
    bool RunEmbeddedPackage(const char* exePath);
    bool HasEmbeddedPackage(const char* exepath);
}
