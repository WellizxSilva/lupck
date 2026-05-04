#pragma once
#include <lua.hpp>

namespace Lupck::VM {
    // Custom searcher for Lua modules within the package
    int PackageSearcher(lua_State* L);

    // Custom searcher for Lua modules within the package (buffer version)
    int PackageSearcherBuffer(lua_State* L);

    // Installs the package searcher into the Lua state
    void InstallPackageSearcher(lua_State* L, lua_CFunction searcher);
}
