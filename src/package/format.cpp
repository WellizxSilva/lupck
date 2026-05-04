#include "lupck/package_format.hpp"
#include <iostream>
#include <cstring>

namespace Lupck::Package::Format {

    bool WriteHeader(std::ofstream& out, uint32_t fileCount, std::streampos& indexOffsetPos) {
        if (!out) return false;

        out.write(MAGIC, sizeof(MAGIC));
        out.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));
        out.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

        // Placeholder for index offset
        uint64_t indexOffsetPlaceholder = 0;
        indexOffsetPos = out.tellp();
        out.write(reinterpret_cast<const char*>(&indexOffsetPlaceholder), sizeof(indexOffsetPlaceholder));

        return true;
    }

    bool ReadHeader(std::ifstream& in, uint32_t& fileCount, uint64_t& indexOffset) {
        if (!in) return false;

        char magic[5];
        in.read(magic, sizeof(magic));
        if (std::string(magic, 5) != std::string(MAGIC, 5)) {
            std::cerr << "Invalid package format" << std::endl;
            return false;
        }

        uint8_t version;
        in.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != VERSION) {
            std::cerr << "Unsupported version" << std::endl;
            return false;
        }

        in.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
        in.read(reinterpret_cast<char*>(&indexOffset), sizeof(indexOffset));

        return true;
    }

    bool ReadHeaderFromBuffer(const std::string& pkgBuffer, uint32_t& fileCount, uint64_t& indexOffset) {
         // MAGIC + VERSION + fileCount + indexOffset
        const size_t minSize = sizeof(MAGIC) + sizeof(VERSION) + sizeof(fileCount) + sizeof(indexOffset);
        if (pkgBuffer.size() < minSize) {
            std::cerr << "Buffer too small for header" << std::endl;
             return false;
        }

        if (std::string(pkgBuffer.data(), sizeof(MAGIC)) != std::string(MAGIC, sizeof(MAGIC))) {
            std::cerr << "Invalid package format (magic mismatch)" << std::endl;
            return false;
        }
        uint8_t version;
        std::memcpy(&version, pkgBuffer.data() + sizeof(MAGIC), sizeof(version));
        if (version != VERSION) {
            std::cerr << "Unsupported package version: " << static_cast<int>(version) << std::endl;
            return false;
        }

        std::memcpy(&fileCount, pkgBuffer.data() + sizeof(MAGIC) + sizeof(version), sizeof(fileCount));
        std::memcpy(&indexOffset, pkgBuffer.data() + sizeof(MAGIC) + sizeof(version) + sizeof(fileCount), sizeof(indexOffset));

        if (indexOffset >= pkgBuffer.size()) {
            std::cerr << "Index offset out of range: " << indexOffset << std::endl;
            return false;
        }
        return true;
    }

    bool WriteIndex(std::ofstream& out, const std::vector<Lupck::Types::FileEntry>& entries, uint64_t& indexOffset) {
        if (!out) return false;

        indexOffset = out.tellp();
        for (const auto& entry : entries) {
            uint16_t nameLen = static_cast<uint16_t>(entry.name.size());
            out.write(reinterpret_cast<const char*>(&nameLen), sizeof(nameLen));
            out.write(entry.name.c_str(), nameLen);

            out.write(reinterpret_cast<const char*>(&entry.offset), sizeof(entry.offset));
            out.write(reinterpret_cast<const char*>(&entry.size), sizeof(entry.size));

            uint16_t hashLen = static_cast<uint16_t>(entry.hash.size());
            out.write(reinterpret_cast<const char*>(&hashLen), sizeof(hashLen));
            out.write(entry.hash.c_str(), hashLen);
        }
        return true;
    }

    bool ReadIndex(std::ifstream& in, uint32_t fileCount, uint64_t indexOffset, std::vector<Lupck::Types::FileEntry>& entries) {
        if (!in) return false;

        in.seekg(indexOffset, std::ios::beg);
        for (uint32_t i = 0; i < fileCount; i++) {
            uint16_t nameLen;
            in.read(reinterpret_cast<char*>(&nameLen), sizeof(nameLen));

            std::string name(nameLen, '\0');
            in.read(&name[0], nameLen);

            uint64_t offset, size;
            in.read(reinterpret_cast<char*>(&offset), sizeof(offset));
            in.read(reinterpret_cast<char*>(&size), sizeof(size));

            uint16_t hashLen;
            in.read(reinterpret_cast<char*>(&hashLen), sizeof(hashLen));
            std::string hash(hashLen, '\0');
            in.read(&hash[0], hashLen);

            entries.push_back({name, offset, size, hash});
        }
        return true;
    }

    bool ReadIndexFromBuffer(const std::string& pkgBuffer, uint32_t fileCount, uint64_t indexOffset, std::vector<Lupck::Types::FileEntry>& entries) {
        if (indexOffset >= pkgBuffer.size()) {
            std::cerr << "Index offset out of range" << std::endl;
            return false;
        }
        const char* ptr = pkgBuffer.data() + indexOffset;
        const char* end = pkgBuffer.data() + pkgBuffer.size();
        for (uint32_t i = 0; i < fileCount; ++i) {
            if (ptr + sizeof(uint16_t) > end) {
                std::cerr << "Unexpected end of buffer while reading name length (entry " << i << ")" << std::endl;
                return false;
            }
            uint16_t nameLen;
            std::memcpy(&nameLen, ptr, sizeof(nameLen));
            ptr += sizeof(nameLen);
            if (ptr + nameLen > end) {
                std::cerr << "Unexpected end of buffer while reading name (entry " << i << ")" << std::endl;
                return false;
            }
            std::string name(nameLen, '\0');
            if (nameLen > 0) {
                std::memcpy(&name[0], ptr, nameLen);
                ptr += nameLen;
            }
            if (ptr + sizeof(uint64_t) * 2 > end) {
                std::cerr << "Unexpected end of buffer while reading offset/size (entry " << i << ")" << std::endl;
                return false;
            }
            uint64_t offset, size;
            std::memcpy(&offset, ptr, sizeof(offset));
            ptr += sizeof(offset);
            std::memcpy(&size, ptr, sizeof(size));
            ptr += sizeof(size);

            if (ptr + sizeof(uint16_t) > end) {
                std::cerr << "Unexpected end of buffer while reading hash length (entry " << i << ")" << std::endl;
                return false;
            }
            uint16_t hashLen;
            std::memcpy(&hashLen, ptr, sizeof(hashLen));
            ptr += sizeof(hashLen);
            if (ptr + hashLen > end) {
                std::cerr << "Unexpected end of buffer while reading hash (entry " << i << ")" << std::endl;
                return false;
            }
            std::string hash(hashLen, '\0');
            if (hashLen > 0) {
                std::memcpy(&hash[0], ptr, hashLen);
                ptr += hashLen;
            }
            entries.push_back({name, offset, size, hash});
            }
            return true;
        }

    bool WriteFooter(std::ofstream& out, uint64_t indexOffset) {
        if (!out) return false;

        uint32_t checksum = 0xDEADBEEF; // placeholder
        out.write(reinterpret_cast<const char*>(&checksum), sizeof(checksum));
        out.write(reinterpret_cast<const char*>(&indexOffset), sizeof(indexOffset));
        return true;
    }

    bool ReadFooter(std::ifstream& in, uint64_t& indexOffset) {
        if (!in) return false;
        in.seekg(-static_cast<int>(sizeof(uint32_t) + sizeof(uint64_t)), std::ios::end);

        uint32_t checksum;
        in.read(reinterpret_cast<char*>(&checksum), sizeof(checksum));
        in.read(reinterpret_cast<char*>(&indexOffset), sizeof(indexOffset));

        // TODO: Implement checksum validation
        return true;
    }

    bool WriteEmbedMarker(std::ofstream& out, uint64_t pkgSize) {
          if (!out) return false;
          out.write(EMBED_MARKER, sizeof(EMBED_MARKER));
          out.write(reinterpret_cast<const char*>(&pkgSize), sizeof(pkgSize));
          return true;
      }

      bool ReadEmbedMarker(std::ifstream& in, uint64_t& pkgSize) {
          if (!in) return false;
          in.seekg(-static_cast<std::streamoff>(sizeof(EMBED_MARKER) + sizeof(pkgSize)), std::ios::end);
          char marker[sizeof(EMBED_MARKER)];
          in.read(marker, sizeof(marker));
          in.read(reinterpret_cast<char*>(&pkgSize), sizeof(pkgSize));
          return std::string(marker, sizeof(marker)) == std::string(EMBED_MARKER, sizeof(EMBED_MARKER));
      }
}
