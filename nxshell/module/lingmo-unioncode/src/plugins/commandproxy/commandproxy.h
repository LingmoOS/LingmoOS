// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDPROXY_H
#define COMMANDPROXY_H

#include <framework/framework.h>
#include <QObject>

class CommandProxy: public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.plugin.unioncode" FILE "commandproxy.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;
};

#endif // COMMANDPROXY_H
