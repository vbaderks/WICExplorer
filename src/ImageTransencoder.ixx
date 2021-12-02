//----------------------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------------------
module;

export module ImageTransencoder;

import CodeGenerator;

import "pch.h";


_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICStream, __uuidof(IWICStream));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder, __uuidof(IWICBitmapEncoder));


export class CImageTransencoder final
{
public:
    CImageTransencoder() = default;
    ~CImageTransencoder() noexcept(false);

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
    void Clear() noexcept;
    HRESULT AddBitmapSource(IWICBitmapSource* bitmapSource);
    HRESULT AddBitmapFrameDecode(IWICBitmapFrameDecode* frame);
    HRESULT CreateFrameEncode(IWICBitmapSource* bitmapSource, IWICBitmapFrameEncodePtr &frameEncode);

    ICodeGenerator       *m_codeGen{};
    IWICStreamPtr         m_stream;
    IWICBitmapEncoderPtr  m_encoder;
    bool                  m_encoding{};
    uint32_t              m_numPalettedFrames{};
};

