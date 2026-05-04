#include "lupck/cli.hpp"
#include "lupck/loader.hpp"
#include "lupck/package.hpp"
#include "lupck/package_format.hpp"
#include "lupck/logger.hpp"
#include "lupck/runtime.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

namespace Lupck::Loader {

    bool Run(const Lupck::CLI::CLIOptions& opts) {
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        const std::string& file = opts.appFile;

        if (!Lupck::Runtime::Init(L, opts.appFile)) {
            std::cerr << "Failed to initialize runtime" << std::endl;
            lua_close(L);
            return false;
        }

        // run packaged .lpk
        if (file.size() > 4 && file.substr(file.size() - 4) == ".lpk") {
            if (!Lupck::Package::Unpacker::ExtractAndRun(L, file)) {
                std::cerr << "Failed to run package" << std::endl;
                Lupck::Runtime::Shutdown(L);
                lua_close(L);
                return false;
            }
        } else {
            std::cerr << "Unknown format: " << file << std::endl;
            lua_close(L);
            return false;
        }
        Lupck::Runtime::Shutdown(L);
        lua_close(L);
        return true;
    }

    bool RunEmbeddedPackage(const char* exePath) {
           Lupck::Logger::Initialize(false, Lupck::LogLevel::DEBUG,"");
           lua_State* L = luaL_newstate();
           luaL_openlibs(L);


           std::ifstream exe(exePath, std::ios::binary);
           if (!exe) {
                std::cerr << "Failed to open exe: " << exePath << std::endl;
                return false;
            }
           uint64_t pkgSize;
           if (!Lupck::Package::Format::ReadEmbedMarker(exe, pkgSize)) {
               std::cerr << "No embedded package found" << std::endl;
               return false;
           }
           LUPCK_DEBUG("Lupck:Loader:RunEmbeddedPackage", "Embed marker found, pkgSize={}", pkgSize);

           exe.clear();
           exe.seekg(0, std::ios::end);
           auto startPos = exe.tellg() - static_cast<std::streamoff>(sizeof(Lupck::Package::Format::EMBED_MARKER) + sizeof(pkgSize) + pkgSize);
           exe.seekg(startPos, std::ios::beg);
           std::string pkg(pkgSize, '\0');
           exe.read(&pkg[0], pkgSize);
           LUPCK_INFO("Lupck:Loader:RunEmbeddedPackage", "Embedded package loaded into memory ({} bytes)", pkgSize);

           if (!Lupck::Runtime::InitFromBuffer(L, pkg)) {
               LUPCK_ERROR("Lupck:Loader:RunEmbeddedPackage", "Runtime init failed");
               return false;
           }
           if (!Lupck::Package::Unpacker::ExtractAndRunFromBuffer(L, pkg)) {
               LUPCK_ERROR("Lupck:Loader:RunEmbeddedPackage", "ExtractAndRunFromBuffer failed");
               return false;
           }
           Lupck::Runtime::Shutdown(L);
           lua_close(L);
           return true;
       }

    bool HasEmbeddedPackage(const char* exePath) {
        std::ifstream file(exePath, std::ios::binary);
        if (!file) return false;
        uint64_t dummyPkgSize = 0;
        return Lupck::Package::Format::ReadEmbedMarker(file, dummyPkgSize);
       }
}
