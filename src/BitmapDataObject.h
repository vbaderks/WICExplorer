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

    virtual ~CBitmapDataObject()
    {
        ReleaseStgMedium(&m_stgmed);
    }

    CBitmapDataObject(const CBitmapDataObject&) = delete;
    CBitmapDataObject(CBitmapDataObject&&) = delete;
    CBitmapDataObject& operator=(const CBitmapDataObject&) = delete;
    CBitmapDataObject& operator=(CBitmapDataObject&&) = delete;

    // IUnknown Interface
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject) noexcept override;
    ULONG STDMETHODCALLTYPE AddRef() noexcept override;
    ULONG STDMETHODCALLTYPE Release() noexcept override;

    // IDataObject Interface
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) noexcept override;
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC *pformatetc, STGMEDIUM *pmedium) noexcept override;
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC *pformatetc ) noexcept override;
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC *pformatectIn, FORMATETC* pformatetcOut) noexcept override;
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC *pformatetc, STGMEDIUM *pmedium, BOOL fRelease) noexcept override;
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC **ppenumFormatEtc) noexcept override;
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) noexcept override;
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD dwConnection) noexcept override;
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA **ppenumAdvise) noexcept override;

private:
    void SetDib(HGLOBAL hGlobal) noexcept;
    HRESULT GetOleObject(IOleClientSite *oleClientSite, IStorage *storage, IOleObject *&oleObject) noexcept;

    ULONG m_numReferences{};

    STGMEDIUM m_stgmed{};
    FORMATETC m_format{};
};

