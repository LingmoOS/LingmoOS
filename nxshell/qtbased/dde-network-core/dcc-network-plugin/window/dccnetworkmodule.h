// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef NETWORKINTERFACE_H
#define NETWORKINTERFACE_H

#include "interface/hlistmodule.h"
#include "interface/plugininterface.h"

namespace dde {
namespace network {
class NetworkDeviceBase;
}
}

class QEvent;

class NetworkModule : public DCC_NAMESPACE::HListModule
{
    Q_OBJECT
public:
    explicit NetworkModule(QObject *parent = nullptr);
    ~NetworkModule() override {}
    virtual void active() override;

private Q_SLOTS:
    void updateVisiable();
    void updateModel();

protected:
    bool event(QEvent *ev) override;
    void init();
    ModuleObject *defultModule() override;

private:
    QList<DCC_NAMESPACE::ModuleObject *> m_modules;
    QList<DCC_NAMESPACE::ModuleObject *> m_wiredModules;
    QList<DCC_NAMESPACE::ModuleObject *> m_wirelessModules;
    QMap<dde::network::NetworkDeviceBase *, DCC_NAMESPACE::ModuleObject *> m_deviceMap;
};

class DccNetworkPlugin : public DCC_NAMESPACE::PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.deepin.dde.network.dcc" FILE "network.json")
    Q_INTERFACES(DCC_NAMESPACE::PluginInterface)
public:
    explicit DccNetworkPlugin(QObject *parent = nullptr);
    ~DccNetworkPlugin() override;

    virtual QString name() const override;
    virtual DCC_NAMESPACE::ModuleObject *module() override;
    virtual QString location() const override;

private:
    DCC_NAMESPACE::ModuleObject *m_moduleRoot;
};

#endif // NETWORKINTERFACE_H
