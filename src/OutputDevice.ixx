// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

// ReSharper disable once CppUnusedIncludeDirective (atlapp.h requires atlbase.h to be included first)
#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>

export module OutputDevice;

import IOutputDevice;

import <std.h>;

export class CRichEditDevice final : public IOutputDevice
{
public:
    explicit CRichEditDevice(CRichEditCtrl &richEditCtrl);

    void SetBackgroundColor(COLORREF color)  noexcept(false) override;
    COLORREF SetTextColor(COLORREF color) override;
    void SetHighlightColor(COLORREF color) noexcept(false) override;

    void SetFontName(PCWSTR name) noexcept(false) override;
    int SetFontSize(int pointSize) override;

    void BeginSection(PCWSTR name) override;
    void AddText(PCWSTR name) override;
    void AddVerbatimText(PCWSTR name) override;
    void AddDib(HGLOBAL hBitmap) override;
    void BeginKeyValues(PCWSTR name) override;
    void AddKeyValue(PCWSTR key, PCWSTR value) override;
    void EndKeyValues() override;
    void EndSection() noexcept override;

private:
    std::vector<std::wstring> m_sections;
    CRichEditCtrl& m_richEditCtrl;
};
