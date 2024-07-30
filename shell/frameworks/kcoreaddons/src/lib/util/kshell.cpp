/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2003, 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kshell.h"
#include "kshell_p.h"
#include "kuser.h"

#include <QDir>

QString KShell::homeDir(const QString &user)
{
    if (user.isEmpty()) {
        return QDir::homePath();
    }
    return KUser(user).homeDir();
}

QString KShell::joinArgs(const QStringList &args)
{
    QString ret;
    for (const auto &arg : args) {
        if (!ret.isEmpty()) {
            ret.append(QLatin1Char(' '));
        }
        ret += quoteArg(arg);
    }
    return ret;
}

#ifdef Q_OS_WIN
#define ESCAPE '^'
#else
#define ESCAPE '\\'
#endif

QString KShell::tildeExpand(const QString &fname)
{
    if (!fname.isEmpty() && fname[0] == QLatin1Char('~')) {
        int pos = fname.indexOf(QLatin1Char('/'));
        if (pos < 0) {
            return homeDir(fname.mid(1));
        }
        QString ret = homeDir(fname.mid(1, pos - 1));
        if (!ret.isNull()) {
            ret += QStringView(fname).mid(pos);
        }
        return ret;
    } else if (fname.length() > 1 && fname[0] == QLatin1Char(ESCAPE) && fname[1] == QLatin1Char('~')) {
        return fname.mid(1);
    }
    return fname;
}

QString KShell::tildeCollapse(const QString &path)
{
    if (!path.isEmpty()) {
        const auto homePath = QDir::homePath();
        if (path.startsWith(homePath)) {
            auto newPath = path;
            newPath.replace(0, homePath.length(), QLatin1Char('~'));
            return newPath;
        }
    }
    return path;
}
