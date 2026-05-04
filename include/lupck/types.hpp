#pragma once
#include <cstdint>

namespace Lupck::Types {
    struct FileEntry {
        std::string name;
        uint64_t offset;
        uint64_t size;
        std::string hash;
    };
}
