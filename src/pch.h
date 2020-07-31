//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

#include <array>
#include <algorithm>
#include <utility>

#include <Windows.h>
#include <tchar.h>
#include <shellapi.h>
#include <comip.h>

#include <wincodec.h>
#include <wincodecsdk.h>
#include <Icm.h>

#pragma warning(push)
#pragma warning(disable: 4471) // enum forward without type
#pragma warning(disable: 5204) // class has virtual functions, but its trivial destructor is not virtual;
#import "msxml2.tlb" named_guids
#pragma warning(pop)

#include <strsafe.h>

#pragma warning(push)
#pragma warning(disable: 4986) // exception specs
#pragma warning(disable: 4755) // conversin rules, inline
#pragma warning(disable: 5204) // class has virtual functions, but its trivial destructor is not virtual;
#include <atlbase.h>
#include <atlstr.h>
#include <atlwin.h>
#pragma warning(pop)

//The Windows Template Library (WTL) is required for this project
#pragma warning(push)
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 5204) // class has virtual functions, but its trivial destructor is not virtual;
#include <atlapp.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlmisc.h>
#include <atlsplit.h>
#include <atlctrlx.h>
#pragma warning(pop)

#include "Interfaces.h"

extern IWICImagingFactoryPtr g_imagingFactory;

#define IFC(c) do { result = (c); if (FAILED(result)) return result; } while(false)

#define READ_WIC_STRING(f, out) do {                                    \
    UINT strLen = 0;                                                    \
    result = f(0, 0, &strLen);                                          \
    if (SUCCEEDED(result) && (strLen > 0)) {                            \
        result = f(strLen, out.GetBufferSetLength(strLen), &strLen);    \
        out.ReleaseBuffer(strLen - 1);                                  \
    } else { out = L""; } } while(0);

CString GetHresultString(HRESULT hr);
