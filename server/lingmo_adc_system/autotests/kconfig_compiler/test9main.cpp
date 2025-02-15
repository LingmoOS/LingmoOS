/*
    SPDX-FileCopyrightText: 2005 Helge Deller <deller@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test9.h"
#include <QDebug>
#include <QDir>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test9 *t = new Test9(QString(), QString());

    QStringList myPathsList2 = t->myPathsList2();
    qWarning() << myPathsList2;

    // add another path
    QStringList newlist;
    myPathsList2 << QDir::homePath() + QLatin1String("/.kde");
    qWarning() << myPathsList2;

    t->setMyPathsList2(myPathsList2);
    qWarning() << t->myPathsList2();

    delete t;
    return 0;
}
