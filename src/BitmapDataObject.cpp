//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include "Macros.h"

#include <RichEdit.h>
#include <RichOle.h>

module BitmapDataObject;

import "pch.h";
import <memory>;


HRESULT CBitmapDataObject::InsertDib(HWND /*hWnd*/, IRichEditOle* pRichEditOle, const HGLOBAL hGlobal)
{
    HRESULT result = S_OK;

    IOleClientSite* oleClientSite{};
    IDataObject* dataObject{};
    IStorage* storage{};
    ILockBytes* lockBytes{};

    auto bitmapDataObject{ std::make_unique<CBitmapDataObject>() };
    VERIFY(SUCCEEDED(bitmapDataObject->QueryInterface(IID_PPV_ARGS(&dataObject))));
    bitmapDataObject->SetDib(hGlobal);

    // Get the RichEdit container site
    if (SUCCEEDED(result))
    {
        result = pRichEditOle->GetClientSite(&oleClientSite);
    }

    // Initialize a Storage Object
    if (SUCCEEDED(result))
    {
        result = CreateILockBytesOnHGlobal(nullptr, true, &lockBytes);
        ASSERT(lockBytes);
    }

    if (SUCCEEDED(result))
    {
        result = StgCreateDocfileOnILockBytes(lockBytes,
            STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &storage);
        ASSERT(storage);
    }

    // Get the ole object which will be inserted in the richedit control
    IOleObject* oleObject{};
    if (SUCCEEDED(result))
    {
        result = bitmapDataObject->GetOleObject(oleClientSite, storage, oleObject);
        OleSetContainedObject(oleObject, true);
    }

    // Add the object to the RichEdit
    if (SUCCEEDED(result) && oleObject)
    {
        CLSID clsid;
        oleObject->GetUserClassID(&clsid);

        REOBJECT reobject{};
        reobject.cbStruct = sizeof(REOBJECT);

        reobject.clsid = clsid;
        reobject.cp = REO_CP_SELECTION;
        reobject.dvaspect = DVASPECT_CONTENT;
        reobject.dwFlags = REO_BELOWBASELINE;
        reobject.poleobj = oleObject;
        reobject.polesite = oleClientSite;
        reobject.pstg = storage;

        result = pRichEditOle->InsertObject(&reobject);
    }

    // Cleanup
    static_cast<void>(bitmapDataObject.release()); // To prevent delete twice.

    if (oleObject)
    {
        oleObject->Release();
    }
    if (oleClientSite)
    {
        oleClientSite->Release();
    }
    if (storage)
    {
        storage->Release();
    }
    if (lockBytes)
    {
        lockBytes->Release();
    }
    if (dataObject)
    {
        dataObject->Release();
    }

    return result;
}


HRESULT STDMETHODCALLTYPE CBitmapDataObject::QueryInterface(REFIID iid, void** ppvObject) noexcept
{
    if (!ppvObject)
        return E_INVALIDARG;

    if (IID_IUnknown == iid || IID_IDataObject == iid)
    {
        *ppvObject = this;
        AddRef();
        return S_OK;
    }

    *ppvObject = nullptr;
    return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE CBitmapDataObject::AddRef() noexcept
{
    m_numReferences++;
    return m_numReferences;
}

ULONG STDMETHODCALLTYPE CBitmapDataObject::Release() noexcept
{
    ULONG result = 0;

    if (m_numReferences > 0)
    {
        result = --m_numReferences;

        if (0 == m_numReferences)
        {
            delete this;
        }
    }

    return result;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::GetData(FORMATETC* /*pformatetcIn*/, STGMEDIUM* pmedium) noexcept
{
    pmedium->tymed = TYMED_HGLOBAL;
    pmedium->hGlobal = OleDuplicateData(m_stgmed.hGlobal, CF_DIB, GMEM_MOVEABLE | GMEM_SHARE);
    pmedium->pUnkForRelease = nullptr;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::GetDataHere(FORMATETC* /*pformatetc*/, STGMEDIUM* /*pmedium*/) noexcept
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::QueryGetData(FORMATETC* pformatetc) noexcept
{
    HRESULT result = DV_E_FORMATETC;
    if (pformatetc)
    {
        if (CF_DIB == pformatetc->cfFormat)
        {
            result = S_OK;
        }
    }
    return result;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::GetCanonicalFormatEtc(FORMATETC* /*pformatectIn*/, FORMATETC* /*pformatetcOut*/) noexcept
{
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL /*fRelease*/) noexcept
{
    m_format = *pformatetc;
    m_stgmed = *pmedium;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::EnumFormatEtc(DWORD /*dwDirection*/, IEnumFORMATETC** ppenumFormatEtc) noexcept
{
    *ppenumFormatEtc = nullptr;
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::DAdvise(FORMATETC* /*pformatetc*/, DWORD /*advf*/, IAdviseSink* /*pAdvSink*/, DWORD* /*pdwConnection*/) noexcept
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::DUnadvise(DWORD /*dwConnection*/) noexcept
{
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT STDMETHODCALLTYPE CBitmapDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise) noexcept
{
    *ppenumAdvise = nullptr;
    return OLE_E_ADVISENOTSUPPORTED;
}

void CBitmapDataObject::SetDib(const HGLOBAL hGlobal) noexcept
{
    ASSERT(hGlobal);

    if (hGlobal)
    {
        WARNING_SUPPRESS_NEXT_LINE(26476) // : Expression / symbol '{(HGLOBAL)hGlobal}' uses a naked union 'union ' with multiple type pointers : Use variant instead(type.7).
        STGMEDIUM stgm{ .tymed = TYMED_HGLOBAL, .hGlobal = hGlobal };
        FORMATETC fm { .dwAspect = DVASPECT_CONTENT, .lindex = -1, .tymed = TYMED_NULL };

        SetData(&fm, &stgm, true);
    }
}

HRESULT CBitmapDataObject::GetOleObject(IOleClientSite* oleClientSite, IStorage* storage, IOleObject*& oleObject) noexcept
{
    HRESULT result = E_UNEXPECTED;

    oleObject = nullptr;

    ASSERT(m_stgmed.hGlobal);

    if (m_stgmed.hGlobal)
    {
        result = OleCreateStaticFromData(this, IID_IOleObject, OLERENDER_DRAW,
            &m_format, oleClientSite, storage, reinterpret_cast<void**>(&oleObject));

        if (FAILED(result))
        {
            oleObject = nullptr;
        }
    }

    return result;
}
