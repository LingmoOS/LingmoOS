// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wirelessmodule.h"
#include "wirelessdevicemodel.h"
#include "widgets/widgetmodule.h"
#include "widgets/settingsitem.h"
#include "widgets/switchwidget.h"
#include "widgets/settingsgroup.h"
#include "editpage/connectionwirelesseditpage.h"

#include <DFontSizeManager>
#include <DListView>

#include <QPushButton>
#include <QApplication>
#include <QScroller>
#include <QLabel>

#include <wirelessdevice.h>
#include <networkcontroller.h>
#include <hotspotcontroller.h>

#include <networkmanagerqt/manager.h>
#include <networkmanagerqt/wirelesssetting.h>
#include <networkmanagerqt/setting.h>

using namespace dde::network;
using namespace DCC_NAMESPACE;
DWIDGET_USE_NAMESPACE

WirelessModule::WirelessModule(WirelessDevice *dev, QObject *parent)
    : PageModule("wireless", dev->deviceName(), QString(), QIcon::fromTheme("dcc_wifi"), parent)
    , m_device(dev)
{
    onNameChanged(m_device->deviceName());
    connect(m_device, &WirelessDevice::nameChanged, this, &WirelessModule::onNameChanged);
    appendChild(new WidgetModule<SwitchWidget>("wireless_adapter", tr("Wireless Network Adapter"), [this](SwitchWidget *devEnabled) {
        QLabel *lblTitle = new QLabel(tr("Wireless Network Adapter")); // 无线网卡
        DFontSizeManager::instance()->bind(lblTitle, DFontSizeManager::T5, QFont::DemiBold);
        devEnabled->setLeftWidget(lblTitle);
        devEnabled->setChecked(m_device->isEnabled());
        connect(devEnabled, &SwitchWidget::checkedChanged, this, &WirelessModule::onNetworkAdapterChanged);
        connect(m_device, &WirelessDevice::enableChanged, devEnabled, [devEnabled](const bool enabled) {
            devEnabled->blockSignals(true);
            devEnabled->setChecked(enabled);
            devEnabled->blockSignals(false);
        });
    }));
    ModuleObject *wirelesslist = new WidgetModule<DListView>("wirelesslist", QString(), this, &WirelessModule::initWirelessList);
    appendChild(wirelesslist);
    ModuleObject *hotspotTips = new WidgetModule<SettingsGroup>("hotspot_tips", tr("Disable hotspot first if you want to connect to a wireless network"), [](SettingsGroup *tipsGroup) {
        QLabel *tips = new QLabel;
        tips->setAlignment(Qt::AlignCenter);
        tips->setWordWrap(true);
        tips->setText(tr("Disable hotspot first if you want to connect to a wireless network"));
        tipsGroup->insertWidget(tips);
    });
    appendChild(hotspotTips);
    ModuleObject *closeHotspot = new WidgetModule<QPushButton>("close_hotspot", tr("Close Hotspot"), [this](QPushButton *closeHotspotBtn) {
        closeHotspotBtn->setText(tr("Close Hotspot"));
        connect(closeHotspotBtn, &QPushButton::clicked, this, [=] {
            // 此处抛出这个信号是为了让外面记录当前关闭热点的设备，因为在关闭热点过程中，当前设备会移除一次，然后又会添加一次，相当于触发了两次信号，
            // 此时外面就会默认选中第一个设备而无法选中当前设备，因此在此处抛出信号是为了让外面能记住当前选择的设备
            NetworkController::instance()->hotspotController()->setEnabled(m_device, false);
        });
    });
    appendChild(closeHotspot);

    auto updateVisible = [this, wirelesslist, hotspotTips, closeHotspot] {
        bool devEnable = m_device->isEnabled();
        bool hotspotEnable = devEnable && m_device->hotspotEnabled();
        wirelesslist->setVisible(devEnable && !hotspotEnable);
        hotspotTips->setVisible(hotspotEnable);
        closeHotspot->setVisible(hotspotEnable);
    };

    updateVisible();
    connect(m_device, &WirelessDevice::enableChanged, this, updateVisible);
    connect(m_device, &WirelessDevice::hotspotEnableChanged, this, updateVisible);
}

void WirelessModule::initWirelessList(DListView *lvAP)
{
    lvAP->setAccessibleName("List_wirelesslist");
    WirelessDeviceModel *model = new WirelessDeviceModel(m_device, lvAP);
    lvAP->setModel(model);
    lvAP->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lvAP->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    lvAP->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lvAP->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    lvAP->setSelectionMode(QAbstractItemView::NoSelection);
    auto adjustHeight = [lvAP]() {
        lvAP->setMinimumHeight(lvAP->model()->rowCount() * 41);
    };
    adjustHeight();
    connect(model, &WirelessDeviceModel::modelReset, lvAP, adjustHeight);
    connect(model, &WirelessDeviceModel::detailClick, this, &WirelessModule::onApWidgetEditRequested);
    QScroller *scroller = QScroller::scroller(lvAP->viewport());
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scroller->setScrollerProperties(sp);

    connect(lvAP, &DListView::clicked, this, [this](const QModelIndex &index) {
        AccessPoints *ap = static_cast<AccessPoints *>(index.internalPointer());
        if (!ap) { // nullptr 为隐藏网络
            onApWidgetEditRequested(nullptr, qobject_cast<QWidget *>(sender()));
            return;
        }
        if (ap->connected())
            return;
        m_device->connectNetwork(ap);
    });
}

void WirelessModule::active() {
    PageModule::active();

    if (m_device->isEnabled()) {
        m_device->scanNetwork();
    }
}

void WirelessModule::onNameChanged(const QString &name)
{
    QString tmp;
    for (auto it = name.begin(); it != name.end(); ++it) {
        if ((*it) >= '0' && (*it) <= '9')
            tmp.append((*it));
    }
    setName("wireless" + tmp);
    setDisplayName(name);
}

void WirelessModule::onNetworkAdapterChanged(bool checked)
{
    m_device->setEnabled(checked);
    if (checked) {
        m_device->scanNetwork();
    }
}

void WirelessModule::onApWidgetEditRequested(AccessPoints *ap, QWidget *parent)
{
    QString uuid;
    QString apPath;
    QString ssid;
    bool isHidden = true;
    if (ap) {
        ssid = ap->ssid();
        apPath = ap->path();
        isHidden = ap->hidden();

        for (auto conn : NetworkManager::activeConnections()) {
            if (conn->type() != NetworkManager::ConnectionSettings::ConnectionType::Wireless || conn->id() != ssid)
                continue;

            NetworkManager::ConnectionSettings::Ptr connSettings = conn->connection()->settings();
            NetworkManager::WirelessSetting::Ptr wSetting = connSettings->setting(NetworkManager::Setting::SettingType::Wireless).staticCast<NetworkManager::WirelessSetting>();
            if (wSetting.isNull())
                continue;

            QString settingMacAddress = wSetting->macAddress().toHex().toUpper();
            QString deviceMacAddress = m_device->realHwAdr().remove(":");
            if (!settingMacAddress.isEmpty() && settingMacAddress != deviceMacAddress)
                continue;

            uuid = conn->uuid();
            break;
        }
        if (uuid.isEmpty()) {
            const QList<WirelessConnection *> lstConnections = m_device->items();
            for (auto item : lstConnections) {
                if (item->connection()->ssid() != ssid)
                    continue;

                uuid = item->connection()->uuid();
                if (!uuid.isEmpty()) {
                    break;
                }
            }
        }
    }
    ConnectionWirelessEditPage *apEditPage = new ConnectionWirelessEditPage(m_device->path(), uuid, apPath, isHidden, parent);
    apEditPage->setAttribute(Qt::WA_DeleteOnClose);
    connect(apEditPage, &ConnectionWirelessEditPage::disconnect, this, [this] {
        m_device->disconnectNetwork();
    });

    if (!uuid.isEmpty() || !ap) {
        apEditPage->initSettingsWidget();
    } else {
        apEditPage->initSettingsWidgetFromAp();
    }
    apEditPage->setLeftButtonEnable(true);

    auto devChange = [this, apEditPage]() {
        bool devEnable = m_device->isEnabled();
        bool hotspotEnable = devEnable && m_device->hotspotEnabled();
        if (!devEnable || hotspotEnable) {
            apEditPage->close();
        }
    };

    connect(m_device, &WirelessDevice::enableChanged, apEditPage, devChange);
    connect(m_device, &WirelessDevice::hotspotEnableChanged, apEditPage, devChange);

    apEditPage->exec();
}
