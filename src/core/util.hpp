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

#define DIE(...)                 \
    do {                         \
        LOG_ERROR(__VA_ARGS__);  \
        std::exit(EXIT_FAILURE); \
    } while (0)

#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ALWAYS_INLINE inline
#endif

#define GB_ASSERT(_cond)                                            \
    (                                                               \
        [&]()                                                       \
        {                                                           \
            /* NOLINTNEXTLINE(readability-simplify-boolean-expr) */ \
            if (!(_cond)) [[unlikely]]                              \
                DIE("Assertion failed!");                           \
        }())

#define GB_ASSERT_MSG(_cond, ...)                                                      \
    (                                                                                  \
        [&]()                                                                          \
        {                                                                              \
            /* NOLINTNEXTLINE(readability-simplify-boolean-expr) */                    \
            if (!(_cond)) [[unlikely]]                                                 \
                DIE("{}:{}: Assertion failed: " __VA_ARGS__, __FILE_NAME__, __LINE__); \
        }())

namespace gb
{
namespace fs
{
[[nodiscard]] std::vector<uint8_t> ReadFile(const std::filesystem::path& path);
}

template <typename... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename V, typename... Ts>
decltype(auto) VariantMatch(V&& v, Ts&&... ts)
{
    return std::visit(Overloaded{FWD(ts)...}, FWD(v));
}

template <size_t Offset, typename T>
constexpr T GetBit(T value)
{
    return (value >> Offset) & T{1};
}

template <size_t Offset, typename T>
constexpr T SetBit(T value)
{
    return value | (T{1} << Offset);
}

template <size_t Offset, typename T>
constexpr T ClearBit(T value)
{
    return value & ~(T{1} << Offset);
}

namespace literals
{
constexpr size_t operator""_KiB(unsigned long long int n) { return 1024ULL * n; }
constexpr size_t operator""_MiB(unsigned long long int n) { return 1024_KiB * n; }
}  // namespace literals
using namespace literals;

}  // namespace gb

template <typename EnumT>
    requires std::is_enum_v<EnumT>
struct fmt::formatter<EnumT> : fmt::formatter<std::underlying_type_t<EnumT>>
{
    auto format(const EnumT& enum_value, format_context& ctx) const
    {
        fmt::formatter<std::underlying_type_t<EnumT>> fmtr;
        return fmtr.format(std::to_underlying(enum_value), ctx);
    }
};
