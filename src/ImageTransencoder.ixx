// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

module;

#include "ComSmartPointers.h"

export module ImageTransencoder;

import ICodeGenerator;

export class CImageTransencoder final
{
public:
    CImageTransencoder() = default;
    ~CImageTransencoder() noexcept(false);

    CImageTransencoder(const CImageTransencoder&) = default;
    CImageTransencoder(CImageTransencoder&&) = default;
    CImageTransencoder& operator=(const CImageTransencoder&) = default;
    CImageTransencoder& operator=(CImageTransencoder&&) = default;

    HRESULT Begin(REFCLSID containerFormat, PCWSTR filename, ICodeGenerator &codeGen);
    HRESULT AddFrame(IWICBitmapSource* bitmapSource);
    HRESULT SetThumbnail(IWICBitmapSource* thumb) const;
    HRESULT SetPreview(IWICBitmapSource* preview) const;
    HRESULT End();

    WICPixelFormatGUID m_format{GUID_WICPixelFormatDontCare};

private:
    void Clear() noexcept;
    HRESULT AddBitmapSource(IWICBitmapSource* bitmapSource);
    HRESULT AddBitmapFrameDecode(IWICBitmapFrameDecode* frame);
    HRESULT CreateFrameEncode(IWICBitmapSource* bitmapSource, IWICBitmapFrameEncodePtr &frameEncode);

    ICodeGenerator*       m_codeGen{};
    IWICStreamPtr         m_stream;
    IWICBitmapEncoderPtr  m_encoder;
    bool                  m_encoding{};
    uint32_t              m_numPalettedFrames{};
};

