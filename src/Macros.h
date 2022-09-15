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


#define IFC(c) do { result = (c); if (FAILED(result)) return result; } while(false)

#define READ_WIC_STRING(f, out) do {                                    \
    uint32_t strLen = 0;                                                    \
    result = f(0, 0, &strLen);                                          \
    if (SUCCEEDED(result) && (strLen > 0)) {                            \
        result = f(strLen, out.GetBufferSetLength(strLen), &strLen);    \
        out.ReleaseBuffer(strLen - 1);                                  \
    } else { out = L""; } } while(0);

// Improved ATL macro that doesn't generate warning C26433 (override is missing
#define BEGIN_MSG_MAP_OVERRIDE() \
public: \
    BOOL ProcessWindowMessage(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam,\
        _In_ LPARAM lParam, _Inout_ LRESULT& lResult, _In_ DWORD dwMsgMapID = 0) override \
    { \
        BOOL bHandled = TRUE; \
        (hWnd); \
        (uMsg); \
        (wParam); \
        (lParam); \
        (lResult); \
        (bHandled); \
        switch(dwMsgMapID) \
        { \
        case 0:
