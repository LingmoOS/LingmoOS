// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "plugins.h"

MyPlugin::MyPlugin(QObject *parent)
    : QObject(parent)
{
}
MyPlugin::~MyPlugin() = default;
MyPlugin2::~MyPlugin2() = default;

MyPlugin2::MyPlugin2(QObject *parent)
    : QObject(parent)
{
}

#include "moc_plugins.cpp"
