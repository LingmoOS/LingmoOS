// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontinfomanager.h"
#include "dfmdbmanager.h"
#include "dfontpreview.h"
#include "freetype/freetype.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QDateTime>
#include <QDirIterator>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <iconv.h>

#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

static QList<DFontInfo> dataList;
static DFontInfoManager *INSTANCE = nullptr;

const QString FONT_USR_DIR = QDir::homePath() + "/.local/share/fonts/";
const QString FONT_SYSTEM_DIR = "/usr/share/fonts/";

/*************************************************************************
 <Function>      isSystemFont
 <Description>   判断是否为系统字体
 <Author>        null
 <Input>
    <param1>     filePath            Description:需要判断的字体路径
 <Return>        bool                Description:是否为系统字体
 <Note>          null
*************************************************************************/
inline bool isSystemFont(const QString &filePath)
{
    if (filePath.contains("/usr/share/fonts/")) {
        return true;
    } else {
        return false;
    }
}

/*************************************************************************
 <Function>      convertToUtf8
 <Description>   将字符串内容转换为utf8格式
 <Author>        null
 <Input>
    <param1>     content            Description:需要转换的内容
    <param2>     len                Description:转换长度
 <Return>        QString            Description:转换之后的内容
 <Note>          null
*************************************************************************/
QString convertToUtf8(unsigned char *content, unsigned int len)
{
    QString convertedStr = "";

    std::size_t inputBufferSize = static_cast<std::size_t>(len);
    std::size_t outputBufferSize = inputBufferSize * 4;
    char *outputBuffer = new char[outputBufferSize];
    char *backupPtr = outputBuffer;

    // UTF16BE to UTF8.
    iconv_t code = iconv_open("UTF-8", "UTF-16BE");
    iconv(code, reinterpret_cast<char **>(&content), &inputBufferSize, &outputBuffer, &outputBufferSize);
    int actuallyUsed = static_cast<int>(outputBuffer - backupPtr);

    convertedStr = QString::fromUtf8(QByteArray(backupPtr, actuallyUsed));
    iconv_close(code);

    delete[] backupPtr;
    return convertedStr;
}

QString getDefaultPreviewText(FT_Face face, qint8 &lang, int len = FTM_DEFAULT_PREVIEW_LENGTH);

/*************************************************************************
 <Function>      getDefaultPreviewText
 <Description>   获取默认的预览显示内容
 <Author>        null
 <Input>
    <param1>     face            Description:字体文件对应的FT_Face对象
    <param2>     lang            Description:传入参数,记录字体类型
    <param3>     len             Description:显示长度
 <Return>        QString         Description:预览显示内容
 <Note>          null
*************************************************************************/
QString getDefaultPreviewText(FT_Face face, qint8 &lang, int len)
{
    QString previewTxt;

    if (face == nullptr || face->num_charmaps == 0)
        return previewTxt;

    lang = FONT_LANG_NONE;
    QString language = QLocale::system().name();

    //first check chinese preview
    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_CN_TEXT)) {
        lang = FONT_LANG_CHINESE;
        if (language.startsWith("zh_")) {
            return previewTxt;
        }
    }

    //not support chinese preview, than check english preview
    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_EN_TEXT)) {
        lang |= FONT_LANG_ENGLISH;
        return previewTxt;
    }

    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_DIGIT_TEXT)) {
        lang = FONT_LANG_DIGIT;
        return previewTxt;
    }

    return DFontPreview::buildCharlistForFace(face, len);
}

DFontInfoManager *DFontInfoManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFontInfoManager;
    }

    return INSTANCE;
}

DFontInfoManager::DFontInfoManager(QObject *parent)
    : QObject(parent)
    , m_strSysLanguage(QLocale::system().name())
{
    //Should not be called in constructor
    //refreshList();

    QMap<QString, ushort> langmap;
    langmap.insert("zh_HK", TT_MS_LANGID_CHINESE_HONG_KONG);
    langmap.insert("zh_TW", TT_MS_LANGID_CHINESE_TAIWAN);
    langmap.insert("zh_CN", TT_MS_LANGID_CHINESE_PRC);
    langmap.insert("zh_MO", TT_MS_LANGID_CHINESE_MACAO);
    langmap.insert("bo_CN", TT_MS_LANGID_TIBETAN_PRC); // 藏语
    langmap.insert("ug_CN", TT_MS_LANGID_UIGHUR_PRC); // 维吾尔语
    langmap.insert("ii_MO", TT_MS_LANGID_YI_PRC); // 彝语
    QSet<int> cnlangset{TT_MS_LANGID_CHINESE_HONG_KONG,
                        TT_MS_LANGID_CHINESE_TAIWAN,
                        TT_MS_LANGID_CHINESE_PRC,
                        TT_MS_LANGID_CHINESE_MACAO,
                        TT_MS_LANGID_TIBETAN_PRC,
                        TT_MS_LANGID_UIGHUR_PRC,
                        TT_MS_LANGID_YI_PRC};
    if (langmap.contains(m_strSysLanguage)) {
        m_langpriority3 = langmap.value(m_strSysLanguage);
        if (cnlangset.contains(m_langpriority3)) {
            m_langpriority2 = TT_MS_LANGID_CHINESE_PRC;
            m_langpriority1 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
        } else {
            m_langpriority2 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
            m_langpriority1 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
        }
    } else {
        m_langpriority3 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
        m_langpriority2 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
        m_langpriority1 = TT_MS_LANGID_ENGLISH_UNITED_STATES;
    }
}

DFontInfoManager::~DFontInfoManager() {}

/*************************************************************************
 <Function>      refreshList
 <Description>   刷新字体链表
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::refreshList(const QStringList &allFontPathList)
{
    if (!dataList.isEmpty()) {
        dataList.clear();
    }

    for (auto &path : allFontPathList) {
        DFontInfo fontInfo = getFontInfo(path, true);
        fontInfo.isSystemFont = isSystemFont(path);
        dataList << fontInfo;
    }

    updateSP3FamilyName(dataList);
}

/*************************************************************************
 <Function>      getAllFontPath
 <Description>   获取所有的字体文件路径
 <Author>        null
 <Input>
    <param1>     isStartup              Description:是否为第一次启动
 <Return>        QStringList            Description:所有的字体文件路径
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllFontPath(bool isStartup) const
{
#if 0
    QStringList pathList = getFonts(DFontInfoManager::All);
#endif
    QStringList pathList;

    pathList = getAllFclistPathList();

    if (isStartup) {
        //系统字体文件
        QStringList systemfilelist = getFileNames(FONT_SYSTEM_DIR);
        for (const QString &str : systemfilelist) {
            if (!pathList.contains(str)) {
                pathList << str;
            }
        }
    }

    //用户字体文件
    QStringList usrfilelist = getFileNames(FONT_USR_DIR);
    for (const QString &str : usrfilelist) {
        if (!pathList.contains(str)) {
            pathList << str;
        }
    }

    return pathList;
}

QStringList DFontInfoManager::getAllFclistPathList() const
{
    QProcess process;
    QStringList pathList;


    process.start("fc-list", QStringList() << ":" << "file");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString &line : lines) {
        QString filePath = line.remove(QChar(':')).simplified();
        if (filePath.length() > 0 && !pathList.contains(filePath)) {
            pathList << filePath;
        }
    }
    return pathList;
}



/*************************************************************************
 <Function>      getFileNames
 <Description>   获取路径下的所有字体文件
 <Author>        null
 <Input>
    <param1>     path                   Description:路径
 <Return>        QStringList            Description:路径下的所有字体文件
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFileNames(const QString &path)const
{
    QStringList string_list;
    //判断路径是否存在
    QDir dir(path);
    if (!dir.exists()) {
        return string_list;
    }

    //获取所选文件类型过滤器
    QStringList filters;
    filters << "*.ttf" << "*.ttc" << "*.otf";

    //定义迭代器并设置过滤器
    QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (dir_iterator.hasNext()) {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        string_list.append(absolute_file_path);
    }
    return string_list;
}

/*************************************************************************
 <Function>      getAllMonoSpaceFontPath
 <Description>   获得所有的中文字体
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:获取到的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllChineseFontPath() const
{
    QStringList pathList;
    QProcess process;
    process.start("fc-list", QStringList() << ":lang=zh");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString &line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
#if 0
    return getFonts(Chinese);
#endif
}

/*************************************************************************
 <Function>      getAllMonoSpaceFontPath
 <Description>   获得所有的等款字体
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:获取到的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllMonoSpaceFontPath() const
{
    QStringList pathList;
    QProcess process;
    process.start("fc-list", QStringList() << ":spacing=mono");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString &line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }
#if 0
    QStringList pathList = getFonts(MonoSpace);
#endif
    return pathList;
}

/*************************************************************************
 <Function>      getFontType
 <Description>   获取字体文件类型
 <Author>        null
 <Input>
    <param1>     filePath            Description:字体文件路径
 <Return>        QString             Description:字体文件类型
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getFontType(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();

    if (suffix == "ttf" || suffix == "ttc") {
        return FONT_TTF;
    } else if (suffix == "otf") {
        return FONT_OTF;
    } else {
        return FONT_UNKNOWN;
    }
}

struct NameIdFlag {
    char TT_NAME_ID_COPYRIGHT_flag = 0;
    char TT_NAME_ID_VERSION_STRING_flag = 0;
    char TT_NAME_ID_DESCRIPTION_flag = 0;
    char TT_NAME_ID_FULL_NAME_flag = 0;
    char TT_NAME_ID_TRADEMARK_flag = 0;
    char TT_NAME_ID_PS_NAME_flag = 0;
};
/*************************************************************************
 <Function>      setFontInfo
 <Description>   按照新规则处理字体familyname
 <Author>        null
 <Input>
    <param1>     DFontInfo     Description:该字体文件的字体信息
 <Return>        null
 <Note>          null
*************************************************************************/
void DFontInfoManager::setFontInfo(DFontInfo& fontInfo)
{
    /*****************************************
    字体显示名称规则
    规则说明：
    规则一：
    条件：familyname不为空，不包含“？”；
    规则：字体名称使用 familyname+“-”+style；
    规则二：
    条件：familyname为空或者包含“？”，fullname不为空；
    规则：字体名称使用 fullname+“-”+style；
    规则三
    条件：familyname为空或者包含“？”，fullname为空；
    规则：字体名称使用 PSname；
    规则四
    条件：familyname为空或者包含“？”，fullname为空，PSname为空；
    规则：字体名称显示“UntitledFont”；
    规则对所有字体（包括系统字体、用户字体）有效；
    ****************************************/
    QString familyName;
    if (fontInfo.sp3FamilyName.isEmpty() || fontInfo.sp3FamilyName.contains(QChar('?'))) {
        int appFontId = QFontDatabase::addApplicationFont(fontInfo.filePath);

        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        for (QString &family : fontFamilyList) {
            if (family.contains(QChar('?')))
                continue;
            familyName = family;
        }
        if (familyName.isEmpty()) {
            if (!fontInfo.fullname.isEmpty() && !fontInfo.fullname.contains(QChar('?'))) {
                familyName = fontInfo.fullname;
            } else if (!fontInfo.psname.isEmpty() && !fontInfo.psname.contains(QChar('?'))) {
                familyName = fontInfo.fullname;
            } else {
                familyName = QLatin1String("UntitledFont");
            }
        }
        fontInfo.sp3FamilyName = familyName;
    } else {
        familyName = fontInfo.sp3FamilyName;
    }

    if (!fontInfo.styleName.isEmpty() && !familyName.endsWith(fontInfo.styleName) && familyName != QLatin1String("UntitledFont")) {
        fontInfo.familyName = familyName;//从Sfnt接口获取的familyname读取宋体文件时解析为韩语或者?，替换为字体显示名称
    } else {
        //从Sfnt接口获取的familyname读取宋体文件时解析为韩语或者?，替换为字体显示名称
        // 例如Consolas-Regular(或Consolas Regular)获取到Consolas
        if(fontInfo.styleName.isEmpty()){
            fontInfo.familyName = familyName;
        }
        else {
            fontInfo.familyName = familyName.replace(QRegExp(QString("[ -]" + fontInfo.styleName + "$")), "");
        }
    }
    return;
}

/*************************************************************************
 <Function>      getFontInfo
 <Description>   获取字体信息
 <Author>        null
 <Input>
    <param1>     filePath             Description:字体文件路径
 <Return>        DFontInfo            Description:该字体文件的字体信息
 <Note>          null
*************************************************************************/
DFontInfo DFontInfoManager:: getFontInfo(const QString &filePath, bool withPreviewTxt)
{
    DFontInfo fontInfo;
    fontInfo.isSystemFont = isSystemFont(filePath);

    FT_Library library = nullptr;
    FT_Init_FreeType(&library);

    FT_Face face = nullptr;
    FT_Error error = FT_New_Face(library, filePath.toUtf8().constData(), 0, &face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error << filePath;
        fontInfo.isError = true;
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return fontInfo;
    }

    // get the basic data.
    fontInfo.isError = false;
    fontInfo.filePath = filePath;

    fontInfo.styleName = QString::fromLatin1(face->style_name);

    fontInfo.type = getFontType(filePath);

    bool flagsetsp3FN = true;

    if (FT_IS_SFNT(face)) {
        FT_SfntName sname;
        const unsigned int count = FT_Get_Sfnt_Name_Count(face);

        struct NameIdFlag nameidflag;
        for (unsigned int i = 0; i < count; ++i) {
            if (FT_Get_Sfnt_Name(face, i, &sname) != 0) {
                continue;
            }

            // only handle the unicode names for US langid.
            if (sname.language_id == 0) {
                continue;
            }

            char flag = 0;
            if (sname.language_id == m_langpriority3) {
                flag = 3;
            } else if (sname.language_id == m_langpriority2) {
                flag = 2;
            } else if (sname.language_id == m_langpriority1) {
                flag = 1;
            }

            switch (sname.name_id) {
            // 0
            case TT_NAME_ID_COPYRIGHT: {
                if (flag >= nameidflag.TT_NAME_ID_COPYRIGHT_flag) {
                    fontInfo.copyright = convertToUtf8(sname.string, sname.string_len).simplified();
                    nameidflag.TT_NAME_ID_COPYRIGHT_flag = flag;
                }
            }
            break;
            // 5
            case TT_NAME_ID_VERSION_STRING: {
                if (flag >= nameidflag.TT_NAME_ID_VERSION_STRING_flag) {
                    fontInfo.version = convertToUtf8(sname.string, sname.string_len);
                    fontInfo.version = fontInfo.version.remove("Version").simplified();
                    nameidflag.TT_NAME_ID_VERSION_STRING_flag = flag;
                }
            }
            break;
            // 10
            case TT_NAME_ID_DESCRIPTION: {
                if (flag >= nameidflag.TT_NAME_ID_DESCRIPTION_flag) {
                    fontInfo.description = convertToUtf8(sname.string, sname.string_len).simplified();
                    nameidflag.TT_NAME_ID_DESCRIPTION_flag = flag;
                }
            }
            break;

            // 4
            case TT_NAME_ID_FULL_NAME: {
                if (flag >= nameidflag.TT_NAME_ID_FULL_NAME_flag) {
                    if (sname.encoding_id == TT_MS_ID_UNICODE_CS) {
                        fontInfo.fullname = convertToUtf8(sname.string, sname.string_len).simplified();
                        flagsetsp3FN = true;
                    } else {
                        fontInfo.fullname = QString::fromLatin1(face->family_name).trimmed();
                        flagsetsp3FN = false;
                        if (fontInfo.fullname.isEmpty()) {
                            fontInfo.fullname = convertToUtf8(sname.string, sname.string_len).simplified();
                        }
                    }
                    nameidflag.TT_NAME_ID_FULL_NAME_flag = flag;
                }
            }
            break;
            // 7
            case TT_NAME_ID_TRADEMARK: {
                if (flag >= nameidflag.TT_NAME_ID_TRADEMARK_flag) {
                    fontInfo.trademark = convertToUtf8(sname.string, sname.string_len).simplified();
                    nameidflag.TT_NAME_ID_TRADEMARK_flag = flag;
                }
            }
            break;
            // 6
            case TT_NAME_ID_PS_NAME: {
                if (flag >= nameidflag.TT_NAME_ID_FULL_NAME_flag) {
                    fontInfo.psname = convertToUtf8(sname.string, sname.string_len).simplified();
                    nameidflag.TT_NAME_ID_FULL_NAME_flag = flag;
                }
            }
            break;

            default:
                break;
            }
        }
    }
    //compitable with SP2 update1 and previous versions
    if (!fontInfo.fullname.isEmpty()) {
        // 例如Consolas-Regular(或Consolas Regular)获取到Consolas
        fontInfo.familyName = fontInfo.fullname.replace(QRegExp(QString("[ -]" + fontInfo.styleName + "$")), "");
    }

    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromLatin1(face->family_name).trimmed();
    }

    // 防止部分sp3FamilyName乱码
    if (flagsetsp3FN) {
        fontInfo.sp3FamilyName = fontInfo.familyName;
    }

    //default preview text
    if (withPreviewTxt) {
        if (fontInfo.familyName == "Noto Sans Grantha") {
            fontInfo.defaultPreview = getDefaultPreviewText(face, fontInfo.previewLang, INT_MAX);
        } else {
            fontInfo.defaultPreview = getDefaultPreviewText(face, fontInfo.previewLang);
        }
    }

    // destroy object.
    FT_Done_Face(face);
    face = nullptr;
    FT_Done_FreeType(library);
    library = nullptr;

    checkStyleName(fontInfo);
    setFontInfo(fontInfo);

    DFMDBManager *dbManager = DFMDBManager::instance();
    if (dbManager->getRecordCount() > 0) {
        fontInfo.sysVersion = fontInfo.version;
        if (dbManager->isFontInfoExist(fontInfo).isEmpty()) {
            fontInfo.isInstalled = false;
        } else {
            fontInfo.isInstalled = true;
        }
    } /*else {
        fontInfo.isInstalled = isFontInstalled(fontInfo);
    }*/
    if(!fontInfo.isInstalled){
        fontInfo.isInstalled = isFontInInstalledDirs(fontInfo.filePath);
    }
    return fontInfo;
}

/*************************************************************************
 <Function>      getDefaultPreview
 <Description>   获取字体初始的预览效果内容
 <Author>        null
 <Input>
    <param1>     filePath               Description:字体文件路径
    <param2>     preivewLang            Description:显示内容长度
 <Return>        QString                Description:字体初始的预览效果内容
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getDefaultPreview(const QString &filePath, qint8 &preivewLang)
{
    FT_Library m_library = nullptr;
    FT_Face m_face = nullptr;
    QString defaultPreview;

    FT_Init_FreeType(&m_library);
    FT_Error error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error << filePath;
        FT_Done_Face(m_face);
        m_face = nullptr;
        FT_Done_FreeType(m_library);
        m_library = nullptr;
        return defaultPreview;
    }

    defaultPreview = getDefaultPreviewText(m_face, preivewLang);
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);
    return defaultPreview;
}

/*************************************************************************
 <Function>      getCurrentFontFamily
 <Description>   获取当前使用字体的familyname
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:当前使用字体的familyname
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getCurrentFontFamily()
{
    QStringList retStrList;
    QProcess process;

    process.start("fc-match");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        retStrList = line.split(" \"");
        int index = 0;
        for (QString &fontStr : retStrList) {
            fontStr.remove(QChar('\"'));
            if (index == 0 && fontStr.endsWith(":"))
                fontStr.remove(":");
            index++;
        }
        if (!retStrList.isEmpty())
            break;
    }
#if 0
    FcPattern *pat = FcPatternCreate();
    if (!pat)
        return retStrList;

    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcFontSet *fs = FcFontSetCreate();
    if (!fs) {
        FcPatternDestroy(pat);
        FcFini();
        return retStrList;
    }

    FcResult result;
    FcPattern *match = FcFontMatch(nullptr, pat, &result);
    if (match)
        FcFontSetAdd(fs, match);
    FcPatternDestroy(pat);

    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fcmatch}");

    for (int j = 0; j < fs->nfont; j++) {
        FcPattern *font = FcPatternFilter(fs->fonts[j], nullptr);
        FcChar8 *s = FcPatternFormat(font, format);

        if (s) {
            QString str = QString(reinterpret_cast<char *>(s));
            retStrList = str.split(" \"");
            int index = 0;
            for (QString &fontStr : retStrList) {
                fontStr.remove(QChar('\"'));
                if (index == 0 && fontStr.endsWith(":"))
                    fontStr.remove(":");
                index++;
            }
            FcStrFree(s);
        }
        qDebug() << __FUNCTION__ << retStrList;
        FcPatternDestroy(font);
        if (!retStrList.isEmpty())
            break;
    }
    FcFontSetDestroy(fs);
    FcFini();
#endif
    return retStrList;
}

/*************************************************************************
 <Function>      getFontPath
 <Description>   获取当前使用字体的路径信息
 <Author>        null
 <Input>
    <param1>     null
 <Return>        字体的路径
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getFontPath()
{
    QString filePath;
    QStringList retStrList;
    QProcess process;

    process.start("fc-match -v |grep file");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        line = line.simplified();
        if (!line.startsWith("file:"))
            continue;

        retStrList = line.split(" \"");
        for (QString &fontStr : retStrList) {
            int lastIndex = fontStr.lastIndexOf(QChar('\"'));
            if (lastIndex == -1)
                continue;

            fontStr.chop(fontStr.size() - lastIndex);
            if (!fontStr.isEmpty()) {
                return fontStr;
            }
        }
    }
    return filePath;
}

/*************************************************************************
 <Function>      getFontFamilyStyle
 <Description>   获取字体的family信息
 <Author>        null
 <Input>
    <param1>     filePah                Description:字体文件路径
 <Return>        QStringList            Description:获取到的信息
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFontFamilyStyle(const QString &filePah)
{

    QStringList fontFamilyList;
    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fclist}");
    FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, nullptr);
    FcPattern *pat = FcPatternCreate();
    FcFontSet *fs = FcFontList(nullptr, pat, os);

    if (os)
        FcObjectSetDestroy(os);
    if (pat)
        FcPatternDestroy(pat);
    if (fs) {
        for (int j = 0; j < fs->nfont; j++) {
            FcChar8 *s = FcPatternFormat(fs->fonts[j], format);
            if (s == nullptr)
                continue;

            QString str = QString(reinterpret_cast<char *>(s));
            QStringList retStrList = str.split(":");

            if (retStrList.size() != 3) {
                FcStrFree(s);
                continue;
            }
            QString fontpath = const_cast<const QString &>(retStrList.at(0));
            if (fontpath.simplified() == filePah) {
                qDebug() << __FUNCTION__ << " found " << filePah;
                QString families = const_cast<const QString &>(retStrList.at(1)).simplified();
                fontFamilyList = families.split(",");
                for (QString &fontStr : fontFamilyList) {
                    fontStr.remove(QChar('\\'));
                }

                FcStrFree(s);
                break;
            }  else {
                FcStrFree(s);
            }
        }
        FcFontSetDestroy(fs);
    }

//    FcFini();

    return fontFamilyList;
}

/*************************************************************************
 <Function>      getFonts
 <Description>   获取所需类型的字体
 <Author>        null
 <Input>
    <param1>     type                   Description:所需字体的类型
 <Return>        QStringList            Description:指定类型的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFonts(DFontInfoManager::FontTYpe type) const
{
    QStringList fontList;
    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fclist}");
    FcPattern *pat = nullptr;
    const FcChar8 *opt = nullptr;
    switch (type) {
    case Chinese:
        opt = reinterpret_cast<const FcChar8 *>(":lang=zh");
        break;
    case MonoSpace:
        opt = reinterpret_cast<const FcChar8 *>(":spacing=mono");
        break;
    case All:
        break;
    }

    if (opt != nullptr) {
        pat = FcNameParse(opt);
        if (pat == nullptr) {
            qDebug() << __FUNCTION__ << " err " << type;
            return fontList;
        }
    } else {
        pat = FcPatternCreate();
    }

    FcObjectSet *os = FcObjectSetBuild(FC_FILE, nullptr);
    FcFontSet *fs = FcFontList(nullptr, pat, os);

    if (os)
        FcObjectSetDestroy(os);
    if (pat)
        FcPatternDestroy(pat);

    if (fs) {
        for (int j = 0; j < fs->nfont; j++) {
            FcChar8 *s = FcPatternFormat(fs->fonts[j], format);
            if (s == nullptr)
                continue;

            QString str = QString(reinterpret_cast<char *>(s));
            str.remove(": ");
//            qDebug() << __FUNCTION__ << str;
            if (!fontList.contains(str) && !str.isEmpty())
                fontList << str;
            FcStrFree(s);
        }
        FcFontSetDestroy(fs);
    }

//    FcFini();
    return fontList;
}

/*************************************************************************
 <Function>      isFontInstalled
 <Description>   字体有没有被安装过
 <Author>        null
 <Input>
    <param1>     data            Description:需要处理的字体信息
 <Return>        bool            Description:有没有被安装过
 <Note>          null
*************************************************************************/
bool DFontInfoManager::isFontInstalled(DFontInfo &data)
{
    for (DFontInfo &item : dataList) {
        if (data == item) {
            data.sysVersion = item.version;
            return true;
        }
    }

    return false;
}

/*************************************************************************
 <Function>      isFontInInstalledDirs
 <Description>   检查需要安装的字体文件是否在.local/share/fonts目录下，或者是否有同名文件
 <Author>
 <Input>
    <param1>     filePath        Description:待安装字体文件
 <Return>        bool            Description:return true 是；return false 否
 <Note>          null
*************************************************************************/
bool DFontInfoManager::isFontInInstalledDirs(const QString &filePath)
{
    if (filePath.contains(QDir::homePath() + "/.local/share/fonts/")) {
        return true;
    }
    if (filePath.lastIndexOf("/") < 0){
        return false;
    }
    QFile file(QDir::homePath() + "/.local/share/fonts" + filePath.mid(filePath.lastIndexOf("/")));
    if (file.exists()) {
        return true;
    }
    return false;
}

/*************************************************************************
 <Function>      getDefaultPreview
 <Description>   获取默认的字体预览效果现实内容
 <Author>        null
 <Input>
    <param1>     data            Description:需要处理的字体信息
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::getDefaultPreview(DFontInfo &data)
{
//    for (DFontInfo &item : dataList) {
//        if (data == item) {
//            data.defaultPreview = item.defaultPreview;
//            data.previewLang = item.previewLang;
//            return;
//        }
//    }

    data.defaultPreview = getDefaultPreview(data.filePath, data.previewLang);
}

/**
* @brief DFontInfoManager::updateSP3FamilyName sp3的familyName兼容sp2 update1及之前的版本
* @param fontList 字体列表
* @param inFontList 是否使用字体列表，true：使用 false：不使用
* @return void
*/
void DFontInfoManager::updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList)
{
    if (fontList.isEmpty())
        return;
    //compitable with sp2 update1 and previous versions
    DFMDBManager::instance()->updateSP3FamilyName(fontList,  inFontList);
}

/*************************************************************************
 <Function>      checkStyleName
 <Description>   对某些特殊字体的stylename进行处理,使得显示内容规范
 <Author>        null
 <Input>
    <param1>     f               Description:需要处理的字体信息
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::checkStyleName(DFontInfo &f)
{
    //有些字体文件因为不规范导致的stylename为空，通过psname来判断该字体的stylename。
    //之前代码顺序出现问题，导致有的时候contains判断出错 比如DemiBold与Bold，现在调整代码顺序
    if (f.styleName.contains("?") || f.styleName.isEmpty()) {
        if (f.psname != "") {
            if (f.psname.contains("Regular", Qt::CaseInsensitive)) {
                f.styleName = "Regular";
            } else if (f.psname.contains("DemiBold", Qt::CaseInsensitive)) {
                f.styleName = "DemiBold";
            } else if (f.psname.contains("ExtraBold", Qt::CaseInsensitive)) {
                f.styleName = "ExtraBold";
            } else if (f.psname.contains("Bold", Qt::CaseInsensitive)) {
                f.styleName = "Bold";
            } else if (f.psname.contains("ExtraLight", Qt::CaseInsensitive)) {
                f.styleName = "ExtraLight";
            } else if (f.psname.contains("Light", Qt::CaseInsensitive)) {
                f.styleName = "Light";
            } else if (f.psname.contains("Thin", Qt::CaseInsensitive)) {
                f.styleName = "Thin";
            } else if (f.psname.contains("Medium", Qt::CaseInsensitive)) {
                f.styleName = "Medium";
            } else if (f.psname.contains("AnyStretch", Qt::CaseInsensitive)) {
                f.styleName = "AnyStretch";
            } else if (f.psname.contains("UltraCondensed", Qt::CaseInsensitive)) {
                f.styleName = "UltraCondensed";
            } else if (f.psname.contains("ExtraCondensed", Qt::CaseInsensitive)) {
                f.styleName = "ExtraCondensed";
            } else if (f.psname.contains("SemiCondensed", Qt::CaseInsensitive)) {
                f.styleName = "SemiCondensed";
            } else if (f.psname.contains("Condensed", Qt::CaseInsensitive)) {
                f.styleName = "Condensed";
            } else if (f.psname.contains("Unstretched", Qt::CaseInsensitive)) {
                f.styleName = "Unstretched";
            } else if (f.psname.contains("SemiExpanded", Qt::CaseInsensitive)) {
                f.styleName = "SemiExpanded";
            } else if (f.psname.contains("ExtraExpanded", Qt::CaseInsensitive)) {
                f.styleName = "ExtraExpanded";
            } else if (f.psname.contains("UltraExpanded", Qt::CaseInsensitive)) {
                f.styleName = "UltraExpanded";
            } else if (f.psname.contains("Expanded", Qt::CaseInsensitive)) {
                f.styleName = "Expanded";
            } else {
                f.styleName = "Unknown";
            }
        }
//        //有些字体及其特殊，解析获得的信息中获取不到任何有关stylename的信息，对于这种字体，stylename设置为Unknown
//        if (!str.contains(f.styleName)) {
//            f.styleName = "Unknown";
//        }
    }
}
