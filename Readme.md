# WIC Explorer sample

WIC Explorer is a tool that uses the Windows Imaging Component API to enumerate and list all of the elements within an image file, including metadata, frames, and thumbnails. It is useful for exploring the contents of images, as well as understanding how the WIC API is interpreting a specific file.

## Modifications

This Microsoft sample application has been modified to compile with Visual Studio 2022 and WTL 10 (included as git submodule).

## How to build

- `git clone --recurse-submodules https://github.com/vbaderks/WICExplorer.git`
- cd WICExplorer
- Open WICExplorer.sln in Visual Studio.  
  Select the platform (x86, x64, or ARM64) and configuration (Debug or Release) and build the solution.

## How to use

### Loading images

To load an image, go to File > Open... and select the file you wish to open.

To load an entire directory of images, go to File > Open Directory... and select the directory you wish to open.

WIC Explorer supports any file format that has a WIC codec installed. On Windows 10, the built-in codecs are:

- JPEG
- BMP
- PNG
- GIF
- ICO
- JPEG-XR
- TIFF
- DDS
- Camera Raw

### Exploring an image's elements

The left hand pane displays a hierarchical tree view of all of the WIC accessible elements in the image.

The lower left hand pane displays information about the WIC component (IWICComponentInfo) that was used to extract the currently highlighted node.

The right hand pane displays the contents of the currently highlighted node. This view changes depending on the type of node. For example, when selecting a frame (IWICBitmapFrameDecode), it displays attributes of the frame including DPI, resolution, and pixel format, as well as rendering the image data. When selecting a metadata reader (IWICMetadataReader), it displays all of the metadata items that are children of the node.

### Saving to another image format

WIC Explorer can save an image to any supported WIC encoder; you can also specify the desired pixel format in which to save. Note that not all of the listed pixel formats may be supported by the encoder; it will automatically perform pixel format conversion when necessary. It also will not preserve any metadata in the original image.