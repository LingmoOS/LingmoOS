// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include <framework/framework.h>

class Collaborators : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.lingmo.plugin.unioncode" FILE "collaborators.json")
public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual dpf::Plugin::ShutdownFlag stop() override;
};

#endif // COREPLUGIN_H
