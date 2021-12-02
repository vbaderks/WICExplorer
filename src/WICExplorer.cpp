//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#include "pch.h"

#include "Macros.h"

#include <array>
#include <algorithm>
#include <utility>

import MainFrame;


#ifndef _UNICODE
#error Only Unicode builds are supported (required by common controls v6 and ANSI doesn't make sense for Windows 10)
#endif

// Ensure the linker creates the manifest for the v6 common controls (modern look and feel).
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

using std::array;
using std::pair;

namespace
{
    constexpr array g_wicErrorCodes{
        pair{WINCODEC_ERR_GENERIC_ERROR, L"WINCODEC_ERR_GENERIC_ERROR"},
        pair{WINCODEC_ERR_INVALIDPARAMETER, L"WINCODEC_ERR_INVALIDPARAMETER"},
        pair{ WINCODEC_ERR_OUTOFMEMORY, L"WINCODEC_ERR_OUTOFMEMORY"},
        pair{ WINCODEC_ERR_NOTIMPLEMENTED, L"WINCODEC_ERR_NOTIMPLEMENTED"},
        pair{ WINCODEC_ERR_ABORTED, L"WINCODEC_ERR_ABORTED"},
        pair{ WINCODEC_ERR_ACCESSDENIED, L"WINCODEC_ERR_ACCESSDENIED"},
        pair{ WINCODEC_ERR_VALUEOVERFLOW, L"WINCODEC_ERR_VALUEOVERFLOW"},
        pair{ WINCODEC_ERR_WRONGSTATE, L"WINCODEC_ERR_WRONGSTATE"},
        pair{ WINCODEC_ERR_VALUEOUTOFRANGE, L"WINCODEC_ERR_VALUEOUTOFRANGE"},
        pair{ WINCODEC_ERR_UNKNOWNIMAGEFORMAT, L"WINCODEC_ERR_UNKNOWNIMAGEFORMAT"},
        pair{ WINCODEC_ERR_UNSUPPORTEDVERSION, L"WINCODEC_ERR_UNSUPPORTEDVERSION"},
        pair{ WINCODEC_ERR_NOTINITIALIZED, L"WINCODEC_ERR_NOTINITIALIZED"},
        pair{ WINCODEC_ERR_ALREADYLOCKED, L"WINCODEC_ERR_ALREADYLOCKED"},
        pair{ WINCODEC_ERR_PROPERTYNOTFOUND, L"WINCODEC_ERR_PROPERTYNOTFOUND"},
        pair{ WINCODEC_ERR_PROPERTYNOTSUPPORTED, L"WINCODEC_ERR_PROPERTYNOTSUPPORTED"},
        pair{ WINCODEC_ERR_PROPERTYSIZE, L"WINCODEC_ERR_PROPERTYSIZE"},
        pair{ WINCODEC_ERR_CODECPRESENT, L"WINCODEC_ERR_CODECPRESENT"},
        pair{ WINCODEC_ERR_CODECNOTHUMBNAIL, L"WINCODEC_ERR_CODECNOTHUMBNAIL"},
        pair{ WINCODEC_ERR_PALETTEUNAVAILABLE, L"WINCODEC_ERR_PALETTEUNAVAILABLE"},
        pair{ WINCODEC_ERR_CODECTOOMANYSCANLINES, L"WINCODEC_ERR_CODECTOOMANYSCANLINES"},
        pair{ WINCODEC_ERR_INTERNALERROR, L"WINCODEC_ERR_INTERNALERROR"},
        pair{ WINCODEC_ERR_SOURCERECTDOESNOTMATCHDIMENSIONS, L"WINCODEC_ERR_SOURCERECTDOESNOTMATCHDIMENSIONS"},
        pair{ WINCODEC_ERR_COMPONENTNOTFOUND, L"WINCODEC_ERR_COMPONENTNOTFOUND"},
        pair{ WINCODEC_ERR_IMAGESIZEOUTOFRANGE, L"WINCODEC_ERR_IMAGESIZEOUTOFRANGE"},
        pair{ WINCODEC_ERR_TOOMUCHMETADATA, L"WINCODEC_ERR_TOOMUCHMETADATA"},
        pair{ WINCODEC_ERR_BADIMAGE, L"WINCODEC_ERR_BADIMAGE"},
        pair{ WINCODEC_ERR_BADHEADER, L"WINCODEC_ERR_BADHEADER"},
        pair{ WINCODEC_ERR_FRAMEMISSING, L"WINCODEC_ERR_FRAMEMISSING"},
        pair{ WINCODEC_ERR_BADMETADATAHEADER, L"WINCODEC_ERR_BADMETADATAHEADER"},
        pair{ WINCODEC_ERR_BADSTREAMDATA, L"WINCODEC_ERR_BADSTREAMDATA"},
        pair{ WINCODEC_ERR_STREAMWRITE, L"WINCODEC_ERR_STREAMWRITE"},
        pair{ WINCODEC_ERR_STREAMREAD, L"WINCODEC_ERR_STREAMREAD"},
        pair{ WINCODEC_ERR_STREAMNOTAVAILABLE, L"WINCODEC_ERR_STREAMNOTAVAILABLE"},
        pair{ WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT, L"WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT"},
        pair{ WINCODEC_ERR_UNSUPPORTEDOPERATION, L"WINCODEC_ERR_UNSUPPORTEDOPERATION"},
        pair{ WINCODEC_ERR_INVALIDREGISTRATION, L"WINCODEC_ERR_INVALIDREGISTRATION"},
        pair{ WINCODEC_ERR_COMPONENTINITIALIZEFAILURE, L"WINCODEC_ERR_COMPONENTINITIALIZEFAILURE"},
        pair{ WINCODEC_ERR_INSUFFICIENTBUFFER, L"WINCODEC_ERR_INSUFFICIENTBUFFER"},
        pair{ WINCODEC_ERR_DUPLICATEMETADATAPRESENT, L"WINCODEC_ERR_DUPLICATEMETADATAPRESENT"},
        pair{ WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE, L"WINCODEC_ERR_PROPERTYUNEXPECTEDTYPE"},
        pair{ WINCODEC_ERR_UNEXPECTEDSIZE, L"WINCODEC_ERR_UNEXPECTEDSIZE"},
        pair{ WINCODEC_ERR_INVALIDQUERYREQUEST, L"WINCODEC_ERR_INVALIDQUERYREQUEST"},
        pair{ WINCODEC_ERR_UNEXPECTEDMETADATATYPE, L"WINCODEC_ERR_UNEXPECTEDMETADATATYPE"},
        pair{ WINCODEC_ERR_REQUESTONLYVALIDATMETADATAROOT, L"WINCODEC_ERR_REQUESTONLYVALIDATMETADATAROOT"},
        pair{ WINCODEC_ERR_INVALIDQUERYCHARACTER, L"WINCODEC_ERR_INVALIDQUERYCHARACTER"} };


int Run(CAppModule& appModule, const LPCWSTR lpCmdLine, const int nCmdShow)
{
    CMessageLoop msgLoop;
    appModule.AddMessageLoop(&msgLoop);

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

}

CString GetHresultString(HRESULT hr)
{
    const auto knownError = std::find_if(g_wicErrorCodes.begin(), g_wicErrorCodes.end(),
        [=](const pair<HRESULT, LPCWSTR>& x) { return x.first == hr; });

    if (FACILITY_WINCODEC_ERR == HRESULT_FACILITY(hr) && knownError != g_wicErrorCodes.end())
    {
        return knownError->second;
    }

    constexpr DWORD MAX_MsgLength = 256;

    WCHAR msg[MAX_MsgLength];

    msg[0] = TEXT('\0');

    if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
    {
        hr = HRESULT_CODE(hr);
    }

    // Try to have windows give a nice message, otherwise just format the HRESULT into a string.
    const DWORD len = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                                     static_cast<DWORD>(hr), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, MAX_MsgLength, nullptr);
    if (len != 0)
    {
        // remove the trailing newline
        if (L'\r' == msg[len - 2])
        {
            msg[len - 2] = L'\0';
        }
        else if (L'\n' == msg[len - 1])
        {
            msg[len - 1] = L'\0';
        }
    }
    else
    {
        StringCchPrintf(msg, MAX_MsgLength, L"0x%.8X", static_cast<unsigned>(hr));
    }

    return msg;
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
        CString msg;
        msg.Format(L"Unable to create ImagingFactory. The error is: %s.", GetHresultString(hr).GetString());
        MessageBoxW(nullptr, msg, L"Error Creating ImagingFactory", MB_ICONERROR);
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
