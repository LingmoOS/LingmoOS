/*
    SPDX-FileCopyrightText: 1999-2003 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2001 Martijn Klingens <klingens@kde.org>
    SPDX-FileCopyrightText: 2007 David Jarvie <software@astrojar.org.uk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLANGUAGENAME_H
#define KLANGUAGENAME_H

#include "kconfigwidgets_export.h"

#include <QStringList>

class QString;

/**
 * @class KLanguageName klanguagename.h KLanguageName
 *
 * KLanguageName is a helper namespace that returns the name of a given language code.
 *
 * @since 5.55
 *
 */
namespace KLanguageName
{
/**
 * Returns the name of the given language code in the current locale.
 *
 * If it can't be found in the current locale it returns the name in English.
 *
 * It it can't be found in English either it returns an empty QString.
 *
 * @param code code (ISO 639-1) of the language whose name is wanted.
 */
KCONFIGWIDGETS_EXPORT QString nameForCode(const QString &code);

/**
 * Returns the name of the given language code in the other given locale code.
 *
 * If it can't be found in the given locale it returns the name in English.
 *
 * It it can't be found in English either it returns an empty QString.
 *
 * @param code code (ISO 639-1) of the language whose name is wanted.
 * @param outputLocale code (ISO 639-1) of the language in which we want the name in.
 */
KCONFIGWIDGETS_EXPORT QString nameForCodeInLocale(const QString &code, const QString &outputLocale);

/**
 * Returns the list of language codes found on the system.
 *
 * @since 5.74
 */
KCONFIGWIDGETS_EXPORT QStringList allLanguageCodes();
}

#endif
