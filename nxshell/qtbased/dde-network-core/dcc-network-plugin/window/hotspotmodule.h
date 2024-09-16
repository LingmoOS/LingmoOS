// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef HOTSPOTMODULE_H
#define HOTSPOTMODULE_H
#include "interface/pagemodule.h"

#include <dtkwidget_global.h>

DWIDGET_BEGIN_NAMESPACE
class DListView;
DWIDGET_END_NAMESPACE

namespace DCC_NAMESPACE {
class SwitchWidget;
}

namespace dde {
namespace network {
class NetworkDeviceBase;
class NetworkController;
class WirelessDevice;
class WiredConnection;
class ControllItems;
enum class DeviceStatus;
}
}
//class ControllItemsModel;

class HotspotDeviceItem : public QObject
{
    Q_OBJECT
public:
    explicit HotspotDeviceItem(dde::network::WirelessDevice *dev, QObject *parent = nullptr);
    inline dde::network::WirelessDevice *device() { return m_device; }
    inline QList<DCC_NAMESPACE::ModuleObject *> &modules() { return m_modules; }

public Q_SLOTS:
    void openEditPage(dde::network::ControllItems *item, QWidget *parent);

private:
    void initHotspotList(DTK_WIDGET_NAMESPACE::DListView *lvProfiles);

private Q_SLOTS:
    void onSwitchToggled(const bool checked);
    void closeHotspot();
    void openHotspot(DCC_NAMESPACE::SwitchWidget *switchWidget);

private:
    dde::network::WirelessDevice *m_device;
    QList<DCC_NAMESPACE::ModuleObject *> m_modules;
};

class HotspotModule : public DCC_NAMESPACE::PageModule
{
    Q_OBJECT
public:
    explicit HotspotModule(QObject *parent = nullptr);

Q_SIGNALS:
    void updateItemOnlyOne(bool visiable);

private Q_SLOTS:
    void onDeviceAdded(const QList<dde::network::WirelessDevice *> &devices);
    void onDeviceRemove(const QList<dde::network::WirelessDevice *> &rmDevices);
    void updateVisiable();

private:
    QList<HotspotDeviceItem *> m_items;
    QString m_newConnectionPath;
};

#endif // HOTSPOTMODULE_H
