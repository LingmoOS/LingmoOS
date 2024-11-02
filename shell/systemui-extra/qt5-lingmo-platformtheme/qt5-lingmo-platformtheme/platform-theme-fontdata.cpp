/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */
#include "platform-theme-fontdata.h"
#include <QDebug>

#undef signals
#undef slots
#undef emit

#include <gio/gio.h>

PlatformThemeFontData::PlatformThemeFontData()
{

}

PlatformThemeFontData::~PlatformThemeFontData()
{

}

bool PlatformThemeFontData::chooseFontFile(QString path)
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

QMap<QString, QString> PlatformThemeFontData::getAllFontInformation()
{
    QMap<QString, QString> fontMap;
    QList<FontInformation> ret;
    ret.clear();

    FT_Library ft_library;
    FT_Error err = FT_Init_FreeType(&ft_library);

    if (err != FT_Err_Ok) {
        qCritical() << "Error : LibFun , getAllFontInformation , init freetype fail";

        return fontMap;
    }

    if (!FcInitReinitialize()) {
        qCritical() << "Error : LibFun , getAllFontInformation , init font list fail";

        return fontMap;
    }

    FcConfig *config = FcInitLoadConfigAndFonts();
    FcPattern *pat = FcPatternCreate();
    FcObjectSet *os = FcObjectSetBuild(FC_FILE , FC_FAMILY , FC_STYLE , FC_INDEX , NULL);

    FcFontSet *fs = FcFontList(config , pat , os);

    qInfo() << "Info : LibFun , getAllFontInformation , total matching fonts is " << fs->nfont;

    for (int i = 0 ; i < fs->nfont && fs != NULL ; i++) {
        FontInformation item;
        FcChar8 *path = NULL;
        FcChar8 *family = NULL;
//        FcChar8 *style = NULL;
        int index;

        FcPattern *font = fs->fonts[i];

        if (FcPatternGetString(font , FC_FILE , 0 , &path) == FcResultMatch &&
            FcPatternGetString(font , FC_FAMILY , 0 , &family) == FcResultMatch &&
//            FcPatternGetString(font , FC_STYLE , 0 , &style) == FcResultMatch &&
            FcPatternGetInteger(font , FC_INDEX , 0 , &index) == FcResultMatch)
        {
            item.path = QString((char *)path);
            item.family = QString((char *)family);
//            item.style = QString((char *)style);
        }

        /* 对字体文件进行判断（判断后缀名是否为.ttf .otf）*/
        if (!chooseFontFile(item.path)) {
            continue;
        }

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

        /*
        ///名字
        if (ft_retval->family_name) {
            item.name = QString((char *)ft_retval->family_name);
        }

        ///样式
        if (ft_retval->style_name) {
            item.style = QString((char *)ft_retval->style_name);
        }

        ///路径
        g_autofree gchar *location = NULL;
        location = g_file_get_path(fd);
        item.path = QString((char *)location);

        ///种类
        g_autoptr(GFileInfo) fileInfo;
        fileInfo = g_file_query_info(fd , G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE "," G_FILE_ATTRIBUTE_STANDARD_SIZE , G_FILE_QUERY_INFO_NONE , NULL , NULL);
        if (fileInfo != NULL) {
            g_autofree gchar *fileType = g_content_type_get_description(g_file_info_get_content_type(fileInfo));

            item.type = QString((char *)fileType);
        }

        qDebug() << "name:" << item.name << "style:" << item.style << "path:" << item.path << "family:" << item.family;
        */

        ret << item;
        fontMap.insert(item.family, item.path);

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
    return fontMap;
}

