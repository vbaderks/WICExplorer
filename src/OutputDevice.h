//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

class IOutputDevice
{
public:
    IOutputDevice() = default;
    virtual ~IOutputDevice() = default;

    IOutputDevice(const IOutputDevice&) = default;
    IOutputDevice(IOutputDevice&&) = default;
    IOutputDevice& operator=(const IOutputDevice&) = default;
    IOutputDevice& operator=(IOutputDevice&&) = default;

    virtual void SetBackgroundColor(COLORREF color) = 0;
    virtual COLORREF SetTextColor(COLORREF color) = 0;
    virtual void SetHighlightColor(COLORREF color) = 0;

    virtual void SetFontName(LPCWSTR name) = 0;
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

class CRichEditDevice final : public IOutputDevice
{
public:
    explicit CRichEditDevice(CRichEditCtrl &richEditCtrl);

    void SetBackgroundColor(COLORREF color) override;
    COLORREF SetTextColor(COLORREF color) override;
    void SetHighlightColor(COLORREF color) override;

    void SetFontName(LPCWSTR name) override;
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
