/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "btwidget.h"
#include "ui_bt.h"

#include <KLocalizedString>

#include <NetworkManagerQt/Utils>

BtWidget::BtWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::BtWidget)
{
    m_ui->setupUi(this);

    m_ui->type->addItem(i18n("DUN (dial up networking)"), NetworkManager::BluetoothSetting::Dun);
    m_ui->type->addItem(i18n("PAN (personal area network)"), NetworkManager::BluetoothSetting::Panu);

    m_ui->type->setEnabled(false);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->bdaddr, &HwAddrComboBox::hwAddressChanged, this, &BtWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

BtWidget::~BtWidget()
{
    delete m_ui;
}

void BtWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::BluetoothSetting::Ptr btSetting = setting.staticCast<NetworkManager::BluetoothSetting>();

    m_ui->bdaddr->init(NetworkManager::Device::Bluetooth, NetworkManager::macAddressAsString(btSetting->bluetoothAddress()));
    m_ui->type->setCurrentIndex(m_ui->type->findData(btSetting->profileType()));
}

QVariantMap BtWidget::setting() const
{
    NetworkManager::BluetoothSetting btSetting;

    btSetting.setBluetoothAddress(NetworkManager::macAddressFromString(m_ui->bdaddr->hwAddress()));
    btSetting.setProfileType(static_cast<NetworkManager::BluetoothSetting::ProfileType>(m_ui->type->itemData(m_ui->type->currentIndex()).toInt()));

    return btSetting.toMap();
}

bool BtWidget::isValid() const
{
    return m_ui->bdaddr->isValid();
}

#include "moc_btwidget.cpp"
