#pragma once
#include <string>

namespace Lupck::Compiler {
    bool CreateExecutable(const std::string& packageFile, const std::string& outputExe, const std::string& basePath, bool& compileLog);
}
