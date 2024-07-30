/*
    SPDX-FileCopyrightText: 2008 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef FONTHELPERS_P_H
#define FONTHELPERS_P_H

// i18n-related helpers for fonts, common to KFont* widgets.

#include <QCoreApplication>
#include <QString>
#include <QStringList>

#include <map>

#ifdef NEVERDEFINE // never true
// Font names up for translation, listed for extraction.

//: Generic sans serif font presented in font choosers. When selected,
//: the system will choose a real font, mandated by distro settings.
QT_TRANSLATE_NOOP3("FontHelpers", "Sans Serif", "@item Font name");
//: Generic serif font presented in font choosers. When selected,
//: the system will choose a real font, mandated by distro settings.
QT_TRANSLATE_NOOP3("FontHelpers", "Serif", "@item Font name");
//: Generic monospace font presented in font choosers. When selected,
//: the system will choose a real font, mandated by distro settings.
QT_TRANSLATE_NOOP3("FontHelpers", "Monospace", "@item Font name");

#endif

/**
 * @internal
 *
 * Split the compound raw font name into family and foundry.
 *
 * @param name the raw font name reported by Qt
 * @param family the storage for family name
 * @param foundry the storage for foundry name
 */
inline void splitFontString(QStringView name, QString *family, QString *foundry = nullptr)
{
    int p1 = name.indexOf(QLatin1Char('['));
    if (p1 < 0) {
        if (family) {
            *family = name.trimmed().toString();
        }
        if (foundry) {
            foundry->clear();
        }
    } else {
        int p2 = name.indexOf(QLatin1Char(']'), p1);
        p2 = p2 > p1 ? p2 : name.length();
        if (family) {
            *family = name.left(p1).trimmed().toString();
        }
        if (foundry) {
            *foundry = name.mid(p1 + 1, p2 - p1 - 1).trimmed().toString();
        }
    }
}

/**
 * @internal
 *
 * Translate the font name for the user.
 * Primarily for generic fonts like Serif, Sans-Serif, etc.
 *
 * @param name the raw font name reported by Qt
 * @return translated font name
 */
inline QString translateFontName(QStringView name)
{
    QString family;
    QString foundry;
    splitFontString(name, &family, &foundry);

    // Obtain any regular translations for the family and foundry.
    QString trFamily = QCoreApplication::translate("FontHelpers", family.toUtf8().constData(), "@item Font name");
    QString trFoundry = foundry;
    if (!foundry.isEmpty()) {
        trFoundry = QCoreApplication::translate("FontHelpers", foundry.toUtf8().constData(), "@item Font foundry");
    }

    // Assemble full translation.
    QString trfont;
    if (foundry.isEmpty()) {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.
        trfont = QCoreApplication::translate("FontHelpers", "%1", "@item Font name").arg(trFamily);
    } else {
        // i18n: Filter by which the translators can translate, or otherwise
        // operate on the font names not put up for regular translation.
        trfont = QCoreApplication::translate("FontHelpers", "%1 [%2]", "@item Font name [foundry]").arg(trFamily, trFoundry);
    }
    return trfont;
}

static bool fontFamilyCompare(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}

using FontFamiliesMap = std::map<QString, QString, decltype(fontFamilyCompare) *>;

/**
 * @internal
 *
 * Compose locale-aware sorted list of translated font names,
 * with generic fonts handled in a special way.
 * The mapping of translated to raw names can be reported too if required.
 *
 * @param names raw font names as reported by Qt
 * @param trToRawNames storage for mapping of translated to raw names
 * @return sorted list of translated font names
 */
inline FontFamiliesMap translateFontNameList(const QStringList &names)
{
    FontFamiliesMap trMap(fontFamilyCompare);

    for (const QString &fName : names) {
        trMap.insert({translateFontName(fName), fName});
    }

    return trMap;
}

#endif
