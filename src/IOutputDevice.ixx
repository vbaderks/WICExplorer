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

    virtual void SetFontName(LPCWSTR name) noexcept(false) = 0;
    virtual int SetFontSize(int pointSize) = 0;

    virtual void BeginSection(LPCWSTR name) = 0;
    virtual void AddText(LPCWSTR text) = 0;
    virtual void AddVerbatimText(LPCWSTR text) = 0;
    virtual void AddDib(HGLOBAL hGlobal) = 0;
    virtual void BeginKeyValues(LPCWSTR name) = 0;
    virtual void AddKeyValue(LPCWSTR key, LPCWSTR value) = 0;
    virtual void EndKeyValues() = 0;
    virtual void EndSection() = 0;
};
