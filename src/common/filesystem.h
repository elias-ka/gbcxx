#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace common::fs
{
    [[nodiscard]] std::vector<uint8_t> read_file_bytes(const std::filesystem::path& file);
}
