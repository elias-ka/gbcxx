#pragma once

#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <filesystem>
#include <type_traits>
#include <utility>

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)

#define DIE(...)                                                                                   \
    do {                                                                                           \
        LOG_ERROR(__VA_ARGS__);                                                                    \
        std::exit(EXIT_FAILURE);                                                                   \
    } while (0)

namespace gbcxx {
namespace fs {
[[nodiscard]] auto read_file(const std::filesystem::path& file) -> std::vector<uint8_t>;
}

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename V, typename... Ts>
auto variant_match(V&& v, Ts&&... ts) -> decltype(auto)
{
    return std::visit(Overloaded { FWD(ts)... }, FWD(v));
}

constexpr auto operator""_KiB(unsigned long long int n) -> size_t
{
    return 1024ULL * n;
}

constexpr auto operator""_MiB(unsigned long long int n) -> size_t
{
    return 1024_KiB * n;
}

} // namespace gbcxx

template <typename EnumT>
    requires std::is_enum_v<EnumT>
struct fmt::formatter<EnumT> : fmt::formatter<std::underlying_type_t<EnumT>> {
    auto format(const EnumT& enum_value, format_context& ctx) const
    {
        fmt::formatter<std::underlying_type_t<EnumT>> formatter;
        return formatter.format(std::to_underlying(enum_value), ctx);
    }
};
