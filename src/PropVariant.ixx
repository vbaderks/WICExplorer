// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include <atlstr.h>

export module PropVariant;

export
{

inline constexpr unsigned PVTSOPTION_IncludeType = 0x01;

HRESULT PropVariantToString(PROPVARIANT* pv, unsigned options, CString& out);
HRESULT VariantTypeToString(VARTYPE vt, CString& out);

}
