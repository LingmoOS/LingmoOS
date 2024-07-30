/*
    SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef GLOBALSERVICE_H
#define GLOBALSERVICE_H

#include <QObject>
#include <QString>

class GlobalService : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    static void globalMute();
    static void volumeUp();
    static void volumeDown();
    static void volumeUpSmall();
    static void volumeDownSmall();

private:
    static void ensureKDEDModule();
    static void invokeShortcut(const QString &name);
};

#endif // GLOBALSERVICE_H
