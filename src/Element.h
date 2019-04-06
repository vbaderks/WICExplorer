//----------------------------------------------------------------------------------------
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

struct InfoElementViewContext
{
    bool bIsAlphaEnable;
};

class CInfoElement
{
public:
    explicit CInfoElement(LPCWSTR name);
    virtual ~CInfoElement();

    const CString &Name()
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

    [[nodiscard]] BOOL IsChild(CInfoElement *element)
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
    static HRESULT CreateFrameAndChildElements(CInfoElement *parent, UINT index, IWICBitmapFrameDecodePtr frameDecode, ICodeGenerator &codeGen);
    static HRESULT CreateMetadataElementsFromBlock(CInfoElement *parent, IWICMetadataBlockReaderPtr blockReader, ICodeGenerator &codeGen);
    static HRESULT CreateMetadataElements(CInfoElement *parent, UINT childIdx, IWICMetadataReaderPtr reader, ICodeGenerator &codeGen);

    static CString queryKey;
    static CString queryValue;

private:
    static CInfoElement root;
};

class CComponentInfoElement : public CInfoElement
{
public:
    CComponentInfoElement(LPCWSTR name)
        : CInfoElement(name)
    {
    }

    HRESULT OutputMetadataHandlerInfo(IOutputDevice &output, IWICMetadataHandlerInfoPtr metaInfo);
    HRESULT OutputDecoderInfo(IOutputDevice &output, IWICBitmapDecoderInfoPtr decoderInfo);
    HRESULT OutputCodecInfo(IOutputDevice &output, IWICBitmapCodecInfoPtr codecInfo);
    HRESULT OutputComponentInfo(IOutputDevice &output, IWICComponentInfoPtr compInfo);
};

class CBitmapDecoderElement : public CComponentInfoElement
{
public:
    CBitmapDecoderElement(const LPCWSTR filename)
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

    HRESULT SaveAsImage(CImageTransencoder &trans, ICodeGenerator &codeGen);

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context);
    HRESULT OutputInfo(IOutputDevice &output);

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
    CBitmapSourceElement(LPCWSTR name, IWICBitmapSourcePtr source)
        : CInfoElement(name)
        , m_source(source)
    {

    }

    HRESULT SaveAsImage(CImageTransencoder &trans, ICodeGenerator &codeGen);

    HRESULT OutputView(IOutputDevice &output, const InfoElementViewContext& context);
    HRESULT OutputInfo(IOutputDevice &output);
    void FillContextMenu(HMENU context);

protected:
    HRESULT CreateDibFromBitmapSource(IWICBitmapSourcePtr source,
        HGLOBAL &hGlobal, HGLOBAL* phAlpha);
    HRESULT CreateHbitmapFromBitmapSource(IWICBitmapSourcePtr source, HBITMAP &hGlobal);

private:
    IWICBitmapSourcePtr m_source;
    IWICBitmapSourcePtr m_colorTransform;
};



class CBitmapFrameDecodeElement final : public CBitmapSourceElement
{
public:
    CBitmapFrameDecodeElement(UINT index, IWICBitmapFrameDecodePtr frameDecode)
        : CBitmapSourceElement(L"", frameDecode)
        , m_index(index)
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
    UINT                     m_index;
    IWICBitmapFrameDecodePtr m_frameDecode;
};

class CMetadataReaderElement final : public CComponentInfoElement
{
public:
    CMetadataReaderElement(CInfoElement *parent, UINT idx, IWICMetadataReaderPtr reader);

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
    HRESULT TranslateValueID(PROPVARIANT *pv, unsigned options, CString &out);
    static HRESULT TrimQuotesFromName(CString &out);
    HRESULT SetNiceName(CInfoElement *parent, UINT idx);

    IWICMetadataReaderPtr m_reader;
};
