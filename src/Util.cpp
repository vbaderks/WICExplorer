﻿module;

#include <atlstr.h>
#include <strsafe.h>

module Util;

import "std.h";
import "Windows-import.h";

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
