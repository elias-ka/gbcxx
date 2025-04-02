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
#ifdef __unix__
    if (const char* xdg_data_home = std::getenv("XDG_DATA_HOME")) { return xdg_data_home; }
    return std::getenv("HOME");
#else
#error "Unsupported operating system"
#endif
}
}  // namespace gb::fs
