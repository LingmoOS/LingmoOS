/*
 * SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KIdleTest.h"
#include <QGuiApplication>

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    KIdleTest t;

    return app.exec();
}
