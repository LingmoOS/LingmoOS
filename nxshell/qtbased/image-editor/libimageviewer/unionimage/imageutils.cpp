// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "baseutils.h"
#include "imageutils.h"
#include "imageutils_libexif.h"
#include "imageutils_freeimage.h"
#include "snifferimageformat.h"
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
#include <QTemporaryDir>

namespace Libutils {

namespace image {

const QImage scaleImage(const QString &path, const QSize &size)
{
    if (!imageSupportRead(path)) {
        return QImage();
    }
    /*lmh0724使用USE_UNIONIMAGE*/
#ifdef USE_UNIONIMAGE
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
#else
    QImageReader reader(path);
    reader.setAutoTransform(true);
    if (! reader.canRead()) {
        qDebug() << "Can't read image: " << path;
        return QImage();
    }

    QSize tSize = reader.size();
    if (! tSize.isValid()) {
        QStringList rl = getAllMetaData(path).value("Dimension").split("x");
        if (rl.length() == 2) {
            tSize = QSize(QString(rl.first()).toInt(),
                          QString(rl.last()).toInt());
        }
    }
    tSize.scale(size, Qt::KeepAspectRatio);
    reader.setScaledSize(tSize);
    QImage tImg = reader.read();
    // Some format does not support scaling
    if (tImg.width() > size.width() || tImg.height() > size.height()) {
        if (tImg.isNull()) {
            return QImage();
        } else {
            // Save as supported format and scale it again
            const QString tmp = QDir::tempPath() + "/scale_tmp_image.png";
            QFile::remove(tmp);
            if (tImg.save(tmp, "png", 50)) {
                return scaleImage(tmp, size);
            } else {
                return QImage();
            }
        }
    } else
        return tImg;
#endif
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
#ifdef USE_UNIONIMAGE
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
#else
    const QString suffix = QFileInfo(path).suffix();
//解决freeimage不支持icns
    if (suffix == "icns") return true;
    //FIXME: file types below will cause freeimage to crash on loading,
    // take them here for good.
    QStringList errorList;
    errorList << "X3F";

    if (errorList.indexOf(suffix.toUpper()) != -1) {
        return false;
    }

    if (freeimage::isSupportsReading(path))
        return true;
    else {
        return (suffix == "svg");
    }
#endif
}

bool imageSupportSave(const QString &path)
{
    /*lmh0724使用USE_UNIONIMAGE*/
#ifdef USE_UNIONIMAGE
    return LibUnionImage_NameSpace::canSave(path);
#else
    const QString suffix = QFileInfo(path).suffix();
    //J2K格式暂时不支持
    if (suffix.toUpper() == "J2K") {
        return false;
    }
    // RAW image decode is too slow, and most of these does not support saving
    // RAW formats render incorrectly by freeimage
    const QStringList raws = QStringList()
                             << "CR2" << "CRW"   // Canon cameras
                             << "DCR" << "KDC"   // Kodak cameras
                             << "MRW"            // Minolta cameras
                             << "NEF"            // Nikon cameras
                             << "ORF"            // Olympus cameras
                             << "PEF"            // Pentax cameras
                             << "RAF"            // Fuji cameras
                             << "SRF"            // Sony cameras
                             << "PSD"
                             << "ICO"
                             << "TGA"
                             << "WEBP"
                             << "PBM"
                             << "XPM"
                             << "PPM"
                             << "PGM"
                             << "X3F";           // Sigma cameras

    if (raws.indexOf(suffix.toUpper()) != -1
            || QImageReader(path).imageCount() > 1) {
        return false;
    } else {
        return freeimage::canSave(path);
    }
#endif
}

bool rotate(const QString &path, int degree)
{
    /*lmh0724使用USE_UNIONIMAGE*/
#ifdef USE_UNIONIMAGE
    QString erroMsg;
    return LibUnionImage_NameSpace::rotateImageFIle(degree, path, erroMsg);
#else
    if (degree % 90 != 0)
        return false;

    int loadFlags = 0;
    int saveFlags = 0;
    FREE_IMAGE_FORMAT fif = freeimage::fFormat(path);
    switch (int(fif)) {
    case FIF_JPEG:
        loadFlags = JPEG_ACCURATE;          // Load the file with the best quality, sacrificing some speed
        saveFlags = JPEG_QUALITYSUPERB;     // Saves with superb quality (100:1)
        break;
    case FIF_JP2:
        // Freeimage3.17 does not support special load flags for JP2
        saveFlags = JP2_DEFAULT;            // Save with a 16:1 rate
        break;
    case FIF_BMP:
        saveFlags = BMP_DEFAULT;            // Save without any compression
        break;
    case FIF_EXR:
        saveFlags = EXR_NONE;               // Save with no compression
        break;
    case FIF_PNG:
        saveFlags = PNG_DEFAULT;   // Save without ZLib compression
        break;
    }

    FIBITMAP *dib = freeimage::readFileToFIBITMAP(path, loadFlags);
    FIBITMAP *rotated = FreeImage_Rotate(dib, -degree);
    if (rotated) {
        // Regenerate thumbnail if it's exits
        // Image formats that currently support thumbnail saving are
        // JPEG (JFIF formats), EXR, TGA and TIFF.
        if (FreeImage_GetThumbnail(dib)) {
            FIBITMAP *thumb = FreeImage_GetThumbnail(dib);
            FIBITMAP *rotateThumb = FreeImage_Rotate(thumb, -degree);
            FreeImage_SetThumbnail(rotated, rotateThumb);
            FreeImage_Unload(rotateThumb);
        }
    }

    bool v = freeimage::writeFIBITMAPToFile(rotated, path, saveFlags);
    FreeImage_Unload(dib);
    FreeImage_Unload(rotated);

    // The thumbnail should regenerate by caller
    removeThumbnail(path);

    return v;
#endif
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
#ifdef USE_UNIONIMAGE
    return LibUnionImage_NameSpace::getOrientation(path);
#else
    return freeimage::getOrientation(path);
#endif
}

#ifndef USE_UNIONIMAGE
const QImage loadTga(QString filePath, bool &success)
{
    QImage img;
    if (!img.load(filePath)) {

        // open the file
        std::fstream fsPicture(filePath.toUtf8().constData(), std::ios::in | std::ios::binary);

        if (!fsPicture.is_open()) {
            img = QImage(1, 1, QImage::Format_RGB32);
            img.fill(Qt::red);
            success = false;
            return img;
        }

        // some variables
        std::vector<std::uint8_t> *vui8Pixels;
        std::uint32_t ui32BpP;
        std::uint32_t ui32Width;
        std::uint32_t ui32Height;

        // read in the header
        std::uint8_t ui8x18Header[19] = { 0 };
        fsPicture.read(reinterpret_cast<char *>(&ui8x18Header), sizeof(ui8x18Header) - 1);

        //get variables
        vui8Pixels = new std::vector<std::uint8_t>;
        bool bCompressed;

        std::uint32_t ui32IDLength;
        std::uint32_t ui32PicType;
        std::uint32_t ui32PaletteLength;
        std::uint32_t ui32Size;

        // extract all information from header
        ui32IDLength = ui8x18Header[0];
        ui32PicType = ui8x18Header[2];
        ui32PaletteLength = ui8x18Header[6] * 0x100 + ui8x18Header[5];
        ui32Width = ui8x18Header[13] * 0x100 + ui8x18Header[12];
        ui32Height = ui8x18Header[15] * 0x100 + ui8x18Header[14];
        ui32BpP = ui8x18Header[16];

        // calculate some more information
        ui32Size = ui32Width * ui32Height * ui32BpP / 8;
        bCompressed = ui32PicType == 9 || ui32PicType == 10;
        //Q_UNUSED放下面，修复style问题
        Q_UNUSED(bCompressed);
        vui8Pixels->resize(ui32Size);

        // jump to the data block
        fsPicture.seekg(ui32IDLength + ui32PaletteLength, std::ios_base::cur);

        if (ui32PicType == 2 && (ui32BpP == 24 || ui32BpP == 32)) {
            fsPicture.read(reinterpret_cast<char *>(vui8Pixels->data()), ui32Size);
        }
        // else if compressed 24 or 32 bit
        else if (ui32PicType == 10 && (ui32BpP == 24 || ui32BpP == 32)) { // compressed
            std::uint8_t tempChunkHeader;
            std::uint8_t tempData[5];
            unsigned int tempByteIndex = 0;

            do {
                fsPicture.read(reinterpret_cast<char *>(&tempChunkHeader), sizeof(tempChunkHeader));

                if (tempChunkHeader >> 7) { // repeat count
                    // just use the first 7 bits
                    tempChunkHeader = (uint8_t(tempChunkHeader << 1) >> 1);

                    fsPicture.read(reinterpret_cast<char *>(&tempData), ui32BpP / 8);

                    for (int i = 0; i <= tempChunkHeader; i++) {
                        vui8Pixels->at(tempByteIndex++) = tempData[0];
                        vui8Pixels->at(tempByteIndex++) = tempData[1];
                        vui8Pixels->at(tempByteIndex++) = tempData[2];
                        if (ui32BpP == 32) vui8Pixels->at(tempByteIndex++) = tempData[3];
                    }
                } else {                    // data count
                    // just use the first 7 bits
                    tempChunkHeader = (uint8_t(tempChunkHeader << 1) >> 1);

                    for (int i = 0; i <= tempChunkHeader; i++) {
                        fsPicture.read(reinterpret_cast<char *>(&tempData), ui32BpP / 8);

                        vui8Pixels->at(tempByteIndex++) = tempData[0];
                        vui8Pixels->at(tempByteIndex++) = tempData[1];
                        vui8Pixels->at(tempByteIndex++) = tempData[2];
                        if (ui32BpP == 32) vui8Pixels->at(tempByteIndex++) = tempData[3];
                    }
                }
            } while (tempByteIndex < ui32Size);
        }
        // not useable format
        else {
            fsPicture.close();
            img = QImage(1, 1, QImage::Format_RGB32);
            img.fill(Qt::red);
            success = false;

            delete vui8Pixels;
            return img;
        }

        fsPicture.close();

        img = QImage(ui32Width, ui32Height, QImage::Format_RGB888);

        int pixelSize = ui32BpP == 32 ? 4 : 3;
        //TODO: write direct into img
        for (unsigned int x = 0; x < ui32Width; x++) {
            for (unsigned int y = 0; y < ui32Height; y++) {
                int valr = vui8Pixels->at(y * ui32Width * pixelSize + x * pixelSize + 2);
                int valg = vui8Pixels->at(y * ui32Width * pixelSize + x * pixelSize + 1);
                int valb = vui8Pixels->at(y * ui32Width * pixelSize + x * pixelSize);

                QColor value(valr, valg, valb);
                img.setPixelColor(x, y, value);
            }
        }

        img = img.mirrored();
        delete vui8Pixels;
        vui8Pixels = nullptr;
    }
    success = true;

    return img;
}
#endif

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
#ifdef USE_UNIONIMAGE
    QString errMsg;
    QSize realSize;
//    if (!UnionImage_NameSpace::loadStaticImageFromFile(path, tImg, realSize, errMsg)) {
//        qDebug() << errMsg;
//    }
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(path, tImg, errMsg)) {
        qDebug() << errMsg;
    }
#else
    QString format = DetectImageFormat(path);
    if (format.isEmpty()) {
        QImageReader reader(path);
        reader.setAutoTransform(true);
        if (reader.canRead()) {
            tImg = reader.read();
        } else if (path.contains(".tga")) {
            bool ret = false;
            tImg = utils::image::loadTga(path, ret);
        }
    } else {
        QImageReader readerF(path, format.toLatin1());
        readerF.setAutoTransform(true);
        if (readerF.canRead()) {
            tImg = readerF.read();
        } else {
            qWarning() << "can't read image:" << readerF.errorString()
                       << format;

            tImg = QImage(path);
        }
    }
#endif
    return tImg;
}

const QMap<QString, QString> getAllMetaData(const QString &path)
{
    /*lmh0724使用USE_UNIONIMAGE*/
#ifdef USE_UNIONIMAGE
    return LibUnionImage_NameSpace::getAllMetaData(path);
#else
    return freeimage::getAllMetaData(path);
#endif
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
        set.insert("Thumb::MTime", QString::number(info.lastModified().toTime_t()));
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
#ifdef USE_UNIONIMAGE
    QStringList list ;
    for (auto str : LibUnionImage_NameSpace::unionImageSupportFormat()) {
        str = "*." + str;
        list += str;
    }
    return list;
#else
    static QStringList list = fromByteArrayList(QImageReader::supportedImageFormats());

    return list;
#endif
}

//直接搬运文管代码
bool imageSupportWallPaper(const QString &path)
{
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchDefault);
    return mt.name().startsWith("image")
           && !mt.name().endsWith("svg+xml")
           && !mt.name().endsWith("raf")
           && !mt.name().endsWith("crw")
           && !mt.name().endsWith("x-portable-anymap");
}

/**
   @return 返回文件类型是否可以直接设置锁屏壁纸
        壁纸支持 jpeg jpg png bmp tif gif
        锁屏支持 jpeg jpg png
        且允许直接设置的图片大小不超过5MB (后端限制<32MB)
 */
bool imageSupportGreeterDirect(const QString &path)
{
    static qint64 s_maxFileSize = 5 * 1024 * 1024;
    if (QFileInfo(path).size() > s_maxFileSize) {
        return false;
    }

    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForFile(path, QMimeDatabase::MatchDefault);
    return "image/jpeg" == mt.name() || "image/png" == mt.name();
}

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
    if (rootPath.back() == "/") {
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

// 缓存文件路径及互斥锁
static QMutex s_CachePathMutex;
static QString s_CachePath;

/**
   @brief 取得图像缓存文件夹路径，未成功初始化将自动创建
   @threadsafe
 */
QString getCacheImagePath()
{
    QMutexLocker _locker(&s_CachePathMutex);
    if (s_CachePath.isEmpty()) {
        _locker.unlock();
        initCacheImageFolder();
        _locker.relock();
    }

    return s_CachePath;
}

/**
   @brief 初始化图像临时缓存文件夹（/tmp/image-viewer-cache_XXXXXX）并返回是否初始化成功，仅在加载图片时调用
    看图为多实例进程，不同实例创建缓存位置不同
   @threadsafe
 */
bool initCacheImageFolder()
{
    QMutexLocker _locker(&s_CachePathMutex);
    if (!s_CachePath.isEmpty()) {
        return true;
    }

    QTemporaryDir dir(QDir::tempPath() + QDir::separator() + "image-viewer-cache_XXXXXX");
    dir.setAutoRemove(false);
    if (dir.isValid()) {
        s_CachePath = dir.path();
        return true;
    }

    qWarning() << QString("Create cache image folder failed, %1:%2").arg(dir.path()).arg(dir.errorString());
    return false;
}

/**
   @brief 清空图像缓存文件夹并返回是否清理成功，在程序退出时调用。
   @threadsafe
 */
bool clearCacheImageFolder()
{
    QMutexLocker _locker(&s_CachePathMutex);
    if (s_CachePath.isEmpty()) {
        return false;
    }

    QDir cacheDir(s_CachePath);
    if (cacheDir.exists()) {
        bool ret = cacheDir.removeRecursively();
        if (ret) {
            s_CachePath.clear();
        }
        return ret;
    }
    return false;
}

/**
 * @brief 返回图像缓存中是否缓存 \a fileName 文件
 */
bool checkCacheImage(const QString &fileName)
{
    QDir cacheDir(getCacheImagePath());
    return cacheDir.exists(fileName);
}

}  // namespace image

}  //namespace utils
