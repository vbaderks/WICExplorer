// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

#pragma once

// Include additional headers as workaround that IntelliSense in VS 2026 18.6.1 fails to parse #import <Windows-import.h>
#ifdef __INTELLISENSE__
#define _AMD64_
#include <cassert>
#include <combaseapi.h>
#include <libloaderapi.h>
#include <sal.h>
#include <winreg.h>
#include <winerror.h>
#include <winnt.h>
#include <wincodec.h>
#include <RichOle.h>

#include <comip.h>
#include <comdef.h>
#include <comdefsp.h>

#endif
