module;

#include "Macros.h"

#include <atlstr.h>

export module Util;

import "std.h";

export [[nodiscard]] inline std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text;

    guid_text.resize(39);
    VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

    // Remove the double null terminator.
    guid_text.resize(guid_text.size() - 1);

    return guid_text;
}

export CString GetHresultString(HRESULT hr);
