/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2010-2012 Martin Sandsmark <martin.sandsmark@kde.org>
    SPDX-FileCopyrightText: 2018 Olivier Churlaud <olivier@churlaud.com>

    SPDX-License-Identifier: LGPL-3.0-or-later
*/

#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Dialog dialog;
    dialog.show();
    return app.exec();
}
