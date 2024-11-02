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

#include "lingmoimagecodec.hpp"

namespace kdk
{
namespace kabase
{

LingmoImageCodecSignals *LingmoImageCodec::m_signalObj = nullptr;

LingmoImageCodecSignals *LingmoImageCodec::getSignalObj()
{
    if (m_signalObj == nullptr) {
        m_signalObj = new LingmoImageCodec;
    }
    return m_signalObj;
}

const QStringList LingmoImageCodec::m_opencvSupportFormats =
    { //"exr","EXR",有问题，opencv只支持其查看，不支持写入。此格式支持设置为壁纸
        "jpg", "jpe", "jpeg", "pbm", "pgm", "ppm", "sr", "ras", "png", "bmp", "dib", "tiff", "tif", "webp"};
const QStringList LingmoImageCodec::m_freeimageSupportFormats = { // 10种可读取可保存的格式
    "exr", "psd", "jfi", "jif", "jp2", "j2k", "jng", "wbmp", "xbm", "pnm"};
const QStringList LingmoImageCodec::m_otherSupportFormats = {"tga", "svg", "gif", "apng", "ico", "xpm"};
const QString LingmoImageCodec::m_supportFrmatsClassify = "(*.jpg *.jpe *.jpeg);;"
                                                         "(*.pnm *.pbm);;"
                                                         "(*.pgm *.ppm);;"
                                                         "(*.tiff *.tif);;"
                                                         "(*.png *.apng);;"
                                                         "(*.ico);;(*.sr);;"
                                                         "(*.ras);;(*.tga);;"
                                                         "(*.svg);;(*.gif);;"
                                                         "(*.webp);;(*.bmp *.dib);;"
                                                         "(*.exr);;(*.psd);;(*.jfi *.jif);;"
                                                         "(*.jp2 *.j2k *.jng);;(*.wbmp);;"
                                                         "(*.xbm *.xpm)";
const QStringList LingmoImageCodec::m_supportFormats = LingmoImageCodec::m_opencvSupportFormats
                                                      + LingmoImageCodec::m_freeimageSupportFormats
                                                      + LingmoImageCodec::m_otherSupportFormats;

QHash<QString, int> LingmoImageCodec::m_formats = LingmoImageCodec::creatFormats();
QHash<QString, int> LingmoImageCodec::creatFormats()
{
    QHash<QString, int> tmpFormats;
    tmpFormats.insert("BMP", FIF_BMP);
    tmpFormats.insert("ICO", FIF_ICO);
    tmpFormats.insert("JPG", FIF_JPEG);
    tmpFormats.insert("JPE", FIF_JPEG);
    tmpFormats.insert("JPS", FIF_JPEG);
    tmpFormats.insert("JPEG", FIF_JPEG);
    tmpFormats.insert("KOALA", FIF_KOALA);
    tmpFormats.insert("KOA", FIF_KOALA);
    tmpFormats.insert("LBM", FIF_LBM);
    tmpFormats.insert("IFF", FIF_LBM);
    tmpFormats.insert("MNG", FIF_MNG);
    tmpFormats.insert("PBM", FIF_PBM);
    tmpFormats.insert("PBMRAW", FIF_PBMRAW);
    tmpFormats.insert("PCD", FIF_PCD);
    tmpFormats.insert("PCX", FIF_PCX);
    tmpFormats.insert("PGM", FIF_PGM);
    tmpFormats.insert("PGMRAW", FIF_PGMRAW);
    tmpFormats.insert("PNG", FIF_PNG);
    tmpFormats.insert("PPM", FIF_PPM);
    tmpFormats.insert("PPMRAW", FIF_PPMRAW);
    tmpFormats.insert("RAS", FIF_RAS);
    tmpFormats.insert("TGA", FIF_TARGA);
    tmpFormats.insert("TARGA", FIF_TARGA);
    tmpFormats.insert("TIFF", FIF_TIFF);
    tmpFormats.insert("TIF", FIF_TIFF);
    tmpFormats.insert("PCT", FIF_PICT);
    tmpFormats.insert("PIC", FIF_PICT);
    tmpFormats.insert("PICT", FIF_PICT);
    tmpFormats.insert("WEBP", FIF_WEBP);
    tmpFormats.insert("JXR", FIF_JXR);
    tmpFormats.insert("PFM", FIF_PFM);
    tmpFormats.insert("DDS", FIF_DDS);
    tmpFormats.insert("GIF", FIF_GIF);
    tmpFormats.insert("HDR", FIF_HDR);
    tmpFormats.insert("FAX", FIF_FAXG3);
    tmpFormats.insert("G3", FIF_FAXG3);
    tmpFormats.insert("SGI", FIF_SGI);
    tmpFormats.insert("CUT", FIF_CUT);
    tmpFormats.insert("JNG", FIF_JNG);
    tmpFormats.insert("WBMP", FIF_WBMP);
    tmpFormats.insert("PSD", FIF_PSD);
    tmpFormats.insert("XBM", FIF_XBM);
    tmpFormats.insert("XPM", FIF_XPM);
    tmpFormats.insert("EXR", FIF_EXR);
    tmpFormats.insert("JP2", FIF_JP2);
    tmpFormats.insert("J2K", FIF_J2K);
    return tmpFormats;
}

QStringList LingmoImageCodec::getSupportFormats()
{
    return m_supportFormats;
}

QString LingmoImageCodec::getOpenFileFormats()
{
    return m_supportFrmatsClassify;
}

} // namespace kabase
} // namespace kdk
