//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------

module;

#include <atlstr.h>

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>

#include "resource.h"
#include "Macros.h"
#include "ComSmartPointers.h"

module EncoderSelectionDlg;

import Element;


GUID CEncoderSelectionDlg::GetContainerFormat()
{
    return m_containers[m_containerSel];
}

GUID CEncoderSelectionDlg::GetPixelFormat()
{
    return m_formats[m_formatSel];
}

LRESULT CEncoderSelectionDlg::OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
    CenterWindow(GetParent());

    CListViewCtrl containerList(::GetDlgItem(m_hWnd, IDC_ENCODER_LIST));
    containerList.InsertColumn(0, L"Friendly Name", LVCFMT_LEFT, 150, 0);

    // Enumerate all of the encoders installed on the system
    m_containers.clear();

    IEnumUnknownPtr e;
    HRESULT result = g_imagingFactory->CreateComponentEnumerator(WICEncoder, WICComponentEnumerateRefresh, &e);
    if (SUCCEEDED(result))
    {
        ULONG num = 0;
        IUnknownPtr unk;

        while (S_OK == e->Next(1, &unk, &num) && 1 == num)
        {
            CString friendlyName;
            IWICBitmapEncoderInfoPtr encoderInfo = unk;

            // Get the name of the container
            READ_WIC_STRING(encoderInfo->GetFriendlyName, friendlyName)

            // Add the container to the ListView
            const int idx = containerList.InsertItem(0, friendlyName);
            containerList.SetItemData(idx, static_cast<DWORD_PTR>(m_containers.size()));

            // Add the container to the list of containers
            GUID containerFormat{};
            encoderInfo->GetContainerFormat(&containerFormat);
            m_containers.push_back(containerFormat);
        }
    }

    // Select the first Encoder in the list
    containerList.SelectItem(0);

    CListViewCtrl formatList(::GetDlgItem(m_hWnd, IDC_FORMAT_LIST));
    formatList.InsertColumn(0, L"Friendly Name", LVCFMT_LEFT, 170, 0);

    // Enumerate all of the pixel formats installed on the system
    m_formats.clear();

    e = nullptr;
    result = g_imagingFactory->CreateComponentEnumerator(WICPixelFormat, WICComponentEnumerateRefresh, &e);
    if (SUCCEEDED(result))
    {
        ULONG num = 0;
        IUnknownPtr unk;

        while (S_OK == e->Next(1, &unk, &num) && 1 == num)
        {
            CString friendlyName;
            IWICPixelFormatInfoPtr formatInfo = unk;

            // Get the name of the format
            READ_WIC_STRING(formatInfo->GetFriendlyName, friendlyName)

            // Add the format to the ListView
            const int idx = formatList.InsertItem(0, friendlyName);
            formatList.SetItemData(idx, static_cast<DWORD_PTR>(m_formats.size()));

            // Add the format to the list of containers
            GUID pixelFormat{};
            formatInfo->GetFormatGUID(&pixelFormat);
            m_formats.push_back(pixelFormat);
        }
    }

    //Add in "don't care" as the default pixel format
    const int idx = formatList.InsertItem(0, L"Don't care");
    formatList.SetItemData(idx, static_cast<DWORD_PTR>(m_formats.size()));
    m_formats.push_back(GUID_WICPixelFormatDontCare);

    // Select the first Format in the list (don't care)
    formatList.SelectItem(idx);

    return 1;
}

LRESULT CEncoderSelectionDlg::OnCloseCmd(uint16_t /*wNotifyCode*/, const uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // If selecting OK, validate that something is selected
    if (IDOK == wID)
    {
        const CListViewCtrl containerList(::GetDlgItem(m_hWnd, IDC_ENCODER_LIST));
        int selIdx = containerList.GetSelectedIndex();

        if ((selIdx < 0) || (selIdx >= static_cast<int>(m_containers.size())))
        {
            ::MessageBoxW(m_hWnd, L"Please select one of the encoders from the list before continuing.",
                L"No Encoder Was Selected", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        m_containerSel = static_cast<int>(containerList.GetItemData(selIdx));

        const CListViewCtrl formatList(::GetDlgItem(m_hWnd, IDC_FORMAT_LIST));
        selIdx = formatList.GetSelectedIndex();

        if ((selIdx < 0) || (selIdx >= static_cast<int>(m_formats.size())))
        {
            ::MessageBoxW(m_hWnd, L"Please select one of the pixel formats from the list before continuing.",
                L"No Pixel Format Was Selected", MB_OK | MB_ICONINFORMATION);
            return 0;
        }

        m_formatSel = static_cast<int>(formatList.GetItemData(selIdx));
    }

    // If we made it here, close the dialog box
    EndDialog(wID);

    return 0;
}

