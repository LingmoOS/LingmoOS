// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "insiderplugin.h"

#include "insidermodule.h"

OceanUIInsiderPlugin::OceanUIInsiderPlugin(QObject *parent)
    : PluginInterface(parent)
{
}

OceanUIInsiderPlugin::~OceanUIInsiderPlugin()
{

}

QString OceanUIInsiderPlugin::name() const
{
    return QStringLiteral("oceanui-insider-plugin");
}

oceanuiV23::ModuleObject *OceanUIInsiderPlugin::module()
{
//    DCC_NAMESPACE::ModuleObject * parentModule = new DCC_NAMESPACE::PageModule();
//    parentModule->setName("insider");
//    parentModule->setDisplayName("insider");

    InsiderModule * insiderModule = new InsiderModule(this);
//    parentModule->appendChild(insiderModule);

    return insiderModule;
}

QString OceanUIInsiderPlugin::follow() const
{
    return QStringLiteral("update");
}

QString OceanUIInsiderPlugin::location() const
{
    return QStringLiteral("13");
}


