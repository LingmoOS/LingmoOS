// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "imageutils.h"
#include "unionimage.h"
#include <fstream>

#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMimeDatabase>
#include <QMutexLocker>
#include <QPixmapCache>
#include <QProcess>
#include <QReadWriteLock>
#include <QUrl>
#include <QApplication>

namespace Libutils {

namespace image {

const QImage scaleImage(const QString &path, const QSize &size)
{
    if (!imageSupportRead(path)) {
        return QImage();
    }
    /*lmh0724使用USE_UNIONIMAGE*/

    QImage tImg(size, QImage::Format_ARGB32);
    QString errMsg;
    QSize realSize;
//    if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, realSize, errMsg)) {
//        qDebug() << errMsg;
//    }
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
    }
    if (tImg.size() != size) { //调用加速接口失败，主动进行缩放
        tImg = tImg.scaled(size);
    }
    return tImg;
}

const QDateTime getCreateDateTime(const QString &path)
{
    QDateTime dt; /*= libexif::getCreateDateTime(path);*/

    // fallback to metadata.
    if (!dt.isValid()) {
        QString s;
        s = getAllMetaData(path).value("DateTimeOriginal");
        if (s.isEmpty()) {
            s = getAllMetaData(path).value("DateTimeDigitized");
        }
        if (s.isEmpty()) {
            s = QDateTime::currentDateTime().toString();
        }
        dt = QDateTime::fromString(s, "yyyy.MM.dd HH:mm:ss");
    }

    // fallback to file create time.
    if (!dt.isValid()) {
        QFileInfo finfo(path);
        dt = finfo.birthTime();
    }

    // fallback to today.
    if (!dt.isValid()) {
        dt = QDateTime::currentDateTime();
    }

    return dt;
}

bool imageSupportRead(const QString &path)
{
    /*lmh0724使用USE_UNIONIMAGE*/
    //修正论坛上提出的格式判断错误，应该采用真实格式
    //20210220真实格式来做判断
    QMap<QString, QString> dataMap = getAllMetaData(path);
    const QString suffix = dataMap.value("FileFormat");
    QStringList errorList;
    errorList << "X3F";
    if (errorList.indexOf(suffix.toUpper()) != -1) {
        return false;
    }
    return LibUnionImage_NameSpace::unionImageSupportFormat().contains(suffix.toUpper());
}

bool imageSupportSave(const QString &path)
{
    /*lmh0724使用USE_UNIONIMAGE*/
    return LibUnionImage_NameSpace::canSave(path);
}

//bool imageSupportWrite(const QString &path)
//{
//        /*lmh0724使用USE_UNIONIMAGE*/
//#ifdef USE_UNIONIMAGE
//    return UnionImage_NameSpace::canSave(path);
//#else
//    return freeimage::isSupportsWriting(path);
//#endif
//}

bool rotate(const QString &path, int degree)
{
    /*lmh0724使用USE_UNIONIMAGE*/
    QString erroMsg;
    return LibUnionImage_NameSpace::rotateImageFIle(degree, path, erroMsg);
}

/*!
 * \brief cutSquareImage
 * Cut square image
 * \param pixmap
 * \return
 */
const QPixmap cutSquareImage(const QPixmap &pixmap)
{
    return Libutils::image::cutSquareImage(pixmap, pixmap.size());
}

/*!
 * \brief cutSquareImage
 * Scale and cut a square image
 * \param pixmap
 * \param size
 * \return
 */
const QPixmap cutSquareImage(const QPixmap &pixmap, const QSize &size)
{
    const qreal ratio = qApp->devicePixelRatio();
    QImage img = pixmap.toImage().scaled(size * ratio,
                                         Qt::KeepAspectRatioByExpanding,
                                         Qt::SmoothTransformation);
    const QSize s(size * ratio);
    const QRect r(0, 0, s.width(), s.height());

    img = img.copy(QRect(img.rect().center() - r.center(), s));
    img.setDevicePixelRatio(ratio);

    return QPixmap::fromImage(img);
}

/*!
 * \brief getImagesInfo
        types<< ".BMP";
        types<< ".GIF";
        types<< ".JPG";
        types<< ".JPEG";
        types<< ".PNG";
        types<< ".PBM";
        types<< ".PGM";
        types<< ".PPM";
        types<< ".XBM";
        types<< ".XPM";
        types<< ".SVG";

        types<< ".DDS";
        types<< ".ICNS";
        types<< ".JP2";
        types<< ".MNG";
        types<< ".TGA";
        types<< ".TIFF";
        types<< ".WBMP";
        types<< ".WEBP";
 * \param dir
 * \param recursive
 * \return
 */
const QFileInfoList getImagesInfo(const QString &dir, bool recursive)
{
    QFileInfoList infos;

    if (! recursive) {
        auto nsl = QDir(dir).entryInfoList(QDir::Files);
        for (QFileInfo info : nsl) {
            if (imageSupportRead(info.absoluteFilePath())) {
                infos << info;
            }
        }
        return infos;
    }

    QDirIterator dirIterator(dir,
                             QDir::Files,
                             QDirIterator::Subdirectories);
    while (dirIterator.hasNext()) {
        dirIterator.next();
        if (imageSupportRead(dirIterator.fileInfo().absoluteFilePath())) {
            infos << dirIterator.fileInfo();
#include "imageutils.h"
        }
    }

    return infos;
}

int getOrientation(const QString &path)
{
    return LibUnionImage_NameSpace::getOrientation(path);
}

/*!
 * \brief getRotatedImage
 * Rotate image base on the exif orientation
 * \param path
 * \return
 */
const QImage getRotatedImage(const QString &path)
{


    QImage tImg;
    /*lmh0724使用USE_UNIONIMAGE*/
    QString errMsg;
    QSize realSize;
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
    }
    return tImg;
}

const QMap<QString, QString> getAllMetaData(const QString &path)
{
    /*lmh0724使用USE_UNIONIMAGE*/
    return LibUnionImage_NameSpace::getAllMetaData(path);
}

const QPixmap cachePixmap(const QString &path)
{
    QPixmap pp;
    if (! QPixmapCache::find(path, &pp)) {
        pp = QPixmap(path);
        QPixmapCache::insert(path, pp);
    }
    return pp;
}

const QString toMd5(const QByteArray &data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
}

/*!
 * \brief thumbnailAttribute
 * Read the attributes of file for generage thumbnail
 * \param url
 * \return
 */
QMap<QString, QString> thumbnailAttribute(const QUrl  &url)
{
    QMap<QString, QString> set;

    if (url.isLocalFile()) {
        const QString path = url.path();
        QFileInfo info(path);
        set.insert("Thumb::Mimetype", QMimeDatabase().mimeTypeForFile(path).name());
        set.insert("Thumb::Size", QString::number(info.size()));
        set.insert("Thumb::URI", url.toString());
        set.insert("Thumb::MTime", QString::number(info.lastModified().toSecsSinceEpoch()));
        set.insert("Software", "Deepin Image Viewer");

        QImageReader reader(path);
        if (reader.canRead()) {
            set.insert("Thumb::Image::Width", QString::number(reader.size().width()));
            set.insert("Thumb::Image::Height", QString::number(reader.size().height()));
        }
    } else {
        //TODO for other's scheme
    }

    return set;
}

const QString thumbnailCachePath()
{
    QString cacheP;

    QStringList systemEnvs = QProcess::systemEnvironment();
    for (QString it : systemEnvs) {
        QStringList el = it.split("=");
        if (el.length() == 2 && el.first() == "XDG_CACHE_HOME") {
            cacheP = el.last();
            break;
        }
    }
    cacheP = cacheP.isEmpty() ? (QDir::homePath() + "/.cache") : cacheP;

    // Check specific size dir
    const QString thumbCacheP = cacheP + "/thumbnails";
    QDir().mkpath(thumbCacheP + "/normal");
    QDir().mkpath(thumbCacheP + "/large");
    QDir().mkpath(thumbCacheP + "/fail");

    return thumbCacheP;
}

QMutex mutex;
const QPixmap getThumbnail(const QString &path, bool cacheOnly)
{
    QMutexLocker locker(&mutex);
    //优先读取自身缓存的图片
//    if (dApp->m_imagemap.value(path).isNull()) {
//        return dApp->m_imagemap.value(path);
//    }
    const QString cacheP = thumbnailCachePath();
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5s = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    const QString encodePath = cacheP + "/large/" + md5s + ".png";
    const QString failEncodePath = cacheP + "/fail/" + md5s + ".png";
    if (QFileInfo(encodePath).exists()) {
        return QPixmap(encodePath);
    }
    /*lmh0724使用USE_UNIONIMAGE*/
    else if (QFileInfo(failEncodePath).exists()) {
        qDebug() << "Fail-thumbnail exist, won't regenerate: " ;
        return QPixmap();
    } else {
        // Try to generate thumbnail and load it later
        if (! cacheOnly && generateThumbnail(path)) {
            return QPixmap(encodePath);
        } else {
            return QPixmap();
        }
    }
}

/*!
 * \brief generateThumbnail
 * Generate and save thumbnail for specific size
 * \return
 */
bool generateThumbnail(const QString &path)
{
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5 = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    const auto attributes = thumbnailAttribute(url);
    const QString cacheP = thumbnailCachePath();

    // Large thumbnail
    QImage lImg = scaleImage(path,
                             QSize(THUMBNAIL_MAX_SIZE, THUMBNAIL_MAX_SIZE));

    // Normal thumbnail
    QImage nImg = lImg.scaled(
                      QSize(THUMBNAIL_NORMAL_SIZE, THUMBNAIL_NORMAL_SIZE)
                      , Qt::KeepAspectRatio
                      , Qt::SmoothTransformation);

    // Create filed thumbnail
    if (lImg.isNull() || nImg.isNull()) {
        const QString failedP = cacheP + "/fail/" + md5 + ".png";
        QImage img(1, 1, QImage::Format_ARGB32_Premultiplied);
        const auto keys = attributes.keys();
        for (QString key : keys) {
            img.setText(key, attributes[key]);
        }

        qDebug() << "Save failed thumbnail:" << img.save(failedP,  "png")
                 << failedP << url;
        return false;
    } else {
        for (QString key : attributes.keys()) {
            lImg.setText(key, attributes[key]);
            nImg.setText(key, attributes[key]);
        }
        const QString largeP = cacheP + "/large/" + md5 + ".png";
        const QString normalP = cacheP + "/normal/" + md5 + ".png";
        if (lImg.save(largeP, "png", 50) && nImg.save(normalP, "png", 50)) {
            return true;
        } else {
            return false;
        }
    }
}

const QString thumbnailPath(const QString &path, ThumbnailType type)
{
    const QString cacheP = thumbnailCachePath();
    const QUrl url = QUrl::fromLocalFile(path);
    const QString md5s = toMd5(url.toString(QUrl::FullyEncoded).toLocal8Bit());
    QString tp;
    switch (type) {
    case ThumbNormal:
        tp = cacheP + "/normal/" + md5s + ".png";
        break;
    case ThumbLarge:
        tp = cacheP + "/large/" + md5s + ".png";
        break;
    case ThumbFail:
        tp = cacheP + "/fail/" + md5s + ".png";
        break;
    default:
        break;
    }
    return tp;
}

void removeThumbnail(const QString &path)
{
    QFile(thumbnailPath(path, ThumbLarge)).remove();
    QFile(thumbnailPath(path, ThumbNormal)).remove();
    QFile(thumbnailPath(path, ThumbFail)).remove();
}

bool thumbnailExist(const QString &path, ThumbnailType type)
{
    if (QFileInfo(thumbnailPath(path, type)).exists()
//            || QFileInfo(thumbnailPath(path, ThumbNormal)).exists()
//            || QFileInfo(thumbnailPath(path, ThumbFail)).exists()
       ) {
        return true;
    } else {
        return false;
    }
}
/*
static QStringList fromByteArrayList(const QByteArrayList &list)
{
    QStringList sList;

    for (const QByteArray &i : list)
        sList << "*." + QString::fromLatin1(i);

    // extern image format
    sList << "*.cr2"
          << "*.dng"
          << "*.nef"
          << "*.mef"
          << "*.raf"
          << "*.raw"
          << "*.orf"
          << "*.mrw"
          << "*.jpe"
          << "*.xbm";

    return sList;
}
*/
QStringList supportedImageFormats()
{
    /*lmh0724使用USE_UNIONIMAGE*/
    QStringList list ;
    for (auto str : LibUnionImage_NameSpace::unionImageSupportFormat()) {
        str = "*." + str;
        list += str;
    }
    return list;

}




bool imageSupportWallPaper(const QString &path)
{
    bool iRet = false;
    QStringList listsupportWallPaper;
    listsupportWallPaper << "bmp"
//                         << "cod"
                         << "png"
                         << "gif"
                         << "ief"
                         << "jpe"
                         << "jpeg"
                         << "jpg"
                         << "jfif"
//                         << "svg"
                         << "tif"
                         << "tiff"
//                         << "ras"
//                         << "cmx"
//                         << "ico"
//                         << "pnm"
//                         << "pbm"
//                         << "pgm"
//                         << "ppm"
//                         << "rgb"
//                         << "xbm"
//                         << "xpm"
//                         << "xwd"
                         ;
    //
    QImageReader reader(path);
    if (reader.imageCount() > 0) {

        //2020/11/12 bug54279
        if (listsupportWallPaper.contains(reader.format().toLower()) && listsupportWallPaper.contains(QFileInfo(path).suffix().toLower())) {
            iRet = true;
        }
        //20201012 lmh ico不支持设置壁纸
//        else {
//            const QString suffix = QFileInfo(path).suffix();
//            if(suffix=="ico")
//            {
//                iRet=true;
//            }
//        }
    }

    return iRet;
}

//bool suffixisImage(const QString &path)
//{
//#ifdef USE_UNIONIMAGE
//    return UnionImage_NameSpace::suffixisImage(path);
//#else
//    bool iRet = false;
//    QFileInfo info(path);
//    QMimeDatabase db;
//    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchContent);
//    QMimeType mt1 = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
//    QString str = info.suffix();
//    // if (!m_nosupportformat.contains(str, Qt::CaseSensitive)) {
//    if (mt.name().startsWith("image/") || mt.name().startsWith("video/x-mng") ||
//            mt1.name().startsWith("image/") || mt1.name().startsWith("video/x-mng")) {
//        iRet = true;
//    }
//    return iRet;
//#endif
//}

QString makeVaultLocalPath(const QString &path, const QString &base)
{
    QString basePath = base;
    if (basePath.isEmpty()) {
        basePath = VAULT_DECRYPT_DIR_NAME;
    }
    return VAULT_BASE_PATH + QDir::separator() + basePath + (path.startsWith('/') ? "" : "/") + path;
}

bool isVaultFile(const QString &path)
{
    bool bVaultFile = false;
    QString rootPath = makeVaultLocalPath("", "");
    if (rootPath.back() == QChar('/')) {
        rootPath.chop(1);
    }

    if (path.contains(rootPath) && path.left(6) != "search") {
        bVaultFile = true;
    }

    return bVaultFile;

}
bool isCanRemove(const QString &path)
{
    bool bRet = true;
    QString trashPath = QDir::homePath() + "/.local/share/Trash";
    //新增保险箱的判断,回收站判断
    if (isVaultFile(path) || path.contains(trashPath)) {
        bRet = false;
    }
    return bRet;
}
}  // namespace image

}  //namespace utils
