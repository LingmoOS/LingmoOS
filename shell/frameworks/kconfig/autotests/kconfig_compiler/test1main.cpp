/*
    SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test1.h"
#include <KConfig>
#include <KConfigGroup>
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    {
        KConfig initialConfig(QStringLiteral("examplerc"));
        KConfigGroup group = initialConfig.group(QStringLiteral("MyOptions"));
        group.writeEntry(QStringLiteral("MyString"), QStringLiteral("The String"));
    }
    Test1 *t = new Test1(QString(), QString());

    bool ok = t->myString() == QLatin1String("The String");

    delete t;
    return ok ? 0 : 1;
}
