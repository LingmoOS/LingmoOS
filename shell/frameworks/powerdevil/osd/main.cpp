/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie.clarius@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <LayerShellQt/Shell>

#include <QGuiApplication>

#include <KCrash>

#include "osdmanager.h"

int main(int argc, char **argv)
{
    LayerShellQt::Shell::useLayerShell();
    QGuiApplication app(argc, argv);
    KCrash::initialize();
    PowerDevil::OsdManager osdManager;
    return app.exec();
}
