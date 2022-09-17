module;

#include "Macros.h"
#include "resource.h"
#include "ComSmartPointers.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>

module ViewInstalledCodecsDlg;

import Util;
import Element;

import <std.h>;


LRESULT CViewInstalledCodecsDlg::OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) const
{
    CListViewCtrl codecListView(::GetDlgItem(m_hWnd, IDC_LIST_CODECS));
    codecListView.InsertColumn(0, L"Friendly Name", LVCFMT_LEFT, 150, 0);
    codecListView.InsertColumn(1, L"Class Id", LVCFMT_LEFT, 150, 0);

    {
        IEnumUnknownPtr e;
        HRESULT result = g_imagingFactory->CreateComponentEnumerator(WICEncoder, WICComponentEnumerateRefresh, &e);
        if (SUCCEEDED(result))
        {
            ULONG num;
            IUnknownPtr unk;

            while (S_OK == e->Next(1, &unk, &num) && 1 == num)
            {
                IWICBitmapCodecInfoPtr encoderInfo{unk};

                // Get the name of the container
                std::wstring friendlyName;
                VERIFY(SUCCEEDED(GetWicString(*encoderInfo, &(IWICBitmapCodecInfo::GetFriendlyName), friendlyName)));

                const int id{codecListView.InsertItem(0, friendlyName.c_str())};

                GUID classId;
                encoderInfo->GetCLSID(&classId);
                codecListView.AddItem(id, 1, guid_to_string(classId).c_str());
            }
        }
    }

    {
        IEnumUnknownPtr e;
        HRESULT result = g_imagingFactory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateRefresh, &e);
        if (SUCCEEDED(result))
        {
            ULONG num;
            IUnknownPtr unk;

            while (S_OK == e->Next(1, &unk, &num) && 1 == num)
            {
                IWICBitmapCodecInfoPtr encoderInfo{unk};

                // Get the name of the container
                std::wstring friendlyName;
                VERIFY(SUCCEEDED(GetWicString(*encoderInfo, &(IWICBitmapCodecInfo::GetFriendlyName), friendlyName)));

                const int id{codecListView.InsertItem(0, friendlyName.c_str())};

                GUID classId;
                encoderInfo->GetCLSID(&classId);
                codecListView.AddItem(id, 1, guid_to_string(classId).c_str());
            }
        }
    }

    return 1;
}

LRESULT CViewInstalledCodecsDlg::OnCloseCmd(uint16_t /*wNotifyCode*/, const uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    // If we made it here, close the dialog box
    VERIFY(EndDialog(wID));

    return 0;
}
