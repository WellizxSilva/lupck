#pragma once
#include <string>
#include <vector>
#include "types.hpp"
#include <lua.hpp>
#include <filesystem>

namespace Lupck::Package::Unpacker {
    bool ReadPackage(const std::string& lpkFile, std::vector<Lupck::Types::FileEntry>& entries);
    // Extracts a specific file into a string buffer
    bool ExtractFile(const std::string& lpkFile, const Lupck::Types::FileEntry& entry, std::string& buffer);

    // Extracts and runs lua app inside the package
    bool ExtractAndRun(lua_State* L, const std::string& lpkFile);

    // Extracts and runs lua app inside the buffer
    bool ExtractAndRunFromBuffer(lua_State* L, const std::string& pkgBuffer);

    // Extracts all files from the package into the output directory
    bool ExtractAll(const std::string& lpkFile, const std::string& outputDir);

    bool LoadPackage(const std::string& lpkFile);

    // Reads index and loads package into buffer
    bool LoadPackageFromBuffer(const std::string& pkgBuffer);

    // Returns the content of a buffer from the loaded package
    bool GetFileContentFromBuffer(const std::string& filename, std::string& buffer);

    bool GetFileContent(const std::string& filename, std::string& buffer);

    // Clears the package cache
    void ClearCache();
}

namespace Lupck::Package::Packer {
    // Creates a .lpk package from a list of input files
    bool CreatePackage(const std::string& outputFile, const std::vector<std::string>& inputFiles, bool& packLog);
    void GenerateBlueprint(const std::filesystem::path& outDir, const std::string& outputFile, const std::vector<Lupck::Types::FileEntry>& entries);
}
