#pragma once

#include <cstdint>
#include <filesystem>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <iostream>
#include <mutex>
#include <source_location>
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

    template <typename T>
    class range
    {
    public:
        range(T low, T high)
            : m_low(low)
            , m_high(high)
        {
        }
        bool contains(T value) const { return m_low <= value && value <= m_high; }

    private:
        T m_low;
        T m_high;
    };

    template <typename T>
    range<T> interval(T low, T high)
    {
        return range<T>(low, high);
    }

    struct rgba
    {
        u8 r{};
        u8 g{};
        u8 b{};
        u8 a{0xFF};

        constexpr rgba() = default;
        constexpr rgba(u8 r, u8 g, u8 b, u8 a = 0xFF)
            : r(r)
            , g(g)
            , b(b)
            , a(a)
        {
        }

        constexpr bool operator==(const rgba&) const = default;

        explicit constexpr operator u32() const
        {
            return (static_cast<u32>(r) << 24) | (static_cast<u32>(g) << 16) |
                   (static_cast<u32>(b) << 8) | (static_cast<u32>(a));
        }
    };

    enum class log_level : u8
    {
        trace,
        debug,
        unimplemented,
        info,
        warn,
        error,
    };

    class logger
    {
    public:
        static logger& instance()
        {
            static logger instance;
            return instance;
        }

        logger(const logger&) = delete;
        logger(logger&&) = delete;
        logger& operator=(const logger&) = delete;
        logger& operator=(logger&&) = delete;

        void set_log_level(log_level level) { m_level = level; }

        template <typename... Args>
        void trace(const fmt::format_string<Args...>& format_str, Args&&... args)
        {
            log(log_level::trace, fmt::fg(fmt::color::dark_gray), format_str, FWD(args)...);
        }

        template <typename... Args>
        void debug(const fmt::format_string<Args...>& format_str, Args&&... args)
        {
            log(log_level::debug, fmt::fg(fmt::color::gray), format_str, FWD(args)...);
        }

        template <typename... Args>
        void unimplemented(std::source_location loc, fmt::format_string<Args...> format_str,
                           Args&&... args)
        {
            log_with_source_location(loc, log_level::unimplemented,
                                     fmt::fg(fmt::color::light_green), format_str, FWD(args)...);
        }

        template <typename... Args>
        void info(fmt::format_string<Args...> format_str, Args&&... args)
        {
            log(log_level::info, fmt::fg(fmt::color::green), format_str, FWD(args)...);
        }

        template <typename... Args>
        void warn(fmt::format_string<Args...> format_str, Args&&... args)
        {
            log(log_level::warn, fmt::fg(fmt::color::yellow), format_str, FWD(args)...);
        }

        template <typename... Args>
        void error(fmt::format_string<Args...> format_str, Args&&... args)
        {
            log(log_level::error, fmt::fg(fmt::color::red), format_str, FWD(args)...);
        }

    private:
        logger() = default;
        ~logger() = default;

        template <typename... Args>
        void log(log_level level, fmt::text_style style, fmt::format_string<Args...> format_str,
                 Args&&... args)
        {
            if (level < m_level)
                return;

            const auto msg = fmt::vformat(format_str, fmt::make_format_args(args...));
            const auto formatted =
                fmt::format("{}: {}\n", fmt::styled(level, style | fmt::emphasis::bold), msg);
            std::lock_guard<std::mutex> lock(m_mutex);
            std::cout << formatted;
        }

        template <typename... Args>
        void log_with_source_location(std::source_location loc, log_level level,
                                      fmt::text_style style, fmt::format_string<Args...> format_str,
                                      Args&&... args)
        {
            if (level < m_level)
                return;

            const auto msg = fmt::vformat(format_str, fmt::make_format_args(args...));
            const auto formatted =
                fmt::format("{}{}: {}\n", fmt::styled(level, style | fmt::emphasis::bold),
                            fmt::styled(fmt::format("[{}:{}]", loc.file_name(), loc.line()),
                                        fmt::fg(fmt::color::gray)),
                            msg);
            std::lock_guard<std::mutex> lock(m_mutex);
            std::cout << formatted;
        }

    private:
        log_level m_level = log_level::info;
        std::mutex m_mutex;
    };
} // namespace cb

#ifndef NDEBUG
#define LOG_DEBUG(...) cb::logger::instance().debug(__VA_ARGS__)
#define LOG_TRACE(...) cb::logger::instance().trace(__VA_ARGS__)
#else
#define LOG_DEBUG(...) (void)0
#define LOG_TRACE(...) (void)0
#endif
#define LOG_UNIMPLEMENTED(...)                                                                     \
    cb::logger::instance().unimplemented(std::source_location::current(), __VA_ARGS__)
#define LOG_INFO(...) cb::logger::instance().info(__VA_ARGS__)
#define LOG_WARN(...) cb::logger::instance().warn(__VA_ARGS__)
#define LOG_ERROR(...) cb::logger::instance().error(__VA_ARGS__)

template <>
struct fmt::formatter<cb::log_level> : fmt::formatter<std::string_view>
{
    format_context::iterator format(cb::log_level level, format_context& ctx) const;
};
