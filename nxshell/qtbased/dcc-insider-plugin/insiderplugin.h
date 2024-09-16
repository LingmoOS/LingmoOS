// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <interface/plugininterface.h>

class DccInsiderPlugin : public DCC_NAMESPACE::PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.dcc.insider" FILE "insider.json")
    Q_INTERFACES(DCC_NAMESPACE::PluginInterface)
public:
    explicit DccInsiderPlugin(QObject *parent = nullptr);
    ~DccInsiderPlugin() override;

    virtual QString name() const override;
    virtual DCC_NAMESPACE::ModuleObject *module() override;
    virtual QString follow() const override;
    virtual QString location() const override;
};
