#pragma once

#include "resource.h"
#include "Macros.h"

#include <atlwin.h>

class CViewInstalledCodecsDlg final : public CDialogImpl<CViewInstalledCodecsDlg>
{
public:
    enum { IDD = IDD_VIEW_INSTALLED_CODECS };

    WARNING_SUPPRESS_NEXT_LINE(26433) //  Function 'ProcessWindowMessage' should be marked with 'override' (c.128).
    BEGIN_MSG_MAP(CEncoderSelectionDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

private:
    LRESULT OnInitDialog(uint32_t /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) const;
    LRESULT OnCloseCmd(uint16_t /*wNotifyCode*/, uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};
