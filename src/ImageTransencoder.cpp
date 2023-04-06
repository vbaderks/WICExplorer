// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "Macros.h"
#include "ComSmartPointers.h"

module ImageTransencoder;

import Element;

import <std.h>;


namespace
{

    uint32_t NumPaletteColorsRequiredByFormat(const GUID& pf) noexcept
    {
        if (GUID_WICPixelFormat1bppIndexed == pf)
        {
            return 1 << 1;
        }
        if (GUID_WICPixelFormat2bppIndexed == pf)
        {
            return 1 << 2;
        }
        if (GUID_WICPixelFormat4bppIndexed == pf)
        {
            return 1 << 4;
        }
        if (GUID_WICPixelFormat8bppIndexed == pf)
        {
            return 1 << 8;
        }

        return 0;
    }

}


CImageTransencoder::~CImageTransencoder() noexcept(false)
{
    End();
}

void CImageTransencoder::Clear() noexcept
{
    m_codeGen           = nullptr;
    m_stream            = nullptr;
    m_encoder           = nullptr;
    m_encoding          = false;
    m_numPalettedFrames = 0;
}

HRESULT CImageTransencoder::Begin(REFGUID containerFormat, const LPCWSTR filename, ICodeGenerator &codeGen)
{
    HRESULT result = S_OK;

    Clear();

    m_codeGen = &codeGen;

    // Get a writable stream
    m_codeGen->BeginVariableScope(L"IWICStream*", L"stream", L"nullptr");

    m_codeGen->CallFunction(L"imagingFactory->CreateStream(&stream);");
    IFC(g_imagingFactory->CreateStream(&m_stream));

#pragma warning(push)
#pragma warning(disable : 4296) // '<': expression is always false [known problem in MSVC/STL, solved in VS 2022, 17.5, but 17.5 has critical flaw in named modules]
    m_codeGen->CallFunction(std::format(L"stream->InitializeFromFilename(\"{}\", GENERIC_WRITE);", filename));
#pragma warning(pop)
    IFC(m_stream->InitializeFromFilename(filename, GENERIC_WRITE));

    // Create the encoder
    m_codeGen->BeginVariableScope(L"IWICBitmapEncoder*", L"encoder", L"nullptr");

    m_codeGen->CallFunction(L"imagingFactory->CreateEncoder(containerFormat, nullptr, &encoder);");
    IFC(g_imagingFactory->CreateEncoder(containerFormat, nullptr, &m_encoder));

    if (m_encoder)
    {
        // Initialize it
        m_codeGen->CallFunction(L"encoder->Initialize(stream, WICBitmapEncoderCacheInMemory);");
        IFC(m_encoder->Initialize(m_stream, WICBitmapEncoderNoCache));

        // This transencoder is now ready to encode
        m_encoding = true;
    }

    return result;
}

HRESULT CImageTransencoder::AddFrame(IWICBitmapSource* bitmapSource)
{
    HRESULT result = S_OK;

    // Check the params
    ASSERT(bitmapSource);
    if (!bitmapSource)
    {
        return E_INVALIDARG;
    }

    // Check the state of the object
    ASSERT(m_encoding);
    if (!m_encoding)
    {
        return E_UNEXPECTED;
    }

    // Perform different operations if this is a frame vs. just a boring BitmapSource
    const IWICBitmapFrameDecodePtr frame{bitmapSource};
    if (frame)
    {
        m_codeGen->BeginVariableScope(L"IWICBitmapFrameDecode*", L"source", L"...");
        IFC(AddBitmapFrameDecode(frame));
        m_codeGen->EndVariableScope();
    }
    else
    {
        m_codeGen->BeginVariableScope(L"IWICBitmapSource*", L"source", L"...");
        IFC(AddBitmapSource(bitmapSource));
        m_codeGen->EndVariableScope();
    }

    return result;
}

HRESULT CImageTransencoder::SetThumbnail(IWICBitmapSource* thumb) const
{
    // Check the state of the object
    ASSERT(m_encoding);
    if (!m_encoding)
    {
        return E_UNEXPECTED;
    }

    // Set the Thumbnail
    m_codeGen->CallFunction(L"encoder->SetThumbnail(thumb);");

    // Allow failure
    m_encoder->SetThumbnail(thumb);

    return S_OK;
}

HRESULT CImageTransencoder::SetPreview(IWICBitmapSource* preview) const
{
    // Check the state of the object
    ASSERT(m_encoding);
    if (!m_encoding)
    {
        return E_UNEXPECTED;
    }

    // Set the Preview
    m_codeGen->CallFunction(L"encoder->SetPreview(preview);");

    // Allow failure
    m_encoder->SetPreview(preview);

    return S_OK;
}

HRESULT CImageTransencoder::End()
{
    if (m_encoder)
    {
        m_encoder->Commit();

        m_codeGen->EndVariableScope();
    }

    if ( m_stream)
    {
        m_codeGen->EndVariableScope();
    }

    Clear();

    return S_OK;
}


HRESULT CImageTransencoder::CreateFrameEncode(IWICBitmapSource* bitmapSource, IWICBitmapFrameEncodePtr &frameEncode)
{
    HRESULT result = S_OK;

    // Try to add a frame encode to the encoder
    m_codeGen->BeginVariableScope(L"IWICBitmapFrameEncode*", L"frame", L"nullptr");
    m_codeGen->CallFunction(L"encoder->CreateNewFrame(&frame, nullptr);");
    IFC(m_encoder->CreateNewFrame(&frameEncode, nullptr));

    // Initialize it
    m_codeGen->CallFunction(L"frame->Initialize(nullptr);");
    IFC(frameEncode->Initialize(nullptr));

    // Set the Size
    uint32_t width;
    uint32_t height;
    m_codeGen->CallFunction(L"source->GetSize(&width, &height);");
    IFC(bitmapSource->GetSize(&width, &height));

    m_codeGen->CallFunction(std::format(L"frame->SetSize({}, {});", width, height));
    IFC(frameEncode->SetSize(width, height));

    // Set the Resolution
    double dpiX;
    double dpiY;
    m_codeGen->CallFunction(L"source->GetResolution(&dpiX, &dpiY);");
    IFC(bitmapSource->GetResolution(&dpiX, &dpiY));

    m_codeGen->CallFunction(std::format(L"frame->SetResolution({}, {});", dpiX, dpiY));
    IFC(frameEncode->SetResolution(dpiX, dpiY));

    // Attempt to set the PixelFormat
    // This call is allowed to fail because the encoder may not support the desired format
    WICPixelFormatGUID desiredPixelFormat{};
    WICPixelFormatGUID supportedPixelFormat{};

    m_codeGen->CallFunction(L"source->GetPixelFormat(&desiredPixelFormat);");
    IFC(bitmapSource->GetPixelFormat(&desiredPixelFormat));

    m_codeGen->CallFunction(L"supportedPixelFormat = desiredPixelFormat;");
    if(m_format == GUID_WICPixelFormatDontCare)
    {
        supportedPixelFormat = desiredPixelFormat;
    }
    else
    {
        supportedPixelFormat = m_format;
    }

    m_codeGen->CallFunction(L"frame->SetPixelFormat(&supportedPixelFormat);");
    frameEncode->SetPixelFormat(&supportedPixelFormat);
    m_format = supportedPixelFormat;

    // If the format that we will encode to requires a palette, then we need to
    // retrieve one. First we will try the palette of the BitmapSource itself.
    // If that fails, we will generate a palette from the BitmapSource.
    const uint32_t numPaletteColors = NumPaletteColorsRequiredByFormat(supportedPixelFormat);
    if (numPaletteColors > 0)
    {
        // Create the palette
        m_codeGen->BeginVariableScope(L"IWICPalette*", L"palette", L"nullptr");
        IWICPalettePtr palette;

        m_codeGen->CallFunction(L"imagingFactory->CreatePalette(&palette);");
        IFC(g_imagingFactory->CreatePalette(&palette));

        // Attempt to get the palette from the source
        m_codeGen->CallFunction(L"source->CopyPalette(palette);");
        result = bitmapSource->CopyPalette(palette);

        if (FAILED(result))
        {
            // We weren't able to get the palette directly from the source,
            // so let's try to generate one from the source's data
            result = S_OK;

            m_codeGen->CallFunction(std::format(L"palette->InitializeFromBitmap(source, {}, false);", numPaletteColors));
            IFC(palette->InitializeFromBitmap(bitmapSource, numPaletteColors, false));
        }

        // Set the palette
        m_codeGen->CallFunction(L"frame->SetPalette(palette);");
        IFC(frameEncode->SetPalette(palette));

        // If this is the first frame to require a palette, use this palette as the
        // global palette
        m_numPalettedFrames++;
        if (1 == m_numPalettedFrames)
        {
            m_codeGen->CallFunction(L"encoder->SetPalette(palette);");
            m_encoder->SetPalette(palette);
        }

        m_codeGen->EndVariableScope();
    }

    // Copy the color profile, if there is one.
    IWICBitmapFrameDecodePtr frame = bitmapSource;
    uint32_t colorContextCount = 0;
    if (frame &&
        SUCCEEDED(frame->GetColorContexts(0, nullptr, &colorContextCount)) &&
        colorContextCount > 0)
    {
        std::vector<IWICColorContext*> contexts(colorContextCount);

        for (uint32_t i = 0; i < colorContextCount; i++)
        {
            IFC(g_imagingFactory->CreateColorContext(&contexts[i]));
        }

        IFC(frame->GetColorContexts(colorContextCount, contexts.data(), &colorContextCount));

        if(FAILED(frameEncode->SetColorContexts(colorContextCount, contexts.data())))
        {
            ::MessageBox(nullptr, L"Unable to copy color contexts", L"Warning", MB_OK);
        }

        for (size_t i{}; i < contexts.size(); ++i)
        {
            if(contexts[i] != nullptr)
            {
                contexts[i]->Release();
            }
        }
    }

    // Finally, write the actual BitmapSource
    WICRect rct{.Width = static_cast<int>(width), .Height = static_cast<int>(height)};

    m_codeGen->CallFunction(L"frame->WriteSource(source, &rct);");
    IFC(frameEncode->WriteSource(bitmapSource, &rct));

    return result;
}

HRESULT CImageTransencoder::AddBitmapSource(IWICBitmapSource* bitmapSource)
{
    HRESULT result = S_OK;

    // Create the frame
    IWICBitmapFrameEncodePtr frameEncode;
    IFC(CreateFrameEncode(bitmapSource, frameEncode));

    // Nothing more to do with it
    m_codeGen->CallFunction(L"frame->Commit();");
    IFC(frameEncode->Commit());

    return result;
}

HRESULT CImageTransencoder::AddBitmapFrameDecode(IWICBitmapFrameDecode* frame)
{
    HRESULT result = S_OK;

    // Create the frame
    IWICBitmapFrameEncodePtr frameEncode;
    IFC(CreateFrameEncode(frame, frameEncode));

    // Output Thumbnail
    m_codeGen->BeginVariableScope(L"IWICBitmapSource*", L"thumb", L"nullptr");
    IWICBitmapSourcePtr thumb;

    m_codeGen->CallFunction(L"source->GetThumbnail(&thumb);");
    frame->GetThumbnail(&thumb);

    if (thumb)
    {
        m_codeGen->CallFunction(L"frame->SetThumbnail(thumb);");

        // This is allowed to fail
        frameEncode->SetThumbnail(thumb);
    }

    // TODO: Output Metadata

    // All done
    m_codeGen->CallFunction(L"frame->Commit();");
    IFC(frameEncode->Commit());

    return result;
}
