//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include <atlstr.h>

export module PropVariant;

export
{

constexpr inline unsigned PVTSOPTION_IncludeType = 0x01;

HRESULT PropVariantToString(PROPVARIANT* pv, unsigned options, CString& out);
HRESULT VariantTypeToString(VARTYPE vt, CString& out);

}
