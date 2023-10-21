// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

export module PropVariant;

import <std.h>;
import <Windows-import.h>;

export
{

inline constexpr unsigned PVTSOPTION_IncludeType{0x01};

HRESULT PropVariantToString(PROPVARIANT* pv, unsigned options, std::wstring& out);
HRESULT VariantTypeToString(VARTYPE vt, std::wstring& out);

}
