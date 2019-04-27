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

    HRESULT Begin(REFCLSID containerFormat, LPCWSTR filename, ICodeGenerator &codeGen);
    HRESULT AddFrame(IWICBitmapSource* bitmapSource);
    HRESULT SetThumbnail(IWICBitmapSourcePtr thumb);
    HRESULT SetPreview(IWICBitmapSourcePtr preview);
    HRESULT End();

    WICPixelFormatGUID     m_format{GUID_WICPixelFormatDontCare};

private:
    void Clear();
    HRESULT AddBitmapSource(IWICBitmapSourcePtr bitmapSource);
    HRESULT AddBitmapFrameDecode(IWICBitmapFrameDecodePtr frame);
    HRESULT CreateFrameEncode(IWICBitmapSourcePtr bitmapSource, IWICBitmapFrameEncodePtr &frameEncode);

    ICodeGenerator       *m_codeGen{};
    IWICStreamPtr         m_stream;
    IWICBitmapEncoderPtr  m_encoder;
    bool                  m_encoding{};
    UINT                  m_numPalettedFrames{};
};

