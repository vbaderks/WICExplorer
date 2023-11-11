// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

#pragma once

#include "resource.h"
#include "Macros.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlsplit.h>
#include <atlctrls.h>
#include <atlctrlx.h>

import Element;

class CMainFrame final : public CFrameWindowImpl<CMainFrame>
{
public:
    WARNING_SUPPRESS_NEXT_LINE(26440) // Function 'CMainFrame::GetWndClassInfo' can be declared 'noexcept' (f.6).
        DECLARE_FRAME_WND_CLASS(nullptr, IDR_MAINFRAME)

    WARNING_SUPPRESS_NEXT_LINE(26433) // Function 'CMainFrame::ProcessWindowMessage' should be marked with 'override' (c.128).
    BEGIN_MSG_MAP(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)

        COMMAND_ID_HANDLER(ID_PANE_CLOSE, OnPaneClose)
        COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
        COMMAND_ID_HANDLER(ID_FILE_OPEN_DIR, OnFileOpenDir)
        COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnAppExit)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(ID_SHOW_VIEWPANE, OnShowViewPane)
        COMMAND_ID_HANDLER(ID_SHOW_ALPHA, OnShowAlpha)
        COMMAND_ID_HANDLER(ID_SHOW_INSTALLED_CODECS, OnShowInstalledCodecs)
        COMMAND_ID_HANDLER(ID_VIEW_NORMALIZE_HISTOGRAM, OnNormalizeHistogram)
        COMMAND_ID_HANDLER(ID_FILE_LOAD, OnContextClick)
        COMMAND_ID_HANDLER(ID_FILE_UNLOAD, OnContextClick)
        COMMAND_ID_HANDLER(ID_FILE_CLOSE, OnContextClick)
        COMMAND_ID_HANDLER(ID_FIND_METADATA, OnContextClick)

        WARNING_SUPPRESS_NEXT_LINE(26454) // Arithmetic overflow: '-' operation produces a negative unsigned result at compile time (io.5).
        NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeViewSelChanged)

        WARNING_SUPPRESS_NEXT_LINE(26454) // Arithmetic overflow: '-' operation produces a negative unsigned result at compile time (io.5).
        NOTIFY_CODE_HANDLER(NM_RCLICK, OnNMRClick)

        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

    HRESULT Load(const PCWSTR* filenames, int count);

private:
    HWND CreateClient();
    static int GetElementTreeImage(const CInfoElement* elem) noexcept;
    // Opens a single file
    HRESULT OpenFile(PCWSTR filename, bool& updateElements);
    // Opens files based on a wildcard expression (not recursive)
    HRESULT OpenWildcard(PCWSTR search, DWORD& attempted, DWORD& opened, bool& updateElements);
    // Opens images recursively in a directory
    HRESULT OpenDirectory(PCWSTR directory, DWORD& attempted, DWORD& opened);
    void UpdateTreeView(bool selectLastRoot);
    HTREEITEM BuildTree(const CInfoElement* elem, HTREEITEM hParent);
    static BOOL DoElementContextMenu(HWND hWnd, CInfoElement& element, POINT point) noexcept;
    static HMENU CreateElementContextMenu(CInfoElement& element) noexcept;
    CInfoElement* GetElementFromTreeItem(HTREEITEM hItem) const;
    HTREEITEM GetTreeItemFromElement(CInfoElement* element) const;
    HTREEITEM FindTreeItem(HTREEITEM start, CInfoElement* element) const;
    static bool ElementCanBeSavedAsImage(const CInfoElement& element) noexcept;
    HRESULT SaveElementAsImage(CInfoElement& element);
    void DrawElement(CInfoElement& element);
    HRESULT QueryMetadata(CInfoElement* elem);

    LRESULT OnCreate(uint32_t, WPARAM, LPARAM, BOOL&);
    LRESULT OnNMRClick(int, NMHDR* pnmh, BOOL&);
    LRESULT OnTreeViewSelChanged(WPARAM wParam, const NMHDR* lpNmHdr, BOOL& bHandled);
    LRESULT OnPaneClose(uint16_t, uint16_t, HWND hWndCtl, BOOL&) const;
    LRESULT OnFileOpen(uint16_t, uint16_t, HWND, BOOL&);
    LRESULT OnFileOpenDir(uint16_t code, uint16_t item, HWND hSender, BOOL& handled);
    LRESULT OnFileSave(uint16_t, uint16_t, HWND, BOOL&);
    LRESULT OnAppExit(uint16_t, uint16_t, HWND, BOOL&) noexcept;
    static LRESULT OnAppAbout(uint16_t, uint16_t, HWND, BOOL&);
    LRESULT OnShowViewPane(uint16_t code, uint16_t item, HWND hSender, BOOL& handled);
    LRESULT OnShowAlpha(uint16_t code, uint16_t item, HWND hSender, BOOL& handled);
    static LRESULT OnShowInstalledCodecs(uint16_t, uint16_t, HWND, BOOL&);
    LRESULT OnNormalizeHistogram(uint16_t code, uint16_t item, HWND hSender, BOOL& handled) noexcept;
    LRESULT OnContextClick(uint16_t code, uint16_t item, HWND hSender, BOOL& handled);

    InfoElementViewContext m_viewContext{false, true};
    CSplitterWindow m_mainSplit;
    CHorSplitterWindow m_infoSplit;

    CPaneContainer m_viewPane;

    CImageList    m_mainTreeImages;
    CTreeViewCtrl m_mainTree;
    CRichEditCtrl m_infoEdit;
    CRichEditCtrl m_viewEdit;

    bool m_suppressMessageBox{};
};
