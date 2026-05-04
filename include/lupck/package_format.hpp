#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include "types.hpp"


namespace Lupck::Package::Format {
    constexpr char MAGIC[5] = {'L','U','P','C','K'};
    constexpr uint8_t VERSION = 1;

    constexpr char EMBED_MARKER[8] = {'L','U','P','C','K','P','K','G'};

    bool WriteHeader(std::ofstream& out, uint32_t fileCount, std::streampos& indexOffsetPos);
    bool ReadHeader(std::ifstream& in, uint32_t& fileCount, uint64_t& indexOffset);
    bool ReadHeaderFromBuffer(const std::string& pkgBuffer, uint32_t& fileCount, uint64_t& indexOffset);

    bool WriteIndex(std::ofstream& out, const std::vector<Lupck::Types::FileEntry>& entries, uint64_t& indexOffset);
    bool ReadIndex(std::ifstream& in, uint32_t fileCount, uint64_t indexOffset, std::vector<Lupck::Types::FileEntry>& entries);
    bool ReadIndexFromBuffer(const std::string& pkgBuffer, uint32_t fileCount, uint64_t indexOffset, std::vector<Lupck::Types::FileEntry>& entries);

    bool WriteFooter(std::ofstream& out, uint64_t indexOffset);
    bool ReadFooter(std::ifstream& in, uint64_t& indexOffset);

    bool WriteEmbedMarker(std::ofstream& out, uint64_t pkgSize);
    bool ReadEmbedMarker(std::ifstream& in, uint64_t& pkgSize);
}
