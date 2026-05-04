#include "lupck/lupck_embedded_lua.h"
#include <lua.hpp>
#include <lauxlib.h>
#include "lupck/logger.hpp"

namespace Lupck::Runtime {
    void SetupInternalModules(lua_State* L) {
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "preload");
        if (luaL_loadbuffer(L, (const char*)LUPCK_INIT_LUA, LUPCK_INIT_LUA_LEN, "lupck") == LUA_OK) {
            lua_setfield(L, -2, "lupck");
        } else {
            LUPCK_ERROR("Lupck:Runtime:SetupInternalModules", "Failed to load embedded lupck lib: {}", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 2);
    }
}
