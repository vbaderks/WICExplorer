// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "Macros.h"
#include "resource.h"
#include "ComSmartPointers.h"

#include <atlres.h>

#include <cwctype>

module Element;

import MetadataTranslator;
import Stopwatch;
import Util;
import PropVariant;

import <std.h>;


namespace {

// Just hardcoded for now
[[nodiscard]]
bool HasAlpha(const GUID& guid) noexcept
{
    return IsEqualGUID(guid, GUID_WICPixelFormat32bppBGRA)
        || IsEqualGUID(guid, GUID_WICPixelFormat32bppPBGRA)
        || IsEqualGUID(guid, GUID_WICPixelFormat64bppRGBA)
        || IsEqualGUID(guid, GUID_WICPixelFormat64bppPRGBA)
        || IsEqualGUID(guid, GUID_WICPixelFormat128bppRGBAFloat)
        || IsEqualGUID(guid, GUID_WICPixelFormat128bppPRGBAFloat)
        || IsEqualGUID(guid, GUID_WICPixelFormat128bppRGBAFixedPoint);
}

[[nodiscard]]
uint16_t GetMaxPixelValue(const uint16_t* pixels, const size_t height, const size_t stride) noexcept {
    const size_t pixelsPerRow{stride / sizeof(uint16_t)};
    uint16_t max{};

    for (size_t y{}; y != height; ++y) {
        const uint16_t* row{pixels + y * pixelsPerRow};
        for (size_t i{}; i != pixelsPerRow; ++i) {
            if (row[i] > max) {
                max = row[i];
            }
        }
    }

    return max;
}


void NormalizeHistogram16BitGrayscale(IWICBitmap* bitmap)
{
    uint32_t width;
    uint32_t height;
    VERIFY(SUCCEEDED(bitmap->GetSize(&width, &height)));

    IWICBitmapLockPtr bitmapLock;
    const WICRect completeImage{0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height)};
    VERIFY(SUCCEEDED(bitmap->Lock(&completeImage, WICBitmapLockWrite, &bitmapLock)));

    uint32_t stride;
    VERIFY(SUCCEEDED(bitmapLock->GetStride(&stride)));

    void* dataBuffer;
    uint32_t dataBufferSize;
    VERIFY(SUCCEEDED(bitmapLock->GetDataPointer(&dataBufferSize, reinterpret_cast<BYTE**>(&dataBuffer))));
    __assume(dataBuffer != nullptr);

    auto max{GetMaxPixelValue(static_cast<const uint16_t*>(dataBuffer), height, stride)};
    auto bitsOfData{1};
    while (max > 1)
    {
        max = max >> 1;
        bitsOfData++;
    }

    const auto bitShift{16 - bitsOfData};
    if (bitShift <= 0)
    {
        return;
    }

    const size_t pixelsPerRow{stride / sizeof(uint16_t)};
    for (size_t y{}; y != height; ++y)
    {
        uint16_t* row{static_cast<uint16_t*>(dataBuffer) + y * pixelsPerRow};
        for (size_t x{}; x != pixelsPerRow; ++x)
        {
            WARNING_SUPPRESS_NEXT_LINE(6385 6386)
            row[x] = row[x] << bitShift;
        }
    }
}

}


HRESULT GetPixelFormatName(wchar_t* dest, uint32_t chars, const WICPixelFormatGUID guid)
{
    HRESULT result;
    if (guid == GUID_WICPixelFormatDontCare)
    {
        VERIFY(wcscpy_s(dest, chars, L"Don't Care") == 0);
        return S_OK;
    }
    IWICComponentInfoPtr info;
    IFC(g_imagingFactory->CreateComponentInfo(guid, &info));
    IFC(info->GetFriendlyName(chars, dest, &chars));
    return S_OK;
}


class CProgressiveBitmapSource final : public IWICBitmapSource
{
public:
    CProgressiveBitmapSource(IWICBitmapSource* source, const int level) :
        m_level(level),
        m_source(source)
    {
        m_source->AddRef();
        m_source->QueryInterface(IID_PPV_ARGS(&m_prog));
        m_prog->GetLevelCount(&m_max);
        m_max--;
    }

    ~CProgressiveBitmapSource()
    {
        WARNING_SUPPRESS_NEXT_LINE(26447) // The function is declared noexcept but calls function 'Release()' which may throw exceptions (f.6).
            m_source->Release();
        WARNING_SUPPRESS_NEXT_LINE(26447) // The function is declared noexcept but calls function 'Release()' which may throw exceptions (f.6).
            m_prog->Release();
    }

    CProgressiveBitmapSource() = delete;
    CProgressiveBitmapSource(const CProgressiveBitmapSource&) = delete;
    CProgressiveBitmapSource(CProgressiveBitmapSource&&) = delete;
    CProgressiveBitmapSource& operator=(const CProgressiveBitmapSource&) = delete;
    CProgressiveBitmapSource& operator=(CProgressiveBitmapSource&&) = delete;

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid,
        void** ppvObject) noexcept override
    {
        if (riid == IID_IUnknown)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        if (riid == IID_IWICBitmapSource)
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    ULONG STDMETHODCALLTYPE AddRef() noexcept override
    {
        m_ref++;
        return m_ref;
    }

    ULONG STDMETHODCALLTYPE Release() noexcept override
    {
        m_ref--;
        if (!m_ref)
        {
            delete this;
            return 0;
        }

        return m_ref;
    }

    HRESULT STDMETHODCALLTYPE GetSize(
        uint32_t* puiWidth,
        uint32_t* puiHeight
    ) override
    {
        return m_source->GetSize(puiWidth, puiHeight);
    }

    HRESULT STDMETHODCALLTYPE GetPixelFormat(
        WICPixelFormatGUID* pPixelFormat
    ) override
    {
        return m_source->GetPixelFormat(pPixelFormat);
    }

    HRESULT STDMETHODCALLTYPE GetResolution(
        double* pDpiX,
        double* pDpiY
    ) override
    {
        return m_source->GetResolution(pDpiX, pDpiY);
    }

    HRESULT STDMETHODCALLTYPE CopyPalette(
        IWICPalette* pIPalette
    ) override
    {
        return m_source->CopyPalette(pIPalette);
    }

    HRESULT STDMETHODCALLTYPE CopyPixels(
        const WICRect* prc,
        const uint32_t cbStride,
        const uint32_t cbBufferSize,
        BYTE* pbBuffer
    ) override
    {
        HRESULT result;

        IFC(m_prog->SetCurrentLevel(static_cast<uint32_t>(m_level)));
        IFC(m_source->CopyPixels(prc, cbStride, cbBufferSize, pbBuffer));
        IFC(m_prog->SetCurrentLevel(static_cast<uint32_t>(m_max)));

        return result;
    }

private:
    int m_level{};
    uint32_t m_max{};
    IWICBitmapSource* m_source{};
    IWICProgressiveLevelControl* m_prog{};
    uint32_t m_ref{};
};


IWICImagingFactoryPtr g_imagingFactory;

CInfoElement::CInfoElement(const PCWSTR name)
    : m_name{name}
{

    CElementManager::RegisterElement(this);
}

CInfoElement::~CInfoElement()
{
    RemoveChildren();
}

void CInfoElement::SetParent(const CInfoElement* element) noexcept
{
    if (m_parent != element)
    {
        if (m_parent)
        {
            m_parent->AddChild(this);
        }
        else
        {
            Unlink();
        }
    }
}

//Adds element after this object
void CInfoElement::AddSibling(CInfoElement* element) noexcept
{
    element->Unlink();
    element->m_prevSibling = this;
    element->m_nextSibling = m_nextSibling;
    m_nextSibling = element;
    element->m_parent = m_parent;
}

void CInfoElement::AddChild(CInfoElement* element)  noexcept
{
    CInfoElement* firstChild = FirstChild();
    if (!firstChild)
    {
        element->Unlink();
        m_firstChild = element;
        element->m_parent = this;
    }
    else
    {
        CInfoElement* prev = firstChild;
        while (firstChild)
        {
            prev = firstChild;
            firstChild = firstChild->NextSibling();
        }
        prev->AddSibling(element);
    }
}

void CInfoElement::RemoveChildren() noexcept
{
    //First unlink the children from the tree, then delete them
    while (m_firstChild)
    {
        RemoveChild(m_firstChild);
    }
}

void CInfoElement::Unlink() noexcept
{
    if (m_parent && m_parent->m_firstChild == this)
    {
        m_parent->m_firstChild = m_nextSibling;
    }
    if (m_nextSibling)
    {
        m_nextSibling->m_prevSibling = m_prevSibling;
    }
    if (m_prevSibling)
    {
        m_prevSibling->m_nextSibling = m_nextSibling;
    }
    m_nextSibling = nullptr;
    m_prevSibling = nullptr;
    m_parent = nullptr;
}

void CInfoElement::RemoveChild(CInfoElement* child) noexcept
{
    if (child->m_parent != this)
    {
        return;
    }
    child->Unlink();
    delete child;
}

HRESULT CElementManager::OpenFile(const PCWSTR filename, ICodeGenerator& codeGen, CInfoElement*& decElem)
{
    HRESULT result = E_UNEXPECTED;

    ASSERT(g_imagingFactory);

    if (g_imagingFactory)
    {
        // Refresh the list of codecs
        IEnumUnknownPtr enumUnknown;
        IFC(g_imagingFactory->CreateComponentEnumerator(WICDecoder, WICComponentEnumerateRefresh, &enumUnknown));

        // Begin monitoring how long this takes
        CStopwatch creationTimer;
        creationTimer.Start();

        decElem = nullptr;
        IFC(CreateDecoderAndChildElements(filename, codeGen, decElem));

        codeGen.EndVariableScope();

        // Remember how long this took
        if (decElem)
        {
            auto* realDecElem = dynamic_cast<CBitmapDecoderElement*>(decElem);
            if (nullptr != realDecElem)
            {
                realDecElem->SetCreationTime(creationTimer.GetTimeMS());
                realDecElem->SetCreationCode(codeGen.GenerateCode().c_str());
            }
        }
    }

    return result;
}

void CBitmapDecoderElement::Unload()
{
    if (m_decoder)
    {
        m_decoder->Release();
        m_decoder = nullptr;
    }

    RemoveChildren();
    m_loaded = false;
}

HRESULT CBitmapDecoderElement::Load(ICodeGenerator& codeGen)
{
    HRESULT result = S_OK;

    Unload();
    codeGen.BeginVariableScope(L"IWICBitmapDecoder*", L"decoder", L"nullptr");

    codeGen.CallFunction(std::format(L"imagingFactory->CreateDecoderFromFilename(\"{}\", nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);", m_filename));
    IFC(g_imagingFactory->CreateDecoderFromFilename(m_filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_decoder));

    // For each of the frames, create an element
    uint32_t frameCount = 0;

    codeGen.CallFunction(L"decoder->GetFrameCount(&frameCount);");
    IFC(m_decoder->GetFrameCount(&frameCount));

    // Even if one frame fails, we keep trying the others. However, we still
    // want to remember the failure so that we can return it from this function.
    HRESULT lastFailResult = result;

    if (frameCount == 0)
    {
        lastFailResult = E_FAIL;
    }

    for (uint32_t i = 0; i < frameCount; i++)
    {
        codeGen.BeginVariableScope(L"IWICBitmapFrameDecode*", L"frameDecode", L"nullptr");
        IWICBitmapFrameDecodePtr frameDecode;

        codeGen.CallFunction(std::format(L"decoder->GetFrame({}, &frameDecode);", i));
        HRESULT frameResult = m_decoder->GetFrame(i, &frameDecode);

        if (SUCCEEDED(frameResult))
        {
            frameResult = CElementManager::CreateFrameAndChildElements(this, i, frameDecode, codeGen);
        }

        if (FAILED(frameResult))
        {
            lastFailResult = frameResult;
        }

        codeGen.EndVariableScope();
    }

    // Add the Thumbnail if it exists
    IWICBitmapSourcePtr thumb;

    codeGen.CallFunction(L"decoder->GetThumbnail(&thumb);");
    result = m_decoder->GetThumbnail(&thumb);

    if (SUCCEEDED(result))
    {
        auto thumbElem = std::make_unique<CBitmapSourceElement>(L"Thumbnail", thumb);
        CElementManager::AddChildToElement(this, thumbElem.release());
    }

    // Add the Preview if it exists
    IWICBitmapSourcePtr preview;

    codeGen.CallFunction(L"decoder->GetPreview(&preview);");
    result = m_decoder->GetPreview(&preview);

    if (SUCCEEDED(result))
    {
        auto prevElem = std::make_unique<CBitmapSourceElement>(L"Preview", preview);
        CElementManager::AddChildToElement(this, prevElem.release());
    }

    // For each of the MetadataReaders attached to the decoder, create an element
    IWICMetadataBlockReaderPtr blockReader;

    result = m_decoder->QueryInterface(IID_PPV_ARGS(&blockReader));

    if (SUCCEEDED(result))
    {
        IFC(CElementManager::CreateMetadataElementsFromBlock(this, blockReader, codeGen));
    }
    else
    {
        result = S_OK;
    }

    codeGen.EndVariableScope();

    m_loaded = FirstChild() != nullptr;

    return (FAILED(lastFailResult)) ? lastFailResult : result;
}

HRESULT CElementManager::CreateDecoderAndChildElements(const PCWSTR filename, ICodeGenerator& codeGen, CInfoElement*& decElem)
{
    decElem = std::make_unique<CBitmapDecoderElement>(filename).release();
    const HRESULT result = (static_cast<CBitmapDecoderElement*>(decElem)->Load(codeGen));
    if (!static_cast<CBitmapDecoderElement*>(decElem)->IsLoaded())
    {
        root.RemoveChild(decElem);
        decElem = nullptr;
    }

    return result;
}

HRESULT CElementManager::CreateFrameAndChildElements(CInfoElement* parent, const uint32_t index, IWICBitmapFrameDecode* frameDecode, ICodeGenerator& codeGen)
{
    // Add the frame itself
    auto frameElemOwned = std::make_unique<CBitmapFrameDecodeElement>(index, frameDecode);
    CBitmapFrameDecodeElement* frameElem = frameElemOwned.get();

    AddChildToElement(parent, frameElemOwned.release());

    // Add the Thumbnail if it exists
    IWICBitmapSourcePtr thumb;

    codeGen.CallFunction(L"frameDecode->GetThumbnail(&thumb);");
    HRESULT result = frameDecode->GetThumbnail(&thumb);

    if (SUCCEEDED(result))
    {
        auto thumbElem = std::make_unique<CBitmapSourceElement>(L"Thumbnail", thumb);
        AddChildToElement(frameElem, thumbElem.release());
    }

    IWICProgressiveLevelControlPtr prog;
    result = frameDecode->QueryInterface(IID_PPV_ARGS(&prog));

    if (SUCCEEDED(result))
    {
        uint32_t count{};
        result = prog->GetLevelCount(&count);
        if (count > 1)
        {
            for (int c = 0; c < static_cast<int>(count); c++)
            {
                AddChildToElement(frameElem, std::make_unique<CBitmapSourceElement>(L"Level",
                    std::make_unique<CProgressiveBitmapSource>(frameDecode, c).release()).release());
            }
        }
    }

    // For each of the MetadataReaders attached to the frame, create an element
    codeGen.BeginVariableScope(L"IWICMetadataBlockReader*", L"blockReader", L"nullptr");
    IWICMetadataBlockReaderPtr blockReader;

    codeGen.CallFunction(L"frameDecode->QueryInterface(IID_IWICMetadataBlockReader, (void**)&blockReader);");
    result = frameDecode->QueryInterface(IID_PPV_ARGS(&blockReader));

    if (SUCCEEDED(result))
    {
        IFC(CreateMetadataElementsFromBlock(frameElem, blockReader, codeGen));
    }
    else
    {
        result = S_OK;
    }

    codeGen.EndVariableScope();

    return result;
}

HRESULT CElementManager::CreateMetadataElementsFromBlock(CInfoElement* parent, IWICMetadataBlockReader* blockReader, ICodeGenerator& codeGen)
{
    HRESULT result;

    uint32_t blockCount = 0;

    codeGen.CallFunction(L"blockReader->GetCount(&count);");
    IFC(blockReader->GetCount(&blockCount));

    for (uint32_t i = 0; i < blockCount; i++)
    {
        codeGen.BeginVariableScope(L"IWICMetadataReader*", L"reader", L"nullptr");
        IWICMetadataReaderPtr reader;

        codeGen.CallFunction(std::format(L"blockReader->GetReaderByIndex({}, &reader);", i));
        IFC(blockReader->GetReaderByIndex(i, &reader));

        IFC(CreateMetadataElements(parent, i, reader, codeGen));

        codeGen.EndVariableScope();
    }

    return result;
}

HRESULT CElementManager::CreateMetadataElements(CInfoElement* parent, uint32_t childIdx, IWICMetadataReader* reader, ICodeGenerator& codeGen)
{
    HRESULT result = S_OK;

    // Add this reader
    auto readerElemOwned = std::make_unique<CMetadataReaderElement>(parent, childIdx, reader);
    CInfoElement* readerElem = readerElemOwned.get();

    AddChildToElement(parent, readerElemOwned.release());

    // Search for any embedded readers
    uint32_t numValues = 0;

    codeGen.CallFunction(L"reader->GetCount(&count)");
    IFC(reader->GetCount(&numValues));

    for (uint32_t i = 0; i < numValues; i++)
    {
        PROPVARIANT id, value;

        PropVariantInit(&id);
        PropVariantInit(&value);

        codeGen.CallFunction(std::format(L"reader->GetValueByIndex({}, nullptr, &id, &value);", i));
        IFC(reader->GetValueByIndex(i, nullptr, &id, &value));

        if (VT_UNKNOWN == value.vt)
        {
            // Attempt to QI for a BlockReader
            codeGen.BeginVariableScope(L"IWICMetadataReader*", L"embReader", L"nullptr");
            IWICMetadataReaderPtr embReader;

            codeGen.CallFunction(L"value.punkVal->QueryInterface(IID_IWICMetadataReader, (void**)&embReader);");
            result = value.punkVal->QueryInterface(IID_PPV_ARGS(&embReader));

            if (SUCCEEDED(result))
            {
                IFC(CreateMetadataElements(readerElem, i, embReader, codeGen));
            }
            else
            {
                result = S_OK;
            }

            codeGen.EndVariableScope();
        }
        else if ((VT_VECTOR | VT_UNKNOWN) == value.vt)
        {

        }

        PropVariantClear(&id);
        PropVariantClear(&value);
    }

    return result;
}

void CElementManager::RegisterElement(CInfoElement* element) noexcept
{
    if (!element->Parent() && !root.IsChild(element) && element != &root)
    {
        root.AddChild(element);
    }
}

void CElementManager::ClearAllElements() noexcept
{
    root.RemoveChildren();
}

void CElementManager::AddSiblingToElement(CInfoElement* element, CInfoElement* sib) noexcept
{
    ASSERT(element);

    if (element)
    {
        // Find the end of the sibling chain
        CInfoElement* curr = element;
        while (nullptr != curr->NextSibling())
        {
            curr = curr->NextSibling();
        }

        // Add
        curr->AddSibling(sib);
    }
}

void CElementManager::AddChildToElement(CInfoElement* element, CInfoElement* child) noexcept
{
    ASSERT(element);

    if (element)
    {
        element->AddChild(child);
    }
}

CInfoElement* CElementManager::GetRootElement() noexcept
{
    return &root;
}

HRESULT CElementManager::SaveElementAsImage(CInfoElement& element, REFGUID containerFormat, WICPixelFormatGUID& format, const PCWSTR filename, ICodeGenerator& codeGen)
{
    HRESULT result;

    CImageTransencoder te;

    IFC(te.Begin(containerFormat, filename, codeGen));
    te.m_format = format;

    IFC(element.SaveAsImage(te, codeGen));
    format = te.m_format;

    IFC(te.End());

    return result;
}

CInfoElement CElementManager::root(L"");

//----------------------------------------------------------------------------------------
// COMPONENT INFO ELEMENT
//----------------------------------------------------------------------------------------



HRESULT CComponentInfoElement::OutputMetadataHandlerInfo(IOutputDevice& output, IWICMetadataHandlerInfo* metaInfo)
{
    output.BeginKeyValues(L"MetadataHandlerInfo");

    HRESULT result;
    GUID guid{};
    IFC(metaInfo->GetMetadataFormat(&guid));
    output.AddKeyValue(L"MetadataFormat", guid_to_string(guid).c_str());

    std::wstring str;
    result = GetWicString(*metaInfo, &(IWICMetadataHandlerInfo::GetDeviceManufacturer), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"DeviceManufacturer", str.c_str());
    }

    result = GetWicString(*metaInfo, &(IWICMetadataHandlerInfo::GetDeviceModels), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"DeviceModels", str.c_str());
    }

    output.EndKeyValues();

    IFC(OutputComponentInfo(output, metaInfo));

    return result;
}

HRESULT CComponentInfoElement::OutputDecoderInfo(IOutputDevice& output, IWICBitmapDecoderInfo* decoderInfo)
{
    HRESULT result = S_OK;

    IFC(OutputCodecInfo(output, decoderInfo));

    return result;
}

HRESULT CComponentInfoElement::OutputCodecInfo(IOutputDevice& output, IWICBitmapCodecInfo* codecInfo)
{
    output.BeginKeyValues(L"CodecInfo");

    HRESULT result = S_OK;
    GUID guid{};
    IFC(codecInfo->GetContainerFormat(&guid));
    output.AddKeyValue(L"ContainerFormat", guid_to_string(guid).c_str());

    std::wstring str;
    result = GetWicString(*codecInfo, &(IWICBitmapCodecInfo::GetColorManagementVersion), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"ColorManagementVersion", str.c_str());
    }

    result = GetWicString(*codecInfo, &(IWICBitmapCodecInfo::GetDeviceManufacturer), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"DeviceManufacturer", str.c_str());
    }

    result = GetWicString(*codecInfo, &(IWICBitmapCodecInfo::GetDeviceModels), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"DeviceModels", str.c_str());
    }

    result = GetWicString(*codecInfo, &(IWICBitmapCodecInfo::GetMimeTypes), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"MimeTypes", str.c_str());
    }

    BOOL b;
    IFC(codecInfo->DoesSupportAnimation(&b));
    output.AddKeyValue(L"DoesSupportAnimation", b ? L"True" : L"False");

    IFC(codecInfo->DoesSupportChromakey(&b));
    output.AddKeyValue(L"DoesSupportChromakey", b ? L"True" : L"False");

    IFC(codecInfo->DoesSupportLossless(&b));
    output.AddKeyValue(L"DoesSupportLossless", b ? L"True" : L"False");

    IFC(codecInfo->DoesSupportMultiframe(&b));
    output.AddKeyValue(L"DoesSupportMultiframe", b ? L"True" : L"False");

    output.EndKeyValues();

    IFC(OutputComponentInfo(output, codecInfo));

    return result;
}

HRESULT CComponentInfoElement::OutputComponentInfo(IOutputDevice& output, IWICComponentInfo* componentInfo)
{
    output.BeginKeyValues(L"ComponentInfo");

    HRESULT result = S_OK;
    GUID guid{};
    IFC(componentInfo->GetCLSID(&guid));
    output.AddKeyValue(L"ClassID", guid_to_string(guid).c_str());

    std::wstring str;
    result = GetWicString(*componentInfo, &(IWICComponentInfo::GetAuthor), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"Author", str.c_str());
    }

    IFC(componentInfo->GetVendorGUID(&guid));
    output.AddKeyValue(L"VendorGUID", guid_to_string(guid).c_str());

    result = GetWicString(*componentInfo, &(IWICComponentInfo::GetVersion), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"Version", str.c_str());
    }

    result = GetWicString(*componentInfo, &(IWICComponentInfo::GetSpecVersion), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"SpecVersion", str.c_str());
    }

    result = GetWicString(*componentInfo, &(IWICComponentInfo::GetFriendlyName), str);
    if (SUCCEEDED(result))
    {
        output.AddKeyValue(L"FriendlyName", str.c_str());
    }

    output.EndKeyValues();

    return result;
}


//----------------------------------------------------------------------------------------
// BITMAP DECODER ELEMENT
//----------------------------------------------------------------------------------------

void CBitmapDecoderElement::FillContextMenu(const HMENU context) noexcept
{
    CComponentInfoElement::FillContextMenu(context);

    MENUITEMINFO itemInfo{
        .cbSize = sizeof itemInfo,
        .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING,
        .fType = MFT_STRING,
        .fState = MFS_ENABLED
    };

    if (m_loaded)
    {
        itemInfo.wID = ID_FILE_SAVE;
        WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
            itemInfo.dwTypeData = const_cast<PWSTR>(L"Save As Image...");
        VERIFY(InsertMenuItem(context, GetMenuItemCount(context), true, &itemInfo));

        itemInfo.wID = ID_FILE_UNLOAD;
        WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
            itemInfo.dwTypeData = const_cast<PWSTR>(L"Unload");
    }
    else
    {
        itemInfo.wID = ID_FILE_LOAD;
        WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
            itemInfo.dwTypeData = const_cast<PWSTR>(L"Load");
    }
    VERIFY(InsertMenuItem(context, GetMenuItemCount(context), true, &itemInfo));

    itemInfo.wID = ID_FILE_CLOSE;
    WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
        itemInfo.dwTypeData = const_cast<PWSTR>(L"Close");
    VERIFY(InsertMenuItem(context, GetMenuItemCount(context), true, &itemInfo));
}

HRESULT CBitmapDecoderElement::SaveAsImage(CImageTransencoder& trans, ICodeGenerator& codeGen) noexcept(false)
{
    HRESULT result = S_OK;
    if (!m_loaded)
    {
        return E_FAIL;
    }

    // Find the frame children and output them
    CInfoElement* child = FirstChild();
    while (nullptr != child)
    {
        auto* frameDecodeElement = dynamic_cast<CBitmapFrameDecodeElement*>(child);
        if (frameDecodeElement)
        {
            IFC(frameDecodeElement->SaveAsImage(trans, codeGen));
        }

        child = child->NextSibling();
    }

    // Output Thumbnail
    codeGen.BeginVariableScope(L"IWICBitmapSource*", L"thumb", L"nullptr");
    IWICBitmapSourcePtr thumb;

    codeGen.CallFunction(L"decoder->GetThumbnail(&thumb);");
    m_decoder->GetThumbnail(&thumb);

    if (thumb)
    {
        IFC(trans.SetThumbnail(thumb));
    }

    codeGen.EndVariableScope();

    // Output Preview
    codeGen.BeginVariableScope(L"IWICBitmapSource*", L"preview", L"nullptr");
    IWICBitmapSourcePtr preview;

    codeGen.CallFunction(L"decoder->GetPreview(&preview);");
    m_decoder->GetPreview(&preview);

    if (preview)
    {
        IFC(trans.SetPreview(preview));
    }

    codeGen.EndVariableScope();

    // TODO: Output ColorContext

    // TODO: Output Metadata

    return result;
}

HRESULT CBitmapDecoderElement::OutputView(IOutputDevice& output, const InfoElementViewContext& context)
{
    if (!m_loaded)
    {
        const int oldSize = output.SetFontSize(20);
        output.AddText(L"File not loaded");
        output.SetFontSize(oldSize);
        return S_OK;
    }

    ASSERT(m_decoder);

    if (m_decoder)
    {
        output.BeginKeyValues(L"");

        output.AddKeyValue(L"Filename", m_filename.c_str());

        // Get the number of frames
        uint32_t numFrames;
        m_decoder->GetFrameCount(&numFrames);
        output.AddKeyValue(L"FrameCount", std::to_wstring(numFrames).c_str());

        // Display the decode time
        WARNING_SUPPRESS_NEXT_LINE(4296) // '<': expression is always false
            output.AddKeyValue(L"CreationTime", std::format(L"{} ms", m_creationTime).c_str());

        output.EndKeyValues();

        // Also show the children
        CInfoElement* child = FirstChild();
        while (nullptr != child)
        {
            output.BeginSection(child->Name().c_str());

            child->OutputView(output, context);
            child = child->NextSibling();

            output.EndSection();
        }

        // Show the code
        if (!m_creationCode.empty())
        {
            output.BeginSection(L"Creation Code");
            output.AddVerbatimText(m_creationCode.c_str());
            output.EndSection();
        }
    }
    else
    {
    }

    return S_OK;
}

HRESULT CBitmapDecoderElement::OutputInfo(IOutputDevice& output) noexcept(false)
{
    HRESULT result = S_OK;

    if (m_loaded)
    {
        IWICBitmapDecoderInfoPtr decoderInfo;
        IFC(m_decoder->GetDecoderInfo(&decoderInfo));
        IFC(OutputDecoderInfo(output, decoderInfo));
    }

    return result;
}

void CBitmapDecoderElement::SetCreationTime(const DWORD ms) noexcept
{
    m_creationTime = ms;
}

void CBitmapDecoderElement::SetCreationCode(const PCWSTR code)
{
    m_creationCode = code;
}


//----------------------------------------------------------------------------------------
// BITMAP SOURCE ELEMENT
//----------------------------------------------------------------------------------------

void CBitmapSourceElement::FillContextMenu(const HMENU context) noexcept
{
    CInfoElement::FillContextMenu(context);

    // Add the Save as Bitmap string if this element supports it
    const MENUITEMINFO itemInfo{
        .cbSize = sizeof itemInfo,
        .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING,
        .fType = MFT_STRING,
        .fState = MFS_ENABLED,
        .wID = ID_FILE_SAVE,
        WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
        .dwTypeData = const_cast<LPWSTR>(L"Save As Image...")
    };

    InsertMenuItem(context, GetMenuItemCount(context), true, &itemInfo);
}

HRESULT CBitmapSourceElement::SaveAsImage(CImageTransencoder& trans, ICodeGenerator& /*codeGen*/) noexcept(false)
{
    HRESULT result = S_OK;

    IFC(trans.AddFrame(m_source));

    return result;
}


HRESULT CBitmapSourceElement::OutputView(IOutputDevice& output, const InfoElementViewContext& context)
{
    HRESULT result;

    ASSERT(m_source);

    IFC(CInfoElement::OutputView(output, context));
    if (m_source)
    {
        // First, some info
        uint32_t width;
        uint32_t height;
        IFC(m_source->GetSize(&width, &height));

        double dpiX;
        double dpiY;
        IFC(m_source->GetResolution(&dpiX, &dpiY));

        WICPixelFormatGUID pixelFormat{};
        IFC(m_source->GetPixelFormat(&pixelFormat));

        output.BeginKeyValues(L"");

        output.AddKeyValue(L"Width", std::to_wstring(width).c_str());
        output.AddKeyValue(L"Height", std::to_wstring(height).c_str());
        output.AddKeyValue(L"DpiX", std::to_wstring(dpiX).c_str());
        output.AddKeyValue(L"DpiY", std::to_wstring(dpiY).c_str());

        wchar_t v[128];
        if (FAILED(GetPixelFormatName(v, ARRAYSIZE(v), pixelFormat)))
        {
            VERIFY(wcscpy_s(v, ARRAYSIZE(v), L"Unknown ") == 0);
        }
        VERIFY(wcscat_s(v, ARRAYSIZE(v), L" ") == 0);
        const size_t len = wcslen(v);
        VERIFY(StringFromGUID2(pixelFormat, v + len, static_cast<int>(ARRAYSIZE(v) - len)) != 0);
        output.AddKeyValue(L"Format", v);

        // Now, the bitmap itself
        CStopwatch renderTimer;
        renderTimer.Start();

        HGLOBAL hGlobal = nullptr;
        HGLOBAL hAlpha = nullptr;
        IWICBitmapSourcePtr source;

        if (!m_colorTransform)
        {
            IWICBitmapFrameDecodePtr frame;
            IWICColorContextPtr colorContextSrc;
            IWICColorContextPtr colorContextDst;
            IWICColorTransformPtr colorTransform;
            uint32_t cActual = 0;

            result = m_source->QueryInterface(IID_PPV_ARGS(&frame));

            if (SUCCEEDED(result))
            {
                IWICColorContext** ppiContextSrc = &colorContextSrc;
                result = g_imagingFactory->CreateColorContext(ppiContextSrc);

                if (SUCCEEDED(result))
                {
                    result = frame->GetColorContexts(1, ppiContextSrc, &cActual);
                    output.AddKeyValue(L"Total ColorContexts", std::to_wstring(cActual).c_str());
                }

                if (SUCCEEDED(result) && cActual > 0)
                {
                    std::wstring filename;
                    result = g_imagingFactory->CreateColorContext(&colorContextDst);

                    if (SUCCEEDED(result))
                    {
                        result = colorContextDst->InitializeFromExifColorSpace(1);
                    }

                    if (SUCCEEDED(result))
                    {
                        result = g_imagingFactory->CreateColorTransformer(&colorTransform);
                    }

                    if (SUCCEEDED(result))
                    {
                        result = colorTransform->Initialize(m_source,
                            colorContextSrc,
                            colorContextDst,
                            GUID_WICPixelFormat32bppBGRA);
                        if (SUCCEEDED(result))
                        {
                            m_colorTransform = colorTransform;
                            output.AddKeyValue(L"Output ColorContext", filename.c_str());
                        }
                    }
                }
            }
        }

        if (m_colorTransform)
        {
            source = m_colorTransform;
        }
        else
        {
            source = m_source;
        }

        if (context.normalizeHistogram && pixelFormat == GUID_WICPixelFormat16bppGray)
        {
            IWICBitmapPtr bitmap;
            result = g_imagingFactory->CreateBitmapFromSource(source, WICBitmapCacheOnDemand, &bitmap);

            NormalizeHistogram16BitGrayscale(bitmap);
            source = bitmap;
        }

        result = CreateDibFromBitmapSource(source, hGlobal, context.bIsAlphaEnable ? &hAlpha : nullptr);

        const DWORD renderTime = renderTimer.GetTimeMS();

        // Note how long it took to render
        output.AddKeyValue(L"Time", std::format(L"{} ms", renderTime).c_str());

        output.EndKeyValues();

        // Output the bitmap
        if (SUCCEEDED(result))
        {
            output.AddText(L"RGB:\n");
            output.AddDib(hGlobal);
            if (hAlpha != nullptr)
            {
                output.AddText(L"Alpha:\n");
                output.AddDib(hAlpha);
            }
        }
        else
        {
            const COLORREF oldColor{output.SetTextColor(RGB(255, 0, 0))};
            output.AddText(std::format(L"Failed to convert IWICBitmapSource to HBITMAP: {}", GetHresultString(result)).c_str());
            output.SetTextColor(oldColor);
        }
    }
    else
    {
    }

    return result;
}

HRESULT CBitmapSourceElement::OutputInfo(IOutputDevice& /*output*/) noexcept
{
    return S_OK;
}


HRESULT CBitmapSourceElement::CreateDibFromBitmapSource(IWICBitmapSource* source, HGLOBAL& hGlobal, HGLOBAL* phAlpha)
{
    if (!source)
        return E_INVALIDARG;

    HRESULT result = S_OK;
    uint32_t width = 0, height = 0;
    uint32_t stride = 0;

    WICPixelFormatGUID pFormatGuid;
    IFC(source->GetPixelFormat(&pFormatGuid));

    const bool bAlphaEnabled = (phAlpha != nullptr) && HasAlpha(pFormatGuid);

    // Create a format converter
    IWICFormatConverterPtr formatConverter;
    IFC(g_imagingFactory->CreateFormatConverter(&formatConverter));

    // Initialize the format converter to output BGRA 32
    IFC(formatConverter->Initialize(source, GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom));

    // Create a FlipRotator because windows requires the bitmap to be bottom-up
    IWICBitmapFlipRotatorPtr flipper;
    IFC(g_imagingFactory->CreateBitmapFlipRotator(&flipper));
    IFC(flipper->Initialize(formatConverter, WICBitmapTransformFlipVertical));

    // Get the size
    IFC(flipper->GetSize(&width, &height));
    stride = width * 4;

    // Force the stride to be a multiple of sizeof(DWORD)
    stride = ((stride + sizeof(DWORD) - 1) / sizeof(DWORD)) * sizeof(DWORD);

    const size_t dibSize = sizeof(BITMAPINFOHEADER) + static_cast<size_t>(stride) * height;

    // Allocate the DIB bytes
    hGlobal = GlobalAlloc(GMEM_MOVEABLE, dibSize);
    ASSERT(hGlobal);

    if (nullptr == hGlobal)
    {
        return E_OUTOFMEMORY;
    }

    BYTE* dibBytes = static_cast<BYTE*>(GlobalLock(hGlobal));
    ASSERT(dibBytes);

    if (nullptr == dibBytes)
    {
        GlobalFree(hGlobal);
        return E_OUTOFMEMORY;
    }

    auto* bmih = reinterpret_cast<BITMAPINFOHEADER*>(dibBytes);
    BYTE* dibPixels = dibBytes + sizeof(BITMAPINFOHEADER);

    // Set the header
    ZeroMemory(bmih, sizeof(BITMAPINFOHEADER));
    bmih->biSize = sizeof(BITMAPINFOHEADER);
    bmih->biPlanes = 1;
    bmih->biBitCount = 32;
    bmih->biCompression = BI_RGB;
    bmih->biWidth = width;
    bmih->biHeight = height;
    bmih->biSizeImage = stride * height;

    // Copy the pixels
    WICRect rct;
    rct.X = 0;
    rct.Y = 0;
    rct.Width = width;
    rct.Height = height;

    result = flipper->CopyPixels(&rct, stride, stride * height, dibPixels);

    if (bAlphaEnabled)
    {
        *phAlpha = GlobalAlloc(GMEM_MOVEABLE, dibSize);
        if (*phAlpha == nullptr)
        {
            return E_OUTOFMEMORY;
        }

        BYTE* dibPixels2 = dibBytes + sizeof(BITMAPINFOHEADER);

        BYTE* dibAlphaBytes = static_cast<BYTE*>(GlobalLock(*phAlpha));

        ASSERT(dibAlphaBytes);
        if (nullptr == dibAlphaBytes)
        {
            GlobalFree(*phAlpha);
            *phAlpha = nullptr;
            return E_OUTOFMEMORY;
        }

        BYTE* dibAlphaPixels = dibAlphaBytes + sizeof(BITMAPINFOHEADER);
        auto* bmih2 = reinterpret_cast<BITMAPINFOHEADER*>(dibAlphaBytes);

        // Set the header
        ZeroMemory(bmih2, sizeof(BITMAPINFOHEADER));
        bmih2->biSize = sizeof(BITMAPINFOHEADER);
        bmih2->biPlanes = 1;
        bmih2->biBitCount = 32;
        bmih2->biCompression = BI_RGB;
        bmih2->biWidth = width;
        bmih2->biHeight = height;
        bmih2->biSizeImage = stride * height;

        // Fill the dib pixels with alpha values:
        for (unsigned y = 0; y < height; y++)
        {
            for (unsigned x = 0; x < width; x++)
            {
                dibAlphaPixels[x * 4 + 0] = dibPixels2[x * 4 + 3];
                dibAlphaPixels[x * 4 + 1] = dibPixels2[x * 4 + 3];
                dibAlphaPixels[x * 4 + 2] = dibPixels2[x * 4 + 3];
                dibAlphaPixels[x * 4 + 3] = dibPixels2[x * 4 + 3];
            }
            dibAlphaPixels += stride;
            dibPixels2 += stride;
        }
        GlobalUnlock(*phAlpha);
        if (FAILED(result))
        {
            GlobalFree(*phAlpha);
        }
    }

    // Unlock the buffer
    GlobalUnlock(hGlobal);

    if (FAILED(result))
    {
        GlobalFree(hGlobal);
    }

    return result;
}


//----------------------------------------------------------------------------------------
// BITMAP FRAME DECODE ELEMENT
//----------------------------------------------------------------------------------------

void CBitmapFrameDecodeElement::FillContextMenu(const HMENU context) noexcept
{
    CBitmapSourceElement::FillContextMenu(context);

    const MENUITEMINFO itemInfo =
    {
        .cbSize = sizeof itemInfo,
        .fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING,
        .fType = MFT_STRING,
        .fState = MFS_ENABLED,
        .wID = ID_FIND_METADATA,
        WARNING_SUPPRESS_NEXT_LINE(26465 26492) // Don't use const_cast to cast away const or volatile.
        .dwTypeData = const_cast<LPWSTR>(L"Find metadata by Query Language")
    };

    InsertMenuItem(context, GetMenuItemCount(context), true, &itemInfo);
}

HRESULT CBitmapFrameDecodeElement::SaveAsImage(CImageTransencoder& trans, ICodeGenerator& /*codeGen*/) noexcept(false)
{
    HRESULT result = S_OK;

    IFC(trans.AddFrame(m_frameDecode));

    return result;
}

HRESULT CBitmapFrameDecodeElement::OutputView(IOutputDevice& output, const InfoElementViewContext& context)
{
    HRESULT result = S_OK;

    ASSERT(m_frameDecode);

    if (m_frameDecode)
    {
        IFC(CBitmapSourceElement::OutputView(output, context));
    }

    return result;
}

HRESULT CBitmapFrameDecodeElement::OutputInfo(IOutputDevice& /*output*/) noexcept
{
    return S_OK;
}


//----------------------------------------------------------------------------------------
// METADATA READER ELEMENT
//----------------------------------------------------------------------------------------

CMetadataReaderElement::CMetadataReaderElement(CInfoElement* parent, const uint32_t idx, IWICMetadataReader* reader)
    : CComponentInfoElement(L"")
    , m_reader(reader)
{
    if (FAILED(SetNiceName(parent, idx)))
    {
        m_name = L"MetadataReader";
    }
}

HRESULT CMetadataReaderElement::SetNiceName(const CInfoElement* parent, const uint32_t idx)
{
    HRESULT result = S_OK;

    // Try to get a better name
    if (m_reader)
    {
        // First, get our FriendlyName
        std::wstring friendlyName;
        IWICMetadataHandlerInfoPtr info;
        result = m_reader->GetMetadataHandlerInfo(&info);
        if (SUCCEEDED(result))
        {
            VERIFY(SUCCEEDED(GetWicString(*info, &(IWICMetadataHandlerInfo::GetFriendlyName), friendlyName)));
        }

        // Next, try to get the name that our parent gave us. We can do this
        // only if our parent is a CMetadataReaderElement
        std::wstring pn;
        const auto* mre = dynamic_cast<const CMetadataReaderElement*>(parent);
        if (mre)
        {
            PROPVARIANT id;
            PropVariantInit(&id);

            result = mre->m_reader->GetValueByIndex(idx, nullptr, &id, nullptr);
            if (SUCCEEDED(result))
            {
                IFC(TranslateValueID(&id, 0, pn));
                TrimQuotesFromName(pn);
            }

            PropVariantClear(&id);
        }

        // Merge them into a name
        if (!pn.empty())
        {
            m_name = pn + (std::wstring{L" ("} + friendlyName + L")").c_str();
        }
        else
        {
            m_name = friendlyName.c_str();
        }
    }
    else
    {
        result = E_INVALIDARG;
    }

    return result;
}

HRESULT CMetadataReaderElement::TrimQuotesFromName(std::wstring& out)
{
    out.erase(remove(out.begin(), out.end(), '\"'), out.end());

    return S_OK;
}

HRESULT CMetadataReaderElement::OutputView(IOutputDevice& output, const InfoElementViewContext& context)
{
    HRESULT result = S_OK;

    ASSERT(m_reader);

    IFC(CComponentInfoElement::OutputView(output, context));

    if (m_reader)
    {
        output.BeginKeyValues(L"Metadata Values");

        uint32_t numValues = 0;
        IFC(m_reader->GetCount(&numValues));

        for (uint32_t i = 0; i < numValues; i++)
        {
            PROPVARIANT id, schema, value;

            PropVariantInit(&id);
            PropVariantInit(&schema);
            PropVariantInit(&value);

            IFC(m_reader->GetValueByIndex(i, &schema, &id, &value));

            std::wstring k;
            std::wstring v;

            IFC(TranslateValueID(&id, pvtsoption_include_type, k));
            IFC(PropVariantToString(&value, pvtsoption_include_type, v));

            if (schema.vt != VT_EMPTY)
            {
                std::wstring s;
                IFC(PropVariantToString(&schema, pvtsoption_include_type, s));
                output.AddKeyValue((k + L" [" + s.c_str() + L"]").c_str(), v.c_str());
            }
            else
            {
                output.AddKeyValue(k.c_str(), v.c_str());
            }

            PropVariantClear(&id);
            PropVariantClear(&schema);
            PropVariantClear(&value);
        }

        output.EndKeyValues();
    }

    return result;
}

HRESULT CMetadataReaderElement::OutputInfo(IOutputDevice& output) noexcept(false)
{
    HRESULT result = S_OK;

    ASSERT(m_reader);

    if (m_reader)
    {
        IWICMetadataHandlerInfoPtr handlerInfo;

        IFC(m_reader->GetMetadataHandlerInfo(&handlerInfo));

        IFC(OutputMetadataHandlerInfo(output, handlerInfo));
    }

    return result;
}

HRESULT CMetadataReaderElement::TranslateValueID(PROPVARIANT* pv, const unsigned options, std::wstring& out) const
{
    HRESULT result = S_OK;

    // Get our format
    GUID metadataFormat{};
    IWICMetadataHandlerInfoPtr handlerInfo;
    IFC(m_reader->GetMetadataHandlerInfo(&handlerInfo));
    IFC(handlerInfo->GetMetadataFormat(&metadataFormat));

    // Try using the translator
    std::wstring str;
    result = CMetadataTranslator::Inst().Translate(metadataFormat, pv, str);

    // If that failed, use the string converter
    if (FAILED(result))
    {
        result = PropVariantToString(pv, options, str);
        out = str.c_str();
    }
    else
    {
        out = str.c_str();
    }

    return result;
}
