/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: MIT
*/

#include "test_emptyentries.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    QMakeBuilderSettings::instance(QStringLiteral("abc"));
    auto *t = QMakeBuilderSettings::self();
    delete t;
    return 0;
}
