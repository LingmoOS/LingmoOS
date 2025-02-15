/*
    SPDX-FileCopyrightText: 2003, 2004 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test4.h"
#include <QFile>
#include <QGuiApplication>
#include <QStandardPaths>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    {
        KConfig initialConfig(QStringLiteral("test4rc"));
        KConfigGroup group = initialConfig.group(QStringLiteral("Foo"));
        group.writeEntry(QStringLiteral("foo bar"), QStringLiteral("Value"));
    }
    Test4 *t = Test4::self();
    const bool ok = QFile::exists(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/test4rc"});
    if (!ok) {
        qWarning() << "config file was not created!";
    }
    if (t->fooBar() != QLatin1String("Value")) {
        qWarning() << "wrong value for foo bar:" << t->fooBar();
    }
    delete t;
    return ok ? 0 : 1;
}
