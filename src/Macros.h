// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

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

// Include additional headers as workaround that IntelliSense in VS 2020 17.9 fails to parse #import <win.h>
#ifdef __INTELLISENSE__
#define _AMD64_
#include <cassert>
#include <combaseapi.h>
#include <libloaderapi.h>
#include <sal.h>
#include <winreg.h>
#include <winerror.h>
#include <winnt.h>
#include <wincodec.h>
#endif
