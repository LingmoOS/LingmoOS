/*
    RAW support for QImage.

    SPDX-FileCopyrightText: 2022 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "raw_p.h"
#include "util_p.h"

#include <QColorSpace>
#include <QDateTime>
#include <QDebug>
#include <QImage>
#include <QSet>
#include <QTimeZone>

#if defined(Q_OS_WINDOWS) && !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <libraw/libraw.h>

#ifdef QT_DEBUG
// This should be used to exclude the local QIODevice wrapper of the LibRaw_abstract_datastream interface.
// If the result changes then the problem is in the local wrapper and must be corrected.
// WARNING: using the LibRaw's streams instead of the local wrapper from a Qt program falls in the LOCALE
//          bug when the RAW file needs the scanf_one() function (e.g. *.MOS files). See also raw_scanf_one().
//#define EXCLUDE_LibRaw_QIODevice // Uncomment this code if you think that the problem is LibRaw_QIODevice (default commented)
#endif

namespace // Private.
{

// smart pointer for processed image
using pi_unique_ptr = std::unique_ptr<libraw_processed_image_t, std::function<void(libraw_processed_image_t *)>>;

// clang-format off
// Known formats supported by LibRaw (in alphabetical order and lower case)
const auto supported_formats = QSet<QByteArray>{
    "3fr",
    "arw", "arq",
    "bay", "bmq",
    "cr2", "cr3", "cap", "cine", "cs1", "crw",
    "dcs", "dc2", "dcr", "dng", "drf", "dxo",
    "eip", "erf",
    "fff",
    "iiq",
    "k25", "kc2", "kdc",
    "mdc", "mef", "mfw", "mos", "mrw",
    "nef", "nrw",
    "obm", "orf", "ori",
    "pef", "ptx", "pxn",
    "qtk",
    "r3d", "raf", "raw", "rdc", "rw2", "rwl", "rwz",
    "sr2", "srf", "srw", "sti",
    "x3f"
};
// clang-format on

inline int raw_scanf_one(const QByteArray &ba, const char *fmt, void *val)
{
    // WARNING: Here it would be nice to use sscanf like LibRaw does but there
    //          is a Big Trouble: THE LOCALE! LibRaw is also affected by the
    //          issue if used in a Qt program:
    //          If you use sscanf here the conversion is wrong when performed
    //          with Italian locale (where the decimal separator is the comma).
    // The solution should be to use std::locale::global() but it's global!
    // I don't want to do it. So, don't use code like that:
    // return sscanf(QByteArray(ba).append('\0').data(), fmt, val);

    // LibRaw is asking only "%d" and "%f" for now. This code is not affected
    // by the LOCALE bug.
    auto s = QString::fromLatin1(ba);
    if (strcmp(fmt, "%d") == 0) {
        auto ok = false;
        auto d = QLocale::c().toInt(s, &ok);
        if (!ok) {
            return EOF;
        }
        *(static_cast<int *>(val)) = d;
    } else {
        auto ok = false;
        auto f = QLocale::c().toFloat(s, &ok);
        if (!ok) {
            return EOF;
        }
        *(static_cast<float *>(val)) = f;
    }
    return 1;
}

/**
 * @brief The LibRaw_QIODevice class
 * Implementation of the LibRaw stream interface over a QIODevice.
 */
class LibRaw_QIODevice : public LibRaw_abstract_datastream
{
public:
    explicit LibRaw_QIODevice(QIODevice *device)
    {
        m_device = device;
    }
    virtual ~LibRaw_QIODevice() override
    {
    }
    virtual int valid() override
    {
        return m_device != nullptr;
    }
    virtual int read(void *ptr, size_t sz, size_t nmemb) override
    {
        qint64 read = 0;
        if (sz == 0) {
            return 0;
        }
        auto data = reinterpret_cast<char*>(ptr);
        for (qint64 r = 0, size = sz * nmemb; read < size; read += r) {
            if (m_device->atEnd()) {
                break;
            }
            r = m_device->read(data + read, size - read);
            if (r < 1) {
                break;
            }
        }
        return read / sz;
    }
    virtual int eof() override
    {
        return m_device->atEnd() ? 1 : 0;
    }
    virtual int seek(INT64 o, int whence) override
    {
        auto pos = o;
        auto size = m_device->size();
        if (whence == SEEK_CUR) {
            pos = m_device->pos() + o;
        }
        if (whence == SEEK_END) {
            pos = size + o;
        }
        if (pos < 0 || m_device->isSequential()) {
            return -1;
        }
        return m_device->seek(pos) ? 0 : -1;
    }
    virtual INT64 tell() override
    {
        return m_device->pos();
    }
    virtual INT64 size() override
    {
        return m_device->size();
    }
    virtual char *gets(char *s, int sz) override
    {
        if (m_device->readLine(s, sz) > 0) {
            return s;
        }
        return nullptr;
    }
    virtual int scanf_one(const char *fmt, void *val) override
    {
        QByteArray ba;
        for (int xcnt = 0; xcnt < 24 && !m_device->atEnd(); ++xcnt) {
            char c;
            if (!m_device->getChar(&c)) {
                return EOF;
            }
            if (ba.isEmpty() && (c == ' ' || c == '\t')) {
                continue;
            }
            if (c == '\0' || c == ' ' || c == '\t' || c == '\n') {
                break;
            }
            ba.append(c);
        }
        return raw_scanf_one(ba, fmt, val);
    }
    virtual int get_char() override
    {
        unsigned char c;
        if (!m_device->getChar(reinterpret_cast<char *>(&c))) {
            return EOF;
        }
        return int(c);
    }
#if (LIBRAW_VERSION < LIBRAW_MAKE_VERSION(0, 21, 0)) || defined(LIBRAW_OLD_VIDEO_SUPPORT)
    virtual void *make_jas_stream() override
    {
        return nullptr;
    }
#endif

private:
    QIODevice *m_device;
};

bool addTag(const QString &tag, QStringList &lines)
{
    auto ok = !tag.isEmpty();
    if (ok) {
        lines << tag;
    }
    return ok;
}

QString createTag(QString value, const char *tag)
{
    if (!value.isEmpty()) {
        value = QStringLiteral("<%1>%2</%1>").arg(QString::fromLatin1(tag), value);
    }
    return value;
}

QString createTag(char *asciiz, const char *tag)
{
    auto value = QString::fromUtf8(asciiz);
    return createTag(value, tag);
}

QString createTimeTag(time_t time, const char *tag)
{
    auto value = QDateTime::fromSecsSinceEpoch(time, QTimeZone::utc());
    if (value.isValid() && time > 0) {
        return createTag(value.toString(Qt::ISODate), tag);
    }
    return QString();
}

QString createFlashTag(short flash, const char *tag)
{
    QStringList l;
    auto lc = QLocale::c();
    // EXIF specifications
    auto t = QStringLiteral("true");
    auto f = QStringLiteral("false");
    l << QStringLiteral("<exif:Fired>%1</exif:Fired>").arg((flash & 1) ? t : f);
    l << QStringLiteral("<exif:Function>%1</exif:Function>").arg((flash & (1 << 5)) ? t : f);
    l << QStringLiteral("<exif:RedEyeMode>%1</exif:RedEyeMode>").arg((flash & (1 << 6)) ? t : f);
    l << QStringLiteral("<exif:Mode>%1</exif:Mode>").arg(lc.toString((int(flash) >> 3) & 3));
    l << QStringLiteral("<exif:Return>%1</exif:Return>").arg(lc.toString((int(flash) >> 1) & 3));
    return createTag(l.join(QChar()), tag);
}

QString createTag(quint64 n, const char *tag, quint64 invalid = 0)
{
    if (n != invalid) {
        return createTag(QLocale::c().toString(n), tag);
    }
    return QString();
}

QString createTag(qint16 n, const char *tag, qint16 invalid = 0)
{
    if (n != invalid) {
        return createTag(QLocale::c().toString(n), tag);
    }
    return QString();
}

QString createTag(quint16 n, const char *tag, quint16 invalid = 0)
{
    if (n != invalid) {
        return createTag(QLocale::c().toString(n), tag);
    }
    return QString();
}

QString createTag(float f, const char *tag, qint32 mul = 1)
{
    if (f != 0) {
        if (mul > 1)
            return QStringLiteral("<%1>%2/%3</%1>").arg(QString::fromLatin1(tag), QLocale::c().toString(int(f * mul))).arg(mul);
        return QStringLiteral("<%1>%2</%1>").arg(QString::fromLatin1(tag), QLocale::c().toString(f));
    }
    return QString();
}

QString createTag(libraw_gps_info_t gps, const char *tag)
{
    auto tmp = QString::fromLatin1(tag);
    if (tmp.contains(QStringLiteral("Latitude"), Qt::CaseInsensitive)) {
        if (gps.latref != '\0') {
            auto lc = QLocale::c();
            auto value = QStringLiteral("%1,%2%3")
                             .arg(lc.toString(gps.latitude[0], 'f', 0))
                             .arg(lc.toString(gps.latitude[1] + gps.latitude[2] / 60, 'f', 4))
                             .arg(QChar::fromLatin1(gps.latref));
            return createTag(value, tag);
        }
    }
    if (tmp.contains(QStringLiteral("Longitude"), Qt::CaseInsensitive)) {
        if (gps.longref != '\0') {
            auto lc = QLocale::c();
            auto value = QStringLiteral("%1,%2%3")
                             .arg(lc.toString(gps.longitude[0], 'f', 0))
                             .arg(lc.toString(gps.longitude[1] + gps.longitude[2] / 60, 'f', 4))
                             .arg(QChar::fromLatin1(gps.longref));
            return createTag(value, tag);
        }
    }
    if (tmp.contains(QStringLiteral("Altitude"), Qt::CaseInsensitive)) {
        if (gps.altitude != 0) {
            return createTag(gps.altitude, tag, 1000);
        }
    }
    return QString();
}

QString createXmpPacket()
{
    QStringList lines;
    lines << QStringLiteral("<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>");
    lines << QStringLiteral("<x:xmpmeta xmlns:x=\"adobe:ns:meta/\" x:xmptk=\"KIMG RAW Plugin\">");
    lines << QStringLiteral("<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">");
    lines << QStringLiteral("</rdf:RDF>");
    lines << QStringLiteral("</x:xmpmeta>");
    for (auto i = 30; i > 0; --i)
        lines << QString::fromLatin1(QByteArray(80, ' '));
    lines << QStringLiteral("<?xpacket end=\"w\"?>");
    return lines.join(QChar::fromLatin1('\n'));
}

QString updateXmpPacket(const QString &xmpPacket, LibRaw *rawProcessor)
{
    auto rdfEnd = xmpPacket.indexOf(QStringLiteral("</rdf:RDF>"), Qt::CaseInsensitive);
    if (rdfEnd < 0) {
        return updateXmpPacket(createXmpPacket(), rawProcessor);
    }

    auto lines = QStringList() << xmpPacket.left(rdfEnd);
    lines << QStringLiteral("<rdf:Description rdf:about=\"\"");
    lines << QStringLiteral("      xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\"");
    lines << QStringLiteral("      xmlns:dc=\"http://purl.org/dc/elements/1.1/\"");
    lines << QStringLiteral("      xmlns:aux=\"http://ns.adobe.com/exif/1.0/aux/\"");
    lines << QStringLiteral("      xmlns:xmpMM=\"http://ns.adobe.com/xap/1.0/mm/\"");
    lines << QStringLiteral("      xmlns:stEvt=\"http://ns.adobe.com/xap/1.0/sType/ResourceEvent#\"");
    lines << QStringLiteral("      xmlns:stRef=\"http://ns.adobe.com/xap/1.0/sType/ResourceRef#\"");
    lines << QStringLiteral("      xmlns:tiff=\"http://ns.adobe.com/tiff/1.0/\"");
    lines << QStringLiteral("      xmlns:exif=\"http://ns.adobe.com/exif/1.0/\"");
    lines << QStringLiteral("      xmlns:xmpRights=\"http://ns.adobe.com/xap/1.0/rights/\">");
    lines << QStringLiteral("<xmpMM:History>");
    lines << QStringLiteral("<rdf:Seq>");
    lines << QStringLiteral("<rdf:li rdf:parseType=\"Resource\">");
    lines << QStringLiteral("<stEvt:action>converted</stEvt:action>");
    lines << QStringLiteral("<stEvt:parameters>Converted from RAW to Qt Image using KIMG RAW plugin</stEvt:parameters>");
    lines << QStringLiteral("<stEvt:softwareAgent>LibRaw %1</stEvt:softwareAgent>").arg(QString::fromLatin1(LibRaw::version()));
    lines << QStringLiteral("<stEvt:when>%1</stEvt:when>").arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    lines << QStringLiteral("</rdf:li>");
    lines << QStringLiteral("</rdf:Seq>");
    lines << QStringLiteral("</xmpMM:History>");

    auto &&iparams = rawProcessor->imgdata.idata;
    addTag(createTag(iparams.normalized_model, "tiff:Model"), lines);
    addTag(createTag(iparams.normalized_make, "tiff:Make"), lines);
    addTag(createTag(iparams.software, "xmp:CreatorTool"), lines);

    auto &&iother = rawProcessor->imgdata.other;
    addTag(createTag(createTag(createTag(iother.desc, "rdf:li"), "rdf:Alt"), "dc:description"), lines);
    addTag(createTag(createTag(createTag(iother.artist, "rdf:li"), "rdf:Seq"), "dc:creator"), lines);
    addTag(createTag(createTag(createTag(iother.iso_speed, "rdf:li"), "rdf:Seq"), "exif:ISOSpeedRatings"), lines);
    addTag(createTag(iother.shutter, "exif:ExposureTime", 1000), lines);
    addTag(createTag(iother.aperture, "exif:ApertureValue", 1000), lines);
    addTag(createTag(iother.focal_len, "exif:FocalLength", 1000), lines);
    addTag(createTimeTag(iother.timestamp, "xmp:CreateDate"), lines);
    addTag(createTag(iother.parsed_gps, "exif:GPSLatitude"), lines);
    addTag(createTag(iother.parsed_gps, "exif:GPSLongitude"), lines);
    addTag(createTag(iother.parsed_gps, "exif:GPSAltitude"), lines);

    auto &&shotinfo = rawProcessor->imgdata.shootinginfo;
    addTag(createTag(shotinfo.ExposureMode, "exif:ExposureMode", short(-1)), lines);
    addTag(createTag(shotinfo.MeteringMode, "exif:MeteringMode", short(-1)), lines);
    addTag(createTag(shotinfo.BodySerial, "aux:SerialNumber"), lines);

    auto &&color = rawProcessor->imgdata.color;
    addTag(createFlashTag(color.flash_used, "exif:Flash"), lines);

    auto &&lens = rawProcessor->imgdata.lens;
    addTag(createTag(lens.FocalLengthIn35mmFormat, "exif:FocalLengthIn35mmFilm"), lines);
    addTag(createTag(lens.Lens, "aux:Lens"), lines);
    addTag(createTag(lens.LensSerial, "aux:LensSerialNumber"), lines);
    addTag(createTag(lens.nikon.LensIDNumber ? quint64(lens.nikon.LensIDNumber) : lens.makernotes.LensID, "aux:LensID"), lines);

    auto &&makernotes = rawProcessor->imgdata.makernotes;
    addTag(createTag(makernotes.common.firmware, "aux:Firmware"), lines);

    lines << QStringLiteral("</rdf:Description>");
    lines << xmpPacket.mid(rdfEnd);

    return lines.join(QChar::fromLatin1('\n'));
}

template<class T>
inline void rgbToRgbX(uchar *target, const uchar *source, qint32 targetSize, qint32 sourceSize)
{
    auto s = reinterpret_cast<const T *>(source);
    auto t = reinterpret_cast<T *>(target);
    auto width = std::min(targetSize / 4, sourceSize / 3) / qint32(sizeof(T));
    for (qint32 x = 0; x < width; ++x) {
        t[x * 4 + 0] = s[x * 3 + 0];
        t[x * 4 + 1] = s[x * 3 + 1];
        t[x * 4 + 2] = s[x * 3 + 2];
        t[x * 4 + 3] = std::numeric_limits<T>::max();
    }
}

// clang-format off
#define C_IQ(a) (((a) & 0xF) << 4)
#define C_OC(a) (((a) & 0xF) << 8)
#define C_CW(a) (((a) & 0x1) << 12)
#define C_AW(a) (((a) & 0x1) << 13)
#define C_BT(a) (((a) & 0x1) << 14)
#define C_HS(a) (((a) & 0x1) << 15)
#define C_CE(a) (((a) & 0x1) << 16)
#define C_NR(a) (((a) & 0x3) << 17)
#define C_FC(a) (((a) & 0x1) << 19)
#define C_SR(a) (((a) & 0x1) << 20)
#define C_FLAGS(a) (((a) & 0x1) << 31) // flags mode

#define T_IQ(a) (((a) >> 4) & 0xF)
#define T_OC(a) (((a) >> 8) & 0xF)
#define T_CW(a) (((a) >> 12) & 0x1)
#define T_AW(a) (((a) >> 13) & 0x1)
#define T_BT(a) (((a) >> 14) & 0x1)
#define T_HS(a) (((a) >> 15) & 0x1)
#define T_CE(a) (((a) >> 16) & 0x1)
#define T_NR(a) (((a) >> 17) & 0x3)
#define T_FC(a) (((a) >> 19) & 0x1)
#define T_SR(a) (((a) >> 20) & 0x1)
#define T_FLAGS(a) (((a) >> 31) & 0x1)
// clang-format on

#define DEFAULT_IMAGE_QUALITY (C_IQ(3) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0) | C_FLAGS(1))

void setParams(QImageIOHandler *handler, LibRaw *rawProcessor)
{
    // *** Set raw params
#if (LIBRAW_VERSION < LIBRAW_MAKE_VERSION(0, 21, 0))
    auto &&rawparams = rawProcessor->imgdata.params;
#else
    auto &&rawparams = rawProcessor->imgdata.rawparams;
#endif
    // Select one raw image from input file (0 - first, ...)
    if (handler->currentImageNumber() > -1) {
        rawparams.shot_select = handler->currentImageNumber();
    }

    // *** Set processing parameters

    // NOTE: The default value set below are the ones that gave the best results
    // on a large sample of images (https://raw.pixls.us/data/)

    /**
     * @brief quality
     * Plugin quality option.
     */
    qint32 quality = -1;
    if (handler->supportsOption(QImageIOHandler::Quality)) {
        quality = handler->option(QImageIOHandler::Quality).toInt();
    }
    if (quality > -1) {
        switch (quality / 10) {
        case 0:
            quality = C_IQ(0) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(0) | C_HS(1);
            break;
        case 1:
            quality = C_IQ(0) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(0) | C_HS(0);
            break;
        case 2:
            quality = C_IQ(3) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(0) | C_HS(0);
            break;
        case 3:
            quality = C_IQ(3) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        case 4:
            quality = C_IQ(3) | C_OC(2) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        case 5:
            quality = C_IQ(3) | C_OC(4) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        case 6:
            quality = C_IQ(11) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(0) | C_HS(0);
            break;
        case 7:
            quality = C_IQ(11) | C_OC(1) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        case 8:
            quality = C_IQ(11) | C_OC(2) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        default:
            quality = C_IQ(11) | C_OC(4) | C_CW(1) | C_AW(1) | C_BT(1) | C_HS(0);
            break;
        }
        quality |= C_FLAGS(1);
    }
    if (quality == -1) {
        quality = DEFAULT_IMAGE_QUALITY;
    }
    Q_ASSERT(T_FLAGS(quality));

    auto &&params = rawProcessor->imgdata.params;

    /**
     * @brief use_camera_wb
     * Use camera white balance, if possible (0 - off, 1 - on)
     *
     * This should to be set. Alternatively, a calibrated white balance should be set on each camera.
     */
    params.use_camera_wb = T_CW(quality);

    /*!
     * \brief use_auto_wb
     * Average the whole image for white balance (0 - off, 1 - on)
     *
     * This is usefull if no camera white balance is available.
     */
    params.use_auto_wb = T_AW(quality);

    /**
     * @brief output_bps
     * Bits per pixel (8 or 16)
     *
     * Professional cameras (and even some smartphones) generate images at 10 or more bits per sample.
     * When using 16-bit images, the highest quality should be maintained.
     */
    params.output_bps = T_BT(quality) ? 16 : 8;

    /**
     * @brief output_color
     * Output colorspace (0 - raw, 1 - sRGB, 2 - Adobe, 3 - Wide, 4 - ProPhoto, 5 - XYZ, 6 - ACES, 7 - DCI-P3, 8 - Rec2020)
     *
     * sRGB allows you to view images correctly on programs that do not support ICC profiles. When most
     * Programs will support icc profiles, ProPhoto may be a better choice.
     * @note sRgb is the LibRaw default: if grayscale image is loaded, LibRaw switches to 0 (Raw) automatically.
     */
    params.output_color = T_OC(quality);

    /**
     * @brief user_qual
     * Interpolation quality (0 - linear, 1 - VNG, 2 - PPG, 3 - AHD, 4 - DCB, 11 - DHT, 12 - AAHD)
     *
     * DHT seems the best option - See In-Depth Demosaicing Algorithm Analysis (https://www.libraw.org/node/2306)
     * but, when used, some FUJI RAF files of my library are poorly rendered (e.g. completely green). This is the
     * why I used AHD: a good compromise between quality and performance with no rendering errors.
     */
    params.user_qual = T_IQ(quality);

    /**
     * @brief half_size
     * Generate an half-size image (0 - off, 1 - on)
     *
     * Very fast and useful for generating previews.
     */
    params.half_size = T_HS(quality);

    /**
     * @dcb_enhance_fl
     * DCB color enhance filter (0 - off, 1 - on)
     */
    params.dcb_enhance_fl = T_CE(quality);

    /**
     * @fbdd_noiserd
     * FBDD noise reduction (0 - off, 1 - light, 2 - full)
     */
    params.fbdd_noiserd = std::min(2, T_NR(quality));

    /**
     * @four_color_rgb
     * Interpolate RGGB as four colors (0 - off, 1 - on)
     */
    params.four_color_rgb = T_FC(quality);

    /**
     * @use_fuji_rotate
     * Don't stretch or rotate raw pixels (0 - off, 1 - on)
     */
    params.use_fuji_rotate = T_SR(quality) ? 0 : 1;
}

bool LoadRAW(QImageIOHandler *handler, QImage &img)
{
    std::unique_ptr<LibRaw> rawProcessor(new LibRaw);

    // *** Set parameters
    setParams(handler, rawProcessor.get());

    // *** Open the stream
    auto device = handler->device();
#ifndef EXCLUDE_LibRaw_QIODevice
    LibRaw_QIODevice stream(device);
    if (rawProcessor->open_datastream(&stream) != LIBRAW_SUCCESS) {
        return false;
    }
#else
    auto ba = device->readAll();
    if (rawProcessor->open_buffer(ba.data(), ba.size()) != LIBRAW_SUCCESS) {
        return false;
    }
#endif

    // *** Unpacking selected image
    if (rawProcessor->unpack() != LIBRAW_SUCCESS) {
        return false;
    }

    // *** Process selected image
    if (rawProcessor->dcraw_process() != LIBRAW_SUCCESS) {
        return false;
    }

    // *** Convert to QImage
    pi_unique_ptr processedImage(rawProcessor->dcraw_make_mem_image(), LibRaw::dcraw_clear_mem);
    if (processedImage == nullptr) {
        return false;
    }

    // clang-format off
    if ((processedImage->type != LIBRAW_IMAGE_BITMAP) ||
        (processedImage->colors != 1 && processedImage->colors != 3 && processedImage->colors != 4) ||
        (processedImage->bits != 8 && processedImage->bits != 16)) {
        return false;
    }
    // clang-format on

    auto format = QImage::Format_Invalid;
    switch (processedImage->colors) {
    case 1: // Gray images (tested with image attached on https://bugs.kde.org/show_bug.cgi?id=401371)
        format = processedImage->bits == 8 ? QImage::Format_Grayscale8 : QImage::Format_Grayscale16;
        break;
    case 3: // Images with R G B components
        format = processedImage->bits == 8 ? QImage::Format_RGB888 : QImage::Format_RGBX64;
        break;
    case 4: // Images with R G B components + Alpha (never seen)
        format = processedImage->bits == 8 ? QImage::Format_RGBA8888 : QImage::Format_RGBA64;
        break;
    }

    if (format == QImage::Format_Invalid) {
        return false;
    }

    img = imageAlloc(processedImage->width, processedImage->height, format);
    if (img.isNull()) {
        return false;
    }

    auto rawBytesPerLine = qint32(processedImage->width * processedImage->bits * processedImage->colors + 7) / 8;
    auto lineSize = std::min(qint32(img.bytesPerLine()), rawBytesPerLine);
    for (int y = 0, h = img.height(); y < h; ++y) {
        auto scanline = img.scanLine(y);
        if (format == QImage::Format_RGBX64)
            rgbToRgbX<quint16>(scanline, processedImage->data + rawBytesPerLine * y, img.bytesPerLine(), rawBytesPerLine);
        else
            memcpy(scanline, processedImage->data + rawBytesPerLine * y, lineSize);
    }

    // *** Set the color space
    auto &&params = rawProcessor->imgdata.params;
    if (params.output_color == 0) {
        auto &&color = rawProcessor->imgdata.color;
        if (auto profile = reinterpret_cast<char *>(color.profile)) {
            img.setColorSpace(QColorSpace::fromIccProfile(QByteArray(profile, color.profile_length)));
        }
    }
    if (processedImage->colors >= 3) {
        if (params.output_color == 1) {
            img.setColorSpace(QColorSpace(QColorSpace::SRgb));
        }
        if (params.output_color == 2) {
            img.setColorSpace(QColorSpace(QColorSpace::AdobeRgb));
        }
        if (params.output_color == 4) {
            img.setColorSpace(QColorSpace(QColorSpace::ProPhotoRgb));
        }
        if (params.output_color == 7) {
            img.setColorSpace(QColorSpace(QColorSpace::DisplayP3));
        }
    }

    // *** Set the metadata
    auto &&iparams = rawProcessor->imgdata.idata;

    auto xmpPacket = QString();
    if (auto xmpdata = iparams.xmpdata) {
        if (auto xmplen = iparams.xmplen)
            xmpPacket = QString::fromUtf8(xmpdata, xmplen);
    }
    // Add info from LibRAW structs (e.g. GPS position, info about lens, info about shot and flash, etc...)
    img.setText(QStringLiteral(META_KEY_XMP_ADOBE), updateXmpPacket(xmpPacket, rawProcessor.get()));

    auto model = QString::fromUtf8(iparams.normalized_model);
    if (!model.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_MODEL), model);
    }
    auto manufacturer = QString::fromUtf8(iparams.normalized_make);
    if (!manufacturer.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_MANUFACTURER), manufacturer);
    }
    auto software = QString::fromUtf8(iparams.software);
    if (!software.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_SOFTWARE), software);
    }

    auto &&iother = rawProcessor->imgdata.other;
    auto description = QString::fromUtf8(iother.desc);
    if (!description.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_DESCRIPTION), description);
    }
    auto artist = QString::fromUtf8(iother.artist);
    if (!artist.isEmpty()) {
        img.setText(QStringLiteral(META_KEY_AUTHOR), artist);
    }

    return true;
}

} // Private

RAWHandler::RAWHandler()
    : m_imageNumber(0)
    , m_imageCount(0)
    , m_quality(-1)
    , m_startPos(-1)
{
}

bool RAWHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("raw");
        return true;
    }
    return false;
}

bool RAWHandler::read(QImage *image)
{
    auto dev = device();

    // set the image position after the first run.
    if (!dev->isSequential()) {
        if (m_startPos < 0) {
            m_startPos = dev->pos();
        } else {
            dev->seek(m_startPos);
        }
    }

    // Check image file format.
    if (dev->atEnd()) {
        return false;
    }

    QImage img;
    if (!LoadRAW(this, img)) {
        return false;
    }

    *image = img;
    return true;
}

void RAWHandler::setOption(ImageOption option, const QVariant &value)
{
    if (option == QImageIOHandler::Quality) {
        bool ok = false;
        auto q = value.toInt(&ok);
        if (ok) {
            m_quality = q;
        }
    }
}

bool RAWHandler::supportsOption(ImageOption option) const
{
#ifndef EXCLUDE_LibRaw_QIODevice
    if (option == QImageIOHandler::Size) {
        return true;
    }
#endif

    if (option == QImageIOHandler::Quality) {
        return true;
    }

    return false;
}

QVariant RAWHandler::option(ImageOption option) const
{
    QVariant v;

#ifndef EXCLUDE_LibRaw_QIODevice
    if (option == QImageIOHandler::Size) {
        auto d = device();
        d->startTransaction();
        std::unique_ptr<LibRaw> rawProcessor(new LibRaw);
        LibRaw_QIODevice stream(d);
#if (LIBRAW_VERSION < LIBRAW_MAKE_VERSION(0, 21, 0))
        rawProcessor->imgdata.params.shot_select = currentImageNumber();
#else
        rawProcessor->imgdata.rawparams.shot_select = currentImageNumber();
#endif
        if (rawProcessor->open_datastream(&stream) == LIBRAW_SUCCESS) {
            if (rawProcessor->unpack() == LIBRAW_SUCCESS) {
                auto w = libraw_get_iwidth(&rawProcessor->imgdata);
                auto h = libraw_get_iheight(&rawProcessor->imgdata);
                // flip & 4: taken from LibRaw code
                v = (rawProcessor->imgdata.sizes.flip & 4) ? QSize(h, w) : QSize(w, h);
            }
        }
        d->rollbackTransaction();
    }
#endif

    if (option == QImageIOHandler::Quality) {
        v = m_quality;
    }

    return v;
}

bool RAWHandler::jumpToNextImage()
{
    return jumpToImage(m_imageNumber + 1);
}

bool RAWHandler::jumpToImage(int imageNumber)
{
    if (imageNumber < 0 || imageNumber >= imageCount()) {
        return false;
    }
    m_imageNumber = imageNumber;
    return true;
}

int RAWHandler::imageCount() const
{
    // NOTE: image count is cached for performance reason
    auto &&count = m_imageCount;
    if (count > 0) {
        return count;
    }

    count = QImageIOHandler::imageCount();

#ifndef EXCLUDE_LibRaw_QIODevice
    auto d = device();
    d->startTransaction();

    std::unique_ptr<LibRaw> rawProcessor(new LibRaw);
    LibRaw_QIODevice stream(d);
    if (rawProcessor->open_datastream(&stream) == LIBRAW_SUCCESS) {
        count = rawProcessor->imgdata.rawdata.iparams.raw_count;
    }

    d->rollbackTransaction();
#endif

    return count;
}

int RAWHandler::currentImageNumber() const
{
    return m_imageNumber;
}

bool RAWHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("RAWHandler::canRead() called with no device");
        return false;
    }
    if (device->isSequential()) {
        return false;
    }

    device->startTransaction();

    std::unique_ptr<LibRaw> rawProcessor(new LibRaw);

#ifndef EXCLUDE_LibRaw_QIODevice
    LibRaw_QIODevice stream(device);
    auto ok = rawProcessor->open_datastream(&stream) == LIBRAW_SUCCESS;
#else
    auto ba = device->readAll();
    auto ok = rawProcessor->open_buffer(ba.data(), ba.size()) == LIBRAW_SUCCESS;
#endif

    device->rollbackTransaction();
    return ok;
}

QImageIOPlugin::Capabilities RAWPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (supported_formats.contains(QByteArray(format).toLower())) {
        return Capabilities(CanRead);
    }
    if (!format.isEmpty()) {
        return {};
    }
    if (!device->isOpen()) {
        return {};
    }

    Capabilities cap;
    if (device->isReadable() && RAWHandler::canRead(device)) {
        cap |= CanRead;
    }
    return cap;
}

QImageIOHandler *RAWPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new RAWHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

#include "moc_raw_p.cpp"
