﻿//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

#include "ImageTransencoder.h"
#include "OutputDevice.h"

HRESULT GetPixelFormatName(WCHAR* dest, UINT chars, WICPixelFormatGUID guid);

struct InfoElementViewContext
{
    bool bIsAlphaEnable;
};

class CInfoElement
{
public:
    explicit CInfoElement(LPCWSTR name);
    virtual ~CInfoElement();

    CInfoElement() = delete;
    CInfoElement(const CInfoElement&) = default;
    CInfoElement(CInfoElement&&) = default;
    CInfoElement& operator=(const CInfoElement&) = default;
    CInfoElement& operator=(CInfoElement&&) = default;

    [[nodiscard]] const CString &Name() const
    {
        return m_name;
    }

    virtual HRESULT SaveAsImage(CImageTransencoder & /*trans*/, ICodeGenerator & /*codeGen*/)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& /*context*/)
    {
        if(m_queryKey != L"")
        {
            output.BeginKeyValues(L"Metadata Query Result");
            output.AddKeyValue(m_queryKey, m_queryValue);
            output.EndKeyValues();
            output.EndSection();
        }
        return S_OK;
    }

    virtual HRESULT OutputInfo(IOutputDevice & /*output*/)
    {
        return S_OK;
    }

    [[nodiscard]] CInfoElement *Parent() const
    {
        return m_parent;
    }

    [[nodiscard]] CInfoElement *NextSibling() const
    {
        return m_nextSibling;
    }

    [[nodiscard]] CInfoElement *FirstChild() const
    {
        return m_firstChild;
    }

    [[nodiscard]] BOOL IsChild(CInfoElement *element) const
    {
        CInfoElement *child = FirstChild();
        while(child)
        {
            if(element == child)
            {
                return 1;
            }
            child = child->NextSibling();
        }
        return 0;
    }

    void SetParent(CInfoElement *element);
    // Adds element after this object
    void AddSibling(CInfoElement *element);
    // Adds element as the last child
    void AddChild(CInfoElement *element);
    void RemoveChild(CInfoElement *child);
    void RemoveChildren();
    virtual void FillContextMenu(HMENU /*context*/)
    {
    }

    virtual CInfoElement *FindElementByReader(IWICMetadataReader *reader)
    {
        CInfoElement *result;
        if(FirstChild() != nullptr &&
            (result = FirstChild()->FindElementByReader(reader)) != nullptr)
        {
            return result;
        }
        if(NextSibling() != nullptr &&
            (result = NextSibling()->FindElementByReader(reader)) != nullptr)
        {
            return result;
        }
        return nullptr;
    }
    virtual HRESULT GetQueryReader(IWICMetadataQueryReader **ppReader)
    {
        *ppReader = nullptr;
        return E_NOTIMPL;
    }

    CString m_queryKey;
    CString m_queryValue;

protected:
    CString   m_name;

private:
    void Unlink();

    CInfoElement *m_parent{};
    CInfoElement *m_prevSibling{};
    CInfoElement *m_nextSibling{};
    CInfoElement *m_firstChild{};
};

class CElementManager
{
public:
    static HRESULT OpenFile(LPCWSTR filename, ICodeGenerator &codeGen, CInfoElement *&decElem);

    static void RegisterElement(CInfoElement *element);
    static void ClearAllElements();

    static void AddSiblingToElement(CInfoElement *element, CInfoElement *sib);
    static void AddChildToElement(CInfoElement *element, CInfoElement *child);

    static CInfoElement *GetRootElement();

    static HRESULT SaveElementAsImage(CInfoElement &element, REFGUID containerFormat, WICPixelFormatGUID &format, LPCWSTR filename, ICodeGenerator &codeGen);
    static HRESULT CreateDecoderAndChildElements(LPCWSTR filename, ICodeGenerator &codeGen, CInfoElement *&decElem);
    static HRESULT CreateFrameAndChildElements(CInfoElement *parent, UINT index, IWICBitmapFrameDecode* frameDecode, ICodeGenerator &codeGen);
    static HRESULT CreateMetadataElementsFromBlock(CInfoElement *parent, IWICMetadataBlockReader* blockReader, ICodeGenerator &codeGen);
    static HRESULT CreateMetadataElements(CInfoElement *parent, UINT childIdx, IWICMetadataReader* reader, ICodeGenerator &codeGen);

    static CString queryKey;
    static CString queryValue;

private:
    static CInfoElement root;
};

class CComponentInfoElement : public CInfoElement
{
public:
    explicit CComponentInfoElement(const LPCWSTR name)
        : CInfoElement(name)
    {
    }

    static HRESULT OutputMetadataHandlerInfo(IOutputDevice &output, IWICMetadataHandlerInfo* metaInfo);
    static HRESULT OutputDecoderInfo(IOutputDevice &output, IWICBitmapDecoderInfo* decoderInfo);
    static HRESULT OutputCodecInfo(IOutputDevice &output, IWICBitmapCodecInfo* codecInfo);
    static HRESULT OutputComponentInfo(IOutputDevice &output, IWICComponentInfo* compInfo);
};

class CBitmapDecoderElement final : public CComponentInfoElement
{
public:
    explicit CBitmapDecoderElement(const LPCWSTR filename)
        : CComponentInfoElement(filename)
        , m_filename(filename)
    {
        const int pathDelim = m_name.ReverseFind('\\');
        if (pathDelim >= 0)
        {
            m_name = m_name.Right(m_name.GetLength() - pathDelim - 1);
        }
    }

    // Creates the decoder and child objects based on the filename
    HRESULT Load(ICodeGenerator &codeGen);

    // Releases the decoder and child objects but keeps the filename
    void Unload();

    [[nodiscard]] bool IsLoaded() const
    {
        return m_loaded;
    }

    HRESULT SaveAsImage(CImageTransencoder &trans, ICodeGenerator &codeGen) override;

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice &output) override;

    void SetCreationTime(DWORD ms);
    void SetCreationCode(LPCWSTR code);
    void FillContextMenu(HMENU context) override;

private:
    CString              m_filename;
    IWICBitmapDecoderPtr m_decoder;
    DWORD                m_creationTime{};
    CString              m_creationCode;
    bool                 m_loaded{};
};

class CBitmapSourceElement : public CInfoElement
{
public:
    CBitmapSourceElement(const LPCWSTR name, IWICBitmapSource* source)
        : CInfoElement(name)
        , m_source(source)
    {

    }

    HRESULT SaveAsImage(CImageTransencoder &trans, ICodeGenerator &codeGen) override;

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice &output) override;
    void FillContextMenu(HMENU context) override;

protected:
    static HRESULT CreateDibFromBitmapSource(IWICBitmapSource* source,
                                             HGLOBAL &hGlobal, HGLOBAL* phAlpha);

private:
    IWICBitmapSourcePtr m_source;
    IWICBitmapSourcePtr m_colorTransform;
};



class CBitmapFrameDecodeElement final : public CBitmapSourceElement
{
public:
    CBitmapFrameDecodeElement(const UINT index, IWICBitmapFrameDecode* frameDecode)
        : CBitmapSourceElement(L"", frameDecode)
        , m_frameDecode(frameDecode)
    {
        m_name.Format(L"Frame #%u", index);
    }

    HRESULT SaveAsImage(CImageTransencoder &trans, ICodeGenerator &codeGen) override;
    void FillContextMenu(HMENU context) override;

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice &output) override;
    HRESULT GetQueryReader(IWICMetadataQueryReader **reader) override
    {
        return m_frameDecode->GetMetadataQueryReader(reader);
    }

private:
    IWICBitmapFrameDecodePtr m_frameDecode;
};

class CMetadataReaderElement final : public CComponentInfoElement
{
public:
    CMetadataReaderElement(CInfoElement *parent, UINT idx, IWICMetadataReader* reader);

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice &output) override;
    CInfoElement *FindElementByReader(IWICMetadataReader *reader) override
    {
        if(static_cast<IWICMetadataReader *>(m_reader) == reader)
        {
            return this;
        }
        return CComponentInfoElement::FindElementByReader(reader);
    }

private:
    HRESULT TranslateValueID(PROPVARIANT *pv, unsigned options, CString &out) const;
    static HRESULT TrimQuotesFromName(CString &out);
    HRESULT SetNiceName(CInfoElement *parent, UINT idx);

    IWICMetadataReaderPtr m_reader;
};
