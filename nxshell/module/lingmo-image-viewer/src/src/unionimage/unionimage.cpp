// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unionimage.h"

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

#include "unionimage/imageutils.h"

#include <cstring>

#define SAVE_QUAITY_VALUE 100

const QString DATETIME_FORMAT_NORMAL = "yyyy.MM.dd";
const QString DATETIME_FORMAT_EXIF = "yyyy:MM:dd HH:mm";

namespace LibUnionImage_NameSpace {

class UnionImage_Private
{

public:
    UnionImage_Private()
    {
        /*
         * 由于原设计方案采用多个key对应一个value的方案，在判断可读可写的过程中是通过value去找key因此造成了多种情况而在下方变量中未将key，写完整因此补全
         * */
        m_qtSupported << "BMP"
                      << "JPG"
                      << "JPEG"
                      << "JPS"
                      << "JPE"
                      << "PNG"
                      << "PBM"
                      << "PGM"
                      << "PPM"
                      << "PNM"
                      << "WBMP"
                      << "WEBP"
                      << "SVG"
                      << "ICNS"
                      << "GIF"
                      << "MNG"
                      << "TIF"
                      << "TIFF"
                      << "BMP"
                      << "XPM"
                      << "DNG"
                      << "RAF"
                      << "CR2"
                      << "MEF"
                      << "ORF"
                      << "ICO"
                      << "RAW"
                      << "MRW"
                      << "NEF"
                      << "JP2"
                      << "HEIF"
                      << "HEIC"
                      << "HEJ2"
                      << "AVIF"
                      << "TGA"
                      << "PSD"
                      << "PXM"
                      << "PIC"
                      << "PEF"
                      << "XBM"
                      << "ARW"
                      << "HDR"
                      << "J2K"
                      << "ICNS"
                      << "AVI"
                      << "VIFF"
                      << "IFF"
                      << "JP2"
                      << "WMF"
                      << "CRW"
                      << "X3F"
                      << "EPS"
                      << "SR2"
                      << "AVIFS";
        m_canSave << "BMP"
                  << "JPG"
                  << "JPEG"
                  << "PNG"
                  << "PGM"
                  << "PPM"
                  << "XPM"
                  << "ICO"
                  << "ICNS";
        /*<< "PGM" << "PBM"*/
        m_qtrotate << "BMP"
                   << "JPG"
                   << "JPEG"
                   << "PNG"
                   << "PGM"
                   << "PPM"
                   << "XPM"
                   << "ICO"
                   << "ICNS";
    }
    ~UnionImage_Private()
    {

    }
    QStringList m_qtSupported;
    QHash<QString, int> m_movie_formats;
    QStringList m_canSave;
    QStringList m_qtrotate;
};

static UnionImage_Private union_image_private;

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
        QStringList list = union_image_private.m_qtSupported;
        res.append(list);
    }
    return res;
}

UNIONIMAGESHARED_EXPORT const QStringList supportStaticFormat()
{
    return (union_image_private.m_qtSupported);
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

UNIONIMAGESHARED_EXPORT bool canSave(const QString &path)
{
    QImageReader r(path);
    if (r.imageCount() > 1) {
        return false;
    }
    QFileInfo info(path);
    if (union_image_private.m_canSave.contains(info.suffix().toUpper()))
        return true;
    return false;
}

UNIONIMAGESHARED_EXPORT QString unionImageVersion()
{
    QString ver;
    ver.append("UnionImage Version:");
    ver.append("0.0.4");
    ver.append("\n");
    return ver;
}

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
    QString file_suffix_lower = file_suffix_upper.toLower();
    if (union_image_private.m_qtSupported.contains(file_suffix_upper)) {
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
                QString format = detectImageFormat(path);
                QImageReader readerF(path, format.toLatin1());
                QImage try_res;
                readerF.setAutoTransform(true);
                if (readerF.canRead()) {
                    try_res = readerF.read();
                } else {
                    errorMsg = "can't read image:" + readerF.errorString() + format;
                    try_res = QImage(path);
                }
                if (try_res.isNull()) {
                    errorMsg = "load image by qt faild, use format:" + reader.format() + " ,path:" + path;
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
    }
    return false;
}

UNIONIMAGESHARED_EXPORT QString detectImageFormat(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "DetectImageFormat() failed to open file:" << path;
        return "";
    }

    const QByteArray data = file.read(1024);

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
    if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
        return "XBM";
    }

    // Check xpm file.
    if (data.startsWith("/* XPM */")) {
        return "XPM";
    }

    QFileInfo info(path);
    return info.suffix().toUpper();
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

/**
 * @brief 根据翻转、旋转类型 \a orientation 对传入的图片 \a image 进行翻转旋转操作。
 * @param image         传入图片
 * @param orientation   翻转、旋转类型
 * @return 翻转、旋转后的图片
 */
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

UNIONIMAGESHARED_EXPORT bool rotateImageFile(int angel, const QString &path, QString &erroMsg, const QString &targetPath)
{
    if (angel % 90 != 0) {
        erroMsg = "unsupported angel";
        return false;
    }

    // 保存文件路径，若未设置则保存至原文件
    QString savePath = targetPath.isEmpty() ? path : targetPath;

    QString format = detectImageFormat(path);
    if (format == "SVG") {
        QImage image_copy;
        if (!loadStaticImageFromFile(path, image_copy, erroMsg)) {
            erroMsg = "rotate load QImage faild, path:" + path + "  ,format:+" + format;
            return false;
        }
        QSvgGenerator generator;
        generator.setFileName(savePath);
        generator.setViewBox(QRect(0, 0, image_copy.width(), image_copy.height()));
        QPainter rotatePainter;
        rotatePainter.begin(&generator);
        rotatePainter.resetTransform();
        rotatePainter.setRenderHint(QPainter::Antialiasing, true);
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
        int orientation = getOrientation(path);
        QImage image_copy(path);
        image_copy = adjustImageToRealPosition(image_copy, orientation);
        if (!image_copy.isNull()) {
            QTransform rotatematrix;
            rotatematrix.rotate(angel);
            image_copy = image_copy.transformed(rotatematrix, Qt::SmoothTransformation);
            if (image_copy.save(savePath, format.toLatin1().data(), SAVE_QUAITY_VALUE)) {
                return true;
            } else {
                erroMsg = "save image failed";
                return false;
            }
        }
        erroMsg = "rotate by qt failed";
        return false;
    }

    erroMsg = "not support rotate image format: " + format;
    return false;
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
        rotatePainter.setRenderHint(QPainter::Antialiasing, true);
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
    return false;
}

UNIONIMAGESHARED_EXPORT QMap<QString, QString> getAllMetaData(const QString &path)
{
    QMap<QString, QString> admMap;
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

    // The value of width and height might incorrect
    QImageReader reader(path);
    int w = reader.size().width();
    int h = reader.size().height();
    admMap.insert("Dimension", QString::number(w) + "x" + QString::number(h));
    // 记录图片宽高
    admMap.insert("Width", QString::number(w));
    admMap.insert("Height", QString::number(h));

    admMap.insert("FileName", info.fileName());
    //应该使用qfileinfo的格式
    admMap.insert("FileFormat", getFileFormat(path));
    admMap.insert("FileSize", size2Human(info.size()));

    return admMap;
}

UNIONIMAGESHARED_EXPORT bool isImageSupportRotate(const QString &path)
{
    return canSave(path) ;
}

UNIONIMAGESHARED_EXPORT int getOrientation(const QString &path)
{
    Q_UNUSED(path);
    return 1;
}

UNIONIMAGESHARED_EXPORT bool creatNewImage(QImage &res, int width, int height, int depth, SupportType type)
{
    Q_UNUSED(type);
    if (depth == 8) {
        res = QImage(width, height, QImage::Format_RGB888);
    } else if (depth == 16) {
        res = QImage(width, height, QImage::Format_RGB16);
    } else {
        res = QImage(width, height, QImage::Format_RGB32);
    }
    return true;
}

imageViewerSpace::ImageType getImageType(const QString &imagepath)
{
    imageViewerSpace::ImageType type = imageViewerSpace::ImageType::ImageTypeBlank;
    //新增获取图片是属于静态图还是动态图还是多页图
    if (!imagepath.isEmpty()) {
        QFileInfo fi(imagepath);
        if (!fi.exists()) {
            // 文件不存在返回空
            return imageViewerSpace::ImageTypeBlank;
        }

        QString strType = fi.suffix().toLower();
        //解决bug57394 【专业版1031】【看图】【5.6.3.74】【修改引入】pic格式图片变为翻页状态，不为动图且首张显示序号为0
        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(imagepath, QMimeDatabase::MatchContent);
        QMimeType mt1 = db.mimeTypeForFile(imagepath, QMimeDatabase::MatchExtension);
        QString path1 = mt.name();
        QString path2 = mt1.name();

        QImageReader imgreader(imagepath);
        int nSize = imgreader.imageCount();
        //
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
        } else {
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

};



