//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include "resource.h"
#include "Macros.h"

#include <atlbase.h>
#include <atlwin.h>

export module EncoderSelectionDlg;


export class CEncoderSelectionDlg final : public CDialogImpl<CEncoderSelectionDlg>
{
public:
    enum { IDD = IDD_ENCODER_SELECTION };

    //CEncoderSelectionDlg() = default;

    WARNING_SUPPRESS_NEXT_LINE(26433) //  Function 'ProcessWindowMessage' should be marked with 'override' (c.128).
    BEGIN_MSG_MAP(CEncoderSelectionDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    GUID GetContainerFormat();
    GUID GetPixelFormat();

private:
    LRESULT OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(uint16_t /*wNotifyCode*/, uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    CSimpleArray<GUID> m_containers;
    int m_containerSel{-1};

    CSimpleArray<GUID> m_formats;
    int m_formatSel{-1};
};
