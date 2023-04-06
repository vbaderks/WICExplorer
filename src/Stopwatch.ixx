// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "Macros.h"
#include <cassert>

export module Stopwatch;

import <std.h>;
import <Windows-import.h>;

export class CStopwatch final
{
public:
    CStopwatch() noexcept
    {
        VERIFY(QueryPerformanceFrequency(&m_frequency));
    }

    void Start() noexcept
    {
        VERIFY(QueryPerformanceCounter(&m_startTime));
    }

    [[nodiscard]] DWORD GetTimeMS() const noexcept
    {
        LARGE_INTEGER now;
        VERIFY(QueryPerformanceCounter(&now));

        const LONGLONG timeMS = (now.QuadPart - m_startTime.QuadPart) * static_cast<LONGLONG>(1000) / m_frequency.QuadPart;

        return static_cast<DWORD>(timeMS);
    }

private:
    LARGE_INTEGER m_frequency{};
    LARGE_INTEGER m_startTime{};
};

