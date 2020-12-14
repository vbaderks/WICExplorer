//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#include "pch.h"

#include "PropVariant.h"
#include "Macros.h"

#include <algorithm>

template<class T> static void WriteValue(const T & /*val*/, CString &out)
{
    out.Format(L"<UnknownValue>");
}

template<class T> static LPCWSTR GetTypeName()
{
    return L"<UnknownType>";
}

template<> void WriteValue<CHAR>(const CHAR &val, CString &out)
{
    out.Format(L"%d", static_cast<int>(val));
}

template<> PCWSTR GetTypeName<CHAR>() noexcept
{
    return L"CHAR";
}

template<> void WriteValue<UCHAR>(const UCHAR &val, CString &out)
{
    out.Format(L"%u", static_cast<unsigned>(val));
}

template<> PCWSTR GetTypeName<UCHAR>() noexcept
{
    return L"UCHAR";
}

template<> void WriteValue<SHORT>(const SHORT &val, CString &out)
{
    out.Format(L"%d", static_cast<int>(val));
}

template<> PCWSTR GetTypeName<SHORT>() noexcept
{
    return L"SHORT";
}

template<> void WriteValue<USHORT>(const USHORT &val, CString &out)
{
    out.Format(L"%u", static_cast<unsigned>(val));
}

template<> PCWSTR GetTypeName<USHORT>() noexcept
{
    return L"USHORT";
}

template<> void WriteValue<LONG>(const LONG &val, CString &out)
{
    out.Format(L"%d", static_cast<int>(val));
}

template<> PCWSTR GetTypeName<LONG>() noexcept
{
    return L"LONG";
}

template<> void WriteValue<ULONG>(const ULONG &val, CString &out)
{
    out.Format(L"%u", static_cast<unsigned>(val));
}

template<> PCWSTR GetTypeName<ULONG>() noexcept
{
    return L"ULONG";
}

template<> void WriteValue<INT>(const INT &val, CString &out)
{
    out.Format(L"%d", static_cast<int>(val));
}

template<> PCWSTR GetTypeName<INT>() noexcept
{
    return L"INT";
}

template<> void WriteValue<uint32_t>(const uint32_t &val, CString &out)
{
    out.Format(L"%u", static_cast<unsigned>(val));
}

template<> PCWSTR GetTypeName<uint32_t>() noexcept
{
    return L"UINT";
}

template<> void WriteValue<LARGE_INTEGER>(const LARGE_INTEGER &val, CString &out)
{
    // "the numerator in low part and denominator in the high part"
    if (0 != val.HighPart)
    {
        WCHAR str[64];
        StringCchPrintfW(str, 64, L"%ul / %dl (%g)", val.LowPart, val.HighPart, static_cast<double>(val.LowPart) / static_cast<double>(val.HighPart));
        out = str;
    }
    else
    {
        out.Format(L"%ul / %dl", val.LowPart, val.HighPart);
    }
}

template<> PCWSTR GetTypeName<LARGE_INTEGER>() noexcept
{
    return L"LARGE_INTEGER";
}

template<> void WriteValue<ULARGE_INTEGER>(const ULARGE_INTEGER &val, CString &out)
{
    // "the numerator in low part and denominator in the high part"
    if (0 != val.HighPart)
    {
        WCHAR str[64];
        StringCchPrintfW(str, 64, L"%u / %u (%g)", val.LowPart, val.HighPart, static_cast<double>(val.LowPart) / static_cast<double>(val.HighPart));
        out = str;
    }
    else
    {
        out.Format(L"%u / %u", val.LowPart, val.HighPart);
    }
}

template<> PCWSTR GetTypeName<ULARGE_INTEGER>() noexcept
{
    return L"ULARGE_INTEGER";
}

template<> void WriteValue<FLOAT>(const FLOAT &val, CString &out)
{
    WCHAR str[64];
    StringCchPrintfW(str, 64, L"%g", static_cast<double>(val));
    out = str;
}

template<> PCWSTR GetTypeName<FLOAT>() noexcept
{
    return L"FLOAT";
}

template<> void WriteValue<DOUBLE>(const DOUBLE &val, CString &out)
{
    WCHAR str[64];
    StringCchPrintfW(str, 64, L"%g", val);
    out = str;
}

template<> PCWSTR GetTypeName<DOUBLE>() noexcept
{
    return L"DOUBLE";
}

template<> void WriteValue<CY>(const CY &val, CString &out)
{
    out.Format(L"%g", static_cast<double>(val.int64) / 10000.0);
}

template<> PCWSTR GetTypeName<CY>() noexcept
{
    return L"CY";
}

template<> void WriteValue<CLSID>(const CLSID &val, CString &out)
{
    WCHAR str[64];
    VERIFY(StringFromGUID2(val, str, 64) != 0);
    out = str;
}

template<> PCWSTR GetTypeName<CLSID>() noexcept
{
    return L"CLSID";
}

template<> void WriteValue<BLOB>(const BLOB &val, CString &out)
{
    constexpr ULONG MAX_BYTES = 10;
    CString b;

    out.Format(L"%u bytes = { ", val.cbSize);
    for (uint32_t i = 0; i < std::min(MAX_BYTES, val.cbSize); i++)
    {
        b.Format(L"0x%.2X ", val.pBlobData[i]);
        out += b;
    }

    if (MAX_BYTES < val.cbSize)
    {
        out += L"... ";
    }

    out += L"}";
}

template<> PCWSTR GetTypeName<BLOB>() noexcept
{
    return L"BLOB";
}

template<> void WriteValue<PSTR>(const PSTR &val, CString &out)
{
    out = L"\"" + CString(val) + L"\"";
}

template<> PCWSTR GetTypeName<PSTR>() noexcept
{
    return L"PSTR";
}

template<> void WriteValue<LPWSTR>(const LPWSTR &val, CString &out)
{
    out = L"\"" + CString(val) + L"\"";
}

template<> PCWSTR GetTypeName<PWSTR>() noexcept
{
    return L"PWSTR";
}

template<> void WriteValue<IUnknown>(const IUnknown &val, CString &out)
{
    out.Format(L"IUnknown * = %p", &val);
}

template<> PCWSTR GetTypeName<IUnknown>() noexcept
{
    return L"IUnknown*";
}

template<typename T> void WriteValues(const ULONG count, T *vals, const VARTYPE type, const unsigned options, CString &out)
{
    constexpr ULONG MAX_VALUES = 10;
    CString b;

    if (options & PVTSOPTION_IncludeType)
    {
        VariantTypeToString(type, b);
        out.Format(L"%s[%u] = { ", b.GetBuffer(0), count);
    }
    else
    {
        out = L"{ ";
    }

    const ULONG num = std::min(MAX_VALUES, count);
    for (ULONG i = 0; i < num; i++)
    {
        WriteValue(vals[i], b);

        if (i + 1 == count)
        {
            out += b + L" ";
        }
        else
        {
            out += b + L", ";
        }
    }

    if (MAX_VALUES < count)
    {
        out += L"... ";
    }

    out += L"}";
}

HRESULT VariantTypeToString(const VARTYPE vt, CString &out)
{
    switch (vt)
    {
    case VT_EMPTY:
        out = L"EMPTY";
        break;
    case VT_NULL:
        out = L"NULL";
        break;
    case VT_I1:
        out = L"I1";
        break;
    case VT_UI1:
        out = L"UI1";
        break;
    case VT_I2:
        out = L"I2";
        break;
    case VT_UI2:
        out = L"UI2";
        break;
    case VT_I4:
        out = L"I4";
        break;
    case VT_UI4:
        out = L"UI4";
        break;
    case VT_INT:
        out = L"INT";
        break;
    case VT_UINT:
        out = L"UINT";
        break;
    case VT_I8:
        out = L"RATIONAL";
        break;
    case VT_UI8:
        out = L"URATIONAL";
        break;
    case VT_R4:
        out = L"R4";
        break;
    case VT_R8:
        out = L"R8";
        break;
    case VT_BOOL:
        out = L"BOOL";
        break;
    case VT_ERROR:
        out = L"ERROR";
        break;
    case VT_CY:
        out = L"CY";
        break;
    case VT_DATE:
        out = L"DATE";
        break;
    case VT_FILETIME:
        out = L"FILETIME";
        break;
    case VT_CLSID:
        out = L"CLSID";
        break;
    case VT_CF:
        out = L"CF";
        break;
    case VT_BSTR:
        out = L"BSTR";
        break;
    case VT_BSTR_BLOB:
        out = L"BSTR_BLOB";
        break;
    case VT_BLOB:
        out = L"BLOB";
        break;
    case VT_LPSTR:
        out = L"LPSTR";
        break;
    case VT_LPWSTR:
        out = L"LPWSTR";
        break;
    case VT_UNKNOWN:
        out = L"UNKNOWN";
        break;
    case VT_DISPATCH:
        out = L"DISPATCH";
        break;
    case VT_STREAM:
        out = L"STREAM";
        break;
    case VT_STREAMED_OBJECT:
        out = L"STREAMED_OBJECT";
        break;
    case VT_STORAGE:
        out = L"STORAGE";
        break;
    case VT_STORED_OBJECT:
        out = L"STORED_OBJECT";
        break;
    case VT_VERSIONED_STREAM:
        out = L"VERSIONED_STREAM";
        break;
    case VT_DECIMAL:
        out = L"DECIMAL";
        break;
    case VT_VECTOR:
        out = L"VECTOR";
        break;
    case VT_ARRAY:
        out = L"ARRAY";
        break;
    case VT_BYREF:
        out = L"BYREF";
        break;
    case VT_VARIANT:
        out = L"VARIANT";
        break;
    default:
        out.Format(L"<vt: 0x%.4X>", static_cast<unsigned>(vt));
        break;
    }

    return S_OK;
}

static HRESULT AddTypeToString(const VARTYPE vt, CString &out)
{
    CString typeName;
    VariantTypeToString(vt, typeName);

    if (out.GetLength() > 0)
    {
        out += L" (" + typeName + L")";
    }
    else
    {
        out = typeName;
    }

    return S_OK;
}

HRESULT PropVariantToString(PROPVARIANT *pv, const unsigned options, CString &out)
{
    if (options & PVTSOPTION_IncludeType)
    {
        out.Format(L"<UnknownType: 0x%.4X>", static_cast<unsigned>(pv->vt));
    }
    else
    {
        out.Empty();
    }

    if (pv->vt & VT_VECTOR)
    {
        out.Format(L"%u elements of type 0x%.4X", pv->cac.cElems, pv->vt & ~VT_VECTOR);

        const VARTYPE typ = pv->vt & ~VT_VECTOR;

        switch (typ)
        {
        case VT_I1:
            WriteValues(pv->cac.cElems, pv->cac.pElems, typ, options, out);
            break;
        case VT_UI1:
            WriteValues(pv->caub.cElems, pv->caub.pElems, typ, options, out);
            break;
        case VT_I2:
            WriteValues<>(pv->cai.cElems, pv->cai.pElems, typ, options, out);
            break;
        case VT_UI2:
            WriteValues(pv->caui.cElems, pv->caui.pElems, typ, options, out);
            break;
        case VT_BOOL:
            WriteValues(pv->cabool.cElems, pv->cabool.pElems, typ, options, out);
            break;
        case VT_I4:
            WriteValues(pv->cal.cElems, pv->cal.pElems, typ, options, out);
            break;
        case VT_UI4:
            WriteValues(pv->caul.cElems, pv->caul.pElems, typ, options, out);
            break;
        case VT_R4:
            WriteValues(pv->caflt.cElems, pv->caflt.pElems, typ, options, out);
            break;
        case VT_R8:
            WriteValues(pv->cadbl.cElems, pv->cadbl.pElems, typ, options, out);
            break;
        case VT_ERROR:
            WriteValues(pv->cascode.cElems, pv->cascode.pElems, typ, options, out);
            break;
        case VT_I8:
            WriteValues(pv->cah.cElems, pv->cah.pElems, typ, options, out);
            break;
        case VT_UI8:
            WriteValues(pv->cauh.cElems, pv->cauh.pElems, typ, options, out);
            break;
        case VT_CY:
            WriteValues(pv->cacy.cElems, pv->cacy.pElems, typ, options, out);
            break;
        case VT_DATE:
            WriteValues(pv->cadate.cElems, pv->cadate.pElems, typ, options, out);
            break;
        case VT_FILETIME:
            WriteValues(pv->cafiletime.cElems, pv->cafiletime.pElems, typ, options, out);
            break;
        case VT_CLSID:
            WriteValues(pv->cauuid.cElems, pv->cauuid.pElems, typ, options, out);
            break;
        case VT_CF:
            WriteValues(pv->caclipdata.cElems, pv->caclipdata.pElems, typ, options, out);
            break;
        case VT_BSTR:
            WriteValues(pv->cabstr.cElems, pv->cabstr.pElems, typ, options, out);
            break;
        case VT_LPSTR:
            WriteValues(pv->calpstr.cElems, pv->calpstr.pElems, typ, options, out);
            break;
        case VT_LPWSTR:
            WriteValues(pv->calpwstr.cElems, pv->calpwstr.pElems, typ, options, out);
            break;
        case VT_VARIANT:
            WriteValues(pv->capropvar.cElems, pv->capropvar.pElems, typ, options, out);
            break;
        default:
            ATLASSERT(false);
            break;
        }
    }
    else
    {
        switch (pv->vt)
        {
        case VT_EMPTY:
        case VT_NULL:
            break;
        case VT_I1:
            WriteValue(pv->cVal, out);
            break;
        case VT_UI1:
            WriteValue(pv->bVal, out);
            break;
        case VT_I2:
            WriteValue(pv->iVal, out);
            break;
        case VT_UI2:
            WriteValue(pv->uiVal, out);
            break;
        case VT_I4:
            WriteValue(pv->lVal, out);
            break;
        case VT_UI4:
            WriteValue(pv->ulVal, out);
            break;
        case VT_INT:
            WriteValue(pv->intVal, out);
            break;
        case VT_UINT:
            WriteValue(pv->uintVal, out);
            break;
        case VT_I8:
            WriteValue(pv->hVal, out);
            break;
        case VT_UI8:
            WriteValue(pv->uhVal, out);
            break;
        case VT_R4:
            WriteValue(pv->fltVal, out);
            break;
        case VT_R8:
            WriteValue(pv->dblVal, out);
            break;
        case VT_BOOL:
            out = (0 == pv->boolVal) ? L"False" : L"True";
            break;
        case VT_ERROR:
            out.Format(L"<Error: %u>", static_cast<unsigned>(pv->scode));
            break;
        case VT_CY:
            WriteValue(pv->cyVal, out);
            break;
        case VT_DATE:
            // date A 64-bit floating point number representing the number of days (not seconds) since December 31, 1899. For example, January 1, 1900, is 2.0, January 2, 1900, is 3.0, and so on). This is stored in the same representation as VT_R8.
        case VT_FILETIME:
            // filetime 64-bit FILETIME structure as defined by Win32. It is recommended that all times be stored in Universal Coordinate Time (UTC).
            break;
        case VT_CLSID:
            WriteValue(*pv->puuid, out);
            break;
        case VT_CF:
            // pclipdata Pointer to a CLIPDATA structure, described above.
            break;
        case VT_BSTR:
            WriteValue(pv->bstrVal, out);
            break;
        case VT_BSTR_BLOB:
            // bstrblobVal For system use only.
            break;
        case VT_BLOB:
            WriteValue(pv->blob, out);
            break;
        case VT_LPSTR:
            WriteValue(pv->pszVal, out);
            break;
        case VT_LPWSTR:
            WriteValue(pv->pwszVal, out);
            break;
        case VT_UNKNOWN:
            WriteValue(*pv->punkVal, out);
            break;
        case VT_DISPATCH:
            // pdispVal New
        case VT_STREAM:
            // pStream Pointer to an IStream interface, representing a stream which is a sibling to the "Contents" stream.
        case VT_STREAMED_OBJECT:
            // pStream As in VT_STREAM, but indicates that the stream contains a serialized object, which is a CLSID followed by initialization data for the class. The stream is a sibling to the "Contents" stream that contains the property set.
        case VT_STORAGE:
            // pStorage Pointer to an IStorage interface, representing a storage object that is a sibling to the "Contents" stream.
        case VT_STORED_OBJECT:
            // pStorage As in VT_STORAGE, but indicates that the designated IStorage contains a loadable object.
        case VT_VERSIONED_STREAM:
            // pVersionedStream A stream with a GUID version.
        case VT_DECIMAL:
            // decVal A DECIMAL structure.
        case VT_VECTOR:
            // ca* If the type indicator is combined with VT_VECTOR by using an OR operator, the value is one of the counted array values. This creates a DWORD count of elements, followed by a pointer to the specified repetitions of the value.
        case VT_ARRAY:
            // Parray If the type indicator is combined with VT_ARRAY by an OR operator, the value is a pointer to a SAFEARRAY. VT_ARRAY can use the OR with the following data types: VT_I1, VT_UI1, VT_I2, VT_UI2, VT_I4, VT_UI4, VT_INT, VT_UINT, VT_R4, VT_R8, VT_BOOL, VT_DECIMAL, VT_ERROR, VT_CY, VT_DATE, VT_BSTR, VT_DISPATCH, VT_UNKNOWN, and VT_VARIANT. VT_ARRAY cannot use OR with VT_VECTOR.
        case VT_BYREF:
            // p* If the type indicator is combined with VT_BYREF by an OR operator, the value is a reference. Reference types are interpreted as a reference to data, similar to the reference type in C++ (for example, "int&").
        case VT_VARIANT:
            // capropvar A DWORD type indicator followed by the corresponding value. VT_VARIANT can be used only with VT_VECTOR or VT_BYREF.
            break;
        default:
            ATLASSERT(false);
            break;
        }

        if (options & PVTSOPTION_IncludeType)
        {
            AddTypeToString(pv->vt, out);
        }
    }

    return S_OK;
}
