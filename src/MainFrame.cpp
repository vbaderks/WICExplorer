// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

#include "MainFrame.h"

#include <atltypes.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlframe.h>
#include <atlsplit.h>
#include <atldlgs.h>

#include "Macros.h"
#include "resource.h"
#include "ComSmartPointers.h"

import AboutDlg;
import OutputDevice;
import PropVariant;
import EncoderSelectionDlg;
import ViewInstalledCodecsDlg;
import Util;
import CodeGenerator;

import <std.h>;
import <Windows-import.h>;


LRESULT CMainFrame::OnCreate(uint32_t, WPARAM, LPARAM, BOOL&)
{
    const HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, false, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_TRANSPARENT | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NORESIZE | CCS_TOP);

    CreateSimpleReBar((ATL_SIMPLE_REBAR_NOBORDER_STYLE | CCS_TOP | RBS_DBLCLKTOGGLE) & ~RBS_AUTOSIZE);
    AddSimpleReBarBand(hWndToolBar, nullptr, true);

    CreateSimpleStatusBar();

    m_hWndClient = CreateClient();

    m_suppressMessageBox = false;
    m_viewContext.bIsAlphaEnable = true;

    return 0;
}

HWND CMainFrame::CreateClient()
{
    CRect clientRect;
    GetClientRect(&clientRect);

    ATLVERIFY(m_mainSplit.Create(m_hWnd, clientRect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_COMPOSITED));

    m_mainSplit.m_cxyMin = 35;                           // Minimum sizes
    m_mainSplit.SetSplitterExtendedStyle(0);             // Extend right pane when resizing main window
    m_mainSplit.m_bFullDrag = true;                      // Show contents during drag

    CRect leftRect;
    GetClientRect(&leftRect);

    ATLVERIFY(m_infoSplit.Create(m_mainSplit.m_hWnd, leftRect, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN));

    m_infoSplit.m_cxyMin = 35;                                 // Minimum sizes
    m_infoSplit.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED); // Extend right pane when resizing main window
    m_infoSplit.m_bFullDrag = true;                            // Show contents during drag

    m_mainSplit.SetSplitterPane(0, m_infoSplit);

    ATLVERIFY(m_viewPane.Create(m_mainSplit.m_hWnd));

    m_mainSplit.SetSplitterPane(1, m_viewPane);

    m_viewPane.SetTitle(L"View");

    m_mainTreeImages.CreateFromImage(IDB_MAINTREE, 16, 0, RGB(255, 255, 255), IMAGE_BITMAP, LR_LOADTRANSPARENT | LR_CREATEDIBSECTION);

    ATLVERIFY(m_mainTree.Create(m_infoSplit.m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | TVS_HASBUTTONS | TVS_TRACKSELECT | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE));

    m_mainTree.SetImageList(m_mainTreeImages, TVSIL_NORMAL);

    m_infoSplit.SetSplitterPane(0, m_mainTree);

    ATLVERIFY(m_infoEdit.Create(m_infoSplit.m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, WS_EX_CLIENTEDGE));

    m_infoEdit.SendMessage(EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_INFOBK));

    m_infoSplit.SetSplitterPane(1, m_infoEdit);

    ATLVERIFY(m_viewEdit.Create(m_viewPane.m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, WS_EX_CLIENTEDGE));

    m_viewEdit.SendMessage(EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_INFOBK));
    m_viewEdit.SetOptions(ECOOP_OR, ECO_AUTOVSCROLL);

    m_viewPane.SetClient(m_viewEdit.m_hWnd);

    m_mainSplit.SetSplitterPos(clientRect.Width() / 4);    // Default size of left pane
    m_infoSplit.SetSplitterPos((2 * leftRect.Height()) / 3); // Default size of bottom pane

    return m_mainSplit.m_hWnd;
}

LRESULT CMainFrame::OnPaneClose(uint16_t, uint16_t, const HWND hWndCtl, BOOL&) const
{
    // hide the container whose Close button was clicked. Use
    // DestroyWindow(hWndCtl) instead if you want to totally
    // remove the container instead of just hiding it
    ::ShowWindow(hWndCtl, SW_HIDE);

    // find the container's parent splitter
    const HWND hWnd = ::GetParent(hWndCtl);

#pragma warning( disable : 4312 )
    auto* pWnd = reinterpret_cast<CSplitterWindow*>(::GetWindowLong(hWnd, GWL_ID));
#pragma warning( default : 4312 ) // TODO: fix with push/pop

    // take the container that was Closed out of the splitter.
    // Use SetSplitterPane(nPane, nullptr) if you want to stay in
    // multi pane mode instead of changing to single pane mode
    constexpr int nCount{pWnd->m_nPanesCount};
    for (int nPane = 0; nPane < nCount; nPane++)
    {
        if (hWndCtl == pWnd->m_hWndPane[nPane])
        {
            pWnd->SetSinglePaneMode(nCount - nPane - 1);
            break;
        }
    }

    if (hWndCtl == m_viewPane.m_hWnd)
    {
        // Clear the "Show View Pane" check mark in the menu
        CheckMenuItem(GetMenu(), ID_SHOW_VIEWPANE, MF_UNCHECKED | MF_BYCOMMAND);
    }

    return 0;
}

CInfoElement* CMainFrame::GetElementFromTreeItem(const HTREEITEM hItem) const
{
    return reinterpret_cast<CInfoElement*>(m_mainTree.GetItemData(hItem));
}

HTREEITEM CMainFrame::GetTreeItemFromElement(CInfoElement* element) const
{
    return FindTreeItem(m_mainTree.GetRootItem(), element);
}

HTREEITEM CMainFrame::FindTreeItem(const HTREEITEM start, CInfoElement* element) const
{
    if (!start)
    {
        return nullptr;
    }
    if (GetElementFromTreeItem(start) == element)
    {
        return start;
    }
    HTREEITEM result;
    if ((result = FindTreeItem(m_mainTree.GetChildItem(start), element)) != nullptr)
    {
        return result;
    }
    return FindTreeItem(m_mainTree.GetNextSiblingItem(start), element);
}

int CMainFrame::GetElementTreeImage(const CInfoElement* elem) noexcept
{
    if (dynamic_cast<const CBitmapDecoderElement*>(elem))
    {
        return 151;
    }

    if (dynamic_cast<const CBitmapFrameDecodeElement*>(elem))
    {
        return 160;
    }

    if (dynamic_cast<const CMetadataReaderElement*>(elem))
    {
        return 7;
    }

    return 85;
}

HTREEITEM CMainFrame::BuildTree(const CInfoElement* elem, const HTREEITEM hParent)
{
    ATLASSERT(elem);

    if (elem)
    {
        const int image = GetElementTreeImage(elem);
        const uint32_t state = (nullptr == hParent) ? TVIS_BOLD : 0;

        const HTREEITEM hItem = m_mainTree.InsertItem(TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE,
            elem->Name().c_str(), image, image, state, state, 0, hParent, nullptr);

        // Set a pointer to the element in the tree
        m_mainTree.SetItemData(hItem, reinterpret_cast<DWORD_PTR>(elem));

        // Add children
        if (elem->FirstChild())
        {
            BuildTree(elem->FirstChild(), hItem);
        }

        // Add the siblings
        if (elem->NextSibling())
        {
            BuildTree(elem->NextSibling(), hParent);
        }

        // Expand this branch
        m_mainTree.Expand(hItem);

        return hItem;
    }

    return nullptr;
}

void CMainFrame::UpdateTreeView(const bool selectLastRoot)
{
    // This method simply destroys the entire tree, then re-creates it from scratch.
    m_mainTree.DeleteAllItems();

    const CInfoElement* root = CElementManager::GetRootElement();
    HTREEITEM rootItem = nullptr;
    if (root->FirstChild())
    {
        // BuildTree traverses by the first child and next sibling.
        // Therefore, all of the elements will be added except for root.
        rootItem = BuildTree(root->FirstChild(), nullptr);
    }

    if (selectLastRoot)
    {
        m_mainTree.SelectItem(rootItem);
        m_mainTree.SetFocus();
    }
}

HRESULT CMainFrame::OpenFile(const PCWSTR filename, bool& updateElements)
{
    updateElements = false;

    CInfoElement* newRoot{};
    CSimpleCodeGenerator codeGen;

    const HRESULT result = CElementManager::OpenFile(filename, codeGen, newRoot);

    if (SUCCEEDED(result))
    {
        updateElements = true;
    }
    else
    {
        std::wstring msg;

        if (result == E_FAIL)
        {
            msg = std::format(L"File \"{}\" loaded but contained 0 frames.\n\n", filename);
        }
        else
        {
            msg = std::format(L"Unable to load the file \"{}\". The error is: {}.\n\n", filename, GetHresultString(result));
        }

        if (nullptr != newRoot)
        {
            updateElements = true;
            msg = std::format(L"Unable to completely load the file \"{}\". The error is: {}. Some parts of the file may still be viewed.\n\n",
                filename, GetHresultString(result));
        }

        msg += codeGen.GenerateCode();

        if (!m_suppressMessageBox)
        {
            MessageBoxW(msg.c_str(), L"Error Opening File", MB_OK | MB_ICONWARNING);
        }
    }

    return result;
}

HRESULT CMainFrame::OpenWildcard(const PCWSTR search, DWORD& attempted, DWORD& opened, bool& updateElements)
{
    updateElements = false;
    HRESULT hr = S_OK;
    WIN32_FIND_DATA fdata;
    const HANDLE hf = FindFirstFile(search, &fdata);

    // fdata.cFileName does not store the directory, nor is the directory
    // stored anywhere else inside of fdata. The directory needs to be copied
    // from the search string, and then cFileName has to be concatenated to the
    // directory.
    wchar_t directoryPrefix[MAX_PATH * 2] = {0};
    const auto* lastSlash = wcsrchr(search, L'\\');
    const auto* lastSlash2 = wcsrchr(search, L'/');

    if (lastSlash2 > lastSlash)
    {
        lastSlash = lastSlash2;
    }

    size_t directorySize = 0;

    if (lastSlash != nullptr)
    {
        // The +1 is to include the slash.
        directorySize = (lastSlash + 1) - search;

        if (directorySize > MAX_PATH)
        {
            return E_INVALIDARG;
        }

        VERIFY(wcsncpy_s(directoryPrefix, ARRAYSIZE(directoryPrefix), search, directorySize) == 0);
    }

    if (hf == INVALID_HANDLE_VALUE)
    {
        if (!m_suppressMessageBox)
        {
            MessageBox(std::format(L"Could not open {}", search).c_str(), L"Error opening expression", MB_OK);
        }
        return 0;
    }

    do
    {
        HRESULT temp;
        bool updateThis{};

        // Concatenate the filename onto the directory.
        VERIFY(wcscpy_s(directoryPrefix + directorySize,
            ARRAYSIZE(directoryPrefix) - directorySize,
            fdata.cFileName) == 0);

        if (SUCCEEDED(temp = OpenFile(directoryPrefix, updateThis)))
        {
            opened++;
        }
        else
        {
            hr = temp;
        }

        attempted++;
        updateElements = updateElements || updateThis;
    } while (FindNextFile(hf, &fdata));

    FindClose(hf);

    return hr;
}

HRESULT CMainFrame::Load(const PCWSTR* filenames, const int count)
{
    HRESULT result = S_OK;
    bool needsUpdate{};

    DWORD attempted{};
    DWORD opened{};
    for (int i = 0; i < count; i++)
    {
        if (_wcsicmp(L"/quiet", filenames[i]) == 0)
        {
            m_suppressMessageBox = true;
        }
        else
        {
            bool thisNeedsUpdate{};
            result = OpenWildcard(filenames[i], attempted, opened, thisNeedsUpdate);
            needsUpdate = needsUpdate || thisNeedsUpdate;
        }
    }

    if (needsUpdate)
    {
        UpdateTreeView(true);
    }

    if (attempted > 1)
    {
        if (!m_suppressMessageBox)
        {
            MessageBox(std::format(L"Successfully opened {} out of {} image files", opened, attempted).c_str(), L"Done", MB_OK);
        }
    }

    return result;
}


LRESULT CMainFrame::OnFileOpen(uint16_t, uint16_t, const HWND hParentWnd, BOOL&)
{
    CSimpleFileDialog fileDlg(true, nullptr, nullptr, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT,
        L"All Files (*.*)\0*.*\0\0", hParentWnd);

    // Create a large buffer to hold the result of the filenames.
    std::vector<wchar_t> filenameBuffer(16 * 1024);
    fileDlg.m_ofn.lpstrFile = filenameBuffer.data();
    fileDlg.m_ofn.nMaxFile = static_cast<DWORD>(filenameBuffer.size());

    // Bring up the dialog
    const INT_PTR result{fileDlg.DoModal()};

    if (result == IDOK)
    {
        bool updateElements{};

        // If m_ofn.lpstrFileTitle is empty, then there are multiple files. Otherwise, there is
        // just one file. We need to handle both cases.
        if (L'\0' != *fileDlg.m_ofn.lpstrFileTitle)
        {
            // Just one file
            OpenFile(fileDlg.m_ofn.lpstrFile, updateElements);
        }
        else
        {
            // Get the root directory to the files
            const std::filesystem::path rootDirectory{fileDlg.m_ofn.lpstrFile};

            // Get each of the files
            wchar_t* filenamePtr = fileDlg.m_ofn.lpstrFile + rootDirectory.wstring().length() + 1;
            while (L'\0' != *filenamePtr)
            {
                // Try opening the file
                bool fileWantsUpdate{};

                OpenFile((rootDirectory / filenamePtr).c_str(), fileWantsUpdate);

                updateElements = updateElements || fileWantsUpdate;

                // Go for the next file
                filenamePtr = filenamePtr + wcslen(filenamePtr) + 1;
            }
        }

        // Update the tree if necessary
        if (updateElements)
        {
            UpdateTreeView(true);
        }
    }

    return 0;
}

HRESULT CMainFrame::OpenDirectory(const PCWSTR directory, DWORD& attempted, DWORD& opened)
{
    HRESULT hr = S_OK;
    WIN32_FIND_DATA fdata;
    const HANDLE hf{FindFirstFile((std::filesystem::path(directory) / L"*").c_str(), &fdata)};

    if (hf == INVALID_HANDLE_VALUE)
    {
        if (!m_suppressMessageBox)
        {
            MessageBox(std::format(L"Could not open {}", directory).c_str(), L"Error opening directory", MB_OK);
        }
        return 0;
    }

    do
    {
        const auto path = std::filesystem::path(directory) / fdata.cFileName;
        const auto extension = std::filesystem::path(fdata.cFileName).extension();
        HRESULT temp;

        if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
            wcscmp(fdata.cFileName, L".") != 0 &&
            wcscmp(fdata.cFileName, L"..") != 0)
        {
            if (FAILED(temp = OpenDirectory(path.c_str(), attempted, opened)))
            {
                hr = temp;
            }
        }
        else if (!_wcsicmp(extension.c_str(), L".jpg") ||
            !_wcsicmp(extension.c_str(), L".jpeg") ||
            !_wcsicmp(extension.c_str(), L".png") ||
            !_wcsicmp(extension.c_str(), L".gif") ||
            !_wcsicmp(extension.c_str(), L".bmp") ||
            !_wcsicmp(extension.c_str(), L".tif") ||
            !_wcsicmp(extension.c_str(), L".tiff") ||
            !_wcsicmp(extension.c_str(), L".ico") ||
            !_wcsicmp(extension.c_str(), L".icon") ||
            !_wcsicmp(extension.c_str(), L".dds"))
        {
            bool updateThis{};
            if (SUCCEEDED(temp = OpenFile(path.c_str(), updateThis)))
            {
                opened++;
            }
            else
            {
                hr = temp;
            }
            attempted++;
        }
    } while (FindNextFile(hf, &fdata));

    FindClose(hf);

    return hr;
}

LRESULT CMainFrame::OnFileOpenDir(uint16_t /*code*/, uint16_t /*item*/, const HWND hSender, BOOL& handled)
{
    CFolderDialog fileDlg(hSender,
        L"Select directory with *.jpg, *.png, *.gif, *.bmp, *.tif, *.ico, or *.dds",
        BIF_VALIDATE | BIF_EDITBOX);

    handled = 1;

    // Bring up the dialog
    if (fileDlg.DoModal() != IDOK)
    {
        return 0;
    }

    DWORD attempted{};
    DWORD opened{};
    OpenDirectory(fileDlg.GetFolderPath(), attempted, opened);

    UpdateTreeView(true);
    if (!m_suppressMessageBox)
    {
        wchar_t buffer[60];
        swprintf_s(buffer, 60, L"Successfully opened %lu out of %lu image files", opened, attempted);
        MessageBox(buffer, L"Done", MB_OK);
    }

    return 0;
}

void CMainFrame::DrawElement(CInfoElement& element)
{
    // Clear the RichEdits
    m_viewEdit.SetSelAll();
    m_viewEdit.ReplaceSel(L"");
    m_infoEdit.SetSelAll();
    m_infoEdit.ReplaceSel(L"");

    // Display the view -- prepend it with a path to the selected element
    auto path{element.Name()};
    CInfoElement* parent = &element;

    while (nullptr != parent->Parent())
    {
        path = parent->Parent()->Name() + L"\\" + path;
        parent = parent->Parent();
    }

    CRichEditDevice view(m_viewEdit);
    view.BeginSection(path.c_str());
    element.OutputView(view, m_viewContext);
    view.EndSection();

    m_viewEdit.SetSel(0, 0);

    // Display the info
    CRichEditDevice info(m_infoEdit);
    element.OutputInfo(info);

    m_infoEdit.SetSel(0, 0);
}

LRESULT CMainFrame::OnTreeViewSelChanged(WPARAM /*wParam*/, const NMHDR* lpNmHdr, BOOL& bHandled)
{
    bHandled = true;

    const auto* lpNmTreeView = reinterpret_cast<const NMTREEVIEWW*>(lpNmHdr);

    CInfoElement* elem = GetElementFromTreeItem(lpNmTreeView->itemNew.hItem);

    if (elem)
    {
        DrawElement(*elem);
    }

    return 0;
}

LRESULT CMainFrame::OnNMRClick(int, NMHDR* pnmh, BOOL&)
{
    // Get the location of the click point in the window
    POINT pt;
    GetCursorPos(&pt);

    POINT ptClient = pt;
    if (nullptr != pnmh->hwndFrom)
    {
        ::ScreenToClient(pnmh->hwndFrom, &ptClient);
    }

    // If it was a right-click in the tree view, then bring up the context menu.
    if (pnmh->hwndFrom == m_mainTree.m_hWnd)
    {
        TVHITTESTINFO tvhti{};
        tvhti.pt = ptClient;
        m_mainTree.HitTest(&tvhti);

        if (0 != (tvhti.flags & TVHT_ONITEMLABEL))
        {
            CInfoElement* elem = GetElementFromTreeItem(tvhti.hItem);
            if (nullptr != elem)
            {
                m_mainTree.SelectItem(tvhti.hItem);
                DoElementContextMenu(::GetParent(m_mainTree.m_hWnd), *elem, pt);
            }
        }
    }

    return 0;
}

HMENU CMainFrame::CreateElementContextMenu(CInfoElement& element) noexcept
{
    const HMENU result = CreatePopupMenu();

    // TODO: why is this MENUITEMINFO  not used?
    //MENUITEMINFO itemInfo{ .cbSize = sizeof itemInfo, .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING, .fType = MFT_STRING };

    element.FillContextMenu(result);

    return result;
}

BOOL CMainFrame::DoElementContextMenu(const HWND hWnd, CInfoElement& element, const POINT point) noexcept
{
    const HMENU hMenu = CreateElementContextMenu(element);
    if (!hMenu)
    {
        return false;
    }

    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, 0, hWnd, nullptr);
    DestroyMenu(hMenu);

    return true;
}

LRESULT CMainFrame::OnFileSave(uint16_t, uint16_t, HWND, BOOL&)
{
    // Get the currently selected tree node.
    const HTREEITEM hItem = m_mainTree.GetSelectedItem();

    if (hItem)
    {
        CInfoElement* elem = GetElementFromTreeItem(hItem);
        if (nullptr != elem)
        {
            SaveElementAsImage(*elem);
        }
    }

    return 0;
}

bool CMainFrame::ElementCanBeSavedAsImage(const CInfoElement& element) noexcept
{
    const CInfoElement* infoElement{&element};

    return ((nullptr != dynamic_cast<const CBitmapDecoderElement*>(infoElement)) ||
        (nullptr != dynamic_cast<const CBitmapSourceElement*>(infoElement)));
}


HRESULT CMainFrame::SaveElementAsImage(CInfoElement& element)
{
    HRESULT result = E_UNEXPECTED;

    if (ElementCanBeSavedAsImage(element))
    {
        // Let the user choose the type of encoder to use
        const auto encoderSelection{GetEncoderSelectionFromUser()};

        if (encoderSelection.has_value())
        {
            // Now that we know what kind of encoder they want to use, let's get a filename from them
            CSimpleFileDialog fileDlg(false, nullptr, nullptr, OFN_HIDEREADONLY,
                L"All Files (*.*)\0*.*\0\0", m_hWnd);
            const INT_PTR id{fileDlg.DoModal()};

            if (IDOK == id)
            {
                // Now we have a filename and an encoder, let's go
                CSimpleCodeGenerator codeGen;

                const auto& [containerFormat, pixelFormat] = *encoderSelection;
                GUID format{pixelFormat};
                result = CElementManager::SaveElementAsImage(element, containerFormat, format, fileDlg.m_szFileName, codeGen);

                if (FAILED(result))
                {
                    auto msg{std::format(L"Unable to encode '{}' as '{}'. The error is: {}.\n\n",
                        element.Name(), fileDlg.m_szFileName, GetHresultString(result))};
                    msg += codeGen.GenerateCode();

                    if (!m_suppressMessageBox)
                    {
                        MessageBoxW(msg.c_str(), L"Error Encoding Image", MB_OK | MB_ICONERROR);
                    }
                }
                else if (pixelFormat != GUID_WICPixelFormatDontCare &&
                    pixelFormat != format)
                {
                    // The user cares about the pixel format, and WIC actually used
                    // a different one than what the user picked.
                    wchar_t picked[30];
                    wchar_t actual[30];
                    if (FAILED(GetPixelFormatName(picked, ARRAYSIZE(picked), pixelFormat)))
                    {
                        VERIFY(wcscpy_s(picked, ARRAYSIZE(picked), L"Unknown") == 0);
                    }
                    if (FAILED(GetPixelFormatName(actual, ARRAYSIZE(actual), format)))
                    {
                        VERIFY(wcscpy_s(actual, ARRAYSIZE(actual), L"Unknown") == 0);
                    }

                    if (!m_suppressMessageBox)
                    {
                        MessageBoxW(std::format(L"You specified '{}' as the pixel format and WIC used '{}'", picked, actual).c_str(), L"Different format picked", MB_OK);
                    }
                }
            }
        }
        else
        {
            result = S_OK;
        }
    }
    else
    {
        if (!m_suppressMessageBox)
        {
            MessageBox(std::format(L"The selected element '{}' cannot be saved as an Image.", element.Name()).c_str(),
                L"Cannot Save Element", MB_OK | MB_ICONERROR);
        }
    }

    return result;
}

LRESULT CMainFrame::OnAppExit(uint16_t, uint16_t, HWND, BOOL&) noexcept
{
    PostMessage(WM_CLOSE);

    return 0;
}

LRESULT CMainFrame::OnAppAbout(uint16_t, uint16_t, HWND, BOOL&)
{
    ShowAboutDialogModal();
    return 0;
}

constexpr HRESULT ERROR_BLOCK_READER = MAKE_HRESULT(1, 0x899, 1);

namespace {

    HRESULT GetReaderFromQueryReader(IWICMetadataQueryReader* queryReader, IWICMetadataReader** reader)
    {
        HRESULT result = S_OK;

        // This will take the query reader and copy it into a CDummyBlockWriter.
        // Internally, AddWriter will be called with the unabstracted metadata reader.
        class CDummyBlockWriter final :
            public IWICMetadataBlockWriter
        {
        public:
            IWICMetadataWriterPtr writer;
            IWICMetadataBlockReaderPtr blockReader;

            ULONG STDMETHODCALLTYPE AddRef() noexcept override
            {
                return 0;
            }

            ULONG STDMETHODCALLTYPE Release() noexcept override
            {
                return 0;
            }

            HRESULT __stdcall QueryInterface(const IID& id, void** dest) noexcept override
            {
                if (id == IID_IWICMetadataBlockWriter || id == IID_IWICMetadataBlockReader)
                {
                    *dest = this;
                    return S_OK;
                }

                return E_NOINTERFACE;
            }

            HRESULT __stdcall InitializeFromBlockReader(IWICMetadataBlockReader* pIMDBlockReader) noexcept override
            {
                blockReader = pIMDBlockReader;
                return S_OK;
            }

            HRESULT __stdcall GetWriterByIndex(uint32_t /*nIndex*/, IWICMetadataWriter** ppIMetadataWriter) noexcept override
            {
                *ppIMetadataWriter = nullptr;
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall AddWriter(IWICMetadataWriter* pIMetadataWriter) noexcept override
            {
                if (writer)
                {
                    return CO_E_NOT_SUPPORTED;
                }
                writer = pIMetadataWriter;
                return S_OK;
            }

            HRESULT __stdcall SetWriterByIndex(uint32_t /*nIndex*/, IWICMetadataWriter* /*pIMetadataWriter*/) noexcept override
            {
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall RemoveWriterByIndex(uint32_t /*nIndex*/) noexcept override
            {
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall GetContainerFormat(GUID* /*pguidContainerFormat*/) noexcept override
            {
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall GetCount(uint32_t* pcCount) noexcept override
            {
                *pcCount = 0;
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall GetReaderByIndex(uint32_t /*nIndex*/, IWICMetadataReader** ppIMetadataReader) noexcept override
            {
                *ppIMetadataReader = nullptr;
                return CO_E_NOT_SUPPORTED;
            }

            HRESULT __stdcall GetEnumerator(IEnumUnknown** ppIEnumMetadata) noexcept override
            {
                *ppIEnumMetadata = nullptr;
                return CO_E_NOT_SUPPORTED;
            }
        } dummyWriter;

        // These lower level metadata data functions will require the component factory.
        IWICComponentFactoryPtr componentFactory;
        IFC(g_imagingFactory->QueryInterface(IID_PPV_ARGS(&componentFactory)));

        // This takes the IWICMetadataBlockWriter and wraps it as a IWICMetadataQueryWriter.
        // This is necessary because only query writer to query writer copying is supported.
        IWICMetadataQueryWriterPtr writerWrapper;
        IFC(componentFactory->CreateQueryWriterFromBlockWriter(&dummyWriter, &writerWrapper));

        // Prepare the propvariant for copying the query reader.
        PROPVARIANT propValue;
        PropVariantInit(&propValue);
        propValue.vt = VT_UNKNOWN;
        propValue.punkVal = queryReader;
        propValue.punkVal->AddRef();

        // Writing to "/" performs the special operation of copying from another query writer.
        IFC(writerWrapper->SetMetadataByName(L"/", &propValue));
        PropVariantClear(&propValue);

        // At this point, dummyWriter.writer should be set to the unabstracted query reader.
        if (dummyWriter.writer == 0)
        {
            if (dummyWriter.blockReader)
            {
                return ERROR_BLOCK_READER;
            }
            return E_FAIL;
        }

        IFC(dummyWriter.writer->QueryInterface(IID_PPV_ARGS(reader)));

        return result;
    }
}

LRESULT CMainFrame::OnShowViewPane(uint16_t /*code*/, const uint16_t item, HWND /*hSender*/, BOOL& handled)
{
    const HMENU menu = GetMenu();
    MENUITEMINFO currentState{.cbSize = sizeof currentState, .fMask = MIIM_STATE};
    GetMenuItemInfo(menu, item, false, &currentState);

    if ((currentState.fState & MFS_CHECKED) == MFS_CHECKED)
    {
        // Hide it
        OnPaneClose(0, 0, m_viewPane.m_hWnd, handled);
    }
    else
    {
        // Show it
        m_mainSplit.SetSinglePaneMode(SPLIT_PANE_NONE);

        // Set the check mark
        CheckMenuItem(menu, item, MF_CHECKED | MF_BYCOMMAND);
        handled = 1;
    }

    return 0;
}

LRESULT CMainFrame::OnShowAlpha(uint16_t /*code*/, const uint16_t item, HWND /*hSender*/, BOOL& handled)
{
    const HMENU menu = GetMenu();
    MENUITEMINFO currentState{.cbSize = sizeof currentState, .fMask = MIIM_STATE};

    GetMenuItemInfo(menu, item, false, &currentState);
    if ((currentState.fState & MFS_CHECKED) == MFS_CHECKED)
    {
        m_viewContext.bIsAlphaEnable = false;
        CheckMenuItem(menu, item, MF_UNCHECKED | MF_BYCOMMAND);
    }
    else
    {
        m_viewContext.bIsAlphaEnable = true;
        CheckMenuItem(menu, item, MF_CHECKED | MF_BYCOMMAND);
        handled = 1;
    }

    const HTREEITEM hItem = m_mainTree.GetSelectedItem();
    CInfoElement* elem = GetElementFromTreeItem(hItem);
    if (elem)
    {
        DrawElement(*elem);
    }

    return 0;
}

LRESULT CMainFrame::OnShowInstalledCodecs(uint16_t, uint16_t, HWND, BOOL&)
{
    ViewInstalledCodecsModal();
    return 0;
}

LRESULT CMainFrame::OnNormalizeHistogram(uint16_t /*code*/, uint16_t item, HWND /*hSender*/, BOOL& handled) noexcept
{
    const HMENU menu{GetMenu()};
    MENUITEMINFO currentState{.cbSize = sizeof currentState, .fMask = MIIM_STATE};

    GetMenuItemInfo(menu, item, false, &currentState);
    if ((currentState.fState & MFS_CHECKED) == MFS_CHECKED)
    {
        m_viewContext.normalizeHistogram = false;
        CheckMenuItem(menu, item, MF_UNCHECKED | MF_BYCOMMAND);
    }
    else
    {
        m_viewContext.normalizeHistogram = true;
        CheckMenuItem(menu, item, MF_CHECKED | MF_BYCOMMAND);
        handled = 1;
    }

    return 0;
}

LRESULT CMainFrame::OnContextClick(const uint16_t /*code*/, const uint16_t item, HWND /*hSender*/, BOOL& handled)
{
    handled = true;
    const HTREEITEM hItem = m_mainTree.GetSelectedItem();
    CInfoElement* elem = GetElementFromTreeItem(hItem);

    switch (item)
    {
    case ID_FILE_LOAD:
    {
        CSimpleCodeGenerator temp;
        dynamic_cast<CBitmapDecoderElement*>(elem)->Load(temp);
        UpdateTreeView(false);
        DrawElement(*elem);
    }
    break;
    case ID_FILE_UNLOAD:
        static_cast<CBitmapDecoderElement*>(elem)->Unload();
        UpdateTreeView(false);
        DrawElement(*elem);
        break;
    case ID_FILE_CLOSE:
        CElementManager::GetRootElement()->RemoveChild(dynamic_cast<CBitmapDecoderElement*>(elem));
        UpdateTreeView(false);
        break;
    case ID_FIND_METADATA:
    {
        const HRESULT result = QueryMetadata(elem);
        if (FAILED(result))
        {
            if (!m_suppressMessageBox)
            {
                MessageBoxW(std::format(L"Unable find metadata. The error is: {}", GetHresultString(result)).c_str(), L"Error Finding Metadata", MB_OK | MB_ICONWARNING);
            }
        }
    }
    break;
    default:
        break; // do nothing
    }

    return 0;
}

#pragma warning(push)
#pragma warning(disable: 26430) // Symbol 'elem' is not tested for nullness on all paths(f.23).
HRESULT CMainFrame::QueryMetadata(CInfoElement* elem)
{
    HRESULT result = S_OK;

    class CQLPath final : public ATL::CDialogImpl<CQLPath>
    {
    public:
        std::wstring m_path;
        enum { IDD = IDD_QLPATH };

        WARNING_SUPPRESS_NEXT_LINE(26433) //  Function 'ProcessWindowMessage' should be marked with 'override' (c.128).
        BEGIN_MSG_MAP(CQLPath)
            COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
            COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        END_MSG_MAP()

        LRESULT OnCloseCmd(uint16_t /*wNotifyCode*/, const uint16_t wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
        {
            m_path = GetDlgItemText(IDC_QLPATH);
            EndDialog(wID);
            return 0;
        }

        std::wstring GetDlgItemText(const int nID) const
        {
            std::wstring strText;

            const int nLength{::GetWindowTextLength(GetDlgItem(nID))};
            if (nLength > 0)
            {
                strText.resize(static_cast<size_t>(nLength) + 1);
                CDialogImpl<CQLPath>::GetDlgItemText(nID, strText.data(), nLength + 1);
            }

            return strText;
        }
    } qlpath;

    if (IDOK != qlpath.DoModal())
    {
        return S_OK;
    }

    IWICMetadataQueryReaderPtr rootQueryReader;
    IFC(elem->GetQueryReader(&rootQueryReader));

    bool inbraces{};
    int lastSlash = -1;
    for (size_t pos{}; pos < qlpath.m_path.length(); pos++)
    {
        switch (qlpath.m_path[pos])
        {
        case L'\\':
            // skip the next character
            pos++;
            break;
        case L'{':
            inbraces = true;
            break;
        case L'}':
            inbraces = false;
            break;
        case L'/':
            // braces escape forward slashes, e.g. /ifd/xmp/{wstr=string with /s}
            if (!inbraces)
            {
                lastSlash = static_cast<int>(pos);
            }
            break;
        default:
            break; // do nothing
        }
    }

    // Now let's get the query reader for everything upto the last slash
    std::wstring parentPath;
    std::wstring childPath;
    if (lastSlash == -1 || lastSlash == 0)
    {
        if (dynamic_cast<CBitmapFrameDecodeElement*>(elem))
        {
            // A frame decode element doesn't normally display metadata. We should punt this.
            parentPath = qlpath.m_path;
        }
        else
        {
            // Display it inside the current element
            childPath = qlpath.m_path;
        }
    }
    else
    {
        parentPath = qlpath.m_path.substr(lastSlash);
        childPath = qlpath.m_path.substr(lastSlash);
    }

    IWICMetadataQueryReaderPtr parentQueryReader;
    PROPVARIANT value;

    PropVariantInit(&value);

    if (parentPath != L"")
    {
        if (FAILED(rootQueryReader->GetMetadataByName(parentPath.c_str(), &value)))
        {
            parentPath = qlpath.m_path;
            IFC(rootQueryReader->GetMetadataByName(parentPath.c_str(), &value));
        }
        if (value.vt == VT_UNKNOWN)
        {
            IFC(value.punkVal->QueryInterface(IID_PPV_ARGS(&parentQueryReader)));
        }
        else
        {
            // This wasn't really a path to a metadata query reader. This means
            // the parsing failed. The best we can do is display this result in
            // the frame decoder.
            parentQueryReader = rootQueryReader;
            childPath = parentPath;
        }

        PropVariantClear(&value);
    }
    else
    {
        parentQueryReader = rootQueryReader;
    }

    IWICMetadataReaderPtr parentReader;
    CInfoElement* parentElem;
    if (FAILED(GetReaderFromQueryReader(parentQueryReader, &parentReader)))
    {
        parentElem = elem;
    }
    else
    {
        parentElem = elem->FindElementByReader(parentReader);
    }

    if (parentElem)
    {
        parentElem->m_queryKey = qlpath.m_path;
        // If there's a child element (might not be true if this is a branch), then
        // read its value. Otherwise, use the parent's path
        if (childPath != L"")
        {
            parentQueryReader->GetMetadataByName(childPath.c_str(), &value);
        }
        else
        {
            rootQueryReader->GetMetadataByName(parentPath.c_str(), &value);
        }
        std::wstring v;
        result = PropVariantToString(&value, pvtsoption_include_type, v);
        PropVariantClear(&value);
        IFC(result);

        parentElem->m_queryValue = v;
        if (elem == parentElem)
        {
            // Redraw the output view with the new query string
            DrawElement(*parentElem);
        }
        else
        {
            // When the tree selection changes, the output view will be redrawn
            m_mainTree.SelectItem(GetTreeItemFromElement(parentElem));
        }
    }
    return result;
}
#pragma warning(pop)
