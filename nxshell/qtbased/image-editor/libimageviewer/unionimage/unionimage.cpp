// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unionimage.h"
#include <FreeImage.h>
//#include "giflib/cmanagerattributeservice.h"

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QDate>
#include <QTime>
#include <QtMath>
#include <QMovie>
#include <QTransform>
#include <QPainter>
#include <QSvgGenerator>
#include <QImageReader>
#include <QMimeDatabase>
#include <QtSvg/QSvgRenderer>
#include <QDir>
#include <QDebug>
#include <QUuid>

#include "unionimage/imageutils.h"
extern "C" {
#include "3rdparty/tiff-tools/converttiff.h"
}

#include <cstring>

#define SAVE_QUAITY_VALUE 100

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm";

// 提供用于非线程安全函数 FreeImage_TagToString() 访问限制的锁
Q_GLOBAL_STATIC(QMutex, g_freeImageTagToStringMutex);

namespace LibUnionImage_NameSpace {

//    enum SupportFormat {
//        UNKNOWN = -1,
//        BMP     =  0,
//        ICO     =  1,
//        JPG     =  2,
//        JPE     =  2,
//        JPS     =  2,
//        JPEG    =  2,
//        JNG     =  3,
//        KOALA   =  4,
//        KOA     =  KOALA,
//        LBM     =  5,
//        IFF     =  LBM,
//        MNG     =  6,
//        PBM     =  7,
//        PBMRAW  =  8,
//        PCD     =  9,
//        PCX     =  10,
//        PGM     =  11,
//        PGMRAW  =  12,
//        PNG     =  13,
//        PPM     =  14,
//        PPMRAW  =  15,
//        RAS     =  16,
//        TGA     =  17,
//        TARGA   =  17,
//        TIFF    =  18,
//        WBMP    =  19,
//        PSD     =  20,
//        CUT     =  21,
//        XBM     =  22,
//        XPM     =  23,
//        DDS     =  24,
//        GIF     =  25,
//        FHDR    =  26,
//        FAXG3   =  27,
//        SGI     =  28,
//        EXR     =  29,
//        J2K     =  30,
//        J2C     =  30,
//        JPC     =  30,
//        JP2     =  31,
//        PFM     =  32,
//        PCT     =  33,
//        PIC     =  33,
//        PICT    =  33,
//        RAW     =  34,
//        WEBP    =  35,
//        JXR     =  36
//    };
//由于freeimage 不支持，而在使用过程中，将qt支持的格式也添加到hash里，所以mrw的value也为0，因此重新定义
//fix 52612
enum FIF_QTSUPPORT {
    FIF_MRW = 37
};

class UnionImage_Private
{



public:
    UnionImage_Private()
    {
        //FreeImage_Initialise(true);
        //m_freeimage_formats["UNKNOWN"] = -1;
        m_freeimage_formats["BMP"]     =  FIF_BMP;
        m_freeimage_formats["ICO"]     =  FIF_ICO;
        m_freeimage_formats["JPG"]     =  FIF_JPEG;
        m_freeimage_formats["JPE"]     =  FIF_JPEG;
        m_freeimage_formats["JPS"]     =  FIF_JPEG;
        m_freeimage_formats["JPEG"]    =  FIF_JPEG;
        m_freeimage_formats["JNG"]     =  FIF_JNG;
        m_freeimage_formats["KOALA"]   =  FIF_KOALA;
        m_freeimage_formats["KOA"]     =  FIF_KOALA;
        m_freeimage_formats["LBM"]     =  FIF_LBM;
        m_freeimage_formats["IFF"]     =  FIF_LBM;
        m_freeimage_formats["MNG"]     =  FIF_MNG;
        m_freeimage_formats["PBM"]     =  FIF_PBM;
        m_freeimage_formats["PBMRAW"]  =  FIF_PBMRAW;
        m_freeimage_formats["PCD"]     =  FIF_PCD;
        m_freeimage_formats["PCX"]     =  FIF_PCX;
        m_freeimage_formats["PGM"]     =  FIF_PGM;
        m_freeimage_formats["PGMRAW"]  =  FIF_PGMRAW;
        m_freeimage_formats["PNG"]     =  FIF_PNG;
        m_freeimage_formats["PPM"]     =  FIF_PPM;
        m_freeimage_formats["PPMRAW"]  =  FIF_PPMRAW;
        m_freeimage_formats["RAS"]     =  FIF_RAS;
        m_freeimage_formats["TGA"]     =  FIF_TARGA;
        m_freeimage_formats["TARGA"]   =  FIF_TARGA;
        m_freeimage_formats["TIFF"]    =  FIF_TIFF;//use qt
        m_freeimage_formats["TIF"]     =  FIF_TIFF;//use qt
        m_freeimage_formats["WBMP"]    =  FIF_WBMP;
        m_freeimage_formats["PSD"]     =  FIF_PSD;
        m_freeimage_formats["CUT"]     =  FIF_CUT;
        m_freeimage_formats["XBM"]     =  FIF_XBM;
        m_freeimage_formats["XPM"]     =  FIF_XPM;
        m_freeimage_formats["DDS"]     =  FIF_DDS;
        m_freeimage_formats["GIF"]     =  FIF_GIF;
        //m_freeimage_formats["HDR"]     =  FIF_HDR;//FHDR covert failed
        m_freeimage_formats["FAX"]     =  27;
        m_freeimage_formats["G3"]      =  27;//FAXG3
        m_freeimage_formats["SGI"]     =  FIF_SGI;
        m_freeimage_formats["EXR"]     =  FIF_EXR;
        //m_freeimage_formats["J2K"]     =  FIF_J2K;
        //m_freeimage_formats["J2C"]     =  FIF_J2K;
        // m_freeimage_formats["JPC"]     =  FIF_J2K;
//        m_freeimage_formats["JP2"]     =  FIF_JP2;
        //m_freeimage_formats["PFM"]     =  FIF_PFM;covert failed
        m_freeimage_formats["PCT"]     =  FIF_PICT;
        m_freeimage_formats["PIC"]     =  FIF_PICT;
        m_freeimage_formats["PICT"]    =  FIF_PICT;
        m_freeimage_formats["PIC"]     =  FIF_PICT;
//        m_freeimage_formats["RAW"]     =  FIF_RAW;
        m_freeimage_formats["WEBP"]    =  FIF_WEBP;
        m_freeimage_formats["JXR"]     =  FIF_JXR;
        m_freeimage_formats["MRW"]     =  FIF_MRW;
        m_movie_formats["MNG"]         =  FIF_MNG;
        m_movie_formats["GIF"]         =  FIF_GIF;
        m_movie_formats["WEBP"]         =  FIF_WEBP;
        /*
         * 由于原设计方案采用多个key对应一个value的方案，在判断可读可写的过程中是通过value去找key因此造成了多种情况而在下方变量中未将key，写完整因此补全
         * */
        m_qtSupported << "BMP" << "JPG" << "JPEG" << "JPS" << "JPE" << "PNG" << "PBM"
                      << "PGM" << "PPM" << "PNM" << "WBMP" << "WEBP"
                      << "SVG" << "ICNS" << "GIF" << "MNG" << "TIF"
                      << "TIFF" << "BMP" << "XPM"  << "DNG"
                      << "RAF"  << "CR2" << "MEF" << "ORF" << "ICO"
                      << "RAW"
                      << "MRW"
                      << "NEF"
                      << "JP2"
                      << "HEIF" << "HEIC" << "HEJ2"
                      << "AVIF" << "AVIFS";
        //pic（多张图片） pcx不支持旋转
        m_canSave << "BMP" << "JPG" << "JPEG"  << "JPS" << "JPE" << "PNG"
                  << "PGM" << "PPM" << "PNM"
                  << "TGA" << "XPM" << "ICO"
                  << "JNG"
//                << "SVG" //svg不再支持旋转
//                << "JP2"
//                << "PCD"
#ifndef  ISSW_64
                  << "WBMP"
#endif
                  << "RAS";

        /*
         * 修改：
         * PNG保存速度Qt是FreeImage的7倍
         * FreeImage旋转BMP会导致后续读图失败
         * PGM保存速度Qt略快于FreeImage
         * PBM保存速度Qt是FreeImage的20倍，且FreeImage旋转后会反色
        */
        /*<< "PGM" << "PBM"*/
        m_qtrotate << "ICNS" << "JPG" << "JPEG" << "PNG" << "BMP" ;
    }
    ~UnionImage_Private()
    {

    }
    QMutex freeimage_mutex;
    QStringList m_qtSupported;
    QHash<QString, int> m_freeimage_formats;
    QHash<QString, int> m_movie_formats;
    QStringList m_canSave;
    QStringList m_qtrotate;
};

static UnionImage_Private union_image_private;

FREE_IMAGE_FORMAT detectImageFormat_f(const QString &path);

/**
 * @brief noneQImage
 * @return QImage
 * 返回空图片
 */
UNIONIMAGESHARED_EXPORT QImage noneQImage()
{
    static QImage none(0, 0, QImage::Format_Invalid);
    return none;
}

UNIONIMAGESHARED_EXPORT const QStringList unionImageSupportFormat()
{
    static QStringList res;
    if (res.empty()) {
        QStringList list = union_image_private.m_freeimage_formats.keys();
        for (const QString &i : union_image_private.m_qtSupported) {
            if (!list.contains(i))
                list.append(i);
        }
        res.append(list);
        res.append(union_image_private.m_movie_formats.keys());
    }
    return res;
}

UNIONIMAGESHARED_EXPORT const QStringList supportStaticFormat()
{
    return (union_image_private.m_freeimage_formats.keys() << union_image_private.m_qtSupported);
}

UNIONIMAGESHARED_EXPORT const QStringList supportMovieFormat()
{
    return (union_image_private.m_movie_formats.keys());
}

/**
 * @brief size2Human
 * @param bytes
 * @author LMH
 * @return QString
 * 照片尺寸转化为QString格式
 */
UNIONIMAGESHARED_EXPORT QString size2Human(const qlonglong bytes)
{
    qlonglong kb = 1024;
    if (bytes < kb) {
        return QString::number(bytes) + " B";
    } else if (bytes < kb * kb) {
        QString vs = QString::number(static_cast<double>(bytes) / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            return QString::number(static_cast<int>(vs.toDouble())) + " KB";
        } else {
            return vs + " KB";
        }
    } else if (bytes < kb * kb * kb) {
        QString vs = QString::number(static_cast<double>(bytes) / kb / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            return QString::number(static_cast<int>(vs.toDouble())) + " MB";
        } else {
            return vs + " MB";
        }
    } else {
        //修改了当超过一个G的图片,应该用G返回,不应该返回一堆数字,bug68094
        QString vs = QString::number(static_cast<double>(bytes) / kb / kb / kb, 'f', 1);
        if (qCeil(vs.toDouble()) == qFloor(vs.toDouble())) {
            return QString::number(static_cast<int>(vs.toDouble())) + " GB";
        } else {
            return vs + " GB";
        }
    }
}

/**
 * @brief getFileFormat
 * @param path
 * @author LMH
 * @return QString
 * 文件路径获取文件后缀名
 */
UNIONIMAGESHARED_EXPORT const QString getFileFormat(const QString &path)
{
    QFileInfo fi(path);
    QString suffix = fi.suffix();
    return suffix;
}

#if 0
/**
 * @brief string2DateTime
 * @param time
 * @author LMH
 * @return QDateTime
 * string转换时间
 */
UNIONIMAGESHARED_EXPORT QDateTime string2DateTime(const QString &time)
{
    QDateTime dt = QDateTime::fromString(time, DATETIME_FORMAT_EXIF);
    if (! dt.isValid()) {
        dt = QDateTime::fromString(time, DATETIME_FORMAT_NORMAL);
    }
    return dt;
}
#endif

/**
 * @brief getMetaData
 * @param model
 * @param dib
 * @author LMH
 * @return QMap<QString, QString>
 * 内部接口获取图片的信息
 */
UNIONIMAGESHARED_EXPORT QMap<QString, QString> getMetaData(FREE_IMAGE_MDMODEL model, FIBITMAP *dib)
{
    QMap<QString, QString> mdMap;  // key-data

    if (FreeImage_GetMetadataCount(model, dib) > 0) {
        FITAG *tag = nullptr;
        FIMETADATA *mdhandle = nullptr;
        mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);
        if (mdhandle) {
            do {
                QString value;
                // FreeImage_TagToString非线程安全，使用前加锁保护
                QMutex *mutex = g_freeImageTagToStringMutex;
                if (mutex) {
                    mutex->lock();
                    value = QString(FreeImage_TagToString(model, tag));
                    mutex->unlock();
                }

                mdMap.insert(FreeImage_GetTagKey(tag), value);
            } while (FreeImage_FindNextMetadata(mdhandle, &tag));
            FreeImage_FindCloseMetadata(mdhandle);
        }
    }
    return mdMap;
}

/**
 * @brief FIBitmapToQImage
 * @param dib
 * @return QImage
 * 由FreeImage转到QImage
 */
UNIONIMAGESHARED_EXPORT QImage FIBitmap2QImage(FIBITMAP *dib)
{
    if (!dib || FreeImage_GetImageType(dib) == FIT_UNKNOWN)
        return noneQImage();
    int width  = static_cast<int>(FreeImage_GetWidth(dib));
    int height = static_cast<int>(FreeImage_GetHeight(dib));
    int depth = static_cast<int>(FreeImage_GetBPP(dib));
    switch (depth) {
    case 1: {
        QImage result(width, height, QImage::Format_Mono);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 1, 0, 0, 0, true
        );
        return result;
    }
    case 4: { /* NOTE: QImage do not support 4-bit, convert it to 8-bit  */
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
        );
        return result;
    }
    case 8: {
        QImage result(width, height, QImage::Format_Indexed8);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 8, 0, 0, 0, true
        );
        return result;
    }
    case 16:
        if ( // 5-5-5
            (FreeImage_GetRedMask(dib)   == FI16_555_RED_MASK) &&
            (FreeImage_GetGreenMask(dib) == FI16_555_GREEN_MASK) &&
            (FreeImage_GetBlueMask(dib)  == FI16_555_BLUE_MASK)) {
            QImage result(width, height, QImage::Format_RGB555);
            FreeImage_ConvertToRawBits(
                result.scanLine(0), dib, result.bytesPerLine(), 16,
                FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                true
            );
            return result;
        } else { // 5-6-5
            QImage result(width, height, QImage::Format_RGB16);
            FreeImage_ConvertToRawBits(
                result.scanLine(0), dib, result.bytesPerLine(), 16,
                FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                true
            );
            return result;
        }
    case 24: {
        QImage result(width, height, QImage::Format_RGB32);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
            true
        );
        return result;
    }
    case 32: {
        QImage result(width, height, QImage::Format_ARGB32);
        FreeImage_ConvertToRawBits(
            result.scanLine(0), dib, result.bytesPerLine(), 32,
            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
            true
        );
        return result;
    }
    case 48:
    case 64:
    case 96:
    //128位暂不支持
    case 128:
    default:
        break;
    }
    return noneQImage();
}

/**
 * @brief QImgeToFIBitMap
 * @param img
 * @return FIBITMAP*
 * 由QImage转到FreeImage
 */
#if 0
UNIONIMAGESHARED_EXPORT FIBITMAP *QImge2FIBitMap(QImage img)
{
    if (img.isNull()) {
        return nullptr;
    }
    int width = img.width();
    int height = img.height();
    uint depth = static_cast<uint>(img.depth());
    QImage::Format format = img.format();
    switch (format) {
    //The image is invalid.
    case QImage::Format_Invalid:
        return nullptr;
    //The image is stored using 1-bit per pixel.(MSB)
    case QImage::Format_Mono:
    //The image is stored using 1-bit per pixel.(LSB)
    case QImage::Format_MonoLSB: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    /* NOTE: QImage do not support 4-bit*/

    //可能会将彩色图转为灰度图
    //The image is stored using an 8-bit alpha only format.
    case QImage::Format_Alpha8:
    //The image is stored using an 8-bit grayscale format.
    case QImage::Format_Grayscale8:
    //The image is stored using 8-bit indexes into a colormap.
    case QImage::Format_Indexed8: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth, 0, 0, 0,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-6-5)
    case QImage::Format_RGB16: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_565_RED_MASK, FI16_565_GREEN_MASK, FI16_565_BLUE_MASK,
                            true);
        return res;
    }
    //The image is stored using a 16-bit RGB format (5-5-5). The unused most significant bit is always zero.
    case QImage::Format_RGB555: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), 16,
                            FI16_555_RED_MASK, FI16_555_GREEN_MASK, FI16_555_BLUE_MASK,
                            true);
        return res;
    }

    //32-bit
    case QImage::Format_RGB32: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }
    case QImage::Format_ARGB32: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }
    //24-bit
    case QImage::Format_RGB666:
    case QImage::Format_ARGB8555_Premultiplied:
    case QImage::Format_ARGB8565_Premultiplied:
    case QImage::Format_ARGB6666_Premultiplied:
    case QImage::Format_RGB888: {
        FIBITMAP *res = FreeImage_ConvertFromRawBits(
                            img.scanLine(0), width, height, img.bytesPerLine(), depth,
                            FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK,
                            true);
        return res;
    }

    //FreeImage is not support
    //The image is stored using a 16-bit RGB format (4-4-4). The unused bits are always zero.
    case QImage::Format_RGB444:
    case QImage::Format_ARGB4444_Premultiplied:
    default:
        break;
    }
    return nullptr;
}
#endif

/**
 * @brief readFile2FIBITMAP
 * @param path
 * @param flags
 * @author LMH
 * @return FIBITMAP
 * 由QString路径转换成FIBITMAP指针
 */
UNIONIMAGESHARED_EXPORT FIBITMAP *readFile2FIBITMAP(const QString &path, int flags FI_DEFAULT(0))
{
    QImage image(path);
    if(image.isNull()) {//判断图片是否有效
        return nullptr;
    }
    QByteArray b;
    b.append(path);
    const char *pc = b.data();
    QString().toStdString();
    const FREE_IMAGE_FORMAT fif = detectImageFormat_f(path);
    if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
        FIBITMAP *dib = FreeImage_Load(fif, pc, flags);
        return dib;
    }
    return nullptr;
}

UNIONIMAGESHARED_EXPORT bool canSave(const QString &path)
{
    QImageReader r(path);
    if (r.imageCount() > 1) {
        return false;
    }
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(path.toUtf8().data());
    if (fif != FIF_UNKNOWN) {
        // Check that the dib can be saved in this format
        if (union_image_private.m_canSave.contains(union_image_private.m_freeimage_formats.key(fif))) {
            return true;
        }
    }
    QFileInfo info(path);
    if (union_image_private.m_canSave.contains(info.suffix().toUpper()))
        return true;
    return false;
}



/**
 * @brief writeFIBITMAPToFile
 * @param dib
 * @param path
 * @param flag
 * @return
 */
UNIONIMAGESHARED_EXPORT bool writeFIBITMAPToFile(FIBITMAP *dib, const QString &path, int flag = 0)
{
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    BOOL bSuccess = FALSE;
    const QByteArray ba = path.toUtf8();
    const char *pc = ba.data();
    // Try to guess the file format from the file extension
    fif = FreeImage_GetFIFFromFilename(pc);
    FREE_IMAGE_FORMAT realfif = FIF_UNKNOWN;
    if (fif == FIF_UNKNOWN) {
        realfif = FreeImage_GetFileType(pc);
    }
    if (fif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(fif, dib, pc, flag);
    } else if (realfif != FIF_UNKNOWN) {
        bSuccess = FreeImage_Save(realfif, dib, pc, flag);
    }
    return bSuccess;
}

UNIONIMAGESHARED_EXPORT QString unionImageVersion()
{
    QString ver;
    ver.append("UnionImage Version:");
    ver.append("0.0.4");
    ver.append("\n");
    return ver;
}



UNIONIMAGESHARED_EXPORT bool creatNewImage(QImage &res, int width, int height, int depth, SupportType type)
{
    Q_UNUSED(type);
    FIBITMAP *dib = FreeImage_Allocate(width, height, depth);
    res = FIBitmap2QImage(dib);
    FreeImage_Unload(dib);
    return true;
}

QString PrivateDetectImageFormat(const QString &filepath);
UNIONIMAGESHARED_EXPORT bool loadStaticImageFromFile(const QString &path, QImage &res, QString &errorMsg, const QString &format_bar)
{
    QFileInfo file_info(path);
    if (file_info.size() == 0) {
        res = QImage();
        errorMsg = "error file!";
        return false;
    }
    QMap<QString, QString> dataMap = getAllMetaData(path);
    QString file_suffix_upper = dataMap.value("FileFormat").toUpper();

    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (f != FIF_UNKNOWN && f != union_image_private.m_freeimage_formats[file_suffix_upper]) {
        file_suffix_upper = union_image_private.m_freeimage_formats.key(f);
    }
    if (f == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    QString file_suffix_lower = file_suffix_upper.toLower();
    //解决欧拉版对于raw格式问题判断为PICT的问题
    bool usingQimage = false;
    usingQimage = (f == FIF_RAW || f == FIF_PICT);
    //如果是pct格式使用freeimage
    if (f == FIF_PICT && file_info.suffix().toLower() == "pct") {
        usingQimage = false;
    }

    if (usingQimage || union_image_private.m_qtSupported.contains(file_suffix_upper)) {
        QImageReader reader;
        QImage res_qt;
        reader.setFileName(path);
        if (format_bar.isEmpty()) {
            reader.setFormat(file_suffix_lower.toLatin1());
        } else {
            reader.setFormat(format_bar.toLatin1());
        }
        reader.setAutoTransform(true);
        if (reader.imageCount() > 0 || file_suffix_upper != "ICNS") {
            res_qt = reader.read();
            if (res_qt.isNull()) {
                //try old loading method
                QString format = PrivateDetectImageFormat(path);
                QImageReader readerF(path, format.toLatin1());
                QImage try_res;
                readerF.setAutoTransform(true);
                if (readerF.canRead()) {
                    try_res = readerF.read();
                } else {
                    errorMsg = "can't read image:" + readerF.errorString() + format;
                    try_res = QImage(path);
                }

                // 单独处理TIF格式情况
                if (try_res.isNull() && (file_suffix_upper == "TIF" || file_suffix_upper == "TIFF")) {
                    // 读取失败，tif需要单独处理，尝试通过转换函数处理
                    QFileInfo imageFile(path);
                    QString cacheFile = Libutils::image::getCacheImagePath() + QDir::separator() + imageFile.fileName();
                    // 判断是否存在缓存图片数据
                    if (Libutils::image::checkCacheImage(imageFile.fileName())) {
                        try_res = QImage(cacheFile);
                    } else {
                        // 由于多线程调用，可能同时访问文件，使用临时文件处理，防止部分线程读取未转码完成的图片文件
                        QString tempFile = Libutils::image::getCacheImagePath() + QDir::separator()
                                + QUuid::createUuid().toString() + imageFile.fileName();
                        qDebug() << "convert" << imageFile.absoluteFilePath() << cacheFile << tempFile;

                        // 转换图像编码格式
                        int nRet = convertOldStyleImage(imageFile.absoluteFilePath().toUtf8().data(), tempFile.toUtf8().data());
                        // 转换成功标识
                        static const int s_nFlagSucc = 0;
                        if (s_nFlagSucc == nRet) {
                            if (QFile::exists(tempFile)) {
                                try_res = QImage(tempFile);

                                // 判断缓存文件是否已存在，保存缓存数据
                                if (QFile::exists(cacheFile)) {
                                    QFile::remove(tempFile);
                                } else {
                                    QFile::rename(tempFile, cacheFile);
                                }
                            }
                        }
                    }
                }

                if (try_res.isNull()) {
                    errorMsg = "load image by qt failed, use format:" + reader.format() + " ,path:" + path;
                    res = QImage();
                    return false;
                }
                errorMsg = "use old method to load QImage";
                res = try_res;
                return true;
            }
            errorMsg = "use QImage";
            res = res_qt;
        } else {
            res = QImage();
            return false;
        }
        return true;
    } else {
        if (f != FREE_IMAGE_FORMAT::FIF_UNKNOWN || union_image_private.m_freeimage_formats.contains(file_suffix_upper)) {
            if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
                f = FREE_IMAGE_FORMAT(union_image_private.m_freeimage_formats[file_suffix_upper]);
            if (f == FREE_IMAGE_FORMAT::FIF_JP2 && file_info.size() > 40960000) {
                errorMsg = "image load failed, format:" + union_image_private.m_freeimage_formats.key(f) + " ,path:" + temp_path;
                res = QImage();
                return false;
            }
            FIBITMAP *dib = FreeImage_Load(f, temp_path.data());
            if (nullptr == dib) {
                errorMsg = "image load failed, format:" + union_image_private.m_freeimage_formats.key(f) + " ,path:" + temp_path;
                //FreeImage_Unload(dib);
                res = QImage();
                return false;
            }
//            uint depth = FreeImage_GetBPP(dib); //just for test
//            Q_UNUSED(depth);
            //32位以上图片qImage不支持,强行读取和转换可能会乱码
            res = QImage(FIBitmap2QImage(dib));
            if (res.isNull()) {
                errorMsg = "convert to QImage failed" + union_image_private.m_freeimage_formats.key(f) + " ,path:" + temp_path;
                FreeImage_Unload(dib);
                res = QImage();
                return false;
            }
            FreeImage_Unload(dib);
            errorMsg = "";
            return true;
        }
        return false;
    }
}

UNIONIMAGESHARED_EXPORT QString detectImageFormat(const QString &path)
{
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (f != FIF_UNKNOWN && f != union_image_private.m_freeimage_formats[file_suffix_upper]) {
        file_suffix_upper = union_image_private.m_freeimage_formats.key(f);
    }
    if (f == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    QString res = file_suffix_upper;
    if (res.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return "";
        }

        //    const QByteArray data = file.read(1024);
        const QByteArray data = file.read(64);

        // Check bmp file.
        if (data.startsWith("BM")) {
            return "BMP";
        }

        // Check dds file.
        if (data.startsWith("DDS")) {
            return "DDS";
        }

        // Check gif file.
        if (data.startsWith("GIF8")) {
            return "GIF";
        }

        // Check Max OS icons file.
        if (data.startsWith("icns")) {
            return "ICNS";
        }

        // Check jpeg file.
        if (data.startsWith("\xff\xd8")) {
            return "JPG";
        }

        // Check mng file.
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return "MNG";
        }

        // Check net pbm file (BitMap).
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return "PBM";
        }

        // Check pgm file (GrayMap).
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return "PGM";
        }

        // Check ppm file (PixMap).
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return "PPM";
        }

        // Check png file.
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return "PNG";
        }

        // Check svg file.
        if (data.indexOf("<svg") > -1) {
            return "SVG";
        }

        // TODO(xushaohua): tga file is not supported yet.

        // Check tiff file.
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return "TIFF";
        }

        // TODO(xushaohua): Support wbmp file.

        // Check webp file.
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return "WEBP";
        }

        // Check xbm file.
        if (data.indexOf("#define max_width ") > -1 &&
                data.indexOf("#define max_height ") > -1) {
            return "XBM";
        }

        // Check xpm file.
        if (data.startsWith("/* XPM */")) {
            return "XPM";
        }
        return "";
    }
    return res;
}

FREE_IMAGE_FORMAT detectImageFormat_f(const QString &path)
{
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (f != FIF_UNKNOWN && f != union_image_private.m_freeimage_formats[file_suffix_upper]) {
        file_suffix_upper = union_image_private.m_freeimage_formats.key(f);
    }
    if (f == FIF_TIFF) {
        file_suffix_upper = "TIFF";
    }
    if (file_suffix_upper.isEmpty()) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            return FIF_UNKNOWN;
        }

        //    const QByteArray data = file.read(1024);
        const QByteArray data = file.read(64);

        // Check bmp file.
        if (data.startsWith("BM")) {
            return FIF_BMP;
        }

        // Check dds file.
        if (data.startsWith("DDS")) {
            return FIF_DDS;
        }

        // Check gif file.
        if (data.startsWith("GIF8")) {
            return FIF_GIF;
        }

        // Check Max OS icons file.
        if (data.startsWith("icns")) {
            return FIF_UNKNOWN;
        }

        // Check jpeg file.
        if (data.startsWith("\xff\xd8")) {
            return FIF_JPEG;
        }

        // Check mng file.
        if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
            return FIF_MNG;
        }

        // Check net pbm file (BitMap).
        if (data.startsWith("P1") || data.startsWith("P4")) {
            return FIF_PBM;
        }

        // Check pgm file (GrayMap).
        if (data.startsWith("P2") || data.startsWith("P5")) {
            return FIF_PGM;
        }

        // Check ppm file (PixMap).
        if (data.startsWith("P3") || data.startsWith("P6")) {
            return FIF_PPM;
        }

        // Check png file.
        if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
            return FIF_PNG;
        }

        // Check svg file.
        if (data.indexOf("<svg") > -1) {
            return FIF_UNKNOWN;
        }

        // TODO(xushaohua): tga file is not supported yet.

        // Check tiff file.
        if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
            // big-endian, little-endian.
            return FIF_TIFF;
        }

        // TODO(xushaohua): Support wbmp file.

        // Check webp file.
        if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
            return FIF_WEBP;
        }

        // Check xbm file.
        if (data.indexOf("#define max_width ") > -1 &&
                data.indexOf("#define max_height ") > -1) {
            return FIF_XBM;
        }

        // Check xpm file.
        if (data.startsWith("/* XPM */")) {
            return FIF_XPM;
        }
        return FIF_UNKNOWN;
    }
    f = static_cast<FREE_IMAGE_FORMAT>(union_image_private.m_freeimage_formats[file_suffix_upper]);
    return f >= 0 ? f : FIF_UNKNOWN;
}

UNIONIMAGESHARED_EXPORT bool isNoneQImage(const QImage &qi)
{
    return (qi == noneQImage());
}

UNIONIMAGESHARED_EXPORT bool rotateImage(int angel, QImage &image)
{
    if (angel % 90 != 0)
        return false;
    if (image.isNull()) {
        return false;
    }
    QImage image_copy(image);
    if (!image_copy.isNull()) {
        QTransform rotatematrix;
        rotatematrix.rotate(angel);
        image = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
        return true;
    }
    return false;
}

QImage adjustImageToRealPosition(const QImage &image, int orientation)
{
    QImage result = image;

    switch (orientation) {
    case 1: //不做操作
    default:
        break;
    case 2: //水平翻转
        result = result.mirrored(true, false);
        break;
    case 3: //180度翻转
        rotateImage(180, result);
        break;
    case 4: //垂直翻转
        result = result.mirrored(false, true);
        break;
    case 5: //顺时针90度+水平翻转
        rotateImage(90, result);
        result = result.mirrored(true, false);
        break;
    case 6: //顺时针90度
        rotateImage(90, result);
        break;
    case 7: //顺时针90度+垂直翻转
        rotateImage(90, result);
        result = result.mirrored(false, true);
        break;
    case 8: //逆时针90度
        rotateImage(-90, result);
        break;
    };

    return result;
}

UNIONIMAGESHARED_EXPORT bool rotateImageFIle(int angel, const QString &path, QString &erroMsg)
{
    if (angel % 90 != 0) {
        erroMsg = "unsupported angel";
        return false;
    }
    QString format = detectImageFormat(path);
    if (format == "SVG") {
        QImage image_copy;
        if (!loadStaticImageFromFile(path, image_copy, erroMsg)) {
            erroMsg = "rotate load QImage failed, path:" + path + "  ,format:+" + format;
            return false;
        }
        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setViewBox(QRect(0, 0, image_copy.width(), image_copy.height()));
        QPainter rotatePainter;
        rotatePainter.begin(&generator);
        rotatePainter.resetTransform();
        rotatePainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        int realangel = angel / 90;
        if (realangel > 0) {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(image_copy.width(), 0);
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        } else {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(0, image_copy.height());
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        }
        rotatePainter.drawImage(image_copy.rect(), image_copy.scaled(image_copy.width(), image_copy.height()));
        rotatePainter.resetTransform();
        generator.setSize(QSize(image_copy.width(), image_copy.height()));
        rotatePainter.end();
        return true;
    } else if (union_image_private.m_qtrotate.contains(format)) {
        //由于Qt内部不会去读图片的EXIF信息来判断当前的图像矩阵的真实位置，同时回写数据的时候会丢失全部的EXIF数据
        //因此在这里需要额外基于FreeImage来读取相关的数据，确保图片能旋转到合理的位置
        int orientation = getOrientation(path);
        QImage image_copy(path);
        image_copy = adjustImageToRealPosition(image_copy, orientation);
        if (!image_copy.isNull()) {
            QMatrix rotatematrix;
            rotatematrix.rotate(angel);
            image_copy = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);

            // 调整图片质量，不再默认使用满质量 SAVE_QUAITY_VALUE
            if (image_copy.save(path, format.toLatin1().data(), -1)) {
                return true;
            }
        }
        erroMsg = "rotate by qt failed";
        return false;
    }
    FIBITMAP *dib = readFile2FIBITMAP(path);
    if (nullptr == dib) {
        erroMsg = "unsupported format";
        FreeImage_Unload(dib);
        return false;
    }
    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angel);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angel);

            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }
    QByteArray temp_path;
    temp_path.append(path);
    FREE_IMAGE_FORMAT f = FREE_IMAGE_FORMAT(union_image_private.m_freeimage_formats[QFileInfo(path).suffix().toUpper()]);
    if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN) {
        erroMsg = "rotate image format error";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    ;
    if (!writeFIBITMAPToFile(rotateRes, path)) {
        erroMsg = "rotate image save failed, unknown format";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    erroMsg = "";
    return true;
}

UNIONIMAGESHARED_EXPORT bool rotateImageFIleWithImage(int angel, QImage &img, const QString &path, QString &erroMsg)
{
    if (angel % 90 != 0) {
        erroMsg = "unsupported angel";
        return false;
    }
    QImage image_copy;
    if (img.isNull()) return false;
    else image_copy = img;

    QString format = detectImageFormat(path);
    if (format == "SVG") {
        QSvgGenerator generator;
        generator.setFileName(path);
        generator.setViewBox(QRect(0, 0, image_copy.width(), image_copy.height()));
        QPainter rotatePainter;
        rotatePainter.begin(&generator);
        rotatePainter.resetTransform();
        rotatePainter.setRenderHint(QPainter::HighQualityAntialiasing, true);
        int realangel = angel / 90;
        if (realangel > 0) {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(image_copy.width(), 0);
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        } else {
            for (int i = 0; i < qAbs(realangel); i++) {
                rotatePainter.translate(0, image_copy.height());
                rotatePainter.rotate(90 * (realangel / qAbs(realangel)));
            }
        }
        rotatePainter.drawImage(image_copy.rect(), image_copy.scaled(image_copy.width(), image_copy.height()));
        rotatePainter.resetTransform();
        generator.setSize(QSize(image_copy.width(), image_copy.height()));
        rotatePainter.end();
        return true;
    } else if (format == "JPG" || format == "JPEG") {
        QImage image_copy(path, "JPG");
        if (!image_copy.isNull()) {
            QPainter rotatePainter(&image_copy);
            rotatePainter.rotate(angel);
            rotatePainter.end();
            image_copy.save(path, "jpg", SAVE_QUAITY_VALUE);
            return true;
        }
    }
    FIBITMAP *dib = readFile2FIBITMAP(path);
    if (nullptr == dib) {
        erroMsg = "unsupported format";
        FreeImage_Unload(dib);
        return false;
    }
    FIBITMAP *rotateRes = FreeImage_Rotate(dib, -angel);
    if (rotateRes) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -angel);
            FreeImage_SetThumbnail(rotateRes, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }
    QByteArray temp_path;
    temp_path.append(path);
    FREE_IMAGE_FORMAT f = FREE_IMAGE_FORMAT(union_image_private.m_freeimage_formats[QFileInfo(path).suffix().toUpper()]);
    if (f == FREE_IMAGE_FORMAT::FIF_UNKNOWN) {
        erroMsg = "rotate image format error";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    img = FIBitmap2QImage(rotateRes);
    if (!writeFIBITMAPToFile(rotateRes, path)) {
        erroMsg = "rotate image save failed, unknown format";
        FreeImage_Unload(dib);
        FreeImage_Unload(rotateRes);
        return false;
    }
    FreeImage_Unload(dib);
    FreeImage_Unload(rotateRes);
    erroMsg = "";
    return true;
}

UNIONIMAGESHARED_EXPORT QMap<QString, QString> getAllMetaData(const QString &path)
{
    FIBITMAP *dib = readFile2FIBITMAP(path, FIF_LOAD_NOPIXELS);
    QMap<QString, QString> admMap;
    admMap.unite(getMetaData(FIMD_EXIF_MAIN, dib));
    admMap.unite(getMetaData(FIMD_EXIF_EXIF, dib));
    admMap.unite(getMetaData(FIMD_EXIF_GPS, dib));
    admMap.unite(getMetaData(FIMD_EXIF_MAKERNOTE, dib));
    admMap.unite(getMetaData(FIMD_EXIF_INTEROP, dib));
    admMap.unite(getMetaData(FIMD_IPTC, dib));
    //移除秒　　2020/6/5 DJH
    //需要转义才能读出：或者/　　2020/8/21 DJH
    QFileInfo info(path);
    if (admMap.contains("DateTime")) {
        QDateTime time = QDateTime::fromString(admMap["DateTime"], "yyyy:MM:dd hh:mm:ss");
        admMap["DateTimeOriginal"] = time.toString("yyyy/MM/dd hh:mm");
    } else {
        admMap.insert("DateTimeOriginal",  info.lastModified().toString("yyyy/MM/dd HH:mm"));
    }
    admMap.insert("DateTimeDigitized",  info.lastModified().toString("yyyy/MM/dd HH:mm"));
//    if (admMap.isEmpty()) {
//        QDateTime emptyTime(QDate(0, 0, 0), QTime(0, 0));
//        admMap.insert("DateTimeOriginal",  emptyTime.toString("yyyy/MM/dd HH:mm"));
//        admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
//    }
    /* else {
        QString qsdto = admMap.value("DateTimeOriginal");
        QString qsdtd = admMap.value("DateTimeDigitized");
        QDateTime ot = QDateTime::fromString(qsdto, "yyyy/MM/dd HH:mm");
        QDateTime dt = QDateTime::fromString(qsdtd, "yyyy/MM/dd HH:mm");
        if (! ot.isValid()) {
            // Exif version 0221
            QString qsdt = admMap.value("DateTime");
            ot = QDateTime::fromString(qsdt, "yyyy/MM/dd HH:mm");
            dt = ot;

            // NO valid date information
            if (! ot.isValid()) {
    //                admMap.insert("DateTimeOriginal", info.created().toString("yyyy/MM/dd HH:mm:dd"));
                admMap.insert("DateTimeOriginal", info.birthTime().toString("yyyy/MM/dd HH:mm"));
                admMap.insert("DateTimeDigitized", info.lastModified().toString("yyyy/MM/dd HH:mm"));
            } else {
                admMap.insert("DateTimeOriginal", ot.toString("yyyy/MM/dd HH:mm"));
                admMap.insert("DateTimeDigitized", dt.toString("yyyy/MM/dd HH:mm"));
            }
        }
    }*/

//    // The value of width and height might incorrect
    QImageReader reader(path);
    int w = reader.size().width();
    w = w > 0 ? w : static_cast<int>(FreeImage_GetWidth(dib));
    int h = reader.size().height();
    h = h > 0 ? h : static_cast<int>(FreeImage_GetHeight(dib));
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));

    admMap.insert("FileName", info.fileName());
    //应该使用qfileinfo的格式
    admMap.insert("FileFormat", getFileFormat(path));
    admMap.insert("FileSize", size2Human(info.size()));
    FreeImage_Unload(dib);

    return admMap;
}

UNIONIMAGESHARED_EXPORT bool isImageSupportRotate(const QString &path)
{
    return canSave(path) ;
}

#if 0
UNIONIMAGESHARED_EXPORT bool isSupportsReading(const QString &path)
{
    const FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.toUtf8().data());
    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif);
}

UNIONIMAGESHARED_EXPORT bool isSupportsWriting(const QString &path)
{
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(path.toUtf8().data());
    return (fif != FIF_UNKNOWN) && FreeImage_FIFSupportsWriting(fif);
}
#endif

UNIONIMAGESHARED_EXPORT int getOrientation(const QString &path)
{
    int result = 1; //1代表不做操作，维持原样

    FIBITMAP *dib = readFile2FIBITMAP(path, FIF_LOAD_NOPIXELS);

    //有时候会存在tag为野指针的情况，根据FreeImage的demo，需要加这个进行预判断
    if (FreeImage_GetMetadataCount(FIMD_EXIF_MAIN, dib) == 0) {
        FreeImage_Unload(dib);
        return result;
    }

    FITAG *tag = nullptr;
    FIMETADATA *mdhandle = nullptr;
    mdhandle = FreeImage_FindFirstMetadata(FIMD_EXIF_MAIN, dib, &tag);
    if (mdhandle) {
        do {
            if (std::strcmp(FreeImage_GetTagKey(tag), "Orientation") == 0) {
                result = *static_cast<const WORD *>(FreeImage_GetTagValue(tag));
                break;
            }
        } while (FreeImage_FindNextMetadata(mdhandle, &tag));
        FreeImage_FindCloseMetadata(mdhandle);
    }

    FreeImage_Unload(dib);
    return result;
}

#if 0
bool getThumbnail(QImage &res, const QString &path)
{
    FIBITMAP *dib = readFile2FIBITMAP(path);
    res = FIBitmap2QImage(FreeImage_GetThumbnail(dib));
    FreeImage_Unload(dib);
    return true;
}
#endif

#if 0
class UnionMovieImagePrivate : public QObject
{
protected:
    explicit UnionMovieImagePrivate(UnionMovieImage *parent): q_ptr(parent)
    {
        Q_UNUSED(padding);
    }
    ~UnionMovieImagePrivate()
    {
//        CManagerAttributeService::getInstance()->setCouldRun(false);
//        CManagerAttributeService::getInstance()->GifFreeFile();
    }


    void setPathAndBegin(const QString &path)
    {
        Q_UNUSED(path)
//        CManagerAttributeService::getInstance()->setfilePathWithSignalPlay(path);
//        QObject::connect(CManagerAttributeService::getInstance(), &CManagerAttributeService::emitImageSignal, this, [ = ](QImage image, bool isFirst) {
//            Q_UNUSED(isFirst);
//            res = image;
//        });
    }

    int getCurrent()
    {
        return currentIndex;
    }

    void reset()
    {
//        CManagerAttributeService::getInstance()->setCouldRun(false);
//        CManagerAttributeService::getInstance()->GifFreeFile();

        delete r;
        r = nullptr;
        errMsg = "";
        res = QImage();
        currentFormat = FIF_UNKNOWN;
        currentIndex = 0;
        frames = 0;
    }

    void setIndex(int i)
    {
        currentIndex = i;
    }
private:
    UnionMovieImage *const q_ptr;
    Q_DECLARE_PUBLIC(UnionMovieImage)
    QImageReader *r = nullptr;
    QString errMsg = "";
    QImage res;
    FREE_IMAGE_FORMAT currentFormat = FIF_UNKNOWN;
    int currentIndex = 0;
    int frames = 0;
    char padding[4];
};

UnionMovieImage::UnionMovieImage(): d_ptr(new UnionMovieImagePrivate(this))
{

}

UnionMovieImage::~UnionMovieImage()
{
    Q_D(UnionMovieImage);
    delete d;
}

void UnionMovieImage::setFileName(const QString &path)
{
    Q_D(UnionMovieImage);
    d->reset();
    QString errMsg;
    QFileInfo file_info(path);
    QString file_suffix_upper = file_info.suffix().toUpper();
    QByteArray temp_path;
    temp_path.append(path.toUtf8());
    FREE_IMAGE_FORMAT f = FreeImage_GetFileType(temp_path.data());
    if (!union_image_private.m_movie_formats.contains(file_suffix_upper) || !union_image_private.m_movie_formats.values().contains(f)) {
        errMsg = "static Image";
    } else {
        switch (f) {
        case FIF_GIF: {
            d->setPathAndBegin(path);
            d->currentFormat = FIF_GIF;
        }
        break;
        case FIF_WEBP:
        case FIF_MNG: {
            d->r = new QImageReader;
            d->r->setFileName(path);
            if (d->r->canRead()) {
                d->currentFormat = FIF_MNG;
                d->frames = d->r->imageCount();
            }
        }
        break;
        default:
            break;
        }
    }
}

QImage UnionMovieImage::next()
{
    Q_D(UnionMovieImage);
    switch (d->currentFormat) {
    case FIF_GIF: {
        return d->res;
    }
    case FIF_WEBP:
    case FIF_MNG: {
        int temp = d->currentIndex;
        d->setIndex(temp + 1);
        if (temp + 1 >= d->frames) {
            d->setIndex(0);
        }
        d->res = d->r->read();
        d->r->jumpToNextImage();
        break;
    }
    default:
        break;
    }
    return d->res;
}
#endif

imageViewerSpace::ImageType getImageType(const QString &imagepath)
{
    imageViewerSpace::ImageType type = imageViewerSpace::ImageType::ImageTypeBlank;
    //新增获取图片是属于静态图还是动态图还是多页图
    if (!imagepath.isEmpty()) {
        QFileInfo fi(imagepath);
        QString strType = fi.suffix().toLower();
        //解决bug57394 【专业版1031】【看图】【5.6.3.74】【修改引入】pic格式图片变为翻页状态，不为动图且首张显示序号为0
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(imagepath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(imagepath, QMimeDatabase::MatchExtension);
        QString path1 = mt.name();
        QString path2 = mt1.name();
        int nSize = -1;
        QImageReader imgreader(imagepath);
        nSize = imgreader.imageCount();

        if (strType == "svg" && QSvgRenderer().load(imagepath)) {
            type = imageViewerSpace::ImageTypeSvg;
        } else if ((strType == "mng")
                   || ((strType == "gif") && nSize > 1)
                   || (strType == "webp" && nSize > 1)
                   || ((mt.name().startsWith("image/gif")) && nSize > 1)
                   || ((mt1.name().startsWith("image/gif")) && nSize > 1)
                   || ((mt.name().startsWith("video/x-mng")))
                   || ((mt1.name().startsWith("video/x-mng")))) {
            type = imageViewerSpace::ImageTypeDynamic;
        } else if (nSize > 1) {
            type = imageViewerSpace::ImageTypeMulti;
        } else if (mt.name().startsWith("image/")
                   || mt.name().startsWith("video/x-mng")
                   || mt1.name().startsWith("image/")
                   || mt1.name().startsWith("video/x-mng")) {
            type = imageViewerSpace::ImageTypeStatic;
        }
    }
    return type;
}

imageViewerSpace::PathType getPathType(const QString &imagepath)
{
    //判断文件路径来自于哪里
    imageViewerSpace::PathType type = imageViewerSpace::PathType::PathTypeLOCAL;
    if (imagepath.indexOf("smb-share:server=") != -1) {
        type = imageViewerSpace::PathTypeSMB;
    } else if (imagepath.indexOf("mtp:host=") != -1) {
        type = imageViewerSpace::PathTypeMTP;
    } else if (imagepath.indexOf("gphoto2:host=") != -1) {
        type = imageViewerSpace::PathTypePTP;
    } else if (imagepath.indexOf("gphoto2:host=Apple") != -1) {
        type = imageViewerSpace::PathTypeAPPLE;
    } else if (Libutils::image::isVaultFile(imagepath)) {
        type = imageViewerSpace::PathTypeSAFEBOX;
    } else if (imagepath.contains(QDir::homePath() + "/.local/share/Trash")) {
        type = imageViewerSpace::PathTypeRECYCLEBIN;
    }
    //todo
    return type;
}

QString PrivateDetectImageFormat(const QString &filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        return "";
    }

    const QByteArray data = file.read(1024);

    // Check bmp file.
    if (data.startsWith("BM")) {
        return "bmp";
    }

    // Check dds file.
    if (data.startsWith("DDS")) {
        return "dds";
    }

    // Check gif file.
    if (data.startsWith("GIF8")) {
        return "gif";
    }

    // Check Max OS icons file.
    if (data.startsWith("icns")) {
        return "icns";
    }

    // Check jpeg file.
    if (data.startsWith("\xff\xd8")) {
        return "jpg";
    }

    // Check mng file.
    if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
        return "mng";
    }

    // Check net pbm file (BitMap).
    if (data.startsWith("P1") || data.startsWith("P4")) {
        return "pbm";
    }

    // Check pgm file (GrayMap).
    if (data.startsWith("P2") || data.startsWith("P5")) {
        return "pgm";
    }

    // Check ppm file (PixMap).
    if (data.startsWith("P3") || data.startsWith("P6")) {
        return "ppm";
    }

    // Check png file.
    if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
        return "png";
    }

    // Check svg file.
    if (data.indexOf("<svg") > -1) {
        return "svg";
    }

    // TODO(xushaohua): tga file is not supported yet.

    // Check tiff file.
    if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
        // big-endian, little-endian.
        return "tiff";
    }

    // TODO(xushaohua): Support wbmp file.

    // Check webp file.
    if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
        return "webp";
    }

    // Check xbm file.
    if (data.indexOf("#define max_width ") > -1 &&
            data.indexOf("#define max_height ") > -1) {
        return "xbm";
    }

    // Check xpm file.
    if (data.startsWith("/* XPM */")) {
        return "xpm";
    }
    return "";
}

};



