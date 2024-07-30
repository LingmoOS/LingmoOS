# KImageFormats

Plugins to allow QImage to support extra file formats.

## Introduction

This framework provides additional image format plugins for QtGui.  As
such it is not required for the compilation of any other software, but
may be a runtime requirement for Qt-based software to support certain
image formats.

## Formats

The following image formats have read-only support:

- Animated Windows cursors (ani)
- Camera RAW images (arw, cr2, cr3, dcs, dng, ...)
- Gimp (xcf)
- Krita (kra)
- OpenRaster (ora)
- Pixar raster (pxr)
- Portable FloatMap (pfm)
- Photoshop documents (psd, psb, pdd, psdt)
- Radiance HDR (hdr)
- Sun Raster (im1, im8, im24, im32, ras, sun)

The following image formats have read and write support:

- AV1 Image File Format (avif)
- Encapsulated PostScript (eps)
- High Efficiency Image File Format (heif). Can be enabled with the KIMAGEFORMATS_HEIF build option.
- JPEG XL (jxl)
- JPEG XR (jxr). Can be enabled with the KIMAGEFORMATS_JXR build option.
- OpenEXR (exr)
- Personal Computer Exchange (pcx)
- Quite OK Image format (qoi)
- SGI images (rgb, rgba, sgi, bw)
- Softimage PIC (pic)
- Targa (tga): supports more formats than Qt's version

## Contributing

See the QImageIOPlugin documentation for information on how to write a
new plugin.

The main difference between this framework and the qimageformats module
of Qt is the license.  As such, if you write an imageformat plugin and
you are willing to sign the Qt Project contributor agreement, it may be
better to submit the plugin directly to the Qt Project.

## Duplicated Plugins

The TGA plugin supports more formats than Qt's own TGA plugin;
specifically, the one provided here supports indexed, greyscale and RLE
images (types 1-3 and 9-11), while Qt's plugin only supports type 2
(RGB) files.

The code for this cannot be contributed upstream directly because of
licensing.  If anyone were willing to write fresh code to improve Qt's
TGA plugin, it would allow the TGA plugin in this framework to be
removed.

## License

This framework is licensed under the
[LGPLv2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html#SEC1).

The CMake code in this framework is licensed under the
[BSD license](http://opensource.org/licenses/BSD-3-Clause).

