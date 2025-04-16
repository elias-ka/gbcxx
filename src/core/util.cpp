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

[[nodiscard]] std::filesystem::path GetHomeDirectory()
{
#if defined(__unix__)
    if (const char* xdg_data_home = std::getenv("XDG_DATA_HOME"))
    {
        return std::filesystem::path{xdg_data_home};
    }
    if (const char* home = std::getenv("HOME")) { return std::filesystem::path{home}; }

    DIE("Neither XDG_DATA_HOME nor HOME is set in the environment");
#else
#error "Unsupported operating system"
#endif
}
}  // namespace gb::fs
