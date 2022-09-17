module;

#include "Macros.h"
#include <cassert>

export module Util;

import "std.h";
import "Windows-import.h";

export {

[[nodiscard]] inline std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text;

    guid_text.resize(39);
    VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

    // Remove the double null terminator.
    guid_text.resize(guid_text.size() - 1);

    return guid_text;
}

std::wstring GetHresultString(HRESULT hr);

template<typename WicInterface, typename MemberFunction>
[[nodiscard]] HRESULT GetWicString(WicInterface& wicIinterface, MemberFunction memberFunction, std::wstring& output) {
    uint32_t bufferSizeInChars;
    const HRESULT result{(wicIinterface.*memberFunction)(0, nullptr, &bufferSizeInChars)};
    if (FAILED(result))
        return result;

    if (bufferSizeInChars == 0)
    {
        output.clear();
        return result;
    }

    // std::wstring will also automatically allocate space for the null terminator.
    output.resize(bufferSizeInChars - size_t{1});

    return (wicIinterface.*memberFunction)(bufferSizeInChars, output.data(), &bufferSizeInChars);
}

}
