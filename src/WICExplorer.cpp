// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

#include "Macros.h"
#include "ComSmartPointers.h"
#include "MainFrame.h"

#include <atlstr.h>

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>

import Util;
import Element;

import "std.h";


#ifndef _UNICODE
#error Only Unicode builds are supported (required by common controls v6 and ANSI doesn't make sense for Windows 10)
#endif

// Ensure the linker creates the manifest for the v6 common controls (modern look and feel).
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")



int Run(CAppModule& appModule, const LPCWSTR lpCmdLine, const int nCmdShow)
{
    CMessageLoop msgLoop;
    appModule.AddMessageLoop(&msgLoop);

    WARNING_SUPPRESS_NEXT_LINE(5262) // implicit fall-through occurs here; are you missing a break statement?
    CMainFrame mainWnd;

    // Create the main window
    if (nullptr == mainWnd.CreateEx(nullptr))
    {
        ATLTRACE(L"Main window creation failed");
        return 0;
    }

    // Center it
    mainWnd.CenterWindow();

    // Load the files
    int argc;
    LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);

    // If argc == 0, then lpCmdLine is the name of the executable (WICExplorer)
    if (*lpCmdLine != 0)
    {
        mainWnd.Load(argv, argc);
    }
    LocalFree(argv);

    // Show the window
    mainWnd.ShowWindow(nCmdShow);

    const int result = msgLoop.Run();

    appModule.RemoveMessageLoop();

    return result;
}


WARNING_SUPPRESS_NEXT_LINE(26461) // The pointer argument 'lpCmdLine' for function 'wWinMain' can be marked as a pointer to const (con.3).
_Use_decl_annotations_
int WINAPI wWinMain(const HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, const LPWSTR lpCmdLine, const int nShowCmd)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Initialize COM
    HRESULT hr = CoInitialize(nullptr);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        return 0;
    }

    // Initialize the Common Controls
    const INITCOMMONCONTROLSEX iccx{.dwSize = sizeof iccx, .dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES};
    const bool initCCRes = InitCommonControlsEx(&iccx);
    ATLASSERT(initCCRes);
    if (!initCCRes)
    {
        CoUninitialize();
        return 0;
    }

    // Initialize WindowsCodecs
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_imagingFactory));
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
    {
#pragma warning(push)
#pragma warning(disable : 4296) // '<': expression is always false
        MessageBoxW(nullptr, std::format(L"Unable to create ImagingFactory. The error is: {}.", GetHresultString(hr)).c_str(),
            L"Error Creating ImagingFactory", MB_ICONERROR);
#pragma warning(pop)
    }

    // Initialize the RichEdit library
    const HINSTANCE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
    ATLASSERT(hInstRich);

    // Start running
    int result = 0;
    if (SUCCEEDED(hr) && hInstRich)
    {
        ::DefWindowProc(nullptr, 0, 0, 0L);

        CAppModule appModule;

        hr = appModule.Init(nullptr, hInstance);
        ATLASSERT(SUCCEEDED(hr));

        result = Run(appModule, lpCmdLine, nShowCmd);

        appModule.Term();

        // Release the factory
        IWICImagingFactory* imagingFactory = g_imagingFactory.Detach();
        imagingFactory->Release();

        CElementManager::ClearAllElements();
        FreeLibrary(hInstRich);
        CoUninitialize();
    }

    return SUCCEEDED(hr) ? 0 : result;
}
