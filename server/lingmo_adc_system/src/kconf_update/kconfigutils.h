/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2010 Canonical Ltd
    SPDX-FileContributor: Aurélien Gâteau <aurelien.gateau@canonical.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCONFIGUTILS_H
#define KCONFIGUTILS_H

#include <QStringList>

class QString;

class KConfig;
class KConfigGroup;

namespace KConfigUtils
{
bool hasGroup(KConfig *, const QStringList &);

KConfigGroup openGroup(KConfig *, const QStringList &);

QStringList parseGroupString(const QString &str, bool *ok, QString *error);

QString unescapeString(const QString &str, bool *ok, QString *error);

} // namespace

#endif /* KCONFIGUTILS_H */
