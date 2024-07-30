/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vlanwidget.h"
#include "ui_vlan.h"
#include "uiutils.h"

#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/VlanSetting>

VlanWidget::VlanWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::VlanWidget)
{
    m_ui->setupUi(this);

    fillConnections();

    connect(m_ui->ifaceName, &KLineEdit::textChanged, this, &VlanWidget::slotWidgetChanged);
    connect(m_ui->parent, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &VlanWidget::slotWidgetChanged);
    connect(m_ui->parent->lineEdit(), &QLineEdit::textChanged, this, &VlanWidget::slotWidgetChanged);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

VlanWidget::~VlanWidget()
{
    delete m_ui;
}

void VlanWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::VlanSetting::Ptr vlanSetting = setting.staticCast<NetworkManager::VlanSetting>();

    m_ui->parent->setCurrentIndex(m_ui->parent->findData(vlanSetting->parent()));

    m_ui->id->setValue(vlanSetting->id());
    m_ui->ifaceName->setText(vlanSetting->interfaceName());

    m_ui->reorderHeaders->setChecked(vlanSetting->flags().testFlag(NetworkManager::VlanSetting::ReorderHeaders));
    m_ui->gvrp->setChecked(vlanSetting->flags().testFlag(NetworkManager::VlanSetting::Gvrp));
    m_ui->looseBinding->setChecked(vlanSetting->flags().testFlag(NetworkManager::VlanSetting::LooseBinding));
}

QVariantMap VlanWidget::setting() const
{
    NetworkManager::VlanSetting setting;

    setting.setParent(m_ui->parent->itemData(m_ui->parent->currentIndex()).toString());
    setting.setId(m_ui->id->value());

    const QString ifaceName = m_ui->ifaceName->text();
    if (!ifaceName.isEmpty())
        setting.setInterfaceName(ifaceName);

    NetworkManager::VlanSetting::Flags flags;
    if (m_ui->reorderHeaders->isChecked())
        flags |= NetworkManager::VlanSetting::ReorderHeaders;
    if (m_ui->gvrp->isChecked())
        flags |= NetworkManager::VlanSetting::Gvrp;
    if (m_ui->looseBinding->isChecked())
        flags |= NetworkManager::VlanSetting::LooseBinding;
    if (flags)
        setting.setFlags(flags);

    return setting.toMap();
}

void VlanWidget::fillConnections()
{
    m_ui->parent->clear();

    for (const NetworkManager::Connection::Ptr &con : NetworkManager::listConnections()) {
        if (!con->settings()->isSlave() && con->settings()->connectionType() == NetworkManager::ConnectionSettings::Wired)
            m_ui->parent->addItem(con->name(), con->uuid());
    }
}

bool VlanWidget::isValid() const
{
    return !m_ui->parent->currentText().isEmpty() || !m_ui->ifaceName->text().isEmpty();
}

#include "moc_vlanwidget.cpp"
