/*
    xcf.cpp: A Qt 5 plug-in for reading GIMP XCF image files
    SPDX-FileCopyrightText: 2001 lignum Computing Inc. <allen@lignumcomputing.com>
    SPDX-FileCopyrightText: 2004 Melchior FRANZ <mfranz@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "util_p.h"
#include "xcf_p.h"

#include <QColorSpace>
#include <QDebug>
#include <QIODevice>
#include <QImage>
#include <QImageReader>
#include <QList>
#include <QLoggingCategory>
#include <QPainter>
#include <QStack>
#include <QtEndian>

#ifndef XCF_QT5_SUPPORT
// Float images are not supported by Qt 5 and can be disabled in QT 6 to reduce memory usage.
// Unfortunately enabling/disabling this define results in slightly different images, so leave the default if possible.
#define USE_FLOAT_IMAGES // default uncommented

// Let's set a "reasonable" maximum size
#define MAX_IMAGE_WIDTH 300000
#define MAX_IMAGE_HEIGHT 300000
#else
// While it is possible to have images larger than 32767 pixels, QPainter seems unable to go beyond this threshold using Qt 5.
#define MAX_IMAGE_WIDTH 32767
#define MAX_IMAGE_HEIGHT 32767
#endif

#ifdef USE_FLOAT_IMAGES
#include <qrgbafloat.h>
#endif

#include <stdlib.h>
#include <string.h>

#include "gimp_p.h"

Q_DECLARE_LOGGING_CATEGORY(XCFPLUGIN)
Q_LOGGING_CATEGORY(XCFPLUGIN, "kf.imageformats.plugins.xcf", QtWarningMsg)

//#define DISABLE_TILE_PROFILE // default commented (comment to use the conversion as intended by Martin)
#define DISABLE_IMAGE_PROFILE // default uncommented (comment to use the conversion as intended by Martin)
#define DISABLE_TILE_PROFILE_CONV // default uncommented (comment to use the conversion as intended by Martin)
#define DISABLE_IMAGE_PROFILE_CONV // default uncommented (comment to use the conversion as intended by Martin)

const float INCHESPERMETER = (100.0f / 2.54f);

namespace
{
struct RandomTable {
    // From glibc
    static constexpr int rand_r(unsigned int *seed)
    {
        unsigned int next = *seed;
        int result = 0;

        next *= 1103515245;
        next += 12345;
        result = (unsigned int)(next / 65536) % 2048;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        next *= 1103515245;
        next += 12345;
        result <<= 10;
        result ^= (unsigned int)(next / 65536) % 1024;

        *seed = next;

        return result;
    }

    constexpr RandomTable()
        : values{}
    {
        unsigned int next = RANDOM_SEED;

        for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
            values[i] = rand_r(&next);
        }

        for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
            int tmp{};
            int swap = i + rand_r(&next) % (RANDOM_TABLE_SIZE - i);
            tmp = values[i];
            values[i] = values[swap];
            values[swap] = tmp;
        }
    }

    int values[RANDOM_TABLE_SIZE]{};
};
} // namespace {

/*!
 * Each layer in an XCF file is stored as a matrix of
 * 64-pixel by 64-pixel images. The GIMP has a sophisticated
 * method of handling very large images as well as implementing
 * parallel processing on a tile-by-tile basis. Here, though,
 * we just read them in en-masse and store them in a matrix.
 */
typedef QList<QList<QImage>> Tiles;

class XCFImageFormat
{
    Q_GADGET
public:
    //! Properties which can be stored in an XCF file.
    enum PropType {
        PROP_END = 0,
        PROP_COLORMAP = 1,
        PROP_ACTIVE_LAYER = 2,
        PROP_ACTIVE_CHANNEL = 3,
        PROP_SELECTION = 4,
        PROP_FLOATING_SELECTION = 5,
        PROP_OPACITY = 6,
        PROP_MODE = 7,
        PROP_VISIBLE = 8,
        PROP_LINKED = 9,
        PROP_LOCK_ALPHA = 10,
        PROP_APPLY_MASK = 11,
        PROP_EDIT_MASK = 12,
        PROP_SHOW_MASK = 13,
        PROP_SHOW_MASKED = 14,
        PROP_OFFSETS = 15,
        PROP_COLOR = 16,
        PROP_COMPRESSION = 17,
        PROP_GUIDES = 18,
        PROP_RESOLUTION = 19,
        PROP_TATTOO = 20,
        PROP_PARASITES = 21,
        PROP_UNIT = 22,
        PROP_PATHS = 23,
        PROP_USER_UNIT = 24,
        PROP_VECTORS = 25,
        PROP_TEXT_LAYER_FLAGS = 26,
        PROP_OLD_SAMPLE_POINTS = 27,
        PROP_LOCK_CONTENT = 28,
        PROP_GROUP_ITEM = 29,
        PROP_ITEM_PATH = 30,
        PROP_GROUP_ITEM_FLAGS = 31,
        PROP_LOCK_POSITION = 32,
        PROP_FLOAT_OPACITY = 33,
        PROP_COLOR_TAG = 34,
        PROP_COMPOSITE_MODE = 35,
        PROP_COMPOSITE_SPACE = 36,
        PROP_BLEND_SPACE = 37,
        PROP_FLOAT_COLOR = 38,
        PROP_SAMPLE_POINTS = 39,
        MAX_SUPPORTED_PROPTYPE, // should always be at the end so its value is last + 1
    };
    Q_ENUM(PropType)

    //! Compression type used in layer tiles.
    enum XcfCompressionType : qint8 {
        COMPRESS_INVALID = -1, /* our own */
        COMPRESS_NONE = 0,
        COMPRESS_RLE = 1,
        COMPRESS_ZLIB = 2, /* unused */
        COMPRESS_FRACTAL = 3, /* unused */
    };
    Q_ENUM(XcfCompressionType)

    enum LayerModeType : quint32 {
        GIMP_LAYER_MODE_NORMAL_LEGACY,
        GIMP_LAYER_MODE_DISSOLVE,
        GIMP_LAYER_MODE_BEHIND_LEGACY,
        GIMP_LAYER_MODE_MULTIPLY_LEGACY,
        GIMP_LAYER_MODE_SCREEN_LEGACY,
        GIMP_LAYER_MODE_OVERLAY_LEGACY,
        GIMP_LAYER_MODE_DIFFERENCE_LEGACY,
        GIMP_LAYER_MODE_ADDITION_LEGACY,
        GIMP_LAYER_MODE_SUBTRACT_LEGACY,
        GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY,
        GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY,
        GIMP_LAYER_MODE_HSV_HUE_LEGACY,
        GIMP_LAYER_MODE_HSV_SATURATION_LEGACY,
        GIMP_LAYER_MODE_HSL_COLOR_LEGACY,
        GIMP_LAYER_MODE_HSV_VALUE_LEGACY,
        GIMP_LAYER_MODE_DIVIDE_LEGACY,
        GIMP_LAYER_MODE_DODGE_LEGACY,
        GIMP_LAYER_MODE_BURN_LEGACY,
        GIMP_LAYER_MODE_HARDLIGHT_LEGACY,
        GIMP_LAYER_MODE_SOFTLIGHT_LEGACY,
        GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY,
        GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY,
        GIMP_LAYER_MODE_COLOR_ERASE_LEGACY,
        GIMP_LAYER_MODE_OVERLAY,
        GIMP_LAYER_MODE_LCH_HUE,
        GIMP_LAYER_MODE_LCH_CHROMA,
        GIMP_LAYER_MODE_LCH_COLOR,
        GIMP_LAYER_MODE_LCH_LIGHTNESS,
        GIMP_LAYER_MODE_NORMAL,
        GIMP_LAYER_MODE_BEHIND,
        GIMP_LAYER_MODE_MULTIPLY,
        GIMP_LAYER_MODE_SCREEN,
        GIMP_LAYER_MODE_DIFFERENCE,
        GIMP_LAYER_MODE_ADDITION,
        GIMP_LAYER_MODE_SUBTRACT,
        GIMP_LAYER_MODE_DARKEN_ONLY,
        GIMP_LAYER_MODE_LIGHTEN_ONLY,
        GIMP_LAYER_MODE_HSV_HUE,
        GIMP_LAYER_MODE_HSV_SATURATION,
        GIMP_LAYER_MODE_HSL_COLOR,
        GIMP_LAYER_MODE_HSV_VALUE,
        GIMP_LAYER_MODE_DIVIDE,
        GIMP_LAYER_MODE_DODGE,
        GIMP_LAYER_MODE_BURN,
        GIMP_LAYER_MODE_HARDLIGHT,
        GIMP_LAYER_MODE_SOFTLIGHT,
        GIMP_LAYER_MODE_GRAIN_EXTRACT,
        GIMP_LAYER_MODE_GRAIN_MERGE,
        GIMP_LAYER_MODE_VIVID_LIGHT,
        GIMP_LAYER_MODE_PIN_LIGHT,
        GIMP_LAYER_MODE_LINEAR_LIGHT,
        GIMP_LAYER_MODE_HARD_MIX,
        GIMP_LAYER_MODE_EXCLUSION,
        GIMP_LAYER_MODE_LINEAR_BURN,
        GIMP_LAYER_MODE_LUMA_DARKEN_ONLY,
        GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY,
        GIMP_LAYER_MODE_LUMINANCE,
        GIMP_LAYER_MODE_COLOR_ERASE,
        GIMP_LAYER_MODE_ERASE,
        GIMP_LAYER_MODE_MERGE,
        GIMP_LAYER_MODE_SPLIT,
        GIMP_LAYER_MODE_PASS_THROUGH,
        GIMP_LAYER_MODE_COUNT,
    };
    Q_ENUM(LayerModeType)

    //! Type of individual layers in an XCF file.
    enum GimpImageType : qint32 {
        RGB_GIMAGE,
        RGBA_GIMAGE,
        GRAY_GIMAGE,
        GRAYA_GIMAGE,
        INDEXED_GIMAGE,
        INDEXEDA_GIMAGE,
    };
    Q_ENUM(GimpImageType)

    //! Type of individual layers in an XCF file.
    enum GimpColorSpace : qint32 {
        AutoColorSpace,
        RgbLinearSpace,
        RgbPerceptualSpace,
        LabSpace,
    };
    Q_ENUM(GimpColorSpace);

    //! Mode to use when compositing layer
    enum GimpCompositeMode : qint32 {
        CompositeAuto,
        CompositeUnion,
        CompositeClipBackdrop,
        CompositeClipLayer,
        CompositeIntersect,
    };
    Q_ENUM(GimpCompositeMode);

    enum GimpPrecision : qint32 {
        GIMP_PRECISION_U8_LINEAR = 100, /*< desc="8-bit linear integer"         >*/
        GIMP_PRECISION_U8_NON_LINEAR = 150, /*< desc="8-bit non-linear integer"          >*/
        GIMP_PRECISION_U8_PERCEPTUAL = 175, /*< desc="8-bit perceptual integer"          >*/
        GIMP_PRECISION_U16_LINEAR = 200, /*< desc="16-bit linear integer"        >*/
        GIMP_PRECISION_U16_NON_LINEAR = 250, /*< desc="16-bit non-linear integer"         >*/
        GIMP_PRECISION_U16_PERCEPTUAL = 275, /*< desc="16-bit perceptual integer"         >*/
        GIMP_PRECISION_U32_LINEAR = 300, /*< desc="32-bit linear integer"        >*/
        GIMP_PRECISION_U32_NON_LINEAR = 350, /*< desc="32-bit non-linear integer"         >*/
        GIMP_PRECISION_U32_PERCEPTUAL = 375, /*< desc="32-bit perceptual integer"         >*/
        GIMP_PRECISION_HALF_LINEAR = 500, /*< desc="16-bit linear floating point" >*/
        GIMP_PRECISION_HALF_NON_LINEAR = 550, /*< desc="16-bit non-linear floating point"  >*/
        GIMP_PRECISION_HALF_PERCEPTUAL = 575, /*< desc="16-bit perceptual floating point"  >*/
        GIMP_PRECISION_FLOAT_LINEAR = 600, /*< desc="32-bit linear floating point" >*/
        GIMP_PRECISION_FLOAT_NON_LINEAR = 650, /*< desc="32-bit non-linear floating point"  >*/
        GIMP_PRECISION_FLOAT_PERCEPTUAL = 675, /*< desc="32-bit perceptual floating point"  >*/
        GIMP_PRECISION_DOUBLE_LINEAR = 700, /*< desc="64-bit linear floating point" >*/
        GIMP_PRECISION_DOUBLE_NON_LINEAR = 750, /*< desc="64-bit non-linear floating point"  >*/
        GIMP_PRECISION_DOUBLE_PERCEPTUAL = 775, /*< desc="64-bit perceptual floating point"  >*/
    };
    Q_ENUM(GimpPrecision);

    XCFImageFormat();
    bool readXCF(QIODevice *device, QImage *image);

    /*!
     * Each GIMP image is composed of one or more layers. A layer can
     * be one of any three basic types: RGB, grayscale or indexed. With an
     * optional alpha channel, there are six possible types altogether.
     *
     * Note: there is only ever one instance of this structure. The
     * layer info is discarded after it is merged into the final QImage.
     */
    class Layer
    {
    public:
        quint32 width; //!< Width of the layer
        quint32 height; //!< Height of the layer
        GimpImageType type; //!< Type of the layer (GimpImageType)
        char *name; //!< Name of the layer
        qint64 hierarchy_offset; //!< File position of Tile hierarchy
        qint64 mask_offset; //!< File position of mask image

        uint nrows; //!< Number of rows of tiles (y direction)
        uint ncols; //!< Number of columns of tiles (x direction)

        Tiles image_tiles; //!< The basic image
        //! For Grayscale and Indexed images, the alpha channel is stored
        //! separately (in this data structure, anyway).
        Tiles alpha_tiles;
        Tiles mask_tiles; //!< The layer mask (optional)

        //! Additional information about a layer mask.
        struct {
            quint32 opacity;
            float opacityFloat = 1.f;
            quint32 visible;
            quint32 show_masked;
            uchar red, green, blue;
            float redF, greenF, blueF; // Floats should override
            quint32 tattoo;
        } mask_channel;

        XcfCompressionType compression = COMPRESS_INVALID; //!< tile compression method (CompressionType)

        bool active; //!< Is this layer the active layer?
        quint32 opacity = 255; //!< The opacity of the layer
        float opacityFloat = 1.f; //!< The opacity of the layer, but floating point (both are set)
        quint32 visible = 1; //!< Is the layer visible?
        quint32 linked; //!< Is this layer linked (geometrically)
        quint32 preserve_transparency; //!< Preserve alpha when drawing on layer?
        quint32 apply_mask = 9; //!< Apply the layer mask? Use 9 as "uninitilized". Spec says "If the property does not appear for a layer which has a layer
                                //!< mask, it defaults to true (1).
                                //   Robust readers should force this to false if the layer has no layer mask.
        quint32 edit_mask; //!< Is the layer mask the being edited?
        quint32 show_mask; //!< Show the layer mask rather than the image?
        qint32 x_offset = 0; //!< x offset of the layer relative to the image
        qint32 y_offset = 0; //!< y offset of the layer relative to the image
        LayerModeType mode = GIMP_LAYER_MODE_NORMAL_LEGACY; //!< Combining mode of layer (LayerModeEffects)
        quint32 tattoo; //!< (unique identifier?)
        GimpColorSpace blendSpace = RgbLinearSpace; //!< What colorspace to use when blending
        GimpColorSpace compositeSpace = RgbLinearSpace; //!< What colorspace to use when compositing
        GimpCompositeMode compositeMode = CompositeUnion; //!< How to composite layer (union, clip, etc.)

        //! As each tile is read from the file, it is buffered here.
#ifdef USE_FLOAT_IMAGES
        uchar tile[quint64(TILE_WIDTH * TILE_HEIGHT * sizeof(QRgbaFloat32) * 1.5)];
#else
        uchar tile[quint64(TILE_WIDTH * TILE_HEIGHT * sizeof(QRgba64) * 1.5)];
#endif

        //! The data from tile buffer is copied to the Tile by this
        //! method.  Depending on the type of the tile (RGB, Grayscale,
        //! Indexed) and use (image or mask), the bytes in the buffer are
        //! copied in different ways.
        bool (*assignBytes)(Layer &layer, uint i, uint j, const GimpPrecision &precision);

        Layer(void)
            : name(nullptr)
        {
        }
        ~Layer(void)
        {
            delete[] name;
        }

        Layer(const Layer &) = delete;
        Layer &operator=(const Layer &) = delete;

        QImage::Format qimageFormat(const GimpPrecision precision, uint num_colors = 0, bool legacyMode = false) const
        {
            int bpc = bytesPerChannel(precision);
#ifdef USE_FLOAT_IMAGES
            bool float16 = !legacyMode && precision >= GIMP_PRECISION_HALF_LINEAR && precision <= GIMP_PRECISION_HALF_PERCEPTUAL;
            bool float32 = !legacyMode && precision >= GIMP_PRECISION_FLOAT_LINEAR && precision <= GIMP_PRECISION_FLOAT_PERCEPTUAL;
#endif

            if (legacyMode) {
                bpc = std::min(bpc, 1);
            }

            switch (type) {
            case RGB_GIMAGE:
                if (opacity == OPAQUE_OPACITY) {
#ifdef USE_FLOAT_IMAGES
                    if (float16) {
                        return QImage::Format_RGBX16FPx4;
                    }
                    if (float32) {
                        return QImage::QImage::Format_RGBX32FPx4;
                    }
#endif

                    if (bpc == 1) {
                        return QImage::Format_RGBX8888;
                    } else if (bpc == 2 || bpc == 4) {
                        return QImage::Format_RGBX64;
                    } else {
                        qCDebug(XCFPLUGIN) << "Layer has invalid bpc" << bpc << precision;
                        return QImage::Format_Invalid;
                    }
                }
                Q_FALLTHROUGH();
            case RGBA_GIMAGE:
#ifdef USE_FLOAT_IMAGES
                if (float16) {
                    return QImage::Format_RGBA16FPx4;
                }
                if (float32) {
                    return QImage::QImage::Format_RGBA32FPx4;
                }
#endif
                if (bpc == 1) {
                    return QImage::Format_RGBA8888;
                } else if (bpc == 2 || bpc == 4) {
                    return QImage::Format_RGBA64;
                } else {
                    qCDebug(XCFPLUGIN) << "Layer has invalid bpc" << bpc;
                    return QImage::Format_Invalid;
                }
                break;

            case GRAY_GIMAGE:
                if (opacity == OPAQUE_OPACITY) {
                    return QImage::Format_Indexed8;
                } // else, fall through to 32-bit representation
                Q_FALLTHROUGH();
            case GRAYA_GIMAGE:
                return QImage::Format_RGBA8888;
                break;

            case INDEXED_GIMAGE:
                // As noted in the table above, there are quite a few combinations
                // which are possible with indexed images, depending on the
                // presence of transparency (note: not translucency, which is not
                // supported by The GIMP for indexed images) and the number of
                // individual colors.

                // Note: Qt treats a bitmap with a Black and White color palette
                // as a mask, so only the "on" bits are drawn, regardless of the
                // order color table entries. Otherwise (i.e., at least one of the
                // color table entries is not black or white), it obeys the one-
                // or two-color palette. Have to ask about this...

                if (num_colors == 1 || num_colors == 2) {
                    return QImage::Format_MonoLSB;
                } else {
                    return QImage::Format_Indexed8;
                }
                break;

            case INDEXEDA_GIMAGE:
                if (num_colors == 1) {
                    return QImage::Format_MonoLSB;
                } else {
                    return QImage::Format_Indexed8;
                }
            }
            qCWarning(XCFPLUGIN) << "Unhandled layer mode" << XCFImageFormat::LayerModeType(type);
            return QImage::Format_Invalid;
        }
    };

    /*!
     * The in-memory representation of the XCF Image. It contains a few
     * metadata items, but is mostly a container for the layer information.
     */
    class XCFImage
    {
    public:
        struct Header {
            GimpPrecision precision = GIMP_PRECISION_U8_LINEAR; //!< Default precision (GimpPrecision)
            quint32 width; //!< width of the XCF image
            quint32 height; //!< height of the XCF image
            qint32 type; //!< type of the XCF image (GimpImageBaseType)
        } header;

        XcfCompressionType compression = COMPRESS_RLE; //!< tile compression method (CompressionType)
        float x_resolution = -1; //!< x resolution in dots per inch
        float y_resolution = -1; //!< y resolution in dots per inch
        qint32 tattoo; //!< (unique identifier?)
        quint32 unit; //!< Units of The GIMP (inch, mm, pica, etc...)
        qint32 num_colors = 0; //!< number of colors in an indexed image
        QList<QRgb> palette; //!< indexed image color palette

        int num_layers; //!< number of layers
        Layer layer; //!< most recently read layer

        bool initialized; //!< Is the QImage initialized?
        QImage image; //!< final QImage

        QHash<QString,QByteArray> parasites;    //!< parasites data

        XCFImage(void)
            : initialized(false)
        {
        }

        QImage::Format qimageFormat() const
        {
            return layer.qimageFormat(header.precision, num_colors, true);
        }

        uint bytesPerChannel() const
        {
            return XCFImageFormat::bytesPerChannel(header.precision);
        }
    };

private:
    static qint64 readOffsetPtr(QDataStream &stream)
    {
        if (stream.version() >= 11) {
            qint64 ret;
            stream >> ret;
            return ret;
        } else {
            quint32 ret;
            stream >> ret;
            return ret;
        }
    }

    //! In layer DISSOLVE mode, a random number is chosen to compare to a
    //! pixel's alpha. If the alpha is greater than the random number, the
    //! pixel is drawn. This table merely contains the random number seeds
    //! for each ROW of an image. Therefore, the random numbers chosen
    //! are consistent from run to run.
    static int random_table[RANDOM_TABLE_SIZE];
    static bool random_table_initialized;

    static constexpr RandomTable randomTable{};

    //! This table is used as a shared grayscale ramp to be set on grayscale
    //! images. This is because Qt does not differentiate between indexed and
    //! grayscale images.
    static QList<QRgb> grayTable;

    //! This table provides the add_pixel saturation values (i.e. 250 + 250 = 255).
    // static int add_lut[256][256]; - this is so lame waste of 256k of memory
    static int add_lut(int, int);

    //! The bottom-most layer is copied into the final QImage by this
    //! routine.
    typedef void (*PixelCopyOperation)(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    //! Higher layers are merged into the final QImage by this routine.
    typedef bool (*PixelMergeOperation)(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static bool modeAffectsSourceAlpha(const quint32 type);

    bool loadImageProperties(QDataStream &xcf_io, XCFImage &image);
    bool loadProperty(QDataStream &xcf_io, PropType &type, QByteArray &bytes, quint32 &rawType);
    bool loadLayer(QDataStream &xcf_io, XCFImage &xcf_image);
    bool loadLayerProperties(QDataStream &xcf_io, Layer &layer);
    bool composeTiles(XCFImage &xcf_image);
    void setGrayPalette(QImage &image);
    void setPalette(XCFImage &xcf_image, QImage &image);
    void setImageParasites(const XCFImage &xcf_image, QImage &image);
    static bool assignImageBytes(Layer &layer, uint i, uint j, const GimpPrecision &precision);
    bool loadHierarchy(QDataStream &xcf_io, Layer &layer, const GimpPrecision precision);
    bool loadLevel(QDataStream &xcf_io, Layer &layer, qint32 bpp, const GimpPrecision precision);
    static bool assignMaskBytes(Layer &layer, uint i, uint j, const GimpPrecision &precision);
    bool loadMask(QDataStream &xcf_io, Layer &layer, const GimpPrecision precision);
    bool loadChannelProperties(QDataStream &xcf_io, Layer &layer);
    bool initializeImage(XCFImage &xcf_image);
    bool loadTileRLE(QDataStream &xcf_io, uchar *tile, int size, int data_length, qint32 bpp, qint64 *bytesParsed);

    static void copyLayerToImage(XCFImage &xcf_image);
    static void copyRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static void copyIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static void mergeLayerIntoImage(XCFImage &xcf_image);
    static bool mergeRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeGrayAToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);
    static bool mergeIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n);

    static void initializeRandomTable();
    static void dissolveRGBPixels(QImage &image, int x, int y);
    static void dissolveAlphaPixels(QImage &image, int x, int y);

    static uint bytesPerChannel(const GimpPrecision precision)
    {
        switch (precision) {
        case GIMP_PRECISION_U8_LINEAR:
        case GIMP_PRECISION_U8_NON_LINEAR:
        case GIMP_PRECISION_U8_PERCEPTUAL:
            return 1;
            break;
        case GIMP_PRECISION_U16_LINEAR:
        case GIMP_PRECISION_U16_NON_LINEAR:
        case GIMP_PRECISION_U16_PERCEPTUAL:
        case GIMP_PRECISION_HALF_LINEAR:
        case GIMP_PRECISION_HALF_NON_LINEAR:
        case GIMP_PRECISION_HALF_PERCEPTUAL:
            return 2;
            break;

        case GIMP_PRECISION_U32_LINEAR:
        case GIMP_PRECISION_U32_NON_LINEAR:
        case GIMP_PRECISION_U32_PERCEPTUAL:
        case GIMP_PRECISION_FLOAT_LINEAR:
        case GIMP_PRECISION_FLOAT_NON_LINEAR:
        case GIMP_PRECISION_FLOAT_PERCEPTUAL:
            return 4;
            break;
        case GIMP_PRECISION_DOUBLE_LINEAR:
        case GIMP_PRECISION_DOUBLE_NON_LINEAR:
        case GIMP_PRECISION_DOUBLE_PERCEPTUAL:
            return 8;
            break;

        default:
            qCDebug(XCFPLUGIN) << "Layer has invalid precision" << precision;
            return 0;
        }
    }

public:
    static bool readXCFHeader(QDataStream &ds, XCFImage::Header *header);
};

int XCFImageFormat::random_table[RANDOM_TABLE_SIZE];
bool XCFImageFormat::random_table_initialized;

constexpr RandomTable XCFImageFormat::randomTable;

QList<QRgb> XCFImageFormat::grayTable;

bool XCFImageFormat::modeAffectsSourceAlpha(const quint32 type)
{
    switch (type) {
    case GIMP_LAYER_MODE_NORMAL_LEGACY:
    case GIMP_LAYER_MODE_DISSOLVE:
    case GIMP_LAYER_MODE_BEHIND_LEGACY:
        return true;

    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
    case GIMP_LAYER_MODE_HSL_COLOR_LEGACY:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
    case GIMP_LAYER_MODE_DODGE_LEGACY:
    case GIMP_LAYER_MODE_BURN_LEGACY:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
        return false;

    case GIMP_LAYER_MODE_COLOR_ERASE_LEGACY:
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_LCH_HUE:
    case GIMP_LAYER_MODE_LCH_CHROMA:
    case GIMP_LAYER_MODE_LCH_COLOR:
    case GIMP_LAYER_MODE_LCH_LIGHTNESS:
        return false;

    case GIMP_LAYER_MODE_NORMAL:
        return true;

    case GIMP_LAYER_MODE_BEHIND:
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_HSV_HUE:
    case GIMP_LAYER_MODE_HSV_SATURATION:
    case GIMP_LAYER_MODE_HSL_COLOR:
    case GIMP_LAYER_MODE_HSV_VALUE:
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_VIVID_LIGHT:
    case GIMP_LAYER_MODE_PIN_LIGHT:
    case GIMP_LAYER_MODE_LINEAR_LIGHT:
    case GIMP_LAYER_MODE_HARD_MIX:
    case GIMP_LAYER_MODE_EXCLUSION:
    case GIMP_LAYER_MODE_LINEAR_BURN:
    case GIMP_LAYER_MODE_LUMA_DARKEN_ONLY:
    case GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LUMINANCE:
    case GIMP_LAYER_MODE_COLOR_ERASE:
    case GIMP_LAYER_MODE_ERASE:
    case GIMP_LAYER_MODE_MERGE:
    case GIMP_LAYER_MODE_SPLIT:
    case GIMP_LAYER_MODE_PASS_THROUGH:
        return false;

    default:
        qCWarning(XCFPLUGIN) << "Unhandled layer mode" << XCFImageFormat::LayerModeType(type);
        return false;
    }
}

//! Change a QRgb value's alpha only.
inline QRgb qRgba(const QRgb rgb, int a)
{
    return ((a & 0xff) << 24 | (rgb & RGB_MASK));
}

/*!
 * The constructor for the XCF image loader.
 */
XCFImageFormat::XCFImageFormat()
{
    static_assert(sizeof(QRgb) == 4, "the code assumes sizeof(QRgb) == 4, if that's not your case, help us fix it :)");
}

/*!
 * This initializes the tables used in the layer dissolving routines.
 */
void XCFImageFormat::initializeRandomTable()
{
    // From GIMP "paint_funcs.c" v1.2
    srand(RANDOM_SEED);

    for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
        random_table[i] = rand();
    }

    for (int i = 0; i < RANDOM_TABLE_SIZE; i++) {
        int tmp;
        int swap = i + rand() % (RANDOM_TABLE_SIZE - i);
        tmp = random_table[i];
        random_table[i] = random_table[swap];
        random_table[swap] = tmp;
    }
}

inline int XCFImageFormat::add_lut(int a, int b)
{
    return qMin(a + b, 255);
}

bool XCFImageFormat::readXCFHeader(QDataStream &xcf_io, XCFImage::Header *header)
{
    QByteArray tag(14, '\0');

    if (xcf_io.readRawData(tag.data(), tag.size()) != tag.size()) {
        qCDebug(XCFPLUGIN) << "XCF: read failure on header tag";
        return false;
    }
    if (!tag.startsWith("gimp xcf") || !tag.endsWith('\0')) {
        qCDebug(XCFPLUGIN) << "XCF: read called on non-XCF file";
        return false;
    }

    // Remove null terminator
    tag.chop(1);

    if (tag.right(4) == "file") {
        xcf_io.setVersion(0);
    } else {
        // Version 1 and onwards use the format "gimp xcf v###" instead of "gimp xcf file"
        bool ok;
        xcf_io.setVersion(tag.right(3).toInt(&ok));
        if (!ok) {
            qCDebug(XCFPLUGIN) << "Failed to parse version" << tag;
            return false;
        }
    }
    qCDebug(XCFPLUGIN) << "version" << xcf_io.version();

    if (xcf_io.version() > 12) {
        qCDebug(XCFPLUGIN) << "Unsupported version" << xcf_io.version();
        return false;
    }

    xcf_io >> header->width >> header->height >> header->type;

    if (xcf_io.version() >= 4) {
        int precision;
        xcf_io >> precision;
        qCDebug(XCFPLUGIN) << "Precision" << GimpPrecision(precision);
        if (xcf_io.version() < 7) {
            switch (precision) {
            case 0:
                precision = GIMP_PRECISION_U8_NON_LINEAR;
                break;
            case 1:
                precision = GIMP_PRECISION_U16_NON_LINEAR;
                break;
            case 2:
                precision = GIMP_PRECISION_U32_LINEAR;
                break;
            case 3:
                precision = GIMP_PRECISION_HALF_LINEAR;
                break;
            case 4:
                precision = GIMP_PRECISION_FLOAT_LINEAR;
                break;
            default:
                if (precision < GIMP_PRECISION_U8_LINEAR) {
                    qCWarning(XCFPLUGIN) << "Invalid precision read" << precision;
                    return false;
                } else {
                    qCDebug(XCFPLUGIN) << "Unexpected precision" << precision << "in version" << xcf_io.version();
                }
            }
        }
        header->precision = GimpPrecision(precision);
    }
    qCDebug(XCFPLUGIN) << "tag:" << tag << " height: " << header->width << " width: " << header->height << " type: " << header->type;

    if ((sizeof(void *) == 4 && qint64(header->width) * header->height > 16384 * 16384)) {
        qCWarning(XCFPLUGIN) << "On 32-bits programs the maximum image size is limited to" << 16384 << "x" << 16384 << "px";
        return false;
    }

    if (header->width > MAX_IMAGE_WIDTH || header->height > MAX_IMAGE_HEIGHT) {
        qCWarning(XCFPLUGIN) << "The maximum image size is limited to" << MAX_IMAGE_WIDTH << "x" << MAX_IMAGE_HEIGHT << "px";
        return false;
    }

    return true;
}

bool XCFImageFormat::readXCF(QIODevice *device, QImage *outImage)
{
    XCFImage xcf_image;
    QDataStream xcf_io(device);

    if (!readXCFHeader(xcf_io, &xcf_image.header)) {
        return false;
    }

    if (!loadImageProperties(xcf_io, xcf_image)) {
        return false;
    }

    // The layers appear to be stored in top-to-bottom order. This is
    // the reverse of how a merged image must be computed. So, the layer
    // offsets are pushed onto a LIFO stack (thus, we don't have to load
    // all the data of all layers before beginning to construct the
    // merged image).

    QStack<qint64> layer_offsets;

    while (true) {
        const qint64 layer_offset = readOffsetPtr(xcf_io);

        if (layer_offset == 0) {
            break;
        }

        if (layer_offset < 0) {
            qCDebug(XCFPLUGIN) << "XCF: negative layer offset";
            return false;
        }

        layer_offsets.push(layer_offset);
    }

    xcf_image.num_layers = layer_offsets.size();

    if (layer_offsets.size() == 0) {
        qCDebug(XCFPLUGIN) << "XCF: no layers!";
        return false;
    }
    qCDebug(XCFPLUGIN) << xcf_image.num_layers << "layers";

    // Load each layer and add it to the image
    while (!layer_offsets.isEmpty()) {
        qint64 layer_offset = layer_offsets.pop();

        xcf_io.device()->seek(layer_offset);

        if (!loadLayer(xcf_io, xcf_image)) {
            return false;
        }
    }

    if (!xcf_image.initialized) {
        qCDebug(XCFPLUGIN) << "XCF: no visible layers!";
        return false;
    }

    // The image was created: now I can set metadata and ICC color profile inside it.
    setImageParasites(xcf_image, xcf_image.image);

    *outImage = xcf_image.image;
    return true;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with the image (and layer and mask).
 * \param xcf_io the data stream connected to the XCF image
 * \param xcf_image XCF image data.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadImageProperties(QDataStream &xcf_io, XCFImage &xcf_image)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading global image properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_COMPRESSION:
            property >> xcf_image.compression;
            break;

        case PROP_RESOLUTION:
            property.setFloatingPointPrecision(QDataStream::SinglePrecision);
            property >> xcf_image.x_resolution >> xcf_image.y_resolution;
            break;

        case PROP_TATTOO:
            property >> xcf_image.tattoo;
            break;

        case PROP_PARASITES:
            while (!property.atEnd()) {
                char *tag;
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
                quint32 size;
#else
                qint64 size;
#endif

                property.readBytes(tag, size);

                quint32 flags;
                QByteArray data;
                property >> flags >> data;

                // WARNING: you cannot add metadata to QImage here because it can be null.
                // Adding a metadata to a QImage when it is null, does nothing (metas are lost).
                if (tag) // store metadata for future use
                    xcf_image.parasites.insert(QString::fromUtf8(tag), data);

                delete[] tag;
            }
            break;

        case PROP_UNIT:
            property >> xcf_image.unit;
            break;

        case PROP_PATHS: // This property is ignored.
            break;

        case PROP_USER_UNIT: // This property is ignored.
            break;

        case PROP_COLORMAP:
            property >> xcf_image.num_colors;
            if (xcf_image.num_colors < 0 || xcf_image.num_colors > 65535) {
                return false;
            }

            xcf_image.palette = QList<QRgb>();
            xcf_image.palette.reserve(xcf_image.num_colors);

            for (int i = 0; i < xcf_image.num_colors; i++) {
                uchar r;
                uchar g;
                uchar b;
                property >> r >> g >> b;
                xcf_image.palette.push_back(qRgb(r, g, b));
            }
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented image property" << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Read a single property from the image file. The property type is returned
 * in type and the data is returned in bytes.
 * \param xcf the image file data stream.
 * \param type returns with the property type.
 * \param bytes returns with the property data.
 * \return true if there were no IO errors.  */
bool XCFImageFormat::loadProperty(QDataStream &xcf_io, PropType &type, QByteArray &bytes, quint32 &rawType)
{
    quint32 size;

    xcf_io >> rawType;
    if (rawType >= MAX_SUPPORTED_PROPTYPE) {
        type = MAX_SUPPORTED_PROPTYPE;
        // we don't support the property, but we still need to read from the device, assume it's like all the
        // non custom properties that is data_length + data
        xcf_io >> size;
        xcf_io.skipRawData(size);
        // return true because we don't really want to totally fail on an unsupported property since it may not be fatal
        return true;
    }

    type = PropType(rawType);

    char *data = nullptr;

    // The colormap property size is not the correct number of bytes:
    // The GIMP source xcf.c has size = 4 + ncolors, but it should be
    // 4 + 3 * ncolors

    if (type == PROP_COLORMAP) {
        xcf_io >> size;
        quint32 ncolors;
        xcf_io >> ncolors;

        size = 3 * ncolors + 4;

        if (size > 65535 || size < 4) {
            return false;
        }

        data = new char[size];

        // since we already read "ncolors" from the stream, we put that data back
        data[0] = 0;
        data[1] = 0;
        data[2] = ncolors >> 8;
        data[3] = ncolors & 255;

        // ... and read the remaining bytes from the stream
        xcf_io.readRawData(data + 4, size - 4);
    } else if (type == PROP_USER_UNIT) {
        // The USER UNIT property size is not correct. I'm not sure why, though.
        float factor;
        qint32 digits;

        xcf_io >> size >> factor >> digits;

        for (int i = 0; i < 5; i++) {
            char *unit_strings;

            xcf_io >> unit_strings;

            delete[] unit_strings;

            if (xcf_io.device()->atEnd()) {
                qCDebug(XCFPLUGIN) << "XCF: read failure on property " << type;
                return false;
            }
        }

        size = 0;
    } else {
        xcf_io >> size;
        if (size > 256000 * 4) {
            // NOTE: I didn't find any reference to maximum property dimensions in the specs, so I assume it's just a sanity check.
            qCDebug(XCFPLUGIN) << "XCF: loadProperty skips" << type << "due to size being too large";
            return false;
        }
        data = new char[size];
        const quint32 dataRead = xcf_io.readRawData(data, size);
        if (dataRead < size) {
            qCDebug(XCFPLUGIN) << "XCF: loadProperty read less data than expected" << size << dataRead;
            memset(&data[dataRead], 0, size - dataRead);
        }
    }

    if (size != 0 && data) {
        bytes = QByteArray(data, size);
    }

    delete[] data;

    return true;
}

/*!
 * Load a layer from the XCF file. The data stream must be positioned at
 * the beginning of the layer data.
 * \param xcf_io the image file data stream.
 * \param xcf_image contains the layer and the color table
 * (if the image is indexed).
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadLayer(QDataStream &xcf_io, XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    delete[] layer.name;

    xcf_io >> layer.width >> layer.height >> layer.type >> layer.name;

    // Don't want to keep passing this around, dumb XCF format
    layer.compression = XcfCompressionType(xcf_image.compression);

    if (!loadLayerProperties(xcf_io, layer)) {
        return false;
    }

    qCDebug(XCFPLUGIN) << "layer: \"" << layer.name << "\", size: " << layer.width << " x " << layer.height << ", type: " << layer.type
                       << ", mode: " << layer.mode << ", opacity: " << layer.opacity << ", visible: " << layer.visible << ", offset: " << layer.x_offset << ", "
                       << layer.y_offset << ", compression" << layer.compression;

    // Skip reading the rest of it if it is not visible. Typically, when
    // you export an image from the The GIMP it flattens (or merges) only
    // the visible layers into the output image.

    if (layer.visible == 0) {
        return true;
    }

    // If there are any more layers, merge them into the final QImage.

    layer.hierarchy_offset = readOffsetPtr(xcf_io);
    layer.mask_offset = readOffsetPtr(xcf_io);

    if (layer.hierarchy_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative layer hierarchy_offset";
        return false;
    }

    if (layer.mask_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative layer mask_offset";
        return false;
    }

    // Allocate the individual tile QImages based on the size and type
    // of this layer.

    if (!composeTiles(xcf_image)) {
        return false;
    }
    xcf_io.device()->seek(layer.hierarchy_offset);

    // As tiles are loaded, they are copied into the layers tiles by
    // this routine. (loadMask(), below, uses a slightly different
    // version of assignBytes().)

    layer.assignBytes = assignImageBytes;

    if (!loadHierarchy(xcf_io, layer, xcf_image.header.precision)) {
        return false;
    }

    if (layer.mask_offset != 0) {
        // 9 means its not on the file. Spec says "If the property does not appear for a layer which has a layer mask, it defaults to true (1).
        if (layer.apply_mask == 9) {
            layer.apply_mask = 1;
        }

        xcf_io.device()->seek(layer.mask_offset);

        if (!loadMask(xcf_io, layer, xcf_image.header.precision)) {
            return false;
        }
    } else {
        // Spec says "Robust readers should force this to false if the layer has no layer mask."
        layer.apply_mask = 0;
    }

    // Now we should have enough information to initialize the final
    // QImage. The first visible layer determines the attributes
    // of the QImage.

    if (!xcf_image.initialized) {
        if (!initializeImage(xcf_image)) {
            return false;
        }
        copyLayerToImage(xcf_image);
        xcf_image.initialized = true;
    } else {
        const QColorSpace colorspaceBefore = xcf_image.image.colorSpace();
        mergeLayerIntoImage(xcf_image);
        if (xcf_image.image.colorSpace() != colorspaceBefore) {
            qCDebug(XCFPLUGIN) << "Converting color space back to" << colorspaceBefore << "after layer composition";
            xcf_image.image.convertToColorSpace(colorspaceBefore);
        }
    }

    return true;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with a layer.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer layer to collect the properties.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadLayerProperties(QDataStream &xcf_io, Layer &layer)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading layer properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_ACTIVE_LAYER:
            layer.active = true;
            break;

        case PROP_OPACITY:
            property >> layer.opacity;
            layer.opacity = std::min(layer.opacity, 255u);
            break;

        case PROP_FLOAT_OPACITY:
            // For some reason QDataStream isn't able to read the float (tried
            // setting the endianness manually)
            if (bytes.size() == 4) {
                layer.opacityFloat = qFromBigEndian(*reinterpret_cast<float *>(bytes.data()));
            } else {
                qCDebug(XCFPLUGIN) << "XCF: Invalid data size for float:" << bytes.size();
            }
            break;

        case PROP_VISIBLE:
            property >> layer.visible;
            break;

        case PROP_LINKED:
            property >> layer.linked;
            break;

        case PROP_LOCK_ALPHA:
            property >> layer.preserve_transparency;
            break;

        case PROP_APPLY_MASK:
            property >> layer.apply_mask;
            break;

        case PROP_EDIT_MASK:
            property >> layer.edit_mask;
            break;

        case PROP_SHOW_MASK:
            property >> layer.show_mask;
            break;

        case PROP_OFFSETS:
            property >> layer.x_offset >> layer.y_offset;
            break;

        case PROP_MODE:
            property >> layer.mode;
            if (layer.mode >= GIMP_LAYER_MODE_COUNT) {
                qCDebug(XCFPLUGIN) << "Found layer with unsupported mode" << LayerModeType(layer.mode) << "Defaulting to mode 0";
                layer.mode = GIMP_LAYER_MODE_NORMAL_LEGACY;
            }
            break;

        case PROP_TATTOO:
            property >> layer.tattoo;
            break;

        case PROP_COMPOSITE_SPACE:
            property >> layer.compositeSpace;
            if (layer.compositeSpace < 0) {
                layer.compositeSpace = GimpColorSpace(-layer.compositeSpace);
            }
            break;

        case PROP_COMPOSITE_MODE:
            property >> layer.compositeMode;
            if (layer.compositeMode < 0) {
                layer.compositeMode = XCFImageFormat::GimpCompositeMode(-layer.compositeMode);
            }
            break;

        case PROP_BLEND_SPACE:
            property >> layer.blendSpace;
            if (layer.blendSpace) {
                layer.blendSpace = GimpColorSpace(-layer.blendSpace);
            }
            break;

        // Just for organization in the UI, doesn't influence rendering
        case PROP_COLOR_TAG:
            break;

        // We don't support editing, so for now just ignore locking
        case PROP_LOCK_CONTENT:
        case PROP_LOCK_POSITION:
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented layer property " << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Compute the number of tiles in the current layer and allocate
 * QImage structures for each of them.
 * \param xcf_image contains the current layer.
 */
bool XCFImageFormat::composeTiles(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);

    layer.nrows = (layer.height + TILE_HEIGHT - 1) / TILE_HEIGHT;
    layer.ncols = (layer.width + TILE_WIDTH - 1) / TILE_WIDTH;

    qCDebug(XCFPLUGIN) << "IMAGE: height=" << xcf_image.header.height << ", width=" << xcf_image.header.width;
    qCDebug(XCFPLUGIN) << "LAYER: height=" << layer.height << ", width=" << layer.width;
    qCDebug(XCFPLUGIN) << "LAYER: rows=" << layer.nrows << ", columns=" << layer.ncols;

    // NOTE: starting from GIMP 2.10, images can be very large. The 32K limit for width and height is obsolete
    //       and it was changed to 300000 (the same as Photoshop Big image). This plugin was able to open an RGB
    //       image of 108000x40000 pixels saved with GIMP 2.10
    // SANITY CHECK: Catch corrupted XCF image file where the width or height
    //               of a tile is reported are bogus. See Bug# 234030.
    if ((sizeof(void *) == 4 && qint64(layer.width) * layer.height > 16384 * 16384)) {
        qCWarning(XCFPLUGIN) << "On 32-bits programs the maximum layer size is limited to" << 16384 << "x" << 16384 << "px";
        return false;
    }
    if (layer.width > MAX_IMAGE_WIDTH || layer.height > MAX_IMAGE_HEIGHT) {
        qCWarning(XCFPLUGIN) << "The maximum layer size is limited to" << MAX_IMAGE_WIDTH << "x" << MAX_IMAGE_HEIGHT << "px";
        return false;
    }

    // NOTE: A layer is a named rectangular area of pixels which has a definite position with respect to the canvas.
    //       It may extend beyond the canvas or (more commonly) only cover some of it.
    // SANITY CHECK: Avoid to load XCF with a layer grater than 10 times the final image
    if (qint64(layer.width) * layer.height / 10 > qint64(xcf_image.header.width) * xcf_image.header.height) {
        if (qint64(layer.width) * layer.height > 16384 * 16384) { // large layers only
            qCWarning(XCFPLUGIN) << "Euristic sanity check: the image may be corrupted!";
            return false;
        }
    }

#ifndef XCF_QT5_SUPPORT
    // Qt 6 image allocation limit calculation: we have to check the limit here because the image is splitted in
    // tiles of 64x64 pixels. The required memory to build the image is at least doubled because tiles are loaded
    // and then the final image is created by copying the tiles inside it.
    // NOTE: on Windows to open a 10GiB image the plugin uses 28GiB of RAM
    qint64 channels = 1 + (layer.type == RGB_GIMAGE ? 2 : 0) + (layer.type == RGBA_GIMAGE ? 3 : 0);
    if (qint64(layer.width) * qint64(layer.height) * channels * 2ll / 1024ll / 1024ll > QImageReader::allocationLimit()) {
        qCDebug(XCFPLUGIN) << "Rejecting image as it exceeds the current allocation limit of" << QImageReader::allocationLimit() << "megabytes";
        return false;
    }
#endif

    layer.image_tiles.resize(layer.nrows);

    if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
        layer.alpha_tiles.resize(layer.nrows);
    }

    if (layer.mask_offset != 0) {
        layer.mask_tiles.resize(layer.nrows);
    }

    for (uint j = 0; j < layer.nrows; j++) {
        layer.image_tiles[j].resize(layer.ncols);

        if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
            layer.alpha_tiles[j].resize(layer.ncols);
        }

        if (layer.mask_offset != 0) {
            layer.mask_tiles[j].resize(layer.ncols);
        }
    }

    const QImage::Format format = layer.qimageFormat(xcf_image.header.precision);

    for (uint j = 0; j < layer.nrows; j++) {
        for (uint i = 0; i < layer.ncols; i++) {
            uint tile_width = (i + 1) * TILE_WIDTH <= layer.width ? TILE_WIDTH : layer.width - i * TILE_WIDTH;

            uint tile_height = (j + 1) * TILE_HEIGHT <= layer.height ? TILE_HEIGHT : layer.height - j * TILE_HEIGHT;

            // Try to create the most appropriate QImage (each GIMP layer
            // type is treated slightly differently)

            switch (layer.type) {
            case RGB_GIMAGE:
            case RGBA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, format);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                layer.image_tiles[j][i].setColorCount(0);
                break;

            case GRAY_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                layer.image_tiles[j][i].setColorCount(256);
                setGrayPalette(layer.image_tiles[j][i]);
                break;

            case GRAYA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(256);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.image_tiles[j][i]);

                layer.alpha_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                if (layer.alpha_tiles[j][i].isNull()) {
                    return false;
                }
                layer.alpha_tiles[j][i].setColorCount(256);
                setGrayPalette(layer.alpha_tiles[j][i]);
                break;

            case INDEXED_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.image_tiles[j][i].setColorCount(xcf_image.num_colors);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                setPalette(xcf_image, layer.image_tiles[j][i]);
                break;

            case INDEXEDA_GIMAGE:
                layer.image_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                if (layer.image_tiles[j][i].isNull()) {
                    return false;
                }
                layer.image_tiles[j][i].setColorCount(xcf_image.num_colors);
                setPalette(xcf_image, layer.image_tiles[j][i]);

                layer.alpha_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                if (layer.alpha_tiles[j][i].isNull()) {
                    return false;
                }
                layer.alpha_tiles[j][i].setColorCount(256);
                setGrayPalette(layer.alpha_tiles[j][i]);
            }
            if (layer.type != GRAYA_GIMAGE && layer.image_tiles[j][i].format() != format) {
                qCWarning(XCFPLUGIN) << "Selected wrong tile format" << layer.image_tiles[j][i].format() << "expected" << format;
                return false;
            }

#ifndef DISABLE_TILE_PROFILE
            switch (xcf_image.header.precision) {
            case XCFImageFormat::GIMP_PRECISION_HALF_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_FLOAT_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_DOUBLE_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U8_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U16_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U32_LINEAR:
                layer.image_tiles[j][i].setColorSpace(QColorSpace::SRgbLinear);
                break;
            case XCFImageFormat::GIMP_PRECISION_HALF_NON_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_FLOAT_NON_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_DOUBLE_NON_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U8_NON_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U16_NON_LINEAR:
            case XCFImageFormat::GIMP_PRECISION_U32_NON_LINEAR:
                layer.image_tiles[j][i].setColorSpace(QColorSpace::SRgb);
                break;
            case XCFImageFormat::GIMP_PRECISION_HALF_PERCEPTUAL:
            case XCFImageFormat::GIMP_PRECISION_FLOAT_PERCEPTUAL:
            case XCFImageFormat::GIMP_PRECISION_DOUBLE_PERCEPTUAL:
            case XCFImageFormat::GIMP_PRECISION_U8_PERCEPTUAL:
            case XCFImageFormat::GIMP_PRECISION_U16_PERCEPTUAL:
            case XCFImageFormat::GIMP_PRECISION_U32_PERCEPTUAL:
                layer.image_tiles[j][i].setColorSpace(QColorSpace::SRgb);
                break;
            }
#endif
            if (layer.mask_offset != 0) {
                layer.mask_tiles[j][i] = QImage(tile_width, tile_height, QImage::Format_Indexed8);
                layer.mask_tiles[j][i].setColorCount(256);
                if (layer.mask_tiles[j][i].isNull()) {
                    return false;
                }
                setGrayPalette(layer.mask_tiles[j][i]);
            }
        }
    }
    return true;
}

/*!
 * Apply a grayscale palette to the QImage. Note that Qt does not distinguish
 * between grayscale and indexed images. A grayscale image is just
 * an indexed image with a 256-color, grayscale palette.
 * \param image image to set to a grayscale palette.
 */
void XCFImageFormat::setGrayPalette(QImage &image)
{
    if (grayTable.isEmpty()) {
        grayTable.resize(256);

        for (int i = 0; i < 256; i++) {
            grayTable[i] = qRgb(i, i, i);
        }
    }

    image.setColorTable(grayTable);
}

/*!
 * Copy the indexed palette from the XCF image into the QImage.
 * \param xcf_image XCF image containing the palette read from the data stream.
 * \param image image to apply the palette to.
 */
void XCFImageFormat::setPalette(XCFImage &xcf_image, QImage &image)
{
    Q_ASSERT(xcf_image.num_colors == xcf_image.palette.size());

    image.setColorTable(xcf_image.palette);
}

/*!
 * Copy the parasites info to QImage.
 * \param xcf_image XCF image containing the parasites read from the data stream.
 * \param image image to apply the parasites data.
 * \note Some comment taken from https://gitlab.gnome.org/GNOME/gimp/-/blob/master/devel-docs/parasites.txt
 */
void XCFImageFormat::setImageParasites(const XCFImage &xcf_image, QImage &image)
{
    auto&& p = xcf_image.parasites;
    auto keys = p.keys();
    for (auto &&key : std::as_const(keys)) {
        auto value = p.value(key);
        if (value.isEmpty())
            continue;

        // "icc-profile" (IMAGE, PERSISTENT | UNDOABLE)
        //     This contains an ICC profile describing the color space the
        //     image was produced in. TIFF images stored in PhotoShop do
        //     oftentimes contain embedded profiles. An experimental color
        //     manager exists to use this parasite, and it will be used
        //     for interchange between TIFF and PNG (identical profiles)
        if (key == QStringLiteral("icc-profile")) {
            auto cs = QColorSpace::fromIccProfile(value);
            if (cs.isValid())
                image.setColorSpace(cs);
            continue;
        }

        // "gimp-comment" (IMAGE, PERSISTENT)
        //    Standard GIF-style image comments.  This parasite should be
        //    human-readable text in UTF-8 encoding.  A trailing \0 might
        //    be included and is not part of the comment.  Note that image
        //    comments may also be present in the "gimp-metadata" parasite.
        if (key == QStringLiteral("gimp-comment")) {
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral(META_KEY_COMMENT), QString::fromUtf8(value));
            continue;
        }

        // "gimp-image-metadata"
        //     Saved by GIMP 2.10.30 but it is not mentioned in the specification.
        //     It is an XML block with the properties set using GIMP.
        if (key == QStringLiteral("gimp-image-metadata")) {
            // NOTE: I arbitrary defined the metadata "XML:org.gimp.xml" because it seems
            //       a GIMP proprietary XML format (no xmlns defined)
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral(META_KEY_XML_GIMP), QString::fromUtf8(value));
            continue;
        }

#if 0   // Unable to generate it using latest GIMP version
        // "gimp-metadata" (IMAGE, PERSISTENT)
        //     The metadata associated with the image, serialized as one XMP
        //     packet.  This metadata includes the contents of any XMP, EXIF
        //     and IPTC blocks from the original image, as well as
        //     user-specified values such as image comment, copyright,
        //     license, etc.
        if (key == QStringLiteral("gimp-metadata")) {
            // NOTE: "XML:com.adobe.xmp" is the meta set by Qt reader when an
            //       XMP packet is found (e.g. when reading a PNG saved by Photoshop).
            //       I reused the same key because some programs could search for it.
            value.replace('\0', QByteArray());
            image.setText(QStringLiteral(META_KEY_XMP_ADOBE), QString::fromUtf8(value));
            continue;
        }
#endif
    }

#ifdef DISABLE_IMAGE_PROFILE
    // final colorspace checks
    if (!image.colorSpace().isValid()) {
        switch (xcf_image.header.precision) {
        case XCFImageFormat::GIMP_PRECISION_HALF_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_FLOAT_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_DOUBLE_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U8_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U16_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U32_LINEAR:
            image.setColorSpace(QColorSpace::SRgbLinear);
            break;
        default:
            image.setColorSpace(QColorSpace::SRgb);
            break;
        }
    }
#endif
}

/*!
 * Copy the bytes from the tile buffer into the image tile QImage, taking into
 * account all the myriad different modes.
 * \param layer layer containing the tile buffer and the image tile matrix.
 * \param i column index of current tile.
 * \param j row index of current tile.
 */
bool XCFImageFormat::assignImageBytes(Layer &layer, uint i, uint j, const GimpPrecision &precision)
{
    QImage &image = layer.image_tiles[j][i];

    const uchar *tile = layer.tile;
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();
    uchar *bits = image.bits();

    // Handle the special cases
    if (layer.type == GRAYA_GIMAGE || layer.type == GRAY_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
        auto bpc = bytesPerChannel(precision);
        for (int y = 0; y < height; y++) {
            uchar *dataPtr = bits + y * bytesPerLine;
            uchar *alphaPtr = nullptr;
            if (!layer.alpha_tiles.isEmpty()) {
                QImage &alphaTile = layer.alpha_tiles[j][i];
                if (alphaTile.width() >= width && alphaTile.height() > y) {
                    alphaPtr = alphaTile.scanLine(y);
                }
            }
            if (bpc == 4) {
#ifdef USE_FLOAT_IMAGES
                if (precision < GimpPrecision::GIMP_PRECISION_HALF_LINEAR) {
                    for (int x = 0; x < width; x++) {
                        auto src = (const quint16 *)tile;
                        *dataPtr++ = qFromBigEndian<quint16>(src[0]) / 257;
                        if (alphaPtr) {
                            *alphaPtr++ = qFromBigEndian<quint16>(src[1]) / 257;
                            tile += sizeof(quint16) * 2;
                        } else {
                            tile += sizeof(quint16);
                        }
                    }
                } else {
                    for (int x = 0; x < width; x++) {
                        auto src = (const float *)tile;
                        *dataPtr++ = qFromBigEndian<float>(src[0]) * 255;
                        if (alphaPtr) {
                            *alphaPtr++ = qFromBigEndian<float>(src[1]) * 255;
                            tile += sizeof(float) * 2;
                        } else {
                            tile += sizeof(float);
                        }
                    }
                }
#else
                for (int x = 0; x < width; x++) {
                    auto src = (const quint16 *)tile;
                    *dataPtr++ = qFromBigEndian<quint16>(src[0]) / 257;
                    if (alphaPtr) {
                        *alphaPtr++ = qFromBigEndian<quint16>(src[1]) / 257;
                        tile += sizeof(quint16) * 2;
                    } else {
                        tile += sizeof(quint16);
                    }
                }
#endif
            } else if (bpc == 2) {
#ifdef USE_FLOAT_IMAGES
                if (precision < GimpPrecision::GIMP_PRECISION_HALF_LINEAR) {
                    for (int x = 0; x < width; x++) {
                        auto src = (const quint16 *)tile;
                        *dataPtr++ = qFromBigEndian<quint16>(src[0]) / 257;
                        if (alphaPtr)
                            *alphaPtr++ = qFromBigEndian<quint16>(src[1]) / 257;
                        tile += sizeof(QRgb);
                    }
                } else {
                    for (int x = 0; x < width; x++) {
                        auto src = (const qfloat16 *)tile;
                        *dataPtr++ = qFromBigEndian<qfloat16>(src[0]) * 255;
                        if (alphaPtr)
                            *alphaPtr++ = qFromBigEndian<qfloat16>(src[1]) * 255;
                        tile += sizeof(QRgb);
                    }
                }
#else
                for (int x = 0; x < width; x++) {
                    auto src = (const quint16 *)tile;
                    *dataPtr++ = qFromBigEndian<quint16>(src[0]) / 257;
                    if (alphaPtr)
                        *alphaPtr++ = qFromBigEndian<quint16>(src[1]) / 257;
                    tile += sizeof(QRgb);
                }
#endif
            } else {
                for (int x = 0; x < width; x++) {
                    if (tile[0] < image.colorCount())
                        *dataPtr++ = tile[0];
                    if (alphaPtr)
                        *alphaPtr++ = tile[1];
                    tile += sizeof(QRgb);
                }
            }
        }
        return true;
    }

    switch (image.format()) {
    case QImage::Format_RGBX8888:
        for (int y = 0; y < height; y++) {
            uchar *dataPtr = image.scanLine(y);
            for (int x = 0; x < width * 4; x += 4, tile += 4) {
                dataPtr[x + 0] = tile[0];
                dataPtr[x + 1] = tile[1];
                dataPtr[x + 2] = tile[2];
                dataPtr[x + 3] = 255;
            }
        }
        break;
    case QImage::Format_RGBA8888:
        for (int y = 0; y < height; y++) {
            const size_t bpl = width * 4;
            memcpy(image.scanLine(y), tile + y * bpl, bpl);
        }
        break;
    case QImage::Format_RGBX64:
        for (int y = 0; y < height; y++) {
            quint16 *dataPtr = (quint16 *)image.scanLine(y);
            const size_t bpl = width * sizeof(QRgba64);
            const quint16 *src = (const quint16 *)(tile + y * bpl);
            for (int x = 0; x < width * 4; x += 4) {
                dataPtr[x + 0] = qFromBigEndian(src[x + 0]);
                dataPtr[x + 1] = qFromBigEndian(src[x + 1]);
                dataPtr[x + 2] = qFromBigEndian(src[x + 2]);
                dataPtr[x + 3] = 65535;
            }
        }
        break;
#ifdef USE_FLOAT_IMAGES
    case QImage::Format_RGBX16FPx4:
        for (int y = 0; y < height; y++) {
            qfloat16 *dataPtr = (qfloat16 *)image.scanLine(y);
            const qfloat16 *src = (const qfloat16 *)(tile + y * width * sizeof(QRgbaFloat16));
            for (int x = 0; x < width * 4; x += 4) {
                dataPtr[x + 0] = qFromBigEndian(src[x + 0]);
                dataPtr[x + 1] = qFromBigEndian(src[x + 1]);
                dataPtr[x + 2] = qFromBigEndian(src[x + 2]);
                dataPtr[x + 3] = qfloat16(1);
            }
        }
        break;
    case QImage::Format_RGBA16FPx4:
        static_assert(sizeof(QRgbaFloat16) == sizeof(QRgba64), "Different sizes for float and int 16 bit pixels");
#endif
    case QImage::Format_RGBA64:
        for (int y = 0; y < height; y++) {
            const size_t bpl = width * sizeof(QRgba64);
            qFromBigEndian<qint16>(tile + y * bpl, width * 4, image.scanLine(y));
        }
        break;
#ifdef USE_FLOAT_IMAGES
    case QImage::Format_RGBA32FPx4:
        for (int y = 0; y < height; y++) {
            const size_t bpl = width * sizeof(QRgbaFloat32);
            qFromBigEndian<qint32>(tile + y * bpl, width * 4, image.scanLine(y));
        }
        break;
    case QImage::Format_RGBX32FPx4:
        for (int y = 0; y < height; y++) {
            float *dataPtr = (float *)image.scanLine(y);
            const float *src = (const float *)(tile + y * width * sizeof(QRgbaFloat32));
            for (int x = 0; x < width * 4; x += 4) {
                dataPtr[x + 0] = qFromBigEndian(src[x + 0]);
                dataPtr[x + 1] = qFromBigEndian(src[x + 1]);
                dataPtr[x + 2] = qFromBigEndian(src[x + 2]);
                dataPtr[x + 3] = 1.f;
            }
        }
        break;
#endif
    case QImage::Format_Indexed8:
        for (int y = 0; y < height; y++) {
            uchar *dataPtr = bits + y * bytesPerLine;
            for (int x = 0; x < width; x++) {
                *dataPtr++ = tile[0];
                tile += sizeof(QRgb);
            }
        }
        break;
    default:
        qCWarning(XCFPLUGIN) << "Unhandled image format" << image.format() << "and/or layer type" << layer.type;
        return false;
    }

    return true;
}

/*!
 * The GIMP stores images in a "mipmap"-like hierarchy. As far as the QImage
 * is concerned, however, only the top level (i.e., the full resolution image)
 * is used.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the image.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadHierarchy(QDataStream &xcf_io, Layer &layer, const GimpPrecision precision)
{
    qint32 width;
    qint32 height;
    quint32 bpp;

    xcf_io >> width >> height >> bpp;
    const qint64 offset = readOffsetPtr(xcf_io);

    qCDebug(XCFPLUGIN) << "width" << width << "height" << height << "bpp" << bpp << "offset" << offset;

    if (offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative hierarchy offset";
        return false;
    }

    const bool isMask = layer.assignBytes == assignMaskBytes;

    // make sure bpp is correct and complain if it is not
    switch (layer.type) {
    case RGB_GIMAGE:
        if (bpp != 3 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type RGB but with bpp != 3" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    case RGBA_GIMAGE:
        if (bpp != 4 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type RGBA but with bpp != 4, got" << bpp << "bpp";

            if (!isMask) {
                return false;
            }
        }
        break;
    case GRAY_GIMAGE:
        if (bpp != 1 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type Gray but with bpp != 1" << bpp;
            return false;
        }
        break;
    case GRAYA_GIMAGE:
        if (bpp != 2 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type Gray+Alpha but with bpp != 2" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    case INDEXED_GIMAGE:
        if (bpp != 1 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type Indexed but with bpp != 1" << bpp;
            return false;
        }
        break;
    case INDEXEDA_GIMAGE:
        if (bpp != 2 * bytesPerChannel(precision)) {
            qCDebug(XCFPLUGIN) << "Found layer of type Indexed+Alpha but with bpp != 2" << bpp;

            if (!isMask) {
                return false;
            }
        }
        break;
    }

    if (bpp > 4 * bytesPerChannel(precision)) {
        qCDebug(XCFPLUGIN) << "bpp is" << bpp << "We don't support layers with bpp > 4";
        return false;
    }

    // GIMP stores images in a "mipmap"-like format (multiple levels of
    // increasingly lower resolution). Only the top level is used here,
    // however.

    quint32 junk;
    do {
        xcf_io >> junk;

        if (xcf_io.device()->atEnd()) {
            qCDebug(XCFPLUGIN) << "XCF: read failure on layer " << layer.name << " level offsets";
            return false;
        }
    } while (junk != 0);

    qint64 saved_pos = xcf_io.device()->pos();

    xcf_io.device()->seek(offset);
    if (!loadLevel(xcf_io, layer, bpp, precision)) {
        return false;
    }

    xcf_io.device()->seek(saved_pos);
    return true;
}

template<typename SourceFormat>
static bool convertFloatTo16Bit(uchar *output, quint64 outputSize, uchar *input)
{
    SourceFormat *source = (SourceFormat *)(input);
    for (quint64 offset = 0; offset < outputSize; offset++) {
        ((uint16_t *)output)[offset] = qToBigEndian(quint16(qBound(0., qFromBigEndian<SourceFormat>(source[offset]) * 65535. + 0.5, 65535.)));
    }
    return true;
}

/*!
 * Load one level of the image hierarchy (but only the top level is ever used).
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the image.
 * \param bpp the number of bytes in a pixel.
 * \return true if there were no I/O errors.
 * \sa loadTileRLE().
 */
bool XCFImageFormat::loadLevel(QDataStream &xcf_io, Layer &layer, qint32 bpp, const GimpPrecision precision)
{
    qint32 width;
    qint32 height;

    xcf_io >> width >> height;
    qint64 offset = readOffsetPtr(xcf_io);

    if (offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative level offset";
        return false;
    }

    if (offset == 0) {
        // offset 0 with rowsxcols != 0 is probably an error since it means we have tiles
        // without data but just clear the bits for now instead of returning false
        for (uint j = 0; j < layer.nrows; j++) {
            for (uint i = 0; i < layer.ncols; i++) {
                layer.image_tiles[j][i].fill(Qt::transparent);
                if (layer.type == GRAYA_GIMAGE || layer.type == INDEXEDA_GIMAGE) {
                    layer.alpha_tiles[j][i].fill(Qt::transparent);
                }
            }
        }
        return true;
    }

    bool needConvert = true;
    switch (precision) {
#ifdef USE_FLOAT_IMAGES
    case GIMP_PRECISION_HALF_LINEAR:
    case GIMP_PRECISION_HALF_NON_LINEAR:
    case GIMP_PRECISION_HALF_PERCEPTUAL:
    case GIMP_PRECISION_FLOAT_LINEAR:
    case GIMP_PRECISION_FLOAT_NON_LINEAR:
    case GIMP_PRECISION_FLOAT_PERCEPTUAL:
#endif
    case GIMP_PRECISION_U8_LINEAR:
    case GIMP_PRECISION_U8_NON_LINEAR:
    case GIMP_PRECISION_U8_PERCEPTUAL:
    case GIMP_PRECISION_U16_LINEAR:
    case GIMP_PRECISION_U16_NON_LINEAR:
    case GIMP_PRECISION_U16_PERCEPTUAL:
        needConvert = false;
        break;
    default:
        break;
    }

    const uint blockSize = TILE_WIDTH * TILE_HEIGHT * bpp * 1.5;

    QList<uchar> buffer;
    if (needConvert) {
        buffer.resize(blockSize * (bpp == 2 ? 2 : 1));
    }
    for (uint j = 0; j < layer.nrows; j++) {
        for (uint i = 0; i < layer.ncols; i++) {
            if (offset == 0) {
                qCDebug(XCFPLUGIN) << "XCF: incorrect number of tiles in layer " << layer.name;
                return false;
            }

            qint64 saved_pos = xcf_io.device()->pos();
            qint64 offset2 = readOffsetPtr(xcf_io);

            if (offset2 < 0) {
                qCDebug(XCFPLUGIN) << "XCF: negative level offset";
                return false;
            }

            // Evidently, RLE can occasionally expand a tile instead of compressing it!
            if (offset2 == 0) {
                offset2 = offset + blockSize;
            }

            xcf_io.device()->seek(offset);
            qint64 bytesParsed = 0;

            switch (layer.compression) {
            case COMPRESS_NONE: {
                if (xcf_io.version() > 11 || size_t(bpp) > sizeof(QRgba64)) {
                    qCDebug(XCFPLUGIN) << "Component reading not supported yet";
                    return false;
                }
                const int data_size = bpp * TILE_WIDTH * TILE_HEIGHT;
                if (data_size > int(blockSize)) {
                    qCDebug(XCFPLUGIN) << "Tile data too big, we can only fit" << sizeof(layer.tile) << "but need" << data_size;
                    return false;
                }
                int dataRead = xcf_io.readRawData(reinterpret_cast<char *>(layer.tile), data_size);
                if (dataRead < data_size) {
                    qCDebug(XCFPLUGIN) << "short read, expected" << data_size << "got" << dataRead;
                    return false;
                }
                bytesParsed = dataRead;
                break;
            }
            case COMPRESS_RLE: {
                int size = layer.image_tiles[j][i].width() * layer.image_tiles[j][i].height();
                const uint data_size = size * bpp;
                if (needConvert) {
                    if (data_size >= unsigned(buffer.size())) {
                        qCDebug(XCFPLUGIN) << "Tile data too big, we can only fit" << buffer.size() << "but need" << data_size;
                        return false;
                    }
                } else {
                    if (data_size > sizeof(layer.tile)) {
                        qCDebug(XCFPLUGIN) << "Tile data too big, we can only fit" << sizeof(layer.tile) << "but need" << data_size;
                        return false;
                    }
                    if (blockSize > sizeof(layer.tile)) {
                        qCWarning(XCFPLUGIN) << "Too small tiles" << sizeof(layer.tile) << "this image requires" << blockSize << sizeof(QRgba64) << bpp;
                        return false;
                    }
                }
                if (!loadTileRLE(xcf_io, needConvert ? buffer.data() : layer.tile, size, offset2 - offset, bpp, &bytesParsed)) {
                    qCDebug(XCFPLUGIN) << "Failed to read RLE";
                    return false;
                }
                break;
            }
            default:
                qCDebug(XCFPLUGIN) << "Unhandled compression" << layer.compression;
                return false;
            }

            if (needConvert) {
                if (bytesParsed > buffer.size()) {
                    qCDebug(XCFPLUGIN) << "Invalid number of bytes parsed" << bytesParsed << buffer.size();
                    return false;
                }

                switch (precision) {
                case GIMP_PRECISION_U32_LINEAR:
                case GIMP_PRECISION_U32_NON_LINEAR:
                case GIMP_PRECISION_U32_PERCEPTUAL: {
                    quint32 *source = (quint32 *)(buffer.data());
                    for (quint64 offset = 0, len = buffer.size() / sizeof(quint32); offset < len; ++offset) {
                        ((quint16 *)layer.tile)[offset] = qToBigEndian<quint16>(qFromBigEndian(source[offset]) / 65537);
                    }
                    break;
                }
#ifndef USE_FLOAT_IMAGES
                case GIMP_PRECISION_HALF_LINEAR:
                case GIMP_PRECISION_HALF_NON_LINEAR:
                case GIMP_PRECISION_HALF_PERCEPTUAL:
                    convertFloatTo16Bit<qfloat16>(layer.tile, buffer.size() / sizeof(qfloat16), buffer.data());
                    break;
                case GIMP_PRECISION_FLOAT_LINEAR:
                case GIMP_PRECISION_FLOAT_NON_LINEAR:
                case GIMP_PRECISION_FLOAT_PERCEPTUAL:
                    convertFloatTo16Bit<float>(layer.tile, buffer.size() / sizeof(float), buffer.data());
                    break;
                case GIMP_PRECISION_DOUBLE_LINEAR:
                case GIMP_PRECISION_DOUBLE_NON_LINEAR:
                case GIMP_PRECISION_DOUBLE_PERCEPTUAL:
                    convertFloatTo16Bit<double>(layer.tile, buffer.size() / sizeof(double), buffer.data());
                    break;
#else
                case GIMP_PRECISION_DOUBLE_LINEAR:
                case GIMP_PRECISION_DOUBLE_NON_LINEAR:
                case GIMP_PRECISION_DOUBLE_PERCEPTUAL: {
                    double *source = (double *)(buffer.data());
                    for (quint64 offset = 0, len = buffer.size() / sizeof(double); offset < len; ++offset) {
                        ((float *)layer.tile)[offset] = qToBigEndian<float>(float(qFromBigEndian(source[offset])));
                    }
                    break;
                }
#endif
                default:
                    qCWarning(XCFPLUGIN) << "Unsupported precision" << precision;
                    return false;
                }
            }

            // The bytes in the layer tile are juggled differently depending on
            // the target QImage. The caller has set layer.assignBytes to the
            // appropriate routine.
            if (!layer.assignBytes(layer, i, j, precision)) {
                return false;
            }

            xcf_io.device()->seek(saved_pos);
            offset = readOffsetPtr(xcf_io);

            if (offset < 0) {
                qCDebug(XCFPLUGIN) << "XCF: negative level offset";
                return false;
            }
        }
    }

    return true;
}

/*!
 * A layer can have a one channel image which is used as a mask.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer the layer to collect the mask image.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadMask(QDataStream &xcf_io, Layer &layer, const GimpPrecision precision)
{
    qint32 width;
    qint32 height;
    char *name;

    xcf_io >> width >> height >> name;

    delete[] name;

    if (!loadChannelProperties(xcf_io, layer)) {
        return false;
    }

    const qint64 hierarchy_offset = readOffsetPtr(xcf_io);

    if (hierarchy_offset < 0) {
        qCDebug(XCFPLUGIN) << "XCF: negative mask hierarchy_offset";
        return false;
    }

    xcf_io.device()->seek(hierarchy_offset);
    layer.assignBytes = assignMaskBytes;

    if (!loadHierarchy(xcf_io, layer, precision)) {
        return false;
    }

    return true;
}

/*!
 * This is the routine for which all the other code is simply
 * infrastructure. Read the image bytes out of the file and
 * store them in the tile buffer. This is passed a full 32-bit deep
 * buffer, even if bpp is smaller. The caller can figure out what to
 * do with the bytes.
 *
 * The tile is stored in "channels", i.e. the red component of all
 * pixels, then the green component of all pixels, then blue then
 * alpha, or, for indexed images, the color indices of all pixels then
 * the alpha of all pixels.
 *
 * The data is compressed with "run length encoding". Some simple data
 * integrity checks are made.
 *
 * \param xcf_io the data stream connected to the XCF image.
 * \param tile the buffer to expand the RLE into.
 * \param image_size number of bytes expected to be in the image tile.
 * \param data_length number of bytes expected in the RLE.
 * \param bpp number of bytes per pixel.
 * \return true if there were no I/O errors and no obvious corruption of
 * the RLE data.
 */
bool XCFImageFormat::loadTileRLE(QDataStream &xcf_io, uchar *tile, int image_size, int data_length, qint32 bpp, qint64 *bytesParsed)
{
    uchar *data = tile;

    uchar *xcfdata;
    uchar *xcfodata;
    uchar *xcfdatalimit;

    int step = sizeof(QRgb);
    switch (bpp) {
    case 1:
    case 2:
    case 3:
    case 4:
        step = sizeof(QRgb);
        break;
    case 6:
    case 8:
        step = sizeof(QRgb) * 2;
        break;
    case 12:
    case 16:
        step = sizeof(QRgb) * 4;
        break;
    default:
        qCDebug(XCFPLUGIN) << "XCF: unhandled bit depth" << bpp;
        return false;
    }

    if (data_length < 0 || data_length > int(TILE_WIDTH * TILE_HEIGHT * step * 1.5)) {
        qCDebug(XCFPLUGIN) << "XCF: invalid tile data length" << data_length;
        return false;
    }

    xcfdata = xcfodata = new uchar[data_length];

    const int dataRead = xcf_io.readRawData((char *)xcfdata, data_length);
    if (dataRead <= 0) {
        delete[] xcfodata;
        qCDebug(XCFPLUGIN) << "XCF: read failure on tile" << dataRead;
        return false;
    }

    if (dataRead < data_length) {
        memset(&xcfdata[dataRead], 0, data_length - dataRead);
    }

    if (!xcf_io.device()->isOpen()) {
        delete[] xcfodata;
        qCDebug(XCFPLUGIN) << "XCF: read failure on tile";
        return false;
    }

    xcfdatalimit = &xcfodata[data_length - 1];

    for (int i = 0; i < bpp; ++i) {
        data = tile + i;

        int size = image_size;

        while (size > 0) {
            if (xcfdata > xcfdatalimit) {
                goto bogus_rle;
            }

            uchar val = *xcfdata++;
            uint length = val;

            if (length >= 128) {
                length = 255 - (length - 1);
                if (length == 128) {
                    if (xcfdata >= xcfdatalimit) {
                        goto bogus_rle;
                    }

                    length = (*xcfdata << 8) + xcfdata[1];

                    xcfdata += 2;
                }

                size -= length;

                if (size < 0) {
                    goto bogus_rle;
                }

                if (&xcfdata[length - 1] > xcfdatalimit) {
                    goto bogus_rle;
                }

                while (length-- > 0) {
                    *data = *xcfdata++;
                    data += step;
                }
            } else {
                length += 1;
                if (length == 128) {
                    if (xcfdata >= xcfdatalimit) {
                        goto bogus_rle;
                    }

                    length = (*xcfdata << 8) + xcfdata[1];
                    xcfdata += 2;
                }

                size -= length;

                if (size < 0) {
                    goto bogus_rle;
                }

                if (xcfdata > xcfdatalimit) {
                    goto bogus_rle;
                }

                qintptr totalLength = qintptr(data - tile) + length * step;
                if (totalLength >= image_size * step * 1.5) {
                    qCDebug(XCFPLUGIN) << "Ran out of space when trying to unpack image, over:" << totalLength - image_size << totalLength << image_size
                                       << length;
                    goto bogus_rle;
                }

                val = *xcfdata++;

                while (length-- > 0) {
                    *data = val;
                    data += step;
                }
            }
        }
    }
    *bytesParsed = qintptr(data - tile);

    delete[] xcfodata;
    return true;

bogus_rle:

    qCDebug(XCFPLUGIN) << "The run length encoding could not be decoded properly";
    delete[] xcfodata;
    return false;
}

/*!
 * An XCF file can contain an arbitrary number of properties associated
 * with a channel. Note that this routine only reads mask channel properties.
 * \param xcf_io the data stream connected to the XCF image.
 * \param layer layer containing the mask channel to collect the properties.
 * \return true if there were no I/O errors.
 */
bool XCFImageFormat::loadChannelProperties(QDataStream &xcf_io, Layer &layer)
{
    while (true) {
        PropType type;
        QByteArray bytes;
        quint32 rawType;

        if (!loadProperty(xcf_io, type, bytes, rawType)) {
            qCDebug(XCFPLUGIN) << "XCF: error loading channel properties";
            return false;
        }

        QDataStream property(bytes);

        switch (type) {
        case PROP_END:
            return true;

        case PROP_OPACITY:
            property >> layer.mask_channel.opacity;
            layer.mask_channel.opacity = std::min(layer.mask_channel.opacity, 255u);
            break;

        case PROP_FLOAT_OPACITY:
            // For some reason QDataStream isn't able to read the float (tried
            // setting the endianness manually)
            if (bytes.size() == 4) {
                layer.mask_channel.opacityFloat = qFromBigEndian(*reinterpret_cast<float *>(bytes.data()));
            } else {
                qCDebug(XCFPLUGIN) << "XCF: Invalid data size for float:" << bytes.size();
            }
            break;

        case PROP_VISIBLE:
            property >> layer.mask_channel.visible;
            break;

        case PROP_SHOW_MASKED:
            property >> layer.mask_channel.show_masked;
            break;

        case PROP_COLOR:
            property >> layer.mask_channel.red >> layer.mask_channel.green >> layer.mask_channel.blue;
            break;

        case PROP_FLOAT_COLOR:
            property >> layer.mask_channel.redF >> layer.mask_channel.greenF >> layer.mask_channel.blueF;
            break;

        case PROP_TATTOO:
            property >> layer.mask_channel.tattoo;
            break;

        // Only used in edit mode
        case PROP_LINKED:
            break;

        // Just for organization in the UI, doesn't influence rendering
        case PROP_COLOR_TAG:
            break;

        // We don't support editing, so for now just ignore locking
        case PROP_LOCK_CONTENT:
        case PROP_LOCK_POSITION:
            break;

        default:
            qCDebug(XCFPLUGIN) << "XCF: unimplemented channel property " << type << "(" << rawType << ")"
                               << ", size " << bytes.size();
            break;
        }
    }
}

/*!
 * Copy the bytes from the tile buffer into the mask tile QImage.
 * \param layer layer containing the tile buffer and the mask tile matrix.
 * \param i column index of current tile.
 * \param j row index of current tile.
 */
bool XCFImageFormat::assignMaskBytes(Layer &layer, uint i, uint j, const GimpPrecision &precision)
{
    QImage &image = layer.mask_tiles[j][i];
    if (image.depth() != 8) {
        qCWarning(XCFPLUGIN) << "invalid bytes per pixel, we only do 8 bit masks" << image.depth();
        return false;
    }

    uchar *tile = layer.tile;
    const int width = image.width();
    const int height = image.height();
    const int bytesPerLine = image.bytesPerLine();
    uchar *bits = image.bits();
    auto bpc = bytesPerChannel(precision);

    // mask management is a house of cards: the mask is always treated as 8 bit by the plugin
    // (I don't want to twist the code) so it needs a conversion here.
    // If previously converted the step is the type size, otherwise is the one set in loadTileRLE().
    for (int y = 0; y < height; y++) {
        uchar *dataPtr = bits + y * bytesPerLine;
#ifdef USE_FLOAT_IMAGES
        if (bpc == 4) {
            if (precision < GimpPrecision::GIMP_PRECISION_HALF_LINEAR) {
                for (int x = 0; x < width; x++) {
                    *dataPtr++ = qFromBigEndian<quint16>(*(const quint16 *)tile) / 257;
                    tile += sizeof(quint16); // was converted to 16 bits in loadLevel()
                }
            } else {
                for (int x = 0; x < width; x++) {
                    *dataPtr++ = qFromBigEndian<float>(*(const float *)tile) * 255;
                    tile += sizeof(QRgb); // yeah! see loadTileRLE()
                }
            }
        } else if (bpc == 2) {
            // when not converted, the step of a
            if (precision < GimpPrecision::GIMP_PRECISION_HALF_LINEAR) {
                for (int x = 0; x < width; x++) {
                    *dataPtr++ = qFromBigEndian<quint16>(*(const quint16 *)tile) / 257;
                    tile += sizeof(QRgb); // yeah! see loadTileRLE()
                }
            } else {
                for (int x = 0; x < width; x++) {
                    *dataPtr++ = qFromBigEndian<qfloat16>(*(const qfloat16 *)tile) * 255;
                    tile += sizeof(QRgb); // yeah! see loadTileRLE()
                }
            }
        }
#else
        if (bpc == 2) {
            for (int x = 0; x < width; x++) {
                *dataPtr++ = qFromBigEndian<quint16>(*(const quint16 *)tile) / 257;
                tile += sizeof(QRgb); // yeah! see loadTileRLE() / loadLevel()
            }
        } else if (bpc == 4) {
            for (int x = 0; x < width; x++) {
                *dataPtr++ = qFromBigEndian<quint16>(*(const quint16 *)tile) / 257;
                tile += sizeof(quint16); // was converted to 16 bits in loadLevel()
            }
        }
#endif
        else {
            for (int x = 0; x < width; x++) {
                *dataPtr++ = tile[0];
                tile += sizeof(QRgb); // yeah! see loadTileRLE()
            }
        }
    }

    return true;
}

/*!
 * Construct the QImage which will eventually be returned to the QImage
 * loader.
 *
 * There are a couple of situations which require that the QImage is not
 * exactly the same as The GIMP's representation. The full table is:
 * \verbatim
 *  Grayscale  opaque      :  8 bpp indexed
 *  Grayscale  translucent : 32 bpp + alpha
 *  Indexed    opaque      :  1 bpp if num_colors <= 2
 *                         :  8 bpp indexed otherwise
 *  Indexed    translucent :  8 bpp indexed + alpha if num_colors < 256
 *                         : 32 bpp + alpha otherwise
 *  RGB        opaque      : 32 bpp
 *  RGBA       translucent : 32 bpp + alpha
 * \endverbatim
 * Whether the image is translucent or not is determined by the bottom layer's
 * alpha channel. However, even if the bottom layer lacks an alpha channel,
 * it can still have an opacity < 1. In this case, the QImage is promoted
 * to 32-bit. (Note this is different from the output from the GIMP image
 * exporter, which seems to ignore this attribute.)
 *
 * Independently, higher layers can be translucent, but the background of
 * the image will not show through if the bottom layer is opaque.
 *
 * For indexed images, translucency is an all or nothing effect.
 * \param xcf_image contains image info and bottom-most layer.
 */
bool XCFImageFormat::initializeImage(XCFImage &xcf_image)
{
    // (Aliases to make the code look a little better.)
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);

    switch (layer.type) {
    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_Indexed8);
            image.setColorCount(256);
            if (image.isNull()) {
                return false;
            }
            setGrayPalette(image);
            image.fill(255);
            break;
        } // else, fall through to 32-bit representation
        Q_FALLTHROUGH();
    case GRAYA_GIMAGE:
    case RGB_GIMAGE:
    case RGBA_GIMAGE:
        image = imageAlloc(xcf_image.header.width, xcf_image.header.height, xcf_image.qimageFormat());
        if (image.isNull()) {
            return false;
        }
        if (image.hasAlphaChannel()) {
            image.fill(Qt::transparent);
        } else {
            image.fill(Qt::white);
        }
        break;

    case INDEXED_GIMAGE:
        // As noted in the table above, there are quite a few combinations
        // which are possible with indexed images, depending on the
        // presence of transparency (note: not translucency, which is not
        // supported by The GIMP for indexed images) and the number of
        // individual colors.

        // Note: Qt treats a bitmap with a Black and White color palette
        // as a mask, so only the "on" bits are drawn, regardless of the
        // order color table entries. Otherwise (i.e., at least one of the
        // color table entries is not black or white), it obeys the one-
        // or two-color palette. Have to ask about this...

        if (xcf_image.num_colors <= 2) {
            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_MonoLSB);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else if (xcf_image.num_colors <= 256) {
            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_Indexed8);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        }
        break;

    case INDEXEDA_GIMAGE:
        if (xcf_image.num_colors == 1) {
            // Plenty(!) of room to add a transparent color
            xcf_image.num_colors++;
            xcf_image.palette.resize(xcf_image.num_colors);
            xcf_image.palette[1] = xcf_image.palette[0];
            xcf_image.palette[0] = qRgba(255, 255, 255, 0);

            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_MonoLSB);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else if (xcf_image.num_colors < 256) {
            // Plenty of room to add a transparent color
            xcf_image.num_colors++;
            xcf_image.palette.resize(xcf_image.num_colors);
            for (int c = xcf_image.num_colors - 1; c >= 1; c--) {
                xcf_image.palette[c] = xcf_image.palette[c - 1];
            }

            xcf_image.palette[0] = qRgba(255, 255, 255, 0);
            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_Indexed8);
            image.setColorCount(xcf_image.num_colors);
            if (image.isNull()) {
                return false;
            }
            image.fill(0);
            setPalette(xcf_image, image);
        } else {
            // No room for a transparent color, so this has to be promoted to
            // true color. (There is no equivalent PNG representation output
            // from The GIMP as of v1.2.)
            image = imageAlloc(xcf_image.header.width, xcf_image.header.height, QImage::Format_ARGB32);
            if (image.isNull()) {
                return false;
            }
            image.fill(qRgba(255, 255, 255, 0));
        }
        break;
    }
    if (image.format() != xcf_image.qimageFormat()) {
        qCWarning(XCFPLUGIN) << "Selected wrong format:" << image.format() << "expected" << layer.qimageFormat(xcf_image.header.precision);
        return false;
    }

    // The final profile should be the one in the Parasite
    // NOTE: if not set here, the colorSpace is aet in setImageParasites() (if no one defined in the parasites)
#ifndef DISABLE_IMAGE_PROFILE
    switch (xcf_image.header.precision) {
    case XCFImageFormat::GIMP_PRECISION_HALF_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_FLOAT_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_DOUBLE_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U8_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U16_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U32_LINEAR:
        image.setColorSpace(QColorSpace::SRgbLinear);
        break;
    case XCFImageFormat::GIMP_PRECISION_HALF_NON_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_FLOAT_NON_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_DOUBLE_NON_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U8_NON_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U16_NON_LINEAR:
    case XCFImageFormat::GIMP_PRECISION_U32_NON_LINEAR:
        image.setColorSpace(QColorSpace::SRgb);
        break;
    case XCFImageFormat::GIMP_PRECISION_HALF_PERCEPTUAL:
    case XCFImageFormat::GIMP_PRECISION_FLOAT_PERCEPTUAL:
    case XCFImageFormat::GIMP_PRECISION_DOUBLE_PERCEPTUAL:
    case XCFImageFormat::GIMP_PRECISION_U8_PERCEPTUAL:
    case XCFImageFormat::GIMP_PRECISION_U16_PERCEPTUAL:
    case XCFImageFormat::GIMP_PRECISION_U32_PERCEPTUAL:
        image.setColorSpace(QColorSpace::SRgb);
        break;
    }
#endif

    if (xcf_image.x_resolution > 0 && xcf_image.y_resolution > 0) {
        const float dpmx = xcf_image.x_resolution * INCHESPERMETER;
        if (dpmx > float(std::numeric_limits<int>::max())) {
            return false;
        }
        const float dpmy = xcf_image.y_resolution * INCHESPERMETER;
        if (dpmy > float(std::numeric_limits<int>::max())) {
            return false;
        }
        image.setDotsPerMeterX((int)dpmx);
        image.setDotsPerMeterY((int)dpmy);
    }
    return true;
}

/*!
 * Copy a layer into an image, taking account of the manifold modes. The
 * contents of the image are replaced.
 * \param xcf_image contains the layer and image to be replaced.
 */
void XCFImageFormat::copyLayerToImage(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);
    PixelCopyOperation copy = nullptr;

    switch (layer.type) {
    case RGB_GIMAGE:
    case RGBA_GIMAGE:
        copy = copyRGBToRGB;
        break;
    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY) {
            copy = copyGrayToGray;
        } else {
            copy = copyGrayToRGB;
        }
        break;
    case GRAYA_GIMAGE:
        copy = copyGrayAToRGB;
        break;
    case INDEXED_GIMAGE:
        copy = copyIndexedToIndexed;
        break;
    case INDEXEDA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            copy = copyIndexedAToIndexed;
        } else {
            copy = copyIndexedAToRGB;
        }
    }

    if (!copy) {
        return;
    }

    // For each tile...

    for (uint j = 0; j < layer.nrows; j++) {
        qint32 y = qint32(j * TILE_HEIGHT);

        for (uint i = 0; i < layer.ncols; i++) {
            qint32 x = qint32(i * TILE_WIDTH);

            // This seems the best place to apply the dissolve because it
            // depends on the global position of each tile's
            // pixels. Apparently it's the only mode which can apply to a
            // single layer.

            if (layer.mode == GIMP_LAYER_MODE_DISSOLVE) {
                if (!random_table_initialized) {
                    initializeRandomTable();
                    random_table_initialized = true;
                }
                if (layer.type == RGBA_GIMAGE) {
                    dissolveRGBPixels(layer.image_tiles[j][i], x, y);
                }

                else if (layer.type == GRAYA_GIMAGE) {
                    dissolveAlphaPixels(layer.alpha_tiles[j][i], x, y);
                }
            }

            // Shortcut for common case
            if (copy == copyRGBToRGB && layer.apply_mask != 1) {
                QPainter painter(&image);
                painter.setOpacity(layer.opacity / 255.0);
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                if (x + layer.x_offset < MAX_IMAGE_WIDTH &&
                    y + layer.y_offset < MAX_IMAGE_HEIGHT) {
                    painter.drawImage(x + layer.x_offset, y + layer.y_offset, layer.image_tiles[j][i]);
                }
                continue;
            }

            for (int l = 0; l < layer.image_tiles[j][i].height(); l++) {
                for (int k = 0; k < layer.image_tiles[j][i].width(); k++) {
                    int m = x + k + layer.x_offset;
                    int n = y + l + layer.y_offset;

                    if (m < 0 || m >= image.width() || n < 0 || n >= image.height()) {
                        continue;
                    }

                    (*copy)(layer, i, j, k, l, image, m, n);
                }
            }
        }
    }
}

/*!
 * Copy an RGB pixel from the layer to the RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    if (image.depth() == 32) {
        QRgb src = layer.image_tiles[j][i].pixel(k, l);
        uchar src_a = layer.opacity;

        if (layer.type == RGBA_GIMAGE) {
            src_a = INT_MULT(src_a, qAlpha(src));
        }

        // Apply the mask (if any)

        if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
            src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
        }

        image.setPixel(m, n, qRgba(src, src_a));
    } else if (image.depth() == 64) {
        QRgba64 src = layer.image_tiles[j][i].pixelColor(k, l).rgba64();
        quint16 src_a = layer.opacity;

        if (layer.type == RGBA_GIMAGE) {
            src_a = INT_MULT(src_a, qAlpha(src));
        }

        // Apply the mask (if any)

        if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
            src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
        }
        src.setAlpha(src_a);

        image.setPixel(m, n, src);
    }
}

/*!
 * Copy a Gray pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Copy a Gray pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.opacity;
    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Copy a GrayA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Copy an Indexed pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
}

/*!
 * Copy an IndexedA pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    uchar src = layer.image_tiles[j][i].pixelIndex(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    if (src_a > 127) {
        src++;
    } else {
        src = 0;
    }

    image.setPixel(m, n, src);
}

/*!
 * Copy an IndexedA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
void XCFImageFormat::copyIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    // This is what appears in the GIMP window
    if (src_a <= 127) {
        src_a = 0;
    } else {
        src_a = OPAQUE_OPACITY;
    }

    image.setPixel(m, n, qRgba(src, src_a));
}

/*!
 * Merge a layer into an image, taking account of the manifold modes.
 * \param xcf_image contains the layer and image to merge.
 */
void XCFImageFormat::mergeLayerIntoImage(XCFImage &xcf_image)
{
    Layer &layer(xcf_image.layer);
    QImage &image(xcf_image.image);

    PixelMergeOperation merge = nullptr;

    if (!layer.opacity) {
        return; // don't bother doing anything
    }

    if (layer.blendSpace == XCFImageFormat::AutoColorSpace) {
        qCDebug(XCFPLUGIN) << "Auto blend space, defaulting to RgbLinearSpace (same as Gimp when writing this)";
        layer.blendSpace = XCFImageFormat::RgbLinearSpace;
    }

    if (layer.blendSpace != XCFImageFormat::RgbLinearSpace) {
        qCDebug(XCFPLUGIN) << "Unimplemented blend color space" << layer.blendSpace;
    }
    qCDebug(XCFPLUGIN) << "Blend color space" << layer.blendSpace;

    if (layer.compositeSpace == XCFImageFormat::AutoColorSpace) {
        qCDebug(XCFPLUGIN) << "Auto composite space, defaulting to RgbLinearSpace (same as Gimp when writing this)";
        layer.compositeSpace = XCFImageFormat::RgbLinearSpace;
    }

    if (layer.compositeSpace != XCFImageFormat::RgbLinearSpace) {
        qCDebug(XCFPLUGIN) << "Unimplemented composite color space" << layer.compositeSpace;
    }
    if (layer.compositeMode != XCFImageFormat::CompositeUnion) {
        qCDebug(XCFPLUGIN) << "Unhandled composite mode" << layer.compositeMode;
    }

    switch (layer.type) {
    case RGB_GIMAGE:
    case RGBA_GIMAGE:
        merge = mergeRGBToRGB;
        break;
    case GRAY_GIMAGE:
        if (layer.opacity == OPAQUE_OPACITY && xcf_image.image.depth() <= 8) {
            merge = mergeGrayToGray;
        } else {
            merge = mergeGrayToRGB;
        }
        break;
    case GRAYA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            merge = mergeGrayAToGray;
        } else {
            merge = mergeGrayAToRGB;
        }
        break;
    case INDEXED_GIMAGE:
        merge = mergeIndexedToIndexed;
        break;
    case INDEXEDA_GIMAGE:
        if (xcf_image.image.depth() <= 8) {
            merge = mergeIndexedAToIndexed;
        } else {
            merge = mergeIndexedAToRGB;
        }
    }

    if (!merge) {
        return;
    }

    if (merge == mergeRGBToRGB && layer.apply_mask != 1) {
        int painterMode = -1;
        switch (layer.mode) {
        case GIMP_LAYER_MODE_NORMAL:
        case GIMP_LAYER_MODE_NORMAL_LEGACY:
            painterMode = QPainter::CompositionMode_SourceOver;
            break;
        case GIMP_LAYER_MODE_MULTIPLY:
        case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
            painterMode = QPainter::CompositionMode_Multiply;
            break;
        case GIMP_LAYER_MODE_SCREEN:
        case GIMP_LAYER_MODE_SCREEN_LEGACY:
            painterMode = QPainter::CompositionMode_Screen;
            break;
        case GIMP_LAYER_MODE_OVERLAY:
        case GIMP_LAYER_MODE_OVERLAY_LEGACY:
            painterMode = QPainter::CompositionMode_Overlay;
            break;
        case GIMP_LAYER_MODE_DIFFERENCE:
        case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
            painterMode = QPainter::CompositionMode_Difference;
            break;
        case GIMP_LAYER_MODE_DARKEN_ONLY:
        case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
            painterMode = QPainter::CompositionMode_Darken;
            break;
        case GIMP_LAYER_MODE_LIGHTEN_ONLY:
        case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
            painterMode = QPainter::CompositionMode_Lighten;
            break;
        case GIMP_LAYER_MODE_DODGE:
        case GIMP_LAYER_MODE_DODGE_LEGACY:
            painterMode = QPainter::CompositionMode_ColorDodge;
            break;
        case GIMP_LAYER_MODE_BURN:
        case GIMP_LAYER_MODE_BURN_LEGACY:
            painterMode = QPainter::CompositionMode_ColorBurn;
            break;
        case GIMP_LAYER_MODE_HARDLIGHT:
        case GIMP_LAYER_MODE_HARDLIGHT_LEGACY:
            painterMode = QPainter::CompositionMode_HardLight;
            break;
        case GIMP_LAYER_MODE_SOFTLIGHT:
        case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY:
            painterMode = QPainter::CompositionMode_SoftLight;
            break;
        case GIMP_LAYER_MODE_ADDITION:
        case GIMP_LAYER_MODE_ADDITION_LEGACY:
            painterMode = QPainter::CompositionMode_Plus;
            break;
        case GIMP_LAYER_MODE_EXCLUSION:
            painterMode = QPainter::CompositionMode_Exclusion;
            break;

            // Not bothered to find what the QPainter equivalent is, or there is none
        case GIMP_LAYER_MODE_GRAIN_EXTRACT:
        case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY:
        case GIMP_LAYER_MODE_GRAIN_MERGE:
        case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY:
        case GIMP_LAYER_MODE_COLOR_ERASE:
        case GIMP_LAYER_MODE_COLOR_ERASE_LEGACY:
        case GIMP_LAYER_MODE_LCH_HUE:
        case GIMP_LAYER_MODE_LCH_CHROMA:
        case GIMP_LAYER_MODE_LCH_COLOR:
        case GIMP_LAYER_MODE_LCH_LIGHTNESS:
        case GIMP_LAYER_MODE_BEHIND:
        case GIMP_LAYER_MODE_BEHIND_LEGACY:
        case GIMP_LAYER_MODE_SUBTRACT:
        case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
        case GIMP_LAYER_MODE_HSV_HUE:
        case GIMP_LAYER_MODE_HSV_SATURATION:
        case GIMP_LAYER_MODE_HSL_COLOR:
        case GIMP_LAYER_MODE_HSV_VALUE:
        case GIMP_LAYER_MODE_DIVIDE:
        case GIMP_LAYER_MODE_VIVID_LIGHT:
        case GIMP_LAYER_MODE_PIN_LIGHT:
        case GIMP_LAYER_MODE_LINEAR_LIGHT:
        case GIMP_LAYER_MODE_HARD_MIX:
        case GIMP_LAYER_MODE_LINEAR_BURN:
        case GIMP_LAYER_MODE_LUMA_DARKEN_ONLY:
        case GIMP_LAYER_MODE_LUMA_LIGHTEN_ONLY:
        case GIMP_LAYER_MODE_LUMINANCE:
        case GIMP_LAYER_MODE_ERASE:
        case GIMP_LAYER_MODE_MERGE:
        case GIMP_LAYER_MODE_SPLIT:
        case GIMP_LAYER_MODE_PASS_THROUGH:
        case GIMP_LAYER_MODE_HSV_HUE_LEGACY:
        case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY:
        case GIMP_LAYER_MODE_HSL_COLOR_LEGACY:
        case GIMP_LAYER_MODE_HSV_VALUE_LEGACY:
        case GIMP_LAYER_MODE_DIVIDE_LEGACY:
            qCDebug(XCFPLUGIN) << "No QPainter equivalent to" << layer.mode;
            break;

            // Special
        case GIMP_LAYER_MODE_DISSOLVE:
        case GIMP_LAYER_MODE_COUNT:
            break;
        }

        if (painterMode != -1) {
            QPainter painter(&image);
            painter.setOpacity(layer.opacity / 255.0);
            painter.setCompositionMode(QPainter::CompositionMode(painterMode));
            qCDebug(XCFPLUGIN) << "Using QPainter for mode" << layer.mode;

            for (uint j = 0; j < layer.nrows; j++) {
                qint32 y = qint32(j * TILE_HEIGHT);

                for (uint i = 0; i < layer.ncols; i++) {
                    qint32 x = qint32(i * TILE_WIDTH);

                    QImage &tile = layer.image_tiles[j][i];
                    if (x + layer.x_offset < MAX_IMAGE_WIDTH &&
                        y + layer.y_offset < MAX_IMAGE_HEIGHT) {
                        painter.drawImage(x + layer.x_offset, y + layer.y_offset, tile);
                    }
                }
            }

            return;
        }
    }

#ifndef DISABLE_IMAGE_PROFILE_CONV // The final profile should be the one in the Parasite
    if (layer.compositeSpace == XCFImageFormat::RgbPerceptualSpace && image.colorSpace() != QColorSpace::SRgb) {
        qCDebug(XCFPLUGIN) << "Converting to composite color space" << layer.compositeSpace;
        image.convertToColorSpace(QColorSpace::SRgb);
    }
    if (layer.compositeSpace == XCFImageFormat::RgbLinearSpace && image.colorSpace() != QColorSpace::SRgbLinear) {
        qCDebug(XCFPLUGIN) << "Converting to composite color space" << layer.compositeSpace;
        image.convertToColorSpace(QColorSpace::SRgbLinear);
    }
#endif

    for (uint j = 0; j < layer.nrows; j++) {
        qint32 y = qint32(j * TILE_HEIGHT);

        for (uint i = 0; i < layer.ncols; i++) {
            qint32 x = qint32(i * TILE_WIDTH);

            // This seems the best place to apply the dissolve because it
            // depends on the global position of each tile's
            // pixels. Apparently it's the only mode which can apply to a
            // single layer.

            if (layer.mode == GIMP_LAYER_MODE_DISSOLVE) {
                if (!random_table_initialized) {
                    initializeRandomTable();
                    random_table_initialized = true;
                }
                if (layer.type == RGBA_GIMAGE) {
                    dissolveRGBPixels(layer.image_tiles[j][i], x, y);
                }

                else if (layer.type == GRAYA_GIMAGE) {
                    dissolveAlphaPixels(layer.alpha_tiles[j][i], x, y);
                }
            }

            // Shortcut for common case
            if (merge == mergeRGBToRGB && layer.apply_mask != 1 && layer.mode == GIMP_LAYER_MODE_NORMAL_LEGACY) {
                QPainter painter(&image);
                painter.setOpacity(layer.opacity / 255.0);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                if (x + layer.x_offset < MAX_IMAGE_WIDTH &&
                    y + layer.y_offset < MAX_IMAGE_HEIGHT) {
                    painter.drawImage(x + layer.x_offset, y + layer.y_offset, layer.image_tiles[j][i]);
                }
                continue;
            }

#ifndef DISABLE_TILE_PROFILE_CONV // not sure about that: left as old plugin
            QImage &tile = layer.image_tiles[j][i];
            if (layer.compositeSpace == XCFImageFormat::RgbPerceptualSpace && tile.colorSpace() != QColorSpace::SRgb) {
                tile.convertToColorSpace(QColorSpace::SRgb);
            }
            if (layer.compositeSpace == XCFImageFormat::RgbLinearSpace && tile.colorSpace() != QColorSpace::SRgbLinear) {
                tile.convertToColorSpace(QColorSpace::SRgbLinear);
            }
#endif

            for (int l = 0; l < layer.image_tiles[j][i].height(); l++) {
                for (int k = 0; k < layer.image_tiles[j][i].width(); k++) {
                    int m = x + k + layer.x_offset;
                    int n = y + l + layer.y_offset;

                    if (m < 0 || m >= image.width() || n < 0 || n >= image.height()) {
                        continue;
                    }

                    if (!(*merge)(layer, i, j, k, l, image, m, n)) {
                        return;
                    }
                }
            }
        }
    }
}

/*!
 * Merge an RGB pixel from the layer to the RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeRGBToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    QRgb dst = image.pixel(m, n);

    uchar src_r = qRed(src);
    uchar src_g = qGreen(src);
    uchar src_b = qBlue(src);
    uchar src_a = qAlpha(src);

    uchar dst_r = qRed(dst);
    uchar dst_g = qGreen(dst);
    uchar dst_b = qBlue(dst);
    uchar dst_a = qAlpha(dst);

    if (!src_a) {
        return false; // nothing to merge
    }

    switch (layer.mode) {
    case GIMP_LAYER_MODE_NORMAL:
    case GIMP_LAYER_MODE_NORMAL_LEGACY:
        break;
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_MULTIPLY_LEGACY:
        src_r = INT_MULT(src_r, dst_r);
        src_g = INT_MULT(src_g, dst_g);
        src_b = INT_MULT(src_b, dst_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY:
        src_r = qMin((dst_r * 256) / (1 + src_r), 255);
        src_g = qMin((dst_g * 256) / (1 + src_g), 255);
        src_b = qMin((dst_b * 256) / (1 + src_b), 255);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_SCREEN_LEGACY:
        src_r = 255 - INT_MULT(255 - dst_r, 255 - src_r);
        src_g = 255 - INT_MULT(255 - dst_g, 255 - src_g);
        src_b = 255 - INT_MULT(255 - dst_b, 255 - src_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY:
        src_r = INT_MULT(dst_r, dst_r + INT_MULT(2 * src_r, 255 - dst_r));
        src_g = INT_MULT(dst_g, dst_g + INT_MULT(2 * src_g, 255 - dst_g));
        src_b = INT_MULT(dst_b, dst_b + INT_MULT(2 * src_b, 255 - dst_b));
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY:
        src_r = dst_r > src_r ? dst_r - src_r : src_r - dst_r;
        src_g = dst_g > src_g ? dst_g - src_g : src_g - dst_g;
        src_b = dst_b > src_b ? dst_b - src_b : src_b - dst_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_ADDITION_LEGACY:
        src_r = add_lut(dst_r, src_r);
        src_g = add_lut(dst_g, src_g);
        src_b = add_lut(dst_b, src_b);
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY:
        src_r = dst_r > src_r ? dst_r - src_r : 0;
        src_g = dst_g > src_g ? dst_g - src_g : 0;
        src_b = dst_b > src_b ? dst_b - src_b : 0;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY:
        src_r = dst_r < src_r ? dst_r : src_r;
        src_g = dst_g < src_g ? dst_g : src_g;
        src_b = dst_b < src_b ? dst_b : src_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY:
        src_r = dst_r < src_r ? src_r : dst_r;
        src_g = dst_g < src_g ? src_g : dst_g;
        src_b = dst_b < src_b ? src_b : dst_b;
        src_a = qMin(src_a, dst_a);
        break;
    case GIMP_LAYER_MODE_HSV_HUE:
    case GIMP_LAYER_MODE_HSV_HUE_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_r = src_r;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSV_SATURATION:
    case GIMP_LAYER_MODE_HSV_SATURATION_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_g = src_g;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSV_VALUE:
    case GIMP_LAYER_MODE_HSV_VALUE_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHSV(src_r, src_g, src_b);
        RGBTOHSV(new_r, new_g, new_b);

        new_b = src_b;

        HSVTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HSL_COLOR:
    case GIMP_LAYER_MODE_HSL_COLOR_LEGACY: {
        uchar new_r = dst_r;
        uchar new_g = dst_g;
        uchar new_b = dst_b;

        RGBTOHLS(src_r, src_g, src_b);
        RGBTOHLS(new_r, new_g, new_b);

        new_r = src_r;
        new_b = src_b;

        HLSTORGB(new_r, new_g, new_b);

        src_r = new_r;
        src_g = new_g;
        src_b = new_b;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_DODGE_LEGACY: {
        uint tmp;

        tmp = dst_r << 8;
        tmp /= 256 - src_r;
        src_r = (uchar)qMin(tmp, 255u);

        tmp = dst_g << 8;
        tmp /= 256 - src_g;
        src_g = (uchar)qMin(tmp, 255u);

        tmp = dst_b << 8;
        tmp /= 256 - src_b;
        src_b = (uchar)qMin(tmp, 255u);

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_BURN_LEGACY: {
        uint tmp;

        tmp = (255 - dst_r) << 8;
        tmp /= src_r + 1;
        src_r = (uchar)qMin(tmp, 255u);
        src_r = 255 - src_r;

        tmp = (255 - dst_g) << 8;
        tmp /= src_g + 1;
        src_g = (uchar)qMin(tmp, 255u);
        src_g = 255 - src_g;

        tmp = (255 - dst_b) << 8;
        tmp /= src_b + 1;
        src_b = (uchar)qMin(tmp, 255u);
        src_b = 255 - src_b;

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY: {
        uint tmp;
        if (src_r > 128) {
            tmp = ((int)255 - dst_r) * ((int)255 - ((src_r - 128) << 1));
            src_r = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_r * ((int)src_r << 1);
            src_r = (uchar)qMin(tmp >> 8, 255u);
        }

        if (src_g > 128) {
            tmp = ((int)255 - dst_g) * ((int)255 - ((src_g - 128) << 1));
            src_g = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_g * ((int)src_g << 1);
            src_g = (uchar)qMin(tmp >> 8, 255u);
        }

        if (src_b > 128) {
            tmp = ((int)255 - dst_b) * ((int)255 - ((src_b - 128) << 1));
            src_b = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst_b * ((int)src_b << 1);
            src_b = (uchar)qMin(tmp >> 8, 255u);
        }
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst_r, src_r);
        tmpS = 255 - INT_MULT((255 - dst_r), (255 - src_r));
        src_r = INT_MULT((255 - dst_r), tmpM) + INT_MULT(dst_r, tmpS);

        tmpM = INT_MULT(dst_g, src_g);
        tmpS = 255 - INT_MULT((255 - dst_g), (255 - src_g));
        src_g = INT_MULT((255 - dst_g), tmpM) + INT_MULT(dst_g, tmpS);

        tmpM = INT_MULT(dst_b, src_b);
        tmpS = 255 - INT_MULT((255 - dst_b), (255 - src_b));
        src_b = INT_MULT((255 - dst_b), tmpM) + INT_MULT(dst_b, tmpS);

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY: {
        int tmp;

        tmp = dst_r - src_r + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_r = (uchar)tmp;

        tmp = dst_g - src_g + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_g = (uchar)tmp;

        tmp = dst_b - src_b + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_b = (uchar)tmp;

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY: {
        int tmp;

        tmp = dst_r + src_r - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_r = (uchar)tmp;

        tmp = dst_g + src_g - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_g = (uchar)tmp;

        tmp = dst_b + src_b - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);
        src_b = (uchar)tmp;

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_LINEAR_LIGHT: {
        if (src_r <= 128) {
            src_r = qBound(0, dst_r + 2 * src_r - 255, 255);
        } else {
            src_r = qBound(0, dst_r + 2 * (src_r - 128), 255);
        }
        if (src_g <= 128) {
            src_g = qBound(0, dst_g + 2 * src_g - 255, 255);
        } else {
            src_g = qBound(0, dst_g + 2 * (src_g - 127), 255);
        }
        if (src_b <= 128) {
            src_b = qBound(0, dst_b + 2 * src_b - 255, 255);
        } else {
            src_b = qBound(0, dst_b + 2 * (src_b - 127), 255);
        }
    } break;
    case GIMP_LAYER_MODE_VIVID_LIGHT: {
        // From http://www.simplefilter.de/en/basics/mixmods.html
        float A[3];
        A[0] = src_r / 255.;
        A[1] = src_g / 255.;
        A[2] = src_b / 255.;
        float B[3];
        B[0] = dst_r / 255.;
        B[1] = dst_g / 255.;
        B[2] = dst_b / 255.;
        float C[3]{};
        for (int i = 0; i < 3; i++) {
            if (A[i] <= 0.5f) {
                if (A[i] > 0.f) {
                    C[i] = 1.f - (1.f - B[i]) / (2.f * A[i]);
                }
            } else {
                if (A[i] < 1.f) {
                    C[i] = B[i] / (2.f * (1.f - A[i]));
                }
            }
        }
        src_r = qBound(0.f, C[0] * 255.f, 255.f);
        src_g = qBound(0.f, C[1] * 255.f, 255.f);
        src_b = qBound(0.f, C[2] * 255.f, 255.f);
    } break;
    default:
        qCWarning(XCFPLUGIN) << "Unhandled mode" << layer.mode;
        return false;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_r;
    uchar new_g;
    uchar new_b;
    uchar new_a;
    new_a = dst_a + INT_MULT(OPAQUE_OPACITY - dst_a, src_a);

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    new_r = (uchar)(src_ratio * src_r + dst_ratio * dst_r + EPSILON);
    new_g = (uchar)(src_ratio * src_g + dst_ratio * dst_g + EPSILON);
    new_b = (uchar)(src_ratio * src_b + dst_ratio * dst_b + EPSILON);

    if (!modeAffectsSourceAlpha(layer.mode)) {
        new_a = dst_a;
    }

    image.setPixel(m, n, qRgba(new_r, new_g, new_b, new_a));
    return true;
}

/*!
 * Merge a Gray pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeGrayToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
    return true;
}

/*!
 * Merge a GrayA pixel from the layer to the Gray image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeGrayAToGray(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = qGray(layer.image_tiles[j][i].pixel(k, l));
    int dst = image.pixelIndex(m, n);

    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);

    if (!src_a) {
        return false; // nothing to merge
    }

    switch (layer.mode) {
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_MULTIPLY_LEGACY: {
        src = INT_MULT(src, dst);
    } break;
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY: {
        src = qMin((dst * 256) / (1 + src), 255);
    } break;
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_SCREEN_LEGACY: {
        src = 255 - INT_MULT(255 - dst, 255 - src);
    } break;
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY: {
        src = INT_MULT(dst, dst + INT_MULT(2 * src, 255 - dst));
    } break;
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY: {
        src = dst > src ? dst - src : src - dst;
    } break;
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_ADDITION_LEGACY: {
        src = add_lut(dst, src);
    } break;
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY: {
        src = dst > src ? dst - src : 0;
    } break;
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY: {
        src = dst < src ? dst : src;
    } break;
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY: {
        src = dst < src ? src : dst;
    } break;
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_DODGE_LEGACY: {
        uint tmp = dst << 8;
        tmp /= 256 - src;
        src = (uchar)qMin(tmp, 255u);
    } break;
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_BURN_LEGACY: {
        uint tmp = (255 - dst) << 8;
        tmp /= src + 1;
        src = (uchar)qMin(tmp, 255u);
        src = 255 - src;
    } break;
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY: {
        uint tmp;
        if (src > 128) {
            tmp = ((int)255 - dst) * ((int)255 - ((src - 128) << 1));
            src = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst * ((int)src << 1);
            src = (uchar)qMin(tmp >> 8, 255u);
        }
    } break;
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst, src);
        tmpS = 255 - INT_MULT((255 - dst), (255 - src));
        src = INT_MULT((255 - dst), tmpM) + INT_MULT(dst, tmpS);

    } break;
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY: {
        int tmp;

        tmp = dst - src + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
    } break;
    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY: {
        int tmp;

        tmp = dst + src - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
    } break;
    default:
        qCWarning(XCFPLUGIN) << "Unhandled mode" << layer.mode;
        return false;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_a = OPAQUE_OPACITY;

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    uchar new_g = (uchar)(src_ratio * src + dst_ratio * dst + EPSILON);

    image.setPixel(m, n, new_g);
    return true;
}

/*!
 * Merge a Gray pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeGrayToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.opacity;
    image.setPixel(m, n, qRgba(src, src_a));
    return true;
}

/*!
 * Merge a GrayA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeGrayAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = qGray(layer.image_tiles[j][i].pixel(k, l));
    int dst = qGray(image.pixel(m, n));

    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    uchar dst_a = qAlpha(image.pixel(m, n));

    if (!src_a) {
        return false; // nothing to merge
    }

    switch (layer.mode) {
    case GIMP_LAYER_MODE_NORMAL:
    case GIMP_LAYER_MODE_NORMAL_LEGACY:
        break;
    case GIMP_LAYER_MODE_MULTIPLY:
    case GIMP_LAYER_MODE_MULTIPLY_LEGACY: {
        src = INT_MULT(src, dst);
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DIVIDE:
    case GIMP_LAYER_MODE_DIVIDE_LEGACY: {
        src = qMin((dst * 256) / (1 + src), 255);
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_SCREEN:
    case GIMP_LAYER_MODE_SCREEN_LEGACY: {
        src = 255 - INT_MULT(255 - dst, 255 - src);
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_OVERLAY:
    case GIMP_LAYER_MODE_OVERLAY_LEGACY: {
        src = INT_MULT(dst, dst + INT_MULT(2 * src, 255 - dst));
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DIFFERENCE:
    case GIMP_LAYER_MODE_DIFFERENCE_LEGACY: {
        src = dst > src ? dst - src : src - dst;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_ADDITION:
    case GIMP_LAYER_MODE_ADDITION_LEGACY: {
        src = add_lut(dst, src);
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_SUBTRACT:
    case GIMP_LAYER_MODE_SUBTRACT_LEGACY: {
        src = dst > src ? dst - src : 0;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DARKEN_ONLY:
    case GIMP_LAYER_MODE_DARKEN_ONLY_LEGACY: {
        src = dst < src ? dst : src;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_LIGHTEN_ONLY:
    case GIMP_LAYER_MODE_LIGHTEN_ONLY_LEGACY: {
        src = dst < src ? src : dst;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_DODGE:
    case GIMP_LAYER_MODE_DODGE_LEGACY: {
        uint tmp = dst << 8;
        tmp /= 256 - src;
        src = (uchar)qMin(tmp, 255u);
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_BURN:
    case GIMP_LAYER_MODE_BURN_LEGACY: {
        uint tmp = (255 - dst) << 8;
        tmp /= src + 1;
        src = (uchar)qMin(tmp, 255u);
        src = 255 - src;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_HARDLIGHT:
    case GIMP_LAYER_MODE_HARDLIGHT_LEGACY: {
        uint tmp;
        if (src > 128) {
            tmp = ((int)255 - dst) * ((int)255 - ((src - 128) << 1));
            src = (uchar)qMin(255 - (tmp >> 8), 255u);
        } else {
            tmp = (int)dst * ((int)src << 1);
            src = (uchar)qMin(tmp >> 8, 255u);
        }
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_SOFTLIGHT:
    case GIMP_LAYER_MODE_SOFTLIGHT_LEGACY: {
        uint tmpS;
        uint tmpM;

        tmpM = INT_MULT(dst, src);
        tmpS = 255 - INT_MULT((255 - dst), (255 - src));
        src = INT_MULT((255 - dst), tmpM) + INT_MULT(dst, tmpS);

        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_EXTRACT:
    case GIMP_LAYER_MODE_GRAIN_EXTRACT_LEGACY: {
        int tmp;

        tmp = dst - src + 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
        src_a = qMin(src_a, dst_a);
    } break;
    case GIMP_LAYER_MODE_GRAIN_MERGE:
    case GIMP_LAYER_MODE_GRAIN_MERGE_LEGACY: {
        int tmp;

        tmp = dst + src - 128;
        tmp = qMin(tmp, 255);
        tmp = qMax(tmp, 0);

        src = (uchar)tmp;
        src_a = qMin(src_a, dst_a);
    } break;
    default:
        qCWarning(XCFPLUGIN) << "Unhandled mode" << layer.mode;
        return false;
    }

    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    uchar new_a = dst_a + INT_MULT(OPAQUE_OPACITY - dst_a, src_a);

    const float src_ratio = new_a == 0 ? 1.0 : (float)src_a / new_a;
    float dst_ratio = 1.0 - src_ratio;

    uchar new_g = (uchar)(src_ratio * src + dst_ratio * dst + EPSILON);

    if (!modeAffectsSourceAlpha(layer.mode)) {
        new_a = dst_a;
    }

    image.setPixel(m, n, qRgba(new_g, new_g, new_g, new_a));
    return true;
}

/*!
 * Merge an Indexed pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeIndexedToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    int src = layer.image_tiles[j][i].pixelIndex(k, l);
    image.setPixel(m, n, src);
    return true;
}

/*!
 * Merge an IndexedA pixel from the layer to the Indexed image. Straight-forward.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeIndexedAToIndexed(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    uchar src = layer.image_tiles[j][i].pixelIndex(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    if (src_a > 127) {
        src++;
        image.setPixel(m, n, src);
    }
    return true;
}

/*!
 * Merge an IndexedA pixel from the layer to an RGB image. Straight-forward.
 * The only thing this has to take account of is the opacity of the
 * layer. Evidently, the GIMP exporter itself does not actually do this.
 * \param layer source layer.
 * \param i x tile index.
 * \param j y tile index.
 * \param k x pixel index of tile i,j.
 * \param l y pixel index of tile i,j.
 * \param image destination image.
 * \param m x pixel of destination image.
 * \param n y pixel of destination image.
 */
bool XCFImageFormat::mergeIndexedAToRGB(const Layer &layer, uint i, uint j, int k, int l, QImage &image, int m, int n)
{
    QRgb src = layer.image_tiles[j][i].pixel(k, l);
    uchar src_a = layer.alpha_tiles[j][i].pixelIndex(k, l);
    src_a = INT_MULT(src_a, layer.opacity);

    // Apply the mask (if any)
    if (layer.apply_mask == 1 && layer.mask_tiles.size() > (int)j && layer.mask_tiles[j].size() > (int)i) {
        src_a = INT_MULT(src_a, layer.mask_tiles[j][i].pixelIndex(k, l));
    }

    // This is what appears in the GIMP window
    if (src_a <= 127) {
        src_a = 0;
    } else {
        src_a = OPAQUE_OPACITY;
    }

    image.setPixel(m, n, qRgba(src, src_a));
    return true;
}

/*!
 * Dissolving pixels: pick a random number between 0 and 255. If the pixel's
 * alpha is less than that, make it transparent.
 * \param image the image tile to dissolve.
 * \param x the global x position of the tile.
 * \param y the global y position of the tile.
 */
void XCFImageFormat::dissolveRGBPixels(QImage &image, int x, int y)
{
    // The apparently spurious rand() calls are to wind the random
    // numbers up to the same point for each tile.

    for (int l = 0; l < image.height(); l++) {
        unsigned int next = randomTable.values[(l + y) % RANDOM_TABLE_SIZE];

        for (int k = 0; k < x; k++) {
            RandomTable::rand_r(&next);
        }

        for (int k = 0; k < image.width(); k++) {
            int rand_val = RandomTable::rand_r(&next) & 0xff;
            QRgb pixel = image.pixel(k, l);

            if (rand_val > qAlpha(pixel)) {
                image.setPixel(k, l, qRgba(pixel, 0));
            }
        }
    }
}

/*!
 * Dissolving pixels: pick a random number between 0 and 255. If the pixel's
 * alpha is less than that, make it transparent. This routine works for
 * the GRAYA and INDEXEDA image types where the pixel alpha's are stored
 * separately from the pixel themselves.
 * \param image the alpha tile to dissolve.
 * \param x the global x position of the tile.
 * \param y the global y position of the tile.
 */
void XCFImageFormat::dissolveAlphaPixels(QImage &image, int x, int y)
{
    // The apparently spurious rand() calls are to wind the random
    // numbers up to the same point for each tile.

    for (int l = 0; l < image.height(); l++) {
        unsigned int next = randomTable.values[(l + y) % RANDOM_TABLE_SIZE];

        for (int k = 0; k < x; k++) {
            RandomTable::rand_r(&next);
        }

        for (int k = 0; k < image.width(); k++) {
            int rand_val = RandomTable::rand_r(&next) & 0xff;
            uchar alpha = image.pixelIndex(k, l);

            if (rand_val > alpha) {
                image.setPixel(k, l, 0);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

XCFHandler::XCFHandler()
{
}

bool XCFHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("xcf");
        return true;
    }
    return false;
}

bool XCFHandler::read(QImage *image)
{
    XCFImageFormat xcfif;
    auto ok = xcfif.readXCF(device(), image);
    m_imageSize = image->size();
    return ok;
}

bool XCFHandler::write(const QImage &)
{
    return false;
}

bool XCFHandler::supportsOption(ImageOption option) const
{
    if (option == QImageIOHandler::Size)
        return true;
    return false;
}

QVariant XCFHandler::option(ImageOption option) const
{
    QVariant v;

    if (option == QImageIOHandler::Size) {
        if (!m_imageSize.isEmpty()) {
            return m_imageSize;
        }
        /*
         * The image structure always starts at offset 0 in the XCF file.
         * byte[9]     "gimp xcf " File type identification
         * byte[4]     version     XCF version
         *                          "file": version 0
         *                          "v001": version 1
         *                          "v002": version 2
         *                          "v003": version 3
         * byte        0            Zero marks the end of the version tag.
         * uint32      width        Width of canvas
         * uint32      height       Height of canvas
         */
        else if (auto d = device()) {
            // transactions works on both random and sequential devices
            d->startTransaction();
            auto ba9 = d->read(9);      // "gimp xcf "
            auto ba5 = d->read(4+1);    // version + null terminator
            auto ba = d->read(8);       // width and height
            d->rollbackTransaction();
            if (ba9 == QByteArray("gimp xcf ") && ba5.size() == 5) {
                QDataStream ds(ba);
                quint32 width;
                ds >> width;
                quint32 height;
                ds >> height;
                if (ds.status() == QDataStream::Ok)
                    v = QVariant::fromValue(QSize(width, height));
            }
        }
    }

    return v;
}

bool XCFHandler::canRead(QIODevice *device)
{
    if (!device) {
        qCDebug(XCFPLUGIN) << "XCFHandler::canRead() called with no device";
        return false;
    }
    if (device->isSequential()) {
        return false;
    }

    const qint64 oldPos = device->pos();
    if (!device->isSequential()) {
        QDataStream ds(device);
        XCFImageFormat::XCFImage::Header header;
        bool failed = !XCFImageFormat::readXCFHeader(ds, &header);
        ds.setDevice(nullptr);
        device->seek(oldPos);
        if (failed) {
            return false;
        }

        switch (header.precision) {
        case XCFImageFormat::GIMP_PRECISION_HALF_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_HALF_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_HALF_PERCEPTUAL:
        case XCFImageFormat::GIMP_PRECISION_FLOAT_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_FLOAT_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_FLOAT_PERCEPTUAL:
        case XCFImageFormat::GIMP_PRECISION_U8_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U8_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U8_PERCEPTUAL:
        case XCFImageFormat::GIMP_PRECISION_U16_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U16_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U16_PERCEPTUAL:
        case XCFImageFormat::GIMP_PRECISION_U32_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U32_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_U32_PERCEPTUAL:
            break;
        case XCFImageFormat::GIMP_PRECISION_DOUBLE_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_DOUBLE_NON_LINEAR:
        case XCFImageFormat::GIMP_PRECISION_DOUBLE_PERCEPTUAL:
        default:
            qCDebug(XCFPLUGIN) << "unsupported precision" << header.precision;
            return false;
        }

        return true;
    }

    char head[8];
    qint64 readBytes = device->read(head, sizeof(head));
    if (readBytes != sizeof(head)) {
        if (device->isSequential()) {
            while (readBytes > 0) {
                device->ungetChar(head[readBytes-- - 1]);
            }
        } else {
            device->seek(oldPos);
        }
        return false;
    }

    if (device->isSequential()) {
        while (readBytes > 0) {
            device->ungetChar(head[readBytes-- - 1]);
        }
    } else {
        device->seek(oldPos);
    }

    return qstrncmp(head, "gimp xcf", 8) == 0;
}

QImageIOPlugin::Capabilities XCFPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "xcf") {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && XCFHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *XCFPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new XCFHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

// Just so I can get enum values printed
#include "xcf.moc"

#include "moc_xcf_p.cpp"
