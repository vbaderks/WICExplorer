//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

class CBitmapDataObject final : IDataObject
{
public:
    static HRESULT InsertDib(HWND hWnd, IRichEditOle *pRichEditOle, HGLOBAL hGlobal);

    CBitmapDataObject() = default;
    ~CBitmapDataObject();

    CBitmapDataObject(const CBitmapDataObject&) = delete;
    CBitmapDataObject(CBitmapDataObject&&) = delete;
    CBitmapDataObject& operator=(const CBitmapDataObject&) = delete;
    CBitmapDataObject& operator=(CBitmapDataObject&&) = delete;

    // IUnknown Interface
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject) noexcept override;
    STDMETHOD_(ULONG, AddRef)() noexcept override;
    STDMETHOD_(ULONG, Release)() noexcept override;

    // IDataObject Interface
    STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) noexcept override;
    STDMETHOD(GetDataHere)(FORMATETC *pformatetc, STGMEDIUM *pmedium) noexcept override;
    STDMETHOD(QueryGetData)(FORMATETC *pformatetc ) noexcept override;
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pformatectIn, FORMATETC* pformatetcOut) noexcept override;
    STDMETHOD(SetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) noexcept override;
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) noexcept override;
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) noexcept override;
    STDMETHOD(DUnadvise)(DWORD dwConnection) noexcept override;
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) noexcept override;

private:
    void SetDib(HGLOBAL hGlobal);
    HRESULT GetOleObject(IOleClientSite *oleClientSite, IStorage *storage, IOleObject *&oleObject);

    ULONG m_numReferences{};

    STGMEDIUM m_stgmed{};
    FORMATETC m_format{};
};

