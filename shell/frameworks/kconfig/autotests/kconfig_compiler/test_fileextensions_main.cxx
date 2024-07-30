/*
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "test_fileextensions.hxx"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    Q_UNUSED(app);
    TestFileExtensions *t = TestFileExtensions::self();
    delete t;
    return 0;
}
