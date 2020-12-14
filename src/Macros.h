#pragma once

#define WARNING_SUPPRESS_NEXT_LINE(x) __pragma(warning(suppress \
                                                            : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)


#ifdef NDEBUG

#define ASSERT(expression) static_cast<void>(0)
#define VERIFY(expression) static_cast<void>(expression)

#else

#define ASSERT(expression)                 \
    __pragma(warning(push))                \
        __pragma(warning(disable : 26493)) \
            assert(expression)             \
                __pragma(warning(pop))
#define VERIFY(expression) assert(expression)

#endif
