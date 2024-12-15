// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COOPERATIONCOREPLUGIN_H
#define COOPERATIONCOREPLUGIN_H

#include <ocean-cooperation-framework/dpf.h>

QT_BEGIN_NAMESPACE
// class QQmlApplicationEngine;
QT_END_NAMESPACE

namespace daemon_core {

class daemonCorePlugin : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.plugin.daemon" FILE "daemoncoreplugin.json")

    DPF_EVENT_NAMESPACE(lingmo_cross_cooperation)

public:
    virtual void initialize() override;
    virtual bool start() override;

private:

};

}
#endif   // COOPERATIONCOREPLUGIN_H
