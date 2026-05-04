#include "lupck/runtime.hpp"
#include "lupck/vm.hpp"
#include "lupck/package.hpp"
#include <iostream>


namespace Lupck::Runtime {

    bool Init(lua_State* L, const std::string& packagePath) {
        // load package metadata and index files
        if (!Lupck::Package::Unpacker::LoadPackage(packagePath)) {
            return false;
        }
        // install custom searcher for Lua modules for `require()`
        // makes `require()` search file in the package (VFS)
        Lupck::VM::InstallPackageSearcher(L, Lupck::VM::PackageSearcher);

        return true;
    }

    bool InitFromBuffer(lua_State* L, const std::string& pkgBuffer) {
        if (!Lupck::Package::Unpacker::LoadPackageFromBuffer(pkgBuffer)) {
            return false;
        }
        // Register internal lua module `lupck`
        // This makes `require("lupck")` work
        Lupck::Runtime::SetupInternalModules(L);

        // install custom searcher for Lua modules for `require()`
        // makes `require()` search file in the package (VFS)
        Lupck::VM::InstallPackageSearcher(L, Lupck::VM::PackageSearcherBuffer);
        return true;
   }

    void Shutdown(lua_State* L) {
        Lupck::Package::Unpacker::ClearCache();
    }
}
