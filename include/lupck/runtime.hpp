#pragma once
#include <lua.hpp>
#include <string>

namespace Lupck::Runtime {
    // Initializes the execution environment
    bool Init(lua_State* L, const std::string& packagePath);

    // Sets up internal Lua modules (lupck)
    void SetupInternalModules(lua_State* L);

    // Initializes the execution environment from a buffer
    bool InitFromBuffer(lua_State* L, const std::string& pkgBuffer);
    void Shutdown(lua_State* L);
}
