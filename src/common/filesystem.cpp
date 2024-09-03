#include "common/filesystem.h"
#include "common/logging.h"
#include <fmt/format.h>
#include <fstream>

namespace common::fs
{
    std::vector<uint8_t> read_file_bytes(const std::filesystem::path& file)
    {
        std::ifstream ifs(file, std::ios::binary);
        if (!ifs.is_open())
        {
            LOG_ERROR("%s", fmt::format("Failed to open file: {}", file.c_str()).c_str());
            return {};
        }

        std::vector<uint8_t> buf;
        buf.reserve(std::filesystem::file_size(file));
        buf.insert<std::istreambuf_iterator<char>>(buf.begin(), ifs, {});
        return buf;
    }
} // namespace common::fs
