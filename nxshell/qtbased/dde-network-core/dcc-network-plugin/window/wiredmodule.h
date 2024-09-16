// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WIREDMODULE_H
#define WIREDMODULE_H

#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
DWIDGET_END_NAMESPACE

namespace dde {
namespace network {
class NetworkDeviceBase;
class NetworkController;
class WiredDevice;
class WiredConnection;
class ControllItems;
enum class DeviceStatus;
}
}

class WiredModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit WiredModule(dde::network::WiredDevice *dev, QObject *parent = nullptr);

private Q_SLOTS:
    void initWirelessList(DTK_WIDGET_NAMESPACE::DListView *lvProfiles);
    void editConnection(dde::network::ControllItems *item, QWidget *parent);
    void onNameChanged(const QString &name);

private:
    dde::network::WiredDevice *m_device;
    QString m_newConnectionPath;
};

#endif // WIREDMODULE_H
