#include "lupck/package.hpp"
#include "lupck/package_format.hpp"
#include "lupck/logger.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <lua.hpp>
#include <unordered_map>
#include <cstring>

namespace fs = std::filesystem;

namespace {
    std::unordered_map<std::string, Lupck::Types::FileEntry> g_entries;
    std::string g_packagePath;
    std::string g_pkgBuffer;
}

namespace Lupck::Package::Unpacker {
    bool ReadPackage(const std::string& lpkFile, std::vector<Lupck::Types::FileEntry>& entries) {
        std::ifstream in(lpkFile, std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open package: " << lpkFile << std::endl;
            return false;
        }

        uint32_t fileCount;
        uint64_t indexOffset;
        if (!Lupck::Package::Format::ReadHeader(in, fileCount, indexOffset)) return false;

        if (!Lupck::Package::Format::ReadIndex(in, fileCount, indexOffset, entries)) return false;

        return true;
    }

    bool LoadPackageFromBuffer(const std::string& pkgBuffer) {
           g_entries.clear();
           g_pkgBuffer = pkgBuffer;

           uint32_t fileCount;
           uint64_t indexOffset;

            if (!Lupck::Package::Format::ReadHeaderFromBuffer(pkgBuffer, fileCount, indexOffset)) {
                LUPCK_ERROR("Lupck:Unpacker:LoadPackageFromBuffer", "Failed to read header in LoadPackageFromBuffer");
                return false;
            }

            std::vector<Lupck::Types::FileEntry> entries;
            if (!Lupck::Package::Format::ReadIndexFromBuffer(pkgBuffer, fileCount, indexOffset, entries)) {
                LUPCK_ERROR("Lupck:Unpacker:LoadPackageFromBuffer", "Failed to read index in LoadPackageFromBuffer");
                return false;
            }

            for (const auto& e : entries) {
                g_entries[e.name] = e;
            }

           LUPCK_INFO("Lupck:Unpacker:LoadPackageFromBuffer", "Loaded package from buffer: {} files", fileCount);
           return true;
       }

       bool GetFileContentFromBuffer(const std::string& filename, std::string& buffer) {
           auto it = g_entries.find(filename);
           if (it == g_entries.end()) {
               return false;
           }

           const auto& entry = it->second;
           buffer.resize(entry.size);
           std::memcpy(&buffer[0], g_pkgBuffer.data() + entry.offset, entry.size);

           return true;
       }


    bool ExtractFile(const std::string& lpkFile, const Lupck::Types::FileEntry& entry, std::string& buffer) {
        std::ifstream in(lpkFile, std::ios::binary);
        if (!in) return false;

        in.seekg(entry.offset, std::ios::beg);
        buffer.resize(entry.size);
        in.read(&buffer[0], entry.size);

        return true;
    }

    bool ExtractAndRunFromBuffer(lua_State* L, const std::string& pkgBuffer) {
        LUPCK_DEBUG("Lupck:Unpacker:ExtractAndRunFromBuffer", "Buffer size={}", pkgBuffer.size());

        uint32_t fileCount;
        uint64_t indexOffset;
        if (!Lupck::Package::Format::ReadHeaderFromBuffer(pkgBuffer, fileCount, indexOffset)) {
            std::cerr << "Failed to read header from buffer" << std::endl;
            return false;
        }
        LUPCK_DEBUG("Lupck:Unpacker:ExtractAndRunFromBuffer", "Header parsed: fileCount={}, indexOffset={}", fileCount, indexOffset);

        std::vector<Lupck::Types::FileEntry> entries;
        if (!Lupck::Package::Format::ReadIndexFromBuffer(pkgBuffer, fileCount, indexOffset, entries)) {
            std::cerr << "Failed to read index from buffer" << std::endl;
            LUPCK_ERROR("Lupck:Unpacker:ExtractAndRunFromBuffer", "Failed to read index from buffer");
            return false;
        }
        for (const auto& e : entries) {
            LUPCK_INFO("Lupck:Unpacker:ExtractAndRunFromBuffer", "Found entry: {} (offset={}, size={})", e.name, e.offset, e.size);
        }

        auto endsWith = [](const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() &&
                   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };

        auto cfgIt = std::find_if(entries.begin(), entries.end(),
            [&](const auto& e){ return endsWith(e.name, "lupck.config.lua"); });

        if (cfgIt == entries.end()) {
            std::cerr << "Config file not found in package index"  << std::endl;
            return false;
        }

        std::string cfgScript(cfgIt->size, '\0');
        std::memcpy(&cfgScript[0], pkgBuffer.data() + cfgIt->offset, cfgIt->size);
        LUPCK_INFO("Lupck:Unpacker:ExtractAndRunFromBuffer", "Loaded config script ({} bytes)", cfgScript.size());

        if (luaL_dostring(L, cfgScript.c_str()) != LUA_OK) {
            std::cerr << "Lua error in config: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        if (!lua_istable(L, -1)) {
            std::cerr << "Config did not return a table" << std::endl;
            lua_pop(L, 1);
            return false;
        }

        lua_getfield(L, -1, "entry");
        std::string entryFile= lua_tostring(L, -1);
        lua_pop(L, 2);

        if (entryFile.empty()) {
            std::cerr << "No entry defined in config" << std::endl;
            return false;
        }
        LUPCK_INFO("Lupck:Unpacker:ExtractAndRunFromBuffer", "Config entry file: {}", entryFile);

        auto entryIt = std::find_if(entries.begin(), entries.end(),
            [&](const auto& e){ return e.name == entryFile; });
        if (entryIt == entries.end()) {
            std::cerr << "Entry file not found: " << entryFile << std::endl;
            return false;
        }

        std::string script(entryIt->size, '\0');
        std::memcpy(&script[0], pkgBuffer.data() + entryIt->offset, entryIt->size);
        LUPCK_INFO("Lupck:Unpacker:ExtractAndRunFromBuffer", "Loaded entry script ({} bytes)", script.size());

        if (luaL_loadbuffer(L, script.c_str(), script.size(), entryFile.c_str()) != LUA_OK) {
            std::cerr << "Lua error loading entry: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            std::cerr << "Lua error running entry: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        return true;
    }

    bool ExtractAndRun(lua_State* L, const std::string& lpkFile) {
        // open the package in disk
        std::ifstream in(lpkFile, std::ios::binary);
        if (!in) {
            std::cerr << "Failed to open package file: " << lpkFile << std::endl;
            return false;
        }

        uint32_t fileCount;
        uint64_t indexOffset;
        if (!Lupck::Package::Format::ReadHeader(in, fileCount, indexOffset)) {
            std::cerr << "Failed to read header from file" << std::endl;
            return false;
        }

        std::vector<Lupck::Types::FileEntry> entries;
        if (!Lupck::Package::Format::ReadIndex(in, fileCount, indexOffset, entries)) {
            std::cerr << "Failed to read index from file" << std::endl;
            return false;
        }
        auto endsWith = [](const std::string& str, const std::string& suffix) {
            return str.size() >= suffix.size() &&
                   str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        };

        auto cfgIt = std::find_if(entries.begin(), entries.end(),
            [&](const auto& e){ return endsWith(e.name, "lupck.config.lua"); });

        if (cfgIt == entries.end()) {
            std::cerr << "Config file not found in package" << std::endl;
            return false;
        }

        // read config from disk using offset/size
        std::string cfgScript(cfgIt->size, '\0');
        in.seekg(cfgIt->offset, std::ios::beg);
        in.read(&cfgScript[0], cfgIt->size);

        if (luaL_dostring(L, cfgScript.c_str()) != LUA_OK) {
            std::cerr << "Lua error in config: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        if (!lua_istable(L, -1)) {
            std::cerr << "Config did not return a table" << std::endl;
            lua_pop(L, 1);
            return false;
        }

        lua_getfield(L, -1, "entry");
        const char* entryFile = lua_tostring(L, -1);
        lua_pop(L, 2);

        if (!entryFile) {
            std::cerr << "No entry defined in config" << std::endl;
            return false;
        }

        auto entryIt = std::find_if(entries.begin(), entries.end(),
            [&](const auto& e){ return e.name == entryFile; });

        if (entryIt == entries.end()) {
            std::cerr << "Entry file not found: " << entryFile << std::endl;
            return false;
        }

        // read entry from disk
        std::string script(entryIt->size, '\0');
        in.seekg(entryIt->offset, std::ios::beg);
        in.read(&script[0], entryIt->size);

        if (luaL_loadbuffer(L, script.c_str(), script.size(), entryFile) != LUA_OK) {
            std::cerr << "Lua error loading entry: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }
        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK) {
            std::cerr << "Lua error running entry: " << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
            return false;
        }

        return true;
    }


    bool ExtractAll(const std::string& lpkFile, const std::string& outputDir) {
        std::vector<Lupck::Types::FileEntry> entries;
        if (!ReadPackage(lpkFile, entries)) return false;

        fs::create_directories(outputDir);

        for (const auto& entry : entries) {
            std::string buffer;
            if (!ExtractFile(lpkFile, entry, buffer)) {
                std::cerr << "Failed to extract: " << entry.name << std::endl;
                return false;
            }

            std::string outPath = (fs::path(outputDir) / entry.name).string();
            std::ofstream out(outPath, std::ios::binary);
            if (!out) {
                std::cerr << "Failed to write: " << outPath << std::endl;
                return false;
            }
            out.write(buffer.data(), buffer.size());
            out.close();

            std::cout << "Extracted: " << outPath << std::endl;
        }
        return true;
    }

    bool LoadPackage(const std::string& lpkFile) {
          g_entries.clear();
          g_packagePath = lpkFile;

          std::ifstream in(lpkFile, std::ios::binary);
          if (!in) {
              std::cerr << "Failed to open package: " << lpkFile << std::endl;
              return false;
          }

          uint32_t fileCount;
          uint64_t indexOffset;
          if (!Lupck::Package::Format::ReadHeader(in, fileCount, indexOffset)) return false;

          std::vector<Lupck::Types::FileEntry> entries;
          if (!Lupck::Package::Format::ReadIndex(in, fileCount, indexOffset, entries)) return false;

          for (auto& e : entries) {
              g_entries[e.name] = e;
          }

          return true;
      }

      bool GetFileContent(const std::string& filename, std::string& buffer) {
          auto it = g_entries.find(filename);
          if (it == g_entries.end()) {
              return false;
          }

          std::ifstream in(g_packagePath, std::ios::binary);
          if (!in) return false;

          const auto& entry = it->second;
          in.seekg(entry.offset, std::ios::beg);
          buffer.resize(entry.size);
          in.read(&buffer[0], entry.size);

          return true;
      }

      void ClearCache() {
          g_entries.clear();
          g_pkgBuffer.clear();
      }
}
