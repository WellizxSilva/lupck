#include "lupck/package.hpp"
#include "lupck/vm.hpp"
#include <lua.hpp>
#include <string>

namespace Lupck::VM {
    /**
    * Custom searcher that loads Lua modules from the package on disk.
    * This function is registered into package.searchers so that `require`
    * can find modules inside our virtual file system.
     */
    int PackageSearcher(lua_State* L) {
        const char* module = luaL_checkstring(L, 1);

        std::string path = module;
        for (auto& c : path) {
            if (c == '.') c = '/';  // Dots -> Slashes
            if (c == '\\') c = '/'; // Backslashes Windows -> Slashes VFS
        }

        std::string filename = path + ".lua";
        std::string content;
        // Read from package file on disk
        if (!Lupck::Package::Unpacker::GetFileContent(filename, content)) {
            lua_pushnil(L);
            lua_pushfstring(L, "\n\tno file '%s' in lupck VFS", filename.c_str());
            return 2;
        }
        std::string virtualName = "@" + filename;
        lua_pushlstring(L, content.c_str(), content.size());
        lua_pushstring(L, virtualName.c_str());
        /**
            * Create a closure (loader function) that compiles and executes the module.
            * Why use a closure?
            * - Closures allow us to capture the file content and name as upvalues
            *   without relying on global state.
            * - Each module gets its own loader function bound to its content.
            * - This keeps the implementation clean and avoids side effects.
        */
        lua_pushcclosure(L, [](lua_State* L) -> int {
            size_t len;
            const char* buf = luaL_checklstring(L, lua_upvalueindex(1), &len);
            const char* name = lua_tostring(L, lua_upvalueindex(2));

            // Compile the Lua chunk
            if (luaL_loadbuffer(L, buf, len, name) != LUA_OK) {
                return lua_error(L);
            }
            // Execute the chunk so that the module's return value is produced
            if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
                return lua_error(L);
            }
            // now the module's return value is on the stack
            return 1;
        }, 2);
        // Return (loaderFunction, moduleName) to Lua
        lua_pushstring(L, virtualName.c_str());
        return 2;
    }
    /**
    * Same as PackageSearcher, but reads from an embedded buffer in memory instead of from disk.
    */
    int PackageSearcherBuffer(lua_State* L) {
        const char* module = luaL_checkstring(L, 1);

        std::string path = module;
        for (auto& c : path) {
           if (c == '.') c = '/';
           if (c == '\\') c = '/';
        }

        std::string filename = path + ".lua";
        std::string content;

        // Read from embedded package buffer
        if (!Lupck::Package::Unpacker::GetFileContentFromBuffer(filename, content)) {
            lua_pushnil(L);
            lua_pushfstring(L, "\n\tno file '%s' in lupck VFS (buffer)", filename.c_str());
            return 2;
        }

        std::string virtualName = "@" + filename;
        lua_pushlstring(L, content.c_str(), content.size());
        lua_pushstring(L, virtualName.c_str());
        lua_pushcclosure(L, [](lua_State* L) -> int {
            size_t len;
            const char* buf = luaL_checklstring(L, lua_upvalueindex(1), &len);
            const char* name = lua_tostring(L, lua_upvalueindex(2));
            if (luaL_loadbuffer(L, buf, len, name) != LUA_OK) {
                return lua_error(L);
            }
            if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
                return lua_error(L);
            }
            return 1;
        }, 2);
        lua_pushstring(L, virtualName.c_str());
        return 2;
    }

    /**
    * Installs the given searcher function into package.searchers at position 2
    * Position 1 is the preload searcher, so we insert ours right after it
    */
    void InstallPackageSearcher(lua_State* L, lua_CFunction searcher) {
         lua_getglobal(L, "package");
         lua_getfield(L, -1, "searchers");
         int n = (int)lua_rawlen(L, -1);

         // Shift existing searchers up to make room at position 2
         for (int i = n; i >= 2; i--) {
             lua_rawgeti(L, -1, i);
             lua_rawseti(L, -2, i + 1);
         }

         // Insert our searcher at position 2
         lua_pushcfunction(L, searcher);
         lua_rawseti(L, -2, 2);

         lua_pop(L, 2);
    }
}
