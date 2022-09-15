﻿//----------------------------------------------------------------------------------------
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

export module AboutDlg;


export class CAboutDlg final : public CDialogImpl<CAboutDlg>
{
public:
    enum { IDD = IDD_ABOUTBOX };

    BEGIN_MSG_MAP_OVERRIDE()
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) noexcept
    {
        CenterWindow(GetParent());
        return 1;
    }

    LRESULT OnCloseCmd(const uint16_t /*wNotifyCode*/, const uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
    {
        EndDialog(wID);
        return 0;
    }
};
