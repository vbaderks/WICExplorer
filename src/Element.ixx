//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

#include <atlstr.h>
#include "ComSmartPointers.h"

export module Element;

import ImageTransencoder;
import IOutputDevice;
import ICodeGenerator;

export HRESULT GetPixelFormatName(wchar_t* dest, uint32_t chars, WICPixelFormatGUID guid);

export extern IWICImagingFactoryPtr g_imagingFactory;

export struct InfoElementViewContext
{
    bool bIsAlphaEnable;
};

export class CInfoElement
{
public:
    explicit CInfoElement(LPCWSTR name);
    virtual ~CInfoElement();

    CInfoElement() = delete;
    CInfoElement(const CInfoElement&) = default;
    CInfoElement(CInfoElement&&) = default;
    CInfoElement& operator=(const CInfoElement&) = default;
    CInfoElement& operator=(CInfoElement&&) = default;

    [[nodiscard]] const ATL::CString& Name() const noexcept
    {
        return m_name;
    }

    virtual HRESULT SaveAsImage(CImageTransencoder& /*trans*/, ICodeGenerator& /*codeGen*/) noexcept(false)
    {
        return E_NOTIMPL;
    }

    virtual HRESULT OutputView(IOutputDevice& output, const InfoElementViewContext& /*context*/)
    {
        if (m_queryKey != L"")
        {
            output.BeginKeyValues(L"Metadata Query Result");
            output.AddKeyValue(m_queryKey, m_queryValue);
            output.EndKeyValues();
            output.EndSection();
        }
        return S_OK;
    }

    virtual HRESULT OutputInfo(IOutputDevice& /*output*/) noexcept(false)
    {
        return S_OK;
    }

    [[nodiscard]] CInfoElement* Parent() const noexcept
    {
        return m_parent;
    }

    [[nodiscard]] CInfoElement* NextSibling() const noexcept
    {
        return m_nextSibling;
    }

    [[nodiscard]] CInfoElement* FirstChild() const noexcept
    {
        return m_firstChild;
    }

    [[nodiscard]] BOOL IsChild(const CInfoElement* element) const noexcept
    {
        CInfoElement* child = FirstChild();
        while (child)
        {
            if (element == child)
            {
                return 1;
            }
            child = child->NextSibling();
        }
        return 0;
    }

    void SetParent(const CInfoElement* element) noexcept;

    // Adds element after this object
    void AddSibling(CInfoElement* element) noexcept;

    // Adds element as the last child
    void AddChild(CInfoElement* element) noexcept;

    void RemoveChild(CInfoElement* child) noexcept;
    void RemoveChildren() noexcept;

    virtual void FillContextMenu(HMENU /*context*/) noexcept
    {
    }

    virtual CInfoElement* FindElementByReader(IWICMetadataReader* reader)
    {
        CInfoElement* result;
        if (FirstChild() != nullptr &&
            (result = FirstChild()->FindElementByReader(reader)) != nullptr)
        {
            return result;
        }
        if (NextSibling() != nullptr &&
            (result = NextSibling()->FindElementByReader(reader)) != nullptr)
        {
            return result;
        }
        return nullptr;
    }

    virtual HRESULT GetQueryReader(IWICMetadataQueryReader** ppReader) noexcept(false)
    {
        *ppReader = nullptr;
        return E_NOTIMPL;
    }

    ATL::CString m_queryKey;
    ATL::CString m_queryValue;

protected:
    ATL::CString   m_name;

private:
    void Unlink() noexcept;

    CInfoElement* m_parent{};
    CInfoElement* m_prevSibling{};
    CInfoElement* m_nextSibling{};
    CInfoElement* m_firstChild{};
};

export class CElementManager
{
public:
    static HRESULT OpenFile(LPCWSTR filename, ICodeGenerator& codeGen, CInfoElement*& decElem);

    static void RegisterElement(CInfoElement* element)  noexcept;
    static void ClearAllElements() noexcept;

    static void AddSiblingToElement(CInfoElement* element, CInfoElement* sib) noexcept;
    static void AddChildToElement(CInfoElement* element, CInfoElement* child) noexcept;

    static CInfoElement* GetRootElement() noexcept;

    static HRESULT SaveElementAsImage(CInfoElement& element, REFGUID containerFormat, WICPixelFormatGUID& format, LPCWSTR filename, ICodeGenerator& codeGen);
    static HRESULT CreateDecoderAndChildElements(LPCWSTR filename, ICodeGenerator& codeGen, CInfoElement*& decElem);
    static HRESULT CreateFrameAndChildElements(CInfoElement* parent, uint32_t index, IWICBitmapFrameDecode* frameDecode, ICodeGenerator& codeGen);
    static HRESULT CreateMetadataElementsFromBlock(CInfoElement* parent, IWICMetadataBlockReader* blockReader, ICodeGenerator& codeGen);
    static HRESULT CreateMetadataElements(CInfoElement* parent, uint32_t childIdx, IWICMetadataReader* reader, ICodeGenerator& codeGen);

    static ATL::CString queryKey;
    static ATL::CString queryValue;

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

    static HRESULT OutputMetadataHandlerInfo(IOutputDevice& output, IWICMetadataHandlerInfo* metaInfo);
    static HRESULT OutputDecoderInfo(IOutputDevice& output, IWICBitmapDecoderInfo* decoderInfo);
    static HRESULT OutputCodecInfo(IOutputDevice& output, IWICBitmapCodecInfo* codecInfo);
    static HRESULT OutputComponentInfo(IOutputDevice& output, IWICComponentInfo* componentInfo);
};

export class CBitmapDecoderElement final : public CComponentInfoElement
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
    HRESULT Load(ICodeGenerator& codeGen);

    // Releases the decoder and child objects but keeps the filename
    void Unload();

    [[nodiscard]] bool IsLoaded() const noexcept
    {
        return m_loaded;
    }

    HRESULT SaveAsImage(CImageTransencoder& trans, ICodeGenerator& codeGen) noexcept(false) override;

    HRESULT OutputView(IOutputDevice& output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice& output) noexcept(false) override;

    void SetCreationTime(DWORD ms) noexcept;
    void SetCreationCode(LPCWSTR code);
    void FillContextMenu(HMENU context) noexcept override;

private:
    ATL::CString              m_filename;
    IWICBitmapDecoderPtr m_decoder;
    DWORD                m_creationTime{};
    ATL::CString              m_creationCode;
    bool                 m_loaded{};
};

export class CBitmapSourceElement : public CInfoElement
{
public:
    CBitmapSourceElement(const LPCWSTR name, IWICBitmapSource* source)
        : CInfoElement(name)
        , m_source(source)
    {

    }

    HRESULT SaveAsImage(CImageTransencoder& trans, ICodeGenerator& codeGen) noexcept(false) override;

    HRESULT OutputView(IOutputDevice& output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice& output) noexcept override;
    void FillContextMenu(HMENU context) noexcept override;

protected:
    static HRESULT CreateDibFromBitmapSource(IWICBitmapSource* source,
        HGLOBAL& hGlobal, HGLOBAL* phAlpha);

private:
    IWICBitmapSourcePtr m_source;
    IWICBitmapSourcePtr m_colorTransform;
};


export class CBitmapFrameDecodeElement final : public CBitmapSourceElement
{
public:
    CBitmapFrameDecodeElement(const uint32_t index, IWICBitmapFrameDecode* frameDecode)
        : CBitmapSourceElement(L"", frameDecode)
        , m_frameDecode(frameDecode)
    {
        m_name.Format(L"Frame #%u", index);
    }

    HRESULT SaveAsImage(CImageTransencoder& trans, ICodeGenerator& codeGen) noexcept(false) override;
    void FillContextMenu(HMENU context) noexcept override;

    HRESULT OutputView(IOutputDevice& output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice& output) noexcept override;
    HRESULT GetQueryReader(IWICMetadataQueryReader** reader) noexcept(false) override
    {
        return m_frameDecode->GetMetadataQueryReader(reader);
    }

private:
    IWICBitmapFrameDecodePtr m_frameDecode;
};

export class CMetadataReaderElement final : public CComponentInfoElement
{
public:
    CMetadataReaderElement(CInfoElement* parent, uint32_t idx, IWICMetadataReader* reader);

    HRESULT OutputView(IOutputDevice& output, const InfoElementViewContext& context) override;
    HRESULT OutputInfo(IOutputDevice& output)  noexcept(false) override;
    CInfoElement* FindElementByReader(IWICMetadataReader* reader) override
    {
        if (static_cast<IWICMetadataReader*>(m_reader) == reader)
        {
            return this;
        }
        return CComponentInfoElement::FindElementByReader(reader);
    }

private:
    HRESULT TranslateValueID(PROPVARIANT* pv, unsigned options, ATL::CString& out) const;
    static HRESULT TrimQuotesFromName(ATL::CString& out);
    HRESULT SetNiceName(const CInfoElement* parent, uint32_t idx);

    IWICMetadataReaderPtr m_reader;
};
