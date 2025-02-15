/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: MIT
*/
#include "test11.h"
#include "test11a.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    Test11 *t = new Test11();
    Test11a *t2 = new Test11a();
    delete t;
    delete t2;
    return 0;
}
