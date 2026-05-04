#pragma once
#include "picosha2/picosha2.h"
#include <iomanip>
#include <string>
#include <vector>

namespace Lupck::Hash {
    inline std::string ComputeSHA256(const std::vector<char>& data) {
        auto hash = picosha2::hash256_hex_string(data);
        // printf("Generated hash: %s\n", hash.c_str());
        return hash;
    }
}
