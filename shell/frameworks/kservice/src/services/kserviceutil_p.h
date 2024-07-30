/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEUTIL_P_H
#define KSERVICEUTIL_P_H

#include <QString>

class KServiceUtilPrivate
{
public:
    /**
     * Lightweight implementation of QFileInfo::completeBaseName.
     *
     * Returns the complete base name of the file without the path.
     * The complete base name consists of all characters in the file up to (but not including) the last '.' character.
     *
     * Example: "/tmp/archive.tar.gz" --> "archive.tar"
     */
    static QString completeBaseName(const QString &filepath)
    {
        QString name = filepath;
        int pos = name.lastIndexOf(QLatin1Char('/'));
        if (pos != -1) {
            name.remove(0, pos + 1);
        }
        pos = name.lastIndexOf(QLatin1Char('.'));
        if (pos != -1) {
            name.truncate(pos);
        }
        return name;
    }
};

#endif
