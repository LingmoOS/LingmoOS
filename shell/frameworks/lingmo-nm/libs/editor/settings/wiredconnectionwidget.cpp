/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wiredconnectionwidget.h"
#include "ui_wiredconnectionwidget.h"

#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WiredSetting>

#include <QRandomGenerator>

WiredConnectionWidget::WiredConnectionWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_widget(new Ui::WiredConnectionWidget)
{
    m_widget->setupUi(this);

    connect(m_widget->btnRandomMacAddr, &QPushButton::clicked, this, &WiredConnectionWidget::generateRandomClonedMac);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_widget->clonedMacAddress, &KLineEdit::textChanged, this, &WiredConnectionWidget::slotWidgetChanged);
    connect(m_widget->macAddress, &HwAddrComboBox::hwAddressChanged, this, &WiredConnectionWidget::slotWidgetChanged);
    connect(m_widget->linkNegotiation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_widget->duplex->setEnabled(index == LinkNegotiation::Manual);
        m_widget->speed->setEnabled(index == LinkNegotiation::Manual);
    });

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

WiredConnectionWidget::~WiredConnectionWidget()
{
    delete m_widget;
}

void WiredConnectionWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::WiredSetting::Ptr wiredSetting = setting.staticCast<NetworkManager::WiredSetting>();

    m_widget->macAddress->init(NetworkManager::Device::Ethernet, NetworkManager::macAddressAsString(wiredSetting->macAddress()));

    if (!wiredSetting->clonedMacAddress().isEmpty()) {
        m_widget->clonedMacAddress->setText(NetworkManager::macAddressAsString(wiredSetting->clonedMacAddress()));
    }

    if (wiredSetting->mtu()) {
        m_widget->mtu->setValue(wiredSetting->mtu());
    }

    if (wiredSetting->autoNegotiate()) {
        m_widget->linkNegotiation->setCurrentIndex(LinkNegotiation::Automatic);
    } else if (wiredSetting->speed() && wiredSetting->duplexType() != NetworkManager::WiredSetting::UnknownDuplexType) {
        m_widget->linkNegotiation->setCurrentIndex(LinkNegotiation::Manual);
    }

    if (wiredSetting->speed()) {
        switch (wiredSetting->speed()) {
        case 10:
            m_widget->speed->setCurrentIndex(0);
            break;
        case 100:
            m_widget->speed->setCurrentIndex(1);
            break;
        case 1000:
            m_widget->speed->setCurrentIndex(2);
            break;
        case 2500:
            m_widget->speed->setCurrentIndex(3);
            break;
        case 5000:
            m_widget->speed->setCurrentIndex(4);
            break;
        case 10000:
            m_widget->speed->setCurrentIndex(5);
            break;
        case 40000:
            m_widget->speed->setCurrentIndex(6);
            break;
        case 100000:
            m_widget->speed->setCurrentIndex(7);
            break;
        }
    }

    if (wiredSetting->duplexType() != NetworkManager::WiredSetting::Half) {
        m_widget->duplex->setCurrentIndex(Duplex::Full);
    } else {
        m_widget->duplex->setCurrentIndex(Duplex::Half);
    }
}

QVariantMap WiredConnectionWidget::setting() const
{
    NetworkManager::WiredSetting wiredSetting;

    wiredSetting.setMacAddress(NetworkManager::macAddressFromString(m_widget->macAddress->hwAddress()));

    if (!m_widget->clonedMacAddress->text().isEmpty() && m_widget->clonedMacAddress->text() != QLatin1String(":::::")) {
        wiredSetting.setClonedMacAddress(NetworkManager::macAddressFromString(m_widget->clonedMacAddress->text()));
    }

    if (m_widget->mtu->value()) {
        wiredSetting.setMtu(m_widget->mtu->value());
    }

    if (m_widget->linkNegotiation->currentIndex() == LinkNegotiation::Automatic || m_widget->linkNegotiation->currentIndex() == LinkNegotiation::Ignore) {
        wiredSetting.setDuplexType(NetworkManager::WiredSetting::UnknownDuplexType);
        wiredSetting.setSpeed(0);
    } else {
        switch (m_widget->speed->currentIndex()) {
        case 0:
            wiredSetting.setSpeed(10);
            break;
        case 1:
            wiredSetting.setSpeed(100);
            break;
        case 2:
            wiredSetting.setSpeed(1000);
            break;
        case 3:
            wiredSetting.setSpeed(2500);
            break;
        case 4:
            wiredSetting.setSpeed(5000);
            break;
        case 5:
            wiredSetting.setSpeed(10000);
            break;
        case 6:
            wiredSetting.setSpeed(40000);
            break;
        case 7:
            wiredSetting.setSpeed(100000);
            break;
        }

        if (m_widget->duplex->currentIndex() == Duplex::Full) {
            wiredSetting.setDuplexType(NetworkManager::WiredSetting::Full);
        } else {
            wiredSetting.setDuplexType(NetworkManager::WiredSetting::Half);
        }
    }

    wiredSetting.setAutoNegotiate(m_widget->linkNegotiation->currentIndex() == LinkNegotiation::Automatic);

    return wiredSetting.toMap();
}

void WiredConnectionWidget::generateRandomClonedMac()
{
    auto generator = QRandomGenerator::global();
    QByteArray mac;
    mac.resize(6);
    for (int i = 0; i < 6; i++) {
        const int random = generator->bounded(255);
        mac[i] = random;
    }

    // Disable the multicast bit and enable the locally administered bit.
    mac[0] = mac[0] & ~0x1;
    mac[0] = mac[0] | 0x2;

    m_widget->clonedMacAddress->setText(NetworkManager::macAddressAsString(mac));
}

bool WiredConnectionWidget::isValid() const
{
    if (!m_widget->macAddress->isValid()) {
        return false;
    }

    if (m_widget->clonedMacAddress->text() != QLatin1String(":::::")) {
        if (!NetworkManager::macAddressIsValid(m_widget->clonedMacAddress->text())) {
            return false;
        }
    }

    return true;
}

#include "moc_wiredconnectionwidget.cpp"
