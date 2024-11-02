#include "lingmoimagecodec.h"

LingmoImageCodecSignals *LingmoImageCodec::m_signalObj = nullptr;

LingmoImageCodecSignals *LingmoImageCodec::getSignalObj()
{
    if (m_signalObj == nullptr) {
        m_signalObj = new LingmoImageCodec;
    }
    return m_signalObj;
}
bool LingmoImageCodec::g_needSaveAs = false;
const QStringList LingmoImageCodec::m_opencvSupportFormats =
    { //"exr","EXR",有问题，opencv只支持其查看，不支持写入。此格式支持设置为壁纸
        "sr", "ras"};
const QStringList LingmoImageCodec::m_freeimageSupportFormats = { // 10种可读取可保存的格式
    "exr", "psd",  "jfi", "jif", "jp2", "j2k", "jng", "wbmp", "xbm", "pnm", "tiff",
    "tif", "webp", "bmp", "dib", "png", "jpg", "jpe", "jpeg", "pbm", "pgm", "ppm"};
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


const QStringList LingmoImageCodec::m_supportFormatsMT = {"image/vnd.microsoft.icon",
                                                         "image/x-jp2-codestream",
                                                         "image/vnd.adobe.photoshop",
                                                         "image/webp",
                                                         "image/svg+xml",
                                                         "image/x-jng",
                                                         "image/jp2",
                                                         "image/gif",
                                                         "image/vnd.wap.wbmp",
                                                         "image/x-xbitmap",
                                                         "image/x-xpixmap",
                                                         "image/x-exr",
                                                         "image/x-portable-graymap",
                                                         "image/x-portable-anymap",
                                                         "image/x-portable-pixmap",
                                                         "image/x-cmu-raster",
                                                         "image/x-portable-bitmap",
                                                         "image/x-tga",
                                                         "image/x-sun-raster",
                                                         "image/bmp",
                                                         "image/jpeg",
                                                         "image/tiff",
                                                         "image/png"};
QHash<QString, int> LingmoImageCodec::m_formats = LingmoImageCodec::creatFormats();
QStringList LingmoImageCodec::g_allFormat = {
    "BMP",    "ICO", "JPG", "JPE",  "JPS",    "JPEG", "KOALA", "KOA",    "LBM", "IFF", "MNG",   "PBM",
    "PBMRAW", "PCD", "PCX", "PGM",  "PGMRAW", "PNG",  "PPM",   "PPMRAW", "RAS", "TGA", "TARGA", "TIFF",
    "TIF",    "PCT", "PIC", "PICT", "WEBP",   "JXR",  "PFM",   "DDS",    "GIF", "HDR", "FAX",   "G3",
    "SGI",    "CUT", "JNG", "WBMP", "PSD",    "XBM",  "XPM",   "EXR",    "JP2", "J2K"};
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
QStringList LingmoImageCodec::g_jpegList = {"jpg", "jpe", "jps", "jpeg", "jif", "jfi"};
QStringList LingmoImageCodec::g_bmpList = {"bmp", "dib"};
QStringList LingmoImageCodec::g_koaList = {"koa", "koala"};
QStringList LingmoImageCodec::g_lbmList = {"lbm", "iff"};
QStringList LingmoImageCodec::g_tgaList = {"tga", "targa"};
QStringList LingmoImageCodec::g_tiffList = {"tiff", "tif"};
QStringList LingmoImageCodec::g_pictList = {"pct", "pic", "pict"};
QStringList LingmoImageCodec::g_faxList = {"fax", "g3"};
QStringList LingmoImageCodec::g_pngList = {"png", "apng"};
QStringList LingmoImageCodec::g_otherFormatList = {"sr",  "ras", "exr",  "psd", "jp2", "j2k", "jng", "wbmp",
                                                  "xbm", "pnm", "webp", "svg", "gif", "ico", "xpm"};
QStringList LingmoImageCodec::g_pbmList = {"pbm", "pgm", "ppm", "pnm"};
QStringList LingmoImageCodec::getSupportFormats()
{
    return m_supportFormats;
}

QString LingmoImageCodec::getOpenFileFormats()
{
    return m_supportFrmatsClassify;
}
