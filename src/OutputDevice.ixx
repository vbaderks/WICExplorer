//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlctrls.h>

export module OutputDevice;

import IOutputDevice;

export class CRichEditDevice final : public IOutputDevice
{
public:
    explicit CRichEditDevice(CRichEditCtrl &richEditCtrl);

    void SetBackgroundColor(COLORREF color)  noexcept(false) override;
    COLORREF SetTextColor(COLORREF color) override;
    void SetHighlightColor(COLORREF color) noexcept(false) override;

    void SetFontName(LPCWSTR name) noexcept(false) override;
    int SetFontSize(int pointSize) override;

    void BeginSection(LPCWSTR name) override;
    void AddText(LPCWSTR name) override;
    void AddVerbatimText(LPCWSTR name) override;
    void AddDib(HGLOBAL hBitmap) override;
    void BeginKeyValues(LPCWSTR name) override;
    void AddKeyValue(LPCWSTR key, LPCWSTR value) override;
    void EndKeyValues() override;
    void EndSection() override;

private:
    enum { TEXT_SIZE = 10 };

    CSimpleArray<CString> m_sections;
    CRichEditCtrl &m_richEditCtrl;
};
