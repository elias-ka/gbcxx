#include "util.h"
#include <fmt/format.h>
#include <fstream>

namespace cb
{
    namespace fs
    {
        std::vector<u8> read(const std::filesystem::path& file)
        {
            std::ifstream ifs(file, std::ios::binary);
            if (!ifs.is_open())
            {
                LOG_ERROR("Failed to open file: {}", file.string());
                return {};
            }

            std::vector<u8> buf(std::filesystem::file_size(file));
            buf.insert<std::istreambuf_iterator<char>>(buf.begin(), ifs, {});
            return buf;
        }
    } // namespace fs
} // namespace cb

fmt::format_context::iterator fmt::formatter<cb::log_level>::format(cb::log_level level,
                                                                    format_context& ctx) const
{
    string_view name = "unknown";
    switch (level)
    {
    case cb::log_level::debug: name = "debug"; break;
    case cb::log_level::unimplemented: name = "unimplemented"; break;
    case cb::log_level::info: name = "info"; break;
    case cb::log_level::warn: name = "warn"; break;
    case cb::log_level::error: name = "error"; break;
    }
    return formatter<string_view>::format(name, ctx);
}
