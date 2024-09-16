// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "insiderplugin.h"

#include "insidermodule.h"

DccInsiderPlugin::DccInsiderPlugin(QObject *parent)
    : PluginInterface(parent)
{
}

DccInsiderPlugin::~DccInsiderPlugin()
{

}

QString DccInsiderPlugin::name() const
{
    return QStringLiteral("dcc-insider-plugin");
}

dccV23::ModuleObject *DccInsiderPlugin::module()
{
//    DCC_NAMESPACE::ModuleObject * parentModule = new DCC_NAMESPACE::PageModule();
//    parentModule->setName("insider");
//    parentModule->setDisplayName("insider");

    InsiderModule * insiderModule = new InsiderModule(this);
//    parentModule->appendChild(insiderModule);

    return insiderModule;
}

QString DccInsiderPlugin::follow() const
{
    return QStringLiteral("update");
}

QString DccInsiderPlugin::location() const
{
    return QStringLiteral("13");
}


