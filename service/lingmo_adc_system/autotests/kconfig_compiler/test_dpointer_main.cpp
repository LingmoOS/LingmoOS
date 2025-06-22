/*
    SPDX-FileCopyrightText: 2005 Duncan Mac-Vicar P. <duncan@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test_dpointer.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestDPointer *t = TestDPointer::self();
    delete t;
    return 0;
}
