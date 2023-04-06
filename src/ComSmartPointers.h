// Copyright (c) Microsoft Corporation \ Victor Derks.
// SPDX-License-Identifier: MIT

#pragma once

#include <wincodecsdk.h>

#include <comip.h>
#include <comdef.h>
#include <comdefsp.h>

_COM_SMARTPTR_TYPEDEF(IWICImagingFactory, __uuidof(IWICImagingFactory));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoder, __uuidof(IWICBitmapDecoder));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(IWICBitmapSource));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));
_COM_SMARTPTR_TYPEDEF(IWICMetadataReader, __uuidof(IWICMetadataReader));

_COM_SMARTPTR_TYPEDEF(IWICComponentInfo, __uuidof(IWICComponentInfo));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockReader, __uuidof(IWICMetadataBlockReader));
_COM_SMARTPTR_TYPEDEF(IWICProgressiveLevelControl, __uuidof(IWICProgressiveLevelControl));
_COM_SMARTPTR_TYPEDEF(IWICBitmapDecoderInfo, __uuidof(IWICBitmapDecoderInfo));
_COM_SMARTPTR_TYPEDEF(IWICColorContext, __uuidof(IWICColorContext));
_COM_SMARTPTR_TYPEDEF(IWICColorTransform, __uuidof(IWICColorTransform));
_COM_SMARTPTR_TYPEDEF(IWICFormatConverter, __uuidof(IWICFormatConverter));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFlipRotator, __uuidof(IWICBitmapFlipRotator));
_COM_SMARTPTR_TYPEDEF(IWICMetadataHandlerInfo, __uuidof(IWICMetadataHandlerInfo));

_COM_SMARTPTR_TYPEDEF(IWICBitmapCodecInfo, __uuidof(IWICBitmapCodecInfo));

_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoderInfo, __uuidof(IWICBitmapEncoderInfo));
_COM_SMARTPTR_TYPEDEF(IWICPixelFormatInfo, __uuidof(IWICPixelFormatInfo));

_COM_SMARTPTR_TYPEDEF(IWICImagingFactory, __uuidof(IWICImagingFactory));

_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICStream, __uuidof(IWICStream));
_COM_SMARTPTR_TYPEDEF(IWICBitmapEncoder, __uuidof(IWICBitmapEncoder));

_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameDecode, __uuidof(IWICBitmapFrameDecode));
_COM_SMARTPTR_TYPEDEF(IWICBitmapFrameEncode, __uuidof(IWICBitmapFrameEncode));
_COM_SMARTPTR_TYPEDEF(IWICPalette, __uuidof(IWICPalette));
_COM_SMARTPTR_TYPEDEF(IWICBitmapSource, __uuidof(IWICBitmapSource));

_COM_SMARTPTR_TYPEDEF(IWICMetadataWriter, __uuidof(IWICMetadataWriter));
_COM_SMARTPTR_TYPEDEF(IWICMetadataBlockReader, __uuidof(IWICMetadataBlockReader));
_COM_SMARTPTR_TYPEDEF(IWICComponentFactory, __uuidof(IWICComponentFactory));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryWriter, __uuidof(IWICMetadataQueryWriter));
_COM_SMARTPTR_TYPEDEF(IWICMetadataQueryReader, __uuidof(IWICMetadataQueryReader));
_COM_SMARTPTR_TYPEDEF(IWICMetadataReader, __uuidof(IWICMetadataReader));
