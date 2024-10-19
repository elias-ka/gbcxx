#pragma once

#include <cstdint>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/spdlog.h>
#include <type_traits>
#include <vector>

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define BIT(n) (1UL << (n))

// The default logger (stdout, multi-threaded, colored) is fine
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)

#define DIE(...)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        LOG_ERROR(__VA_ARGS__);                                                                    \
        std::exit(EXIT_FAILURE);                                                                   \
    } while (0)

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using usz = std::size_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;
using ssz = std::make_signed_t<std::size_t>;

using f32 = float;
using f64 = double;

namespace cb
{
    template <typename Enum>
    inline constexpr auto to_underlying(Enum e)
    {
        return static_cast<std::underlying_type_t<Enum>>(e);
    }

    namespace fs
    {
        std::vector<u8> read(const std::filesystem::path& file);
    }

    template <typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <typename V, typename... Ts>
    decltype(auto) variant_match(V&& v, Ts&&... ts)
    {
        return std::visit(Overloaded{FWD(ts)...}, FWD(v));
    }

    constexpr usz operator""_KiB(unsigned long long int n) { return 1024ULL * n; }
    constexpr usz operator""_MiB(unsigned long long int n) { return 1024_KiB * n; }

    struct alignas(1) Rgba
    {
        u8 a{0xFF};
        u8 b{};
        u8 g{};
        u8 r{};

        constexpr Rgba() = default;
        constexpr Rgba(u8 r, u8 g, u8 b, u8 a = 0xFF)
            : a(a)
            , b(b)
            , g(g)
            , r(r)
        {
        }

        static Rgba from_bgp(u8 bgp)
        {
            return Rgba{static_cast<u8>(bgp & 0x03), static_cast<u8>((bgp >> 2) & 0x03),
                        static_cast<u8>((bgp >> 4) & 0x03)};
        }
    };
} // namespace cb

template <typename EnumT>
    requires std::is_enum_v<EnumT>
struct fmt::formatter<EnumT> : fmt::formatter<std::underlying_type_t<EnumT>>
{
    auto format(const EnumT& enum_value, format_context& ctx) const
    {
        fmt::formatter<std::underlying_type_t<EnumT>> formatter;
        return formatter.format(cb::to_underlying(enum_value), ctx);
    }
};
