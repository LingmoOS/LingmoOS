/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "projecttest.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    ProjectTest foo;
    foo.show();
    return app.exec();
}
