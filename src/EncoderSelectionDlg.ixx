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

import <std.h>;


export class CEncoderSelectionDlg final : public CDialogImpl<CEncoderSelectionDlg>
{
public:
    enum { IDD = IDD_ENCODER_SELECTION };

    BEGIN_MSG_MAP_OVERRIDE()
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    [[nodiscard]] GUID GetContainerFormat() const noexcept;
    [[nodiscard]] GUID GetPixelFormat() const noexcept;

private:
    LRESULT OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
    LRESULT OnCloseCmd(uint16_t /*wNotifyCode*/, uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

    std::vector<GUID> m_containers;
    int m_containerSel{-1};

    std::vector<GUID> m_formats;
    int m_formatSel{-1};
};
