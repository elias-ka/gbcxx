#include "util.hpp"

#include <fmt/format.h>

#include <fstream>

namespace gb::fs
{
std::vector<uint8_t> ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        LOG_ERROR("Failed to open file: {}", path.string());
        return {};
    }

    std::vector<uint8_t> buf(std::filesystem::file_size(path));
    buf.insert<std::istreambuf_iterator<char>>(buf.begin(), file, {});
    return buf;
}
}  // namespace gb::fs
