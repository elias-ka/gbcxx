#pragma once

// The TRY macro is used for propagating errors from tl::expected.
// Since it uses a GCC extension, it isn't portable across all compiler.
// As far as I know only Clang and GCC support it.
#if __clang__ || __GNUC__
#define TRY(expr)                                                                                  \
    ({                                                                                             \
        auto&& result = (expr);                                                                    \
        if (!result.has_value())                                                                   \
        {                                                                                          \
            return tl::make_unexpected(std::move(result).error());                                 \
        }                                                                                          \
        *result;                                                                                   \
    })
#else
#error "Unsupported compiler: TRY macro is only supported by Clang and GCC"
#endif
