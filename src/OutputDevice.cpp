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
#include <atlapp.h>
#include <atlctrls.h>

module OutputDevice;

import BitmapDataObject;
import Util;

namespace {

LPCWSTR NormalFontName = L"Verdana";
LPCWSTR VerbatimFontName = L"Lucida Console";

constexpr auto TEXT_SIZE{10};

}


CRichEditDevice::CRichEditDevice(CRichEditCtrl &richEditCtrl)
: m_richEditCtrl(richEditCtrl)
{
    SetFontName(NormalFontName);
    SetFontSize(TEXT_SIZE);
    SetBackgroundColor(GetSysColor(COLOR_INFOBK));
    SetTextColor(GetSysColor(COLOR_INFOTEXT));
}

void CRichEditDevice::SetBackgroundColor(const COLORREF color)
{
    m_richEditCtrl.SendMessage(EM_SETBKGNDCOLOR, 0, color);
    InvalidateRect(m_richEditCtrl.GetParent(), nullptr, true);
}

COLORREF CRichEditDevice::SetTextColor(const COLORREF color)
{
    CHARFORMAT2 cf{{.cbSize = sizeof cf}};
    m_richEditCtrl.GetSelectionCharFormat(cf);
    cf.dwEffects = 0;

    const COLORREF result = cf.crTextColor;

    cf.wWeight = FW_NORMAL;
    cf.dwMask = CFM_COLOR | CFM_WEIGHT;
    cf.crTextColor = color;

    m_richEditCtrl.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, reinterpret_cast<LPARAM>(&cf));

    return result;
}

void CRichEditDevice::SetHighlightColor(const COLORREF color) noexcept(false)
{
    CHARFORMAT2 cf{{.cbSize = sizeof cf, .dwMask = CFM_BACKCOLOR}, 0, 0, color};
    m_richEditCtrl.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, reinterpret_cast<LPARAM>(&cf));
}

void CRichEditDevice::SetFontName(const LPCWSTR name) noexcept(false)
{
    CHARFORMAT2 cf{{.cbSize = sizeof cf}, FW_NORMAL};

    const int len = static_cast<int>(wcslen(name));
    for (int i = 0; i < len; i++)
    {
        cf.szFaceName[i] = name[i];
    }
    cf.szFaceName[len] = L'\0';
    cf.dwMask = CFM_FACE | CFM_WEIGHT;

    m_richEditCtrl.SendMessage(EM_SETCHARFORMAT, SCF_SELECTION | SCF_WORD, reinterpret_cast<LPARAM>(&cf));
}

int CRichEditDevice::SetFontSize(const int pointSize)
{
    CHARFORMAT2 cf{{.cbSize = sizeof cf}};
    m_richEditCtrl.GetSelectionCharFormat(cf);
    cf.dwEffects = 0;

    const int result = cf.yHeight / 20;

    cf.wWeight = FW_NORMAL;
    cf.yHeight = 20 * pointSize; // Convert to twips
    cf.dwMask  = CFM_SIZE | CFM_WEIGHT;

    m_richEditCtrl.SetSelectionCharFormat(cf);

    return result;
}

void CRichEditDevice::BeginSection(const LPCWSTR name)
{
    // Add this new level to the output stack
    m_sections.push_back(std::wstring{name});

    // Actually write the section heading
    if ((nullptr != name) && (L'\0' != *name))
    {
        AddText(L"\n");

        const size_t headingSize = TEXT_SIZE + ((m_sections.size() <= 4) ? (4 - m_sections.size())*TEXT_SIZE/4 : 0);
        const int oldSize = SetFontSize(static_cast<int>(headingSize));
        const COLORREF oldColor = SetTextColor(RGB(192, 0, 0));

        AddText(name);

        SetTextColor(oldColor);
        SetFontSize(oldSize);

        AddText(L"\n\n");
    }
}

void CRichEditDevice::AddText(const LPCWSTR name)
{
    m_richEditCtrl.AppendText(name);
}

void CRichEditDevice::AddVerbatimText(const LPCWSTR name)
{
    SetFontName(VerbatimFontName);
    m_richEditCtrl.AppendText(name);
    SetFontName(NormalFontName);
}

void CRichEditDevice::AddDib(const HGLOBAL hBitmap)
{
    IRichEditOle* oleInterface = m_richEditCtrl.GetOleInterface();

    if (oleInterface)
    {
        const HRESULT result = CBitmapDataObject::InsertDib(m_richEditCtrl.m_hWnd, oleInterface, hBitmap);

        if (FAILED(result))
        {
            const COLORREF oldColor = SetTextColor(RGB(255, 0, 0));
#pragma warning(push)
#pragma warning(disable : 4296) // '<': expression is always false
            AddText(std::format(L"Failed to render bitmap: {}\n", GetHresultString(result)).c_str());
            SetTextColor(oldColor);
#pragma warning(pop)
        }
        else
        {
            AddText(L"\n");
        }

        oleInterface->Release();
    }
}

void CRichEditDevice::BeginKeyValues(const LPCWSTR name)
{
    // Write the heading if one was specified
    if ((nullptr != name) && (L'\0' != *name))
    {
        const COLORREF oldColor = SetTextColor(RGB(0, 0, 128));

        AddText(name);

        SetTextColor(oldColor);

        AddText(L"\n");
    }
}

void CRichEditDevice::AddKeyValue(const LPCWSTR key, const LPCWSTR value)
{
    const COLORREF oldColor = SetTextColor(RGB(0, 128, 0));

    AddText(key);

    SetTextColor(oldColor);

    AddText(L"\t\t");
    AddText(value);
    AddText(L"\n");
}

void CRichEditDevice::EndKeyValues()
{
    AddText(L"\n");
}

void CRichEditDevice::EndSection() noexcept
{
    if (m_sections.size() > 0)
    {
        m_sections.erase(m_sections.cbegin() + (m_sections.size() - 1));
    }
}
