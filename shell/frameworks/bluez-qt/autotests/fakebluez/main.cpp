/*
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "fakebluez.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    new FakeBluez(&app);

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.bluezqt.test"))) {
        qWarning() << "Cannot register org.kde.bluezqt.test service!";
        return 1;
    }

    return app.exec();
}
