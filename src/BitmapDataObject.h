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

    CBitmapDataObject();
    ~CBitmapDataObject();

    // IUnknown Interface
    STDMETHOD(QueryInterface)(REFIID iid, void **ppvObject) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    // IDataObject Interface
    STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) override;
    STDMETHOD(GetDataHere)(FORMATETC *pformatetc, STGMEDIUM *pmedium) override;
    STDMETHOD(QueryGetData)(FORMATETC *pformatetc ) override;
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC *pformatectIn, FORMATETC* pformatetcOut) override;
    STDMETHOD(SetData)(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) override;
    STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) override;
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) override;
    STDMETHOD(DUnadvise)(DWORD dwConnection) override;
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) override;

private:
    void SetDib(HGLOBAL hGlobal);
    HRESULT GetOleObject(IOleClientSite *oleClientSite, IStorage *storage, IOleObject *&oleObject);

    ULONG m_numReferences{};

    STGMEDIUM m_stgmed{};
    FORMATETC m_format{};
};

