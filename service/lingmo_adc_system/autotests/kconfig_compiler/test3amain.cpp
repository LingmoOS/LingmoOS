/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test3a.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestNameSpace::InnerNameSpace::Test3a *t = new TestNameSpace::InnerNameSpace::Test3a();
    delete t;
    return 0;
}
