#include "lupck/compiler.hpp"
#include "lupck/package.hpp"
#include "lupck/package_format.hpp"
#include "lupck/logger.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Lupck::Compiler {
    bool CreateExecutable(const std::string& packageFile, const std::string& outputExe, const std::string& basePath, bool& compileLog) {
        auto outDir = fs::path(outputExe).parent_path();
        if (!outDir.empty()) {
            fs::create_directories(outDir);
        }

        Lupck::Logger::Initialize(compileLog, Lupck::LogLevel::INFO, (outDir / "build.log").string());
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "--- Starting standalone executable generation ---");
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Configuration: [Base: {}] [Package: {}] [Output: {}]", basePath, packageFile, outputExe);

        std::ifstream baseEngine(basePath, std::ios::binary);
        std::ofstream out(outputExe, std::ios::binary);
        if (!baseEngine) {
            LUPCK_ERROR("Lupck:Compiler:CreateExecutable", "Critical Error: Base engine not found at: {}", basePath);
            return false;
        }
        if (!out) {
            LUPCK_ERROR("Lupck:Compiler:CreateExecutable", "Critical Error: Could not create output file at: {}", outputExe);
            return false;
        }
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Step 1/4: Copying executable base (Base engine)...");
        // writes Base engine to executable
        out << baseEngine.rdbuf();
        auto baseEngineSize = static_cast<uint64_t>(out.tellp());
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Success: Base written ({} bytes)", baseEngineSize);


        // read and append '.lpk' file
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Step 2/4: Preparing data package (.lpk)...");
        std::ifstream pkg(packageFile, std::ios::binary | std::ios::ate);
        if (!pkg) {
            LUPCK_ERROR("Lupck:Compiler:CreateExecutable", "Error: Failed to open package file '{}'", packageFile);
            return false;
        }
        uint64_t pkgSize = static_cast<uint64_t>(pkg.tellg());
        pkg.seekg(0, std::ios::beg);
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Step 3/4: Appending scripts and resources to binary...");
        // concatenate package to Base
        out << pkg.rdbuf();
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Success: {} bytes package merged into executable", pkgSize);

       // write embed marker (footer)
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Step 4/4: Writing anchoring metadata (Embed Marker)...");
        if (!Lupck::Package::Format::WriteEmbedMarker(out, pkgSize)) {
            LUPCK_ERROR("Lupck:Compiler:CreateExecutable", "Fatal Error: Failed to write footer anchor metadata");
            return false;
        }
        uint64_t totalSize = static_cast<uint64_t>(out.tellp());
        out.close();
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "--- Compilation Completed Successfully ---");
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Generated file: {}", outputExe);
        LUPCK_INFO("Lupck:Compiler:CreateExecutable", "Total binary size: {} bytes", totalSize);
        return true;
    }
}
