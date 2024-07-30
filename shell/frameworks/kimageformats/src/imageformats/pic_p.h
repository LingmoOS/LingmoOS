/*
    PIC_RW - Qt PIC Support
    SPDX-FileCopyrightText: 2007 Ruben Lopez <r.lopez@bren.es>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_PIC_P_H
#define KIMG_PIC_P_H

#include <QDataStream>
#include <QImageIOPlugin>

/**
 * The magic number at the start of a SoftImage PIC file.
 */
static const qint32 PIC_MAGIC_NUMBER = 0x5380f634;

/**
 * How fields are distributed over the image.
 *
 * This information is not used by this image format code.
 */
enum PicFields {
    NoPicture = 0, /**< No picture */
    OddScanlines = 1, /**< Odd scanlines */
    EvenScanlines = 2, /**< Even scanlines */
    BothScanlines = 3, /**< Every scanline */
};

/**
 * How the data for a channel is encoded.
 */
enum PicChannelEncoding {
    Uncompressed = 0, /**< Image is uncompressed */
    MixedRLE = 2, /**< Run length compression */
};

/**
 * What components are encoded in a channel.
 */
enum PicChannelCode {
    RED = 0x80, /**< Red channel */
    GREEN = 0x40, /**< Green channel */
    BLUE = 0x20, /**< Blue channel */
    ALPHA = 0x10, /**< Alpha channel */
};

/**
 * The header for a SoftImage PIC file.
 *
 * @private
 */
struct PicHeader {
    /**
     * Construct a valid header for a SoftImage PIC file.
     *
     * Note that the comment will be truncated to 80 bytes when written.
     *
     * @param _width    The width of the image in pixels
     * @param _height   The height of the image in pixels
     * @param _comment  A comment to add to the image
     */
    PicHeader(quint16 _width, quint16 _height, const QByteArray &_comment = QByteArray())
        : magic(PIC_MAGIC_NUMBER)
        , version(3.71f)
        , comment(_comment)
        , id("PICT")
        , width(_width)
        , height(_height)
        , ratio(1.0f)
        , fields(BothScanlines)
    {
    }
    /** Construct an invalid header. */
    PicHeader()
    {
    }

    quint32 magic; /**< Should be PIC_MAGIC_NUMBER */
    float version; /**< Version of something (header? file format?) (ignored) */
    QByteArray comment; /**< A free comment field (truncated to 80 bytes when
                             written) */
    QByteArray id; /**< The file format ID (should be "PICT") */
    quint16 width; /**< The width of the image in pixels */
    quint16 height; /**< The height of the image in pixels */
    float ratio; /**< The aspect ratio: width/height of each individual pixel
                      (ignored) */
    PicFields fields; /**< The interlace type (ignored) */

    /**
     * Returns true if the @p magic and @p id fields are set correctly.
     */
    bool isValid() const
    {
        return magic == PIC_MAGIC_NUMBER && id == "PICT";
    }

    /**
     * The length of the encoded data, in bytes.
     */
    static const qint64 encodedLength = 104;
};

/**
 * Describes a channel in a SoftImage PIC file.
 *
 * @private
 */
struct PicChannel {
    quint8 size; /**< Bits per component per pixel. */
    quint8 encoding; /**< How the channel's data is encoded. */
    quint8 code; /**< Flag field to describe which components are encoded in
                      this channel. */

    /**
     * Constructs a channel description for a SoftImage PIC file.
     *
     * @param _encoding  How the channel's data is or will be encoded.
     * @param _code      What components are or will be encoded by this
     *                   channel.
     * @param _size      The number of bits used to encoded a single component
     *                   for a single pixel in this channel (should be 8).
     */
    PicChannel(PicChannelEncoding _encoding, quint8 _code, quint8 _size = 8)
        : size(_size)
        , encoding(_encoding)
        , code(_code)
    {
    }
    /**
     * Constructs a default channel description for a SoftImage PIC file.
     *
     * This will have size set to 8, encoding set to Uncompressed and the code
     * set to 0 (so that the channel does not encode any information).
     *
     * The result of this should not be written to a file without setting the
     * encoding and channel fields correctly.
     */
    PicChannel()
        : size(8)
    {
    }
};

class SoftimagePICHandler : public QImageIOHandler
{
public:
    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &) override;

    QVariant option(ImageOption option) const override;
    void setOption(ImageOption option, const QVariant &value) override;
    bool supportsOption(ImageOption option) const override;

    static bool canRead(QIODevice *device);

    enum State {
        Error,
        Ready,
        ReadHeader,
        ReadChannels,
    };

    SoftimagePICHandler()
        : m_state(Ready)
        , m_compression(true)
    {
    }

    bool readHeader();
    bool readChannels();

private:
    State m_state;
    QDataStream m_dataStream;
    PicHeader m_header;
    QList<PicChannel> m_channels;
    // mostly used for writing:
    bool m_compression;
    QByteArray m_description;
};

class SoftimagePICPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "pic.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_PIC_P_H
