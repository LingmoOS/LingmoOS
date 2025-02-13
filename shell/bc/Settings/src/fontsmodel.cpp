/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "fontsmodel.h"

#include <QLocale>
#include <QFontDatabase>
#include <QFontInfo>
#include <QTimer>
#include <QDir>
#include <QDirIterator>

// Freetype and Fontconfig
#include <ft2build.h>
#include <freetype/freetype.h>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>

static QString getFcString(FcPattern *pat, const char *val, int index)
{
    QString rv;
    FcChar8 *fcStr;

    if (FcPatternGetString(pat, val, index, &fcStr) == FcResultMatch)
        rv = QString::fromUtf8((char *)fcStr);

    return rv;
}

static QString getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString rv;
    int langIndex=-1;

    for (int i = 0; true; ++i) {
        QString lang = getFcString(pat, valLang, i);

        if (lang.isEmpty())
            break;
        else if (QString::fromLatin1("en") == lang)
            return getFcString(pat, val, i);
        else if (QString::fromLatin1("xx") != lang && -1 == langIndex)
            langIndex=i;
    }

    return getFcString(pat, val, langIndex > 0 ? langIndex : 0);
}

FontsModel::FontsModel(QObject *parent)
    : QThread(parent)
{
    m_lanCode = QLocale::system().name().replace('_', "-");

    QThread::start();
    QThread::setPriority(QThread::HighestPriority);
}

void FontsModel::run()
{
    m_generalFonts.clear();
    m_fixedFonts.clear();

    FcPattern *pat = FcPatternCreate();
    if (!pat)
        return;

    FcObjectSet *objSet = FcObjectSetBuild(
                FC_FAMILY,
                FC_FAMILYLANG,
                FC_CHARSET,
                FC_STYLE,
                FC_LANG,
                FC_SPACING,
                NULL);

    if (!objSet) {
        FcPatternDestroy(pat);
        return;
    }

    FcFontSet *fs = FcFontList(0, pat, objSet);
    FcObjectSetDestroy(objSet);
    FcPatternDestroy(pat);

    if (!fs) {
         return;
    }

    for (int i = 0; i < fs->nfont; ++i) {
         char *charset = (char*)FcPatternFormat(fs->fonts[i], (FcChar8*)"%{charset}");

         if (charset == NULL || strlen(charset) == 0) {
             free(charset);
             continue;
         }
         free(charset);

         QString family = getFcLangString(fs->fonts[i], FC_FAMILY, FC_FAMILYLANG);
         QString language = (char*)FcPatternFormat(fs->fonts[i], (FcChar8*)"%{lang}");

         int spacing = FC_PROPORTIONAL;
         if (FcPatternGetInteger(fs->fonts[i], FC_SPACING, 0, &spacing) != FcResultMatch)
             spacing = FC_PROPORTIONAL;
         bool fixedPitch = spacing >= FC_MONO;

         if (family.isEmpty())
             continue;

         if (fixedPitch && !m_fixedFonts.contains(family)) {
             m_fixedFonts.append(family);
         } else {
             if (!m_generalFonts.contains(family)
                     && (language.contains(m_lanCode, Qt::CaseInsensitive) ||
                         language.contains(m_lanCode.split('-').first(), Qt::CaseInsensitive)))
                 m_generalFonts.append(family);
         }
    }
    FcFontSetDestroy(fs);

    std::sort(m_fixedFonts.begin(), m_fixedFonts.end());
    std::sort(m_generalFonts.begin(), m_generalFonts.end());

    emit loadFinished();
}

QStringList FontsModel::generalFonts() const
{
    return m_generalFonts;
}

QStringList FontsModel::fixedFonts() const
{
    return m_fixedFonts;
}

QString FontsModel::systemGeneralFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
}

QString FontsModel::systemFixedFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
}
