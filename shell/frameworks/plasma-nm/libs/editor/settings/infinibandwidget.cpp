/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "infinibandwidget.h"
#include "ui_infiniband.h"
#include "uiutils.h"

#include <KLocalizedString>

#include <NetworkManagerQt/InfinibandSetting>
#include <NetworkManagerQt/Utils>

InfinibandWidget::InfinibandWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::InfinibandWidget)
{
    m_ui->setupUi(this);

    m_ui->transport->addItem(i18nc("infiniband transport mode", "Datagram"), NetworkManager::InfinibandSetting::Datagram);
    m_ui->transport->addItem(i18nc("infiniband transport mode", "Connected"), NetworkManager::InfinibandSetting::Connected);
    m_ui->transport->setCurrentIndex(0);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->macAddress, &HwAddrComboBox::hwAddressChanged, this, &InfinibandWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

InfinibandWidget::~InfinibandWidget()
{
    delete m_ui;
}

void InfinibandWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::InfinibandSetting::Ptr infinibandSetting = setting.staticCast<NetworkManager::InfinibandSetting>();

    if (infinibandSetting->transportMode() != NetworkManager::InfinibandSetting::Unknown) {
        if (infinibandSetting->transportMode() == NetworkManager::InfinibandSetting::Datagram) {
            m_ui->transport->setCurrentIndex(0);
        } else if (infinibandSetting->transportMode() == NetworkManager::InfinibandSetting::Connected) {
            m_ui->transport->setCurrentIndex(1);
        }
    }
    m_ui->macAddress->init(NetworkManager::Device::InfiniBand, NetworkManager::macAddressAsString(infinibandSetting->macAddress()));
    if (infinibandSetting->mtu()) {
        m_ui->mtu->setValue(infinibandSetting->mtu());
    }
}

QVariantMap InfinibandWidget::setting() const
{
    NetworkManager::InfinibandSetting setting;
    if (m_ui->transport->currentIndex() == 0) {
        setting.setTransportMode(NetworkManager::InfinibandSetting::Datagram);
    } else {
        setting.setTransportMode(NetworkManager::InfinibandSetting::Connected);
    }
    setting.setMacAddress(NetworkManager::macAddressFromString(m_ui->macAddress->hwAddress()));
    if (m_ui->mtu->value()) {
        setting.setMtu(m_ui->mtu->value());
    }

    return setting.toMap();
}

bool InfinibandWidget::isValid() const
{
    return m_ui->macAddress->isValid();
}

#include "moc_infinibandwidget.cpp"
