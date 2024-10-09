#pragma once

#include <cstdint>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <type_traits>
#include <vector>

#define FWD(...) static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)

#define BIT(n) (1UL << (n))

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
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    template <typename V, typename... Ts>
    decltype(auto) variant_match(V&& v, Ts&&... ts)
    {
        return std::visit(overloaded{FWD(ts)...}, FWD(v));
    }

    constexpr usz operator""_KiB(unsigned long long int n) { return 1024ULL * n; }
    constexpr usz operator""_MiB(unsigned long long int n) { return 1024_KiB * n; }

    struct alignas(1) rgba
    {
        u8 a{0xFF};
        u8 b{};
        u8 g{};
        u8 r{};

        constexpr rgba() = default;
        constexpr rgba(u8 r, u8 g, u8 b, u8 a = 0xFF)
            : a(a)
            , b(b)
            , g(g)
            , r(r)
        {
        }
    };
} // namespace cb

// The default logger (stdout, multi-threaded, colored) is fine
#define LOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define LOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define LOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define LOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define LOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define LOG_UNIMPLEMENTED(...) LOG_WARN("Unimplemented: {}:{}: {}", __FILE__, __LINE__, __VA_ARGS__)
