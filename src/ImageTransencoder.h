//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
#pragma once

#include "CodeGenerator.h"

class CImageTransencoder final
{
public:
    CImageTransencoder() = default;
    ~CImageTransencoder();

    CImageTransencoder(const CImageTransencoder&) = default;
    CImageTransencoder(CImageTransencoder&&) = default;
    CImageTransencoder& operator=(const CImageTransencoder&) = default;
    CImageTransencoder& operator=(CImageTransencoder&&) = default;

    HRESULT Begin(REFCLSID containerFormat, LPCWSTR filename, ICodeGenerator &codeGen);
    HRESULT AddFrame(IWICBitmapSource* bitmapSource);
    HRESULT SetThumbnail(IWICBitmapSource* thumb) const;
    HRESULT SetPreview(IWICBitmapSource* preview) const;
    HRESULT End();

    WICPixelFormatGUID     m_format{GUID_WICPixelFormatDontCare};

private:
    void Clear();
    HRESULT AddBitmapSource(IWICBitmapSource* bitmapSource);
    HRESULT AddBitmapFrameDecode(IWICBitmapFrameDecode* frame);
    HRESULT CreateFrameEncode(IWICBitmapSource* bitmapSource, IWICBitmapFrameEncodePtr &frameEncode);

    ICodeGenerator       *m_codeGen{};
    IWICStreamPtr         m_stream;
    IWICBitmapEncoderPtr  m_encoder;
    bool                  m_encoding{};
    uint32_t              m_numPalettedFrames{};
};

