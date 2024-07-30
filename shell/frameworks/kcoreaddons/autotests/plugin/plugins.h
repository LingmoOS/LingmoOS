// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later
#pragma once

#include "plugin_classes_export.h"
#include <QObject>

class PLUGIN_CLASSES_EXPORT MyPlugin : public QObject
{
    Q_OBJECT
public:
    MyPlugin(QObject *parent);
    ~MyPlugin() override;
};

class PLUGIN_CLASSES_EXPORT MyPlugin2 : public QObject
{
    Q_OBJECT
public:
    MyPlugin2(QObject *parent);
    ~MyPlugin2() override;
};
