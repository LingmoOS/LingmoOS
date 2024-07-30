/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include <QCoreApplication>
#include <QTimer>
#include <QtGlobal>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <stdio.h>

class Tester
{
public:
    void initConfig();
    ~Tester();
};

void Tester::initConfig()
{
    fprintf(stderr, "app Tester\n");
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("test"));
    group.writeEntry("test", 0);
}

Tester::~Tester()
{
    fprintf(stderr, "app ~Tester\n");
    KConfigGroup group(KSharedConfig::openConfig(), QStringLiteral("test"));
    group.writeEntry("test", 1);
}

Q_GLOBAL_STATIC(Tester, globalTestObject)

int main(int argc, char **argv)
{
    qputenv("QT_FATAL_WARNINGS", "1");
    QCoreApplication app(argc, argv);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    Tester *t = globalTestObject();
    t->initConfig();

    QTimer::singleShot(0, qApp, SLOT(quit()));
    return app.exec();
}
