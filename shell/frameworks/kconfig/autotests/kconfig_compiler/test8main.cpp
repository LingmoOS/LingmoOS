/*
    SPDX-FileCopyrightText: 2005 Michael Brade <brade@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test8a.h"
#include "test8b.h"
#include "test8c.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test8a *config1 = new Test8a(KSharedConfig::openConfig(QString()));
    Test8a *config2 = new Test8a();
    Test8b::self();
    Test8c::instance(KSharedConfig::openConfig(QString()));
    Test8c::self();
    delete config1;
    delete config2;
    return 0;
}
