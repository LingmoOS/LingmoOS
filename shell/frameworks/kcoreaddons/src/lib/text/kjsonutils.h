/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJSONUTILS_H
#define KJSONUTILS_H

#include "kcoreaddons_export.h"

#include <QJsonValue>

class QJsonObject;
class QString;

namespace KJsonUtils
{
/**
 * Reads a value from @p jo but unlike QJsonObject::value() it allows different entries for each locale
 * This is done by appending the locale identifier in brackets to the key (e.g. "[de_DE]" or "[es]")
 * When looking for a key "foo" with German (Germany) locale we will first attempt to read "foo[de_DE]",
 * if that does not exist "foo[de]", finally falling back to "foo" if that also doesn't exist.
 * @return the translated value for @p key from @p jo or @p defaultValue if @p key was not found
 * @since 5.88
 */
KCOREADDONS_EXPORT QJsonValue readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue = QJsonValue());

/**
 * @return the translated value of @p key from @p jo as a string or @p defaultValue if @p key was not found
 * or the value for @p key is not of type string
 * @see KPluginMetaData::readTranslatedValue(const QJsonObject &jo, const QString &key)
 * @since 5.88
 */
KCOREADDONS_EXPORT QString readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue = QString());
}

#endif // KJSONUTILS_H
