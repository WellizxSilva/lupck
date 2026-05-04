#include "lupck/package.hpp"
#include "lupck/package_format.hpp"
#include "lupck/types.hpp"
#include "lupck/logger.hpp"
#include "lupck/hash.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>


namespace fs = std::filesystem;

namespace Lupck::Package::Packer {
    bool CreatePackage(const std::string& outputFile, const std::vector<std::string>& inputFiles, bool& packLog) {
        fs::path outDir = fs::path(outputFile).parent_path();
        fs::create_directories(outDir);
        Lupck::Logger::Initialize(packLog, Lupck::LogLevel::INFO, (outDir / "build.log").string());
        LUPCK_INFO("Lupck:Packer:CreatePackage:CreatePackage", "--- Starting resource packaging operation ---");

        std::vector<std::string> finalFiles = inputFiles;
        bool hasConfig = false;
        for (const auto& f : finalFiles) {
            if (fs::path(f).filename() == "lupck.config.lua") {
                hasConfig = true;
                break;
            }
        }
        if (!hasConfig && !finalFiles.empty()) {
            // get parent directory of the first input file (possible entry point)
            fs::path entryDir = fs::path(finalFiles.front()).parent_path();
            fs::path cfgPath  = entryDir / "lupck.config.lua";
            LUPCK_INFO("Lupck:Packer:CreatePackage", "Config file 'lupck.config.lua' not found in list. Searching in {}...", entryDir.string());
            if (fs::exists(cfgPath)) {
                finalFiles.push_back(cfgPath.string());
                LUPCK_INFO("Lupck:Packer:CreatePackage", "Successfully auto-included '{}'", cfgPath.string());
            } else {
                LUPCK_WARN("Lupck:Packer:CreatePackage", "Warning: 'lupck.config.lua' not found in {}. Package might not run correctly.", entryDir.string());
            }
        }

        std::ofstream out(outputFile, std::ios::binary);
        if (!out) {
            LUPCK_ERROR("Lupck:Packer:CreatePackage", "Critical Error: Could not create output package at {}", outputFile);
            return false;
        }
        // --- HEADER ---
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Step 1/4: Writing package header...");
        std::streampos indexOffsetPos;
        if (!Lupck::Package::Format::WriteHeader(out, static_cast<uint32_t>(finalFiles.size()), indexOffsetPos)) {
            LUPCK_ERROR("Lupck:Packer:CreatePackage", "Error: Failed to write package signature and version");
            return false;
        }
        // LUPCK_DEBUG"Lupck:Packer:CreatePackage", "Header written with {} entries", inputFiles.size());

        // --- PAYLOAD ---
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Step 2/4: Processing {} files into payload...", finalFiles.size());
        std::vector<Lupck::Types::FileEntry> entries;
        fs::path rootPath = fs::current_path();
        for (const auto& file : finalFiles) {
            std::ifstream in(file, std::ios::binary | std::ios::ate);
            if (!in) {
                LUPCK_ERROR("Lupck:Packer:CreatePackage", "Error: Failed to read source file: {}", file);
                return false;
            }

            uint64_t size = static_cast<uint64_t>(in.tellg());
            in.seekg(0, std::ios::beg);

            std::vector<char> buffer(size);
            in.read(buffer.data(), size);

            Lupck::Types::FileEntry entry;
            fs::path relativePath = fs::relative(file, rootPath);
            std::string nameStr = relativePath.generic_string();
            entry.name = nameStr;
            entry.offset = static_cast<uint64_t>(out.tellp());
            entry.size = size;

            // write data to package
            out.write(buffer.data(), size);

            entry.hash = Lupck::Hash::ComputeSHA256(buffer);

            entries.push_back(entry);
            LUPCK_INFO("Lupck:Packer:CreatePackage", "-> Encapsulated: {} [Size: {} bytes] [Offset: {}]", entry.name, entry.size, entry.offset);
        }

        // --- INDEX ---
        uint64_t indexOffset = static_cast<uint64_t>(out.tellp());
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Step 3/4: Building file index at offset {}...", indexOffset);
        if (!Lupck::Package::Format::WriteIndex(out, entries, indexOffset)) {
            LUPCK_ERROR("Lupck:Packer:CreatePackage", "Error: Failed to write index table");
            return false;
        }
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Index written at offset {}", indexOffset);

        // --- FOOTER ---
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Step 4/4: Finalizing package and patching header...");
        if (!Lupck::Package::Format::WriteFooter(out, indexOffset)) {
            LUPCK_ERROR("Lupck:Packer:CreatePackage", "Error: Failed to write package footer");
            return false;
        }
        // Go back and write real index offset in header
        out.seekp(indexOffsetPos);
        out.write(reinterpret_cast<char*>(&indexOffset), sizeof(indexOffset));
        uint64_t totalSize = static_cast<uint64_t>(out.tellp());
        out.close();
        LUPCK_INFO("Lupck:Packer:CreatePackage", "--- Package Creation Completed ---");
        LUPCK_INFO("Lupck:Packer:CreatePackage", "Output: {} (Total Size: {} bytes)", outputFile, totalSize);

        // Blueprint
        GenerateBlueprint(outDir, outputFile, entries);
        return true;
    }

    void GenerateBlueprint(const fs::path& outDir, const std::string& outputFile, const std::vector<Lupck::Types::FileEntry>& entries) {
        std::ofstream yaml(outDir / "blueprint.yaml");
        if (!yaml) return;

        yaml << "package: " << fs::path(outputFile).filename().string() << "\n";
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        yaml << "created_at: " << std::put_time(std::gmtime(&now), "%Y-%m-%dT%H:%M:%SZ") << "\n";
        yaml << "blueprint:\n";
        yaml << "  files:\n";
        for (const auto& entry : entries) {
            yaml << "    - name: " << entry.name << "\n";
            yaml << "      size: " << entry.size << "\n";
            yaml << "      offset: " << entry.offset << "\n";
            yaml << "      hash: " << entry.hash << "\n";
            }
            yaml.close();
        }
}
