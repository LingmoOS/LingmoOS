#include <gio/gio.h>
#include <fontconfig/fontconfig.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_MULTIPLE_MASTERS_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QProcess>

#include "include/libfun.h"

void LibFun::remove_whitespace(char **ori)
{
    g_auto(GStrv) split = NULL;
    g_autoptr(GString) reassembled = NULL;
    gchar *str = NULL;
    gint i = 0;
    gint stripped = 0;
    size_t len = 0;

    reassembled = g_string_new(NULL);

    split = g_strsplit(*ori , "\n" , -1);

    for (i = 0 ; split[i] != NULL ; i++) {
        str = split[i];

        len = strspn(str , WHITESPACE_CHARS);
        if (len) {
            str += len;
        }

        if (strlen(str) == 0 && ((split[i + 1] == NULL) || (strlen(split[i + 1]) == 0))) {
            continue;
        }

        if (stripped++ > 0) {
            g_string_append(reassembled , "\n");
        }

        g_string_append(reassembled , str);
    }

    g_free(*ori);
    *ori = g_strdup(reassembled->str);

    return;
}

void LibFun::remove_version(char **ori)
{
    gchar *p = NULL;
    gchar *stripped = NULL;

    p = g_strstr_len(*ori , -1 , MATCH_VERSION_STR);
    if (p == NULL) {
        return;
    }

    p += strlen(MATCH_VERSION_STR);
    stripped = g_strdup(p);

    remove_whitespace(&stripped);

    g_free(*ori);
    *ori = stripped;

    return;
}

QList<FontInformation> LibFun::getAllFontInformation(void)
{
    QList<FontInformation> ret;
    ret.clear();

    FT_Library ft_library;
    FT_Error err = FT_Init_FreeType(&ft_library);

    if (err != FT_Err_Ok) {
        qCritical() << "Error : LibFun , getAllFontInformation , init freetype fail";

        return ret;
    }

    if (!FcInitReinitialize()) {
        qCritical() << "Error : LibFun , getAllFontInformation , init font list fail";

        return ret;
    }

    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pat = FcPatternCreate();
//    FcObjectSet *os = FcObjectSetBuild(FC_FILE , FC_FAMILY , FC_STYLE , FC_INDEX , NULL);
    FcObjectSet *os = FcObjectSetBuild(FC_FILE , FC_FAMILY , FC_STYLE, FC_FOUNDRY, FC_INDEX , NULL); // 构建对象（字体文件路径，字体名，类别，字体铸造商）

    FcFontSet *fs = FcFontList(config , pat , os);

    qInfo() << "Info : LibFun , getAllFontInformation , total matching fonts is " << fs->nfont;

    for (int i = 0 ; i < fs->nfont && fs != NULL ; i++) {
        FontInformation item;
        FcChar8 *path = NULL;
        FcChar8 *family = NULL;
        FcChar8 *style = NULL;
        FcChar8 *foundry = NULL;
        int index;

        FcPattern *font = fs->fonts[i];

        if (FcPatternGetString(font , FC_FILE , 0 , &path) == FcResultMatch &&
            FcPatternGetString(font , FC_FAMILY , 0 , &family) == FcResultMatch &&
            FcPatternGetString(font , FC_STYLE , 0 , &style) == FcResultMatch &&
            FcPatternGetString(font , FC_FOUNDRY , 0 , &foundry) == FcResultMatch &&
            FcPatternGetInteger(font , FC_INDEX , 0 , &index) == FcResultMatch)
        {
            item.path = QString((char *)path);
            item.family = QString((char *)family);
            item.style = QString((char *)style);
            item.foundry = QString((char *)foundry);
        }

        /* 对字体文件进行判断（判断后缀名是否为.ttf .otf）*/
        // if (!chooseFontFile(item.path)) {
            // continue;
        // }

        gchar *fontData = NULL;
        gsize fontDataLenth;
        g_autoptr(GError) error = NULL;

        GFile *fd = g_file_new_for_path((const gchar *)path);
        if (!g_file_load_contents(fd , NULL , &fontData , &fontDataLenth , NULL , &error)) {
            qWarning() << "Waring : LibFun , getAllFontInformation , load font file fail , Path is [ " << path << " ]" << " error is [ " << error->message << " ]";

            ret << item;

            continue;
        }

        FT_Error ft_error;
        FT_Face ft_retval;

        ft_error = FT_New_Memory_Face(ft_library , (const FT_Byte *)fontData , (FT_Long)fontDataLenth , index , &ft_retval);
        if (ft_error != FT_Err_Ok) {
            qWarning() << "Waring : LibFun , getAllFontInformation , read font data fail , Path is [ " << path << " ]";

            ret << item;

            continue;
        }

        /* 名字 */
        if (ft_retval->family_name) {
            item.name = QString((char *)ft_retval->family_name);
        }

        /* 样式 */
        if (ft_retval->style_name) {
            item.style = QString((char *)ft_retval->style_name);
        }

        /* 路径 */
        g_autofree gchar *location = NULL;
        location = g_file_get_path(fd);
        item.path = QString((char *)location);

        /* 种类 */
        g_autoptr(GFileInfo) fileInfo;
        fileInfo = g_file_query_info(fd , G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SIZE , G_FILE_QUERY_INFO_NONE , NULL , NULL);
        if (fileInfo != NULL) {
            g_autofree gchar *fileType = g_content_type_get_description(g_file_info_get_content_type(fileInfo));

            item.type = QString((char *)fileType);
        }

        if (FT_IS_SFNT(ft_retval)) {
            gint i;
            gint len;
            g_autofree gchar *version = NULL;
            g_autofree gchar *copyright = NULL;
            g_autofree gchar *description = NULL;
            g_autofree gchar *designer = NULL;
            g_autofree gchar *manufacturer = NULL;
            g_autofree gchar *license = NULL;

            len = FT_Get_Sfnt_Name_Count(ft_retval);

            for (i = 0 ; i < len ; i++) {
                FT_SfntName ft_sname;

                if (FT_Get_Sfnt_Name(ft_retval , i , &ft_sname) != 0) {
                    continue;
                }

                if (!(ft_sname.platform_id == TT_PLATFORM_MICROSOFT &&
                      ft_sname.encoding_id == TT_MS_ID_UNICODE_CS &&
                      ft_sname.language_id == TT_MS_LANGID_ENGLISH_UNITED_STATES))
                {
                    continue;
                }

                switch (ft_sname.name_id) {
                case TT_NAME_ID_COPYRIGHT :
                    if (!copyright) {
                        copyright = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                case TT_NAME_ID_VERSION_STRING :
                    if (!version) {
                        version = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                case TT_NAME_ID_DESCRIPTION :
                    if (!description) {
                        description = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                case TT_NAME_ID_MANUFACTURER :
                    if (!manufacturer) {
                        manufacturer = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                case TT_NAME_ID_DESIGNER :
                    if (!designer) {
                        designer = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                case TT_NAME_ID_LICENSE :
                    if (!license) {
                        license = g_convert((gchar *)ft_sname.string , ft_sname.string_len , "UTF-8" , "UTF-16BE" , NULL , NULL , NULL);
                    }
                    break;
                default :
                    break;
                }
            }

            /* 版本 */
            if (version) {
                remove_version(&version);
                item.version = QString((char *)version);
            }

            /* 版权 */
            if (copyright) {
                remove_whitespace(&copyright);
                item.copyright = QString((char *)copyright);
            }

            /* 描述 */
            if (description) {
                remove_whitespace(&description);
                item.description = QString((char *)description);
            }

            /* 商标 */
            if (manufacturer) {
                remove_whitespace(&manufacturer);
                item.manufacturer = QString((char *)manufacturer);
            }

            /* 设计师 */
            if (designer) {
                remove_whitespace(&designer);
                item.designer = QString((char *)designer);
            }

            /* 许可证 */
            if (license) {
                remove_whitespace(&license);
                item.license = QString((char *)license);
            }
        }
        ret << item;

        FT_Done_Face(ft_retval);
        g_object_unref(fd);
        g_free(fontData);
    }

    if (pat) {
        FcPatternDestroy(pat);
    }

    if (os) {
        FcObjectSetDestroy(os);
    }

    if (fs) {
        FcFontSetDestroy(fs);
    }
    
    qSort(ret.begin(), ret.end(), caseInsensitiveLessThan);

    return ret;
}

bool LibFun::installFont(QString path)
{
    if (path.isEmpty()) {
        return false;
    }

    QString installPath = QString(getenv("HOME")) + QString(INSTALL_PATH);

    QDir dir(installPath);
    if (!dir.exists()) {
        if (!dir.mkpath(installPath)) {
            qCritical() << "Error : LibFun , installFont , install path create fail";

            return false;
        }
    }

    QFileInfo fileInfo(path);
    QString installFilePath = installPath + fileInfo.fileName();

    QFile file(installFilePath);
    if (!file.exists()) {
        if (!QFile::copy(path , installFilePath)) {
            qCritical() << "Error : LibFun , installFont , copy font file fail";

            return false;
        }
    }

    if (system("fc-cache")) {
        qWarning() << "Waring : LibFun , installFont , update cache fail";
    }

    return true;
}

bool LibFun::uninstallFont(QString path)
{
    if (path.isEmpty()) {
        return false;
    }

    QFile file(path);
    if (file.exists()) {
        if (!file.remove()) {
            qCritical() << "Error : LibFun , uninstallFont , remove font file fail";

            return false;
        }
    }

    if (system("fc-cache")) {
        qWarning() << "Waring : LibFun , uninstall , update font cache fail";
    }

    return true;
}

bool LibFun::copyFont(QString src , QString dst)
{
    if (src.isEmpty() || dst.isEmpty()) {
        return false;
    }

    QString dstName;
    QFileInfo fileInfo(src);
    if (dst.back() == QChar('/')) {
        dstName = dst + fileInfo.fileName();
    } else {
        dstName = dst + "/" + fileInfo.fileName();
    }

    if (!QFile::copy(src , dstName)) {
        qCritical() << "Error : LibFun , copyFont , font file copy fail";

        return false;
    }

    return true;
}

bool LibFun::chooseFontFile(QString path)
{
    QStringList list = path.split(".");
    QString fontTypeTTF = "ttf";
    QString fontTypeOTF = "otf";
    QString fontType = list.at(list.size()-1);

    if (fontType.compare(fontTypeTTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeOTF, Qt::CaseInsensitive) == 0) {
        return true;
    }

    return false;
}

bool LibFun::caseInsensitiveLessThan(const FontInformation &s1, const FontInformation &s2)
{
    if (s1.family == s2.family){
        return s1.style > s2.style;
    }
    return s1.family > s2.family;
}

QString LibFun::getFontInfo(QString path)
{
    QString ret;
    ret.clear();

    // // 判断后缀
    // if (!chooseFontFile(path)) {
    //     qDebug() << "判断字体文件后缀，字体文件损坏：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //     ret = path;
    //     return ret;
    // }
    
    // 判断文件type类型
    // if (isFontFile(path) != true) {
    //     qDebug() << "判断字体文件type类型，字体文件损坏：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    //     ret = path;
    //     return ret;
    // }

    std::string str = path.toStdString();
    FcChar8* file = (FcChar8*)(str.c_str());
    FcPattern* pat = nullptr;
    int count;
    FcBlanks* blanks = FcConfigGetBlanks(NULL);
    pat = FcFreeTypeQuery((FcChar8 *)file, 0, blanks, &count);
    
    // 判断是否从字体文件中获取到字体数据
    if (pat == nullptr) {
        qDebug() << "判断字体文件，字体文件损坏：" <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
        ret = path;
    }
        
    FcPatternDestroy(pat);

    return ret;
}

bool LibFun::isFontFile(QString path)
{
    QString command = QString("file -i ") + '"' + path + '"';

    QProcess process;
    process.start(command);

    if (!process.waitForFinished()) {
        return false;
    }

    QString ret = process.readAll();
    
    if (ret.contains("font/sfnt") || ret.contains("application/vnd.ms-opentype")) {
        return true;
    }

    return false;
}
