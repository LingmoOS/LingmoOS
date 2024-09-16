// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINCREATOR_H
#define PLUGINCREATOR_H

#include <dde-cooperation-framework/dde_cooperation_framework_global.h>
#include <dde-cooperation-framework/lifecycle/plugin.h>

#include <QObject>

DPF_BEGIN_NAMESPACE

class PluginCreator : public QObject
{
    Q_OBJECT

public:
    virtual QSharedPointer<Plugin> create(const QString &pluginName) = 0;
};

DPF_END_NAMESPACE

#endif   // PLUGINCREATOR_H
