/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000, 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "keditbookmarks_p.h"

#include <QGuiApplication>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>

#include "kbookmarksettings_p.h"

void KEditBookmarks::setBrowserMode(bool browserMode)
{
    m_browserMode = browserMode;
}

KEditBookmarks::OpenResult KEditBookmarks::openForFile(const QString &file)
{
    QStringList args;
    args << QStringLiteral("--customcaption") << QGuiApplication::applicationDisplayName();

    if (!m_browserMode) {
        args << QStringLiteral("--nobrowser");
    }

    args << file;
    return startKEditBookmarks(args);
}

KEditBookmarks::OpenResult KEditBookmarks::openForFileAtAddress(const QString &file, const QString &address)
{
    QStringList args;
    args << QStringLiteral("--customcaption") << QGuiApplication::applicationDisplayName();

    if (!m_browserMode) {
        args << QStringLiteral("--nobrowser");
    }

    args << QStringLiteral("--address") << address;

    args << file;
    return startKEditBookmarks(args);
}

KEditBookmarks::OpenResult KEditBookmarks::startKEditBookmarks(const QStringList &args)
{
    const QString exec = QStandardPaths::findExecutable(QStringLiteral(KEDITBOOKMARKS_BINARY));
    if (!exec.isEmpty()) {
        bool success = QProcess::startDetached(exec, args);

        if (!success) {
            OpenResult result;
            result.m_sucess = false;
            result.m_errorMessage = QObject::tr("keditbookmarks could not be started");
            return result;
        }

    } else {
        OpenResult result;
        result.m_sucess = false;
        result.m_errorMessage = QObject::tr("The keditbookmarks executable was not found");
        return result;
    }

    OpenResult result;
    result.m_sucess = true;
    return result;
}
