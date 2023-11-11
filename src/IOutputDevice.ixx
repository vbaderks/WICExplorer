// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

export module IOutputDevice;

import <Windows-import.h>;

export class IOutputDevice
{
public:
    IOutputDevice() = default;
    virtual ~IOutputDevice() = default;

    IOutputDevice(const IOutputDevice&) = default;
    IOutputDevice(IOutputDevice&&) = default;
    IOutputDevice& operator=(const IOutputDevice&) = default;
    IOutputDevice& operator=(IOutputDevice&&) = default;

    virtual void SetBackgroundColor(COLORREF color) noexcept(false) = 0;
    virtual COLORREF SetTextColor(COLORREF color) = 0;
    virtual void SetHighlightColor(COLORREF color) noexcept(false) = 0;

    virtual void SetFontName(PCWSTR name) noexcept(false) = 0;
    virtual int SetFontSize(int pointSize) = 0;

    virtual void BeginSection(PCWSTR name) = 0;
    virtual void AddText(PCWSTR text) = 0;
    virtual void AddVerbatimText(PCWSTR text) = 0;
    virtual void AddDib(HGLOBAL hGlobal) = 0;
    virtual void BeginKeyValues(PCWSTR name) = 0;
    virtual void AddKeyValue(PCWSTR key, PCWSTR value) = 0;
    virtual void EndKeyValues() = 0;
    virtual void EndSection() = 0;
};
