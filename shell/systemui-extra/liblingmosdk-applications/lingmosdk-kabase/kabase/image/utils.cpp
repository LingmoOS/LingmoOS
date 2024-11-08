/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QFileInfo>
#include <QFile>
#include <QByteArray>
#include <QProcess>

#include "log.hpp"
#include "image/utils.hpp"

namespace kdk
{
namespace kabase
{

QHash<QString, image::ImageFormat> Utils::getSuffixFormatMap(void)
{
    QHash<QString, image::ImageFormat> suffixFormatMap;

    suffixFormatMap.insert("BMP", image::ImageFormat::BMP);
    suffixFormatMap.insert("ICO", image::ImageFormat::ICO);
    suffixFormatMap.insert("JPG", image::ImageFormat::JPG);
    suffixFormatMap.insert("JPE", image::ImageFormat::JPE);
    suffixFormatMap.insert("JPS", image::ImageFormat::JPS);
    suffixFormatMap.insert("JPEG", image::ImageFormat::JPEG);
    suffixFormatMap.insert("KOALA", image::ImageFormat::KOALA);
    suffixFormatMap.insert("KOA", image::ImageFormat::KOA);
    suffixFormatMap.insert("LBM", image::ImageFormat::LBM);
    suffixFormatMap.insert("IFF", image::ImageFormat::IFF);
    suffixFormatMap.insert("MNG", image::ImageFormat::MNG);
    suffixFormatMap.insert("PBM", image::ImageFormat::PBM);
    suffixFormatMap.insert("PBMRAW", image::ImageFormat::PBMRAW);
    suffixFormatMap.insert("PCD", image::ImageFormat::PCD);
    suffixFormatMap.insert("PCX", image::ImageFormat::PCX);
    suffixFormatMap.insert("PGM", image::ImageFormat::PGM);
    suffixFormatMap.insert("PGMRAW", image::ImageFormat::PGMRAW);
    suffixFormatMap.insert("PNG", image::ImageFormat::PNG);
    suffixFormatMap.insert("PPM", image::ImageFormat::PPM);
    suffixFormatMap.insert("PPMRAW", image::ImageFormat::PPMRAW);
    suffixFormatMap.insert("RAS", image::ImageFormat::RAS);
    suffixFormatMap.insert("TGA", image::ImageFormat::TGA);
    suffixFormatMap.insert("TARGA", image::ImageFormat::TARGA);
    suffixFormatMap.insert("TIFF", image::ImageFormat::TIFF);
    suffixFormatMap.insert("TIF", image::ImageFormat::TIF);
    suffixFormatMap.insert("PCT", image::ImageFormat::PCT);
    suffixFormatMap.insert("PIC", image::ImageFormat::PIC);
    suffixFormatMap.insert("PICT", image::ImageFormat::PICT);
    suffixFormatMap.insert("WEBP", image::ImageFormat::WEBP);
    suffixFormatMap.insert("JXR", image::ImageFormat::JXR);
    suffixFormatMap.insert("PFM", image::ImageFormat::PFM);
    suffixFormatMap.insert("DDS", image::ImageFormat::DDS);
    suffixFormatMap.insert("GIF", image::ImageFormat::GIF);
    suffixFormatMap.insert("HDR", image::ImageFormat::HDR);
    suffixFormatMap.insert("FAX", image::ImageFormat::FAX);
    suffixFormatMap.insert("G3", image::ImageFormat::G3);
    suffixFormatMap.insert("SGI", image::ImageFormat::SGI);
    suffixFormatMap.insert("CUT", image::ImageFormat::CUT);
    suffixFormatMap.insert("JNG", image::ImageFormat::JNG);
    suffixFormatMap.insert("WBMP", image::ImageFormat::WBMP);
    suffixFormatMap.insert("PSD", image::ImageFormat::PSD);
    suffixFormatMap.insert("XBM", image::ImageFormat::XBM);
    suffixFormatMap.insert("XPM", image::ImageFormat::XPM);
    suffixFormatMap.insert("EXR", image::ImageFormat::EXR);
    suffixFormatMap.insert("JP2", image::ImageFormat::JP2);
    suffixFormatMap.insert("J2K", image::ImageFormat::J2K);
    suffixFormatMap.insert("SVG", image::ImageFormat::SVG);
    suffixFormatMap.insert("APNG", image::ImageFormat::APNG);

    return suffixFormatMap;
}

image::ImageFormat Utils::getImageFormat(QString path)
{
    QFileInfo fileInfo(path);
    QString suffix = fileInfo.suffix().toUpper();
    image::ImageFormat suffixFormat = Utils::getSuffixFormatMap().value(suffix);
    image::ImageFormat realFormat = Utils::getImageRealFormat(path);

    if (realFormat != image::ImageFormat::UNKNOW && suffixFormat != realFormat) {
        return realFormat;
    }

    return suffixFormat;
}

FREE_IMAGE_FORMAT Utils::getImageFiFormat(QString path)
{
    image::ImageFormat imageFormat = Utils::getImageFormat(path);
    switch (imageFormat) {
    case image::ImageFormat::BMP:
        return FIF_BMP;
    case image::ImageFormat::ICO:
        return FIF_ICO;
    case image::ImageFormat::JPG:
    case image::ImageFormat::JPE:
    case image::ImageFormat::JPS:
    case image::ImageFormat::JPEG:
        return FIF_JPEG;
    case image::ImageFormat::KOALA:
    case image::ImageFormat::KOA:
        return FIF_KOALA;
    case image::ImageFormat::LBM:
    case image::ImageFormat::IFF:
        return FIF_LBM;
    case image::ImageFormat::MNG:
        return FIF_MNG;
    case image::ImageFormat::PBM:
        return FIF_PBM;
    case image::ImageFormat::PBMRAW:
        return FIF_PBMRAW;
    case image::ImageFormat::PCD:
        return FIF_PCD;
    case image::ImageFormat::PCX:
        return FIF_PCX;
    case image::ImageFormat::PGM:
        return FIF_PGM;
    case image::ImageFormat::PGMRAW:
        return FIF_PGMRAW;
    case image::ImageFormat::PNG:
        return FIF_PNG;
    case image::ImageFormat::PPM:
        return FIF_PPM;
    case image::ImageFormat::PPMRAW:
        return FIF_PPMRAW;
    case image::ImageFormat::RAS:
        return FIF_RAS;
    case image::ImageFormat::TGA:
    case image::ImageFormat::TARGA:
        return FIF_TARGA;
    case image::ImageFormat::TIF:
    case image::ImageFormat::TIFF:
        return FIF_TIFF;
    case image::ImageFormat::PCT:
    case image::ImageFormat::PIC:
    case image::ImageFormat::PICT:
        return FIF_TIFF;
    case image::ImageFormat::WEBP:
        return FIF_WEBP;
    case image::ImageFormat::JXR:
        return FIF_JXR;
    case image::ImageFormat::PFM:
        return FIF_PFM;
    case image::ImageFormat::DDS:
        return FIF_DDS;
    case image::ImageFormat::GIF:
        return FIF_GIF;
    case image::ImageFormat::HDR:
        return FIF_HDR;
    case image::ImageFormat::FAX:
    case image::ImageFormat::G3:
        return FIF_FAXG3;
    case image::ImageFormat::SGI:
        return FIF_SGI;
    case image::ImageFormat::CUT:
        return FIF_CUT;
    case image::ImageFormat::JNG:
        return FIF_JNG;
    case image::ImageFormat::WBMP:
        return FIF_WBMP;
    case image::ImageFormat::PSD:
        return FIF_PSD;
    case image::ImageFormat::XBM:
        return FIF_XBM;
    case image::ImageFormat::XPM:
        return FIF_XPM;
    case image::ImageFormat::EXR:
        return FIF_EXR;
    case image::ImageFormat::JP2:
        return FIF_JP2;
    case image::ImageFormat::J2K:
        return FIF_J2K;
    default:
        return FIF_UNKNOWN;
    }
}

image::ImageFormat Utils::getImageRealFormat(QString path)
{
    QFile file(path);
    if (!file.exists()) {
        return image::ImageFormat::UNKNOW;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return image::ImageFormat::UNKNOW;
    }

    const QByteArray data = file.read(64);

    /* Check bmp file */
    if (data.startsWith("BM")) {
        return image::ImageFormat::BMP;
    }

    /* Check dds file */
    if (data.startsWith("DDS")) {
        return image::ImageFormat::DDS;
    }

    /* Check gif file */
    if (data.startsWith("GIF8")) {
        return image::ImageFormat::GIF;
    }

    /* Check Max OS icons file */
    if (data.startsWith("icns")) {
        return image::ImageFormat::UNKNOW;
    }

    /* Check jpeg file */
    if (data.startsWith("\xff\xd8")) {
        return image::ImageFormat::JPEG;
    }

    /* Check mng file */
    if (data.startsWith("\x8a\x4d\x4e\x47\x0d\x0a\x1a\x0a")) {
        return image::ImageFormat::MNG;
    }

    /* Check net pbm file (BitMap) */
    if (data.startsWith("P1") || data.startsWith("P4")) {
        return image::ImageFormat::PBM;
    }

    /* Check pgm file (GrayMap) */
    if (data.startsWith("P2") || data.startsWith("P5")) {
        return image::ImageFormat::PGM;
    }

    /* Check ppm file (PixMap) */
    if (data.startsWith("P3") || data.startsWith("P6")) {
        return image::ImageFormat::PPM;
    }

    /* Check png file */
    if (data.startsWith("\x89PNG\x0d\x0a\x1a\x0a")) {
        return image::ImageFormat::PNG;
    }

    /* Check svg file */
    if (data.indexOf("<svg") > -1) {
        return image::ImageFormat::SVG;
    }

    /* Check tiff file */
    if (data.startsWith("MM\x00\x2a") || data.startsWith("II\x2a\x00")) {
        /* big-endian, little-endian */
        return image::ImageFormat::TIFF;
    }

    /* Check webp file */
    if (data.startsWith("RIFFr\x00\x00\x00WEBPVP")) {
        return image::ImageFormat::WEBP;
    }

    /* Check xbm file */
    if (data.indexOf("#define max_width ") > -1 && data.indexOf("#define max_height ") > -1) {
        return image::ImageFormat::XBM;
    }

    /* Check xpm file */
    if (data.startsWith("/* XPM */")) {
        return image::ImageFormat::XPM;
    }

    return image::ImageFormat::UNKNOW;
}

bool Utils::copyFile(QString srcPath, QString dstPath)
{
    if (!QFile::copy(srcPath, dstPath)) {
        warning << "kabase : copy file fail!";
        return false;
    }

    return true;
}

bool Utils::deleteDir(QString path)
{
    QString command = QString("rm -rf %1").arg(path);
    if (QProcess::execute(command)) {
        warning << "kabase : delete dir fail!";
        return false;
    }

    return true;
}

} /* namespace kabase */
} /* namespace kdk */
