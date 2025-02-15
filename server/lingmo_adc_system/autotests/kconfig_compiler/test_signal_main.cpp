/*
    SPDX-FileCopyrightText: 2006 Michael Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: MIT
*/

#include "test_signal.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestSignal *t = TestSignal::self();
    delete t;
    return 0;
}
