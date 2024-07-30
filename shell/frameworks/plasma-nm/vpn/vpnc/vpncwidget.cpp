/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vpncwidget.h"
#include "nm-vpnc-service.h"
#include "ui_vpnc.h"

#include <QUrl>

#include <QDBusMetaType>

VpncWidget::VpncWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::VpncWidget)
    , m_setting(setting)
{
    qDBusRegisterMetaType<NMStringMap>();

    m_ui->setupUi(this);

    m_ui->groupPassword->setPasswordOptionsEnabled(true);
    m_ui->groupPassword->setPasswordNotRequiredEnabled(true);
    m_ui->userPassword->setPasswordOptionsEnabled(true);
    m_ui->userPassword->setPasswordNotRequiredEnabled(true);

    connect(m_ui->btnAdvanced, &QPushButton::clicked, this, &VpncWidget::showAdvanced);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->gateway, &QLineEdit::textChanged, this, &VpncWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    m_advancedWidget = new VpncAdvancedWidget(m_setting, this);
    NMStringMap advData = m_advancedWidget->setting();
    if (!advData.isEmpty()) {
        if (m_tmpSetting.isNull()) {
            m_tmpSetting = NetworkManager::VpnSetting::Ptr(new NetworkManager::VpnSetting);
        }
        m_tmpSetting->setData(advData);
    }

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

VpncWidget::~VpncWidget()
{
    m_tmpSetting.clear();
    m_advancedWidget->deleteLater();
    delete m_ui;
}

void VpncWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);

    const NMStringMap data = m_setting->data();

    const QString gateway = data.value(NM_VPNC_KEY_GATEWAY);
    if (!gateway.isEmpty()) {
        m_ui->gateway->setText(gateway);
    }

    const QString user = data.value(NM_VPNC_KEY_XAUTH_USER);
    if (!user.isEmpty()) {
        m_ui->user->setText(user);
    }

    const NetworkManager::Setting::SecretFlags userPassType =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_VPNC_KEY_XAUTH_PASSWORD "-flags").toInt());
    if (userPassType.testFlag(NetworkManager::Setting::None)) {
        m_ui->userPassword->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (userPassType.testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->userPassword->setPasswordOption(PasswordField::StoreForUser);
    } else if (userPassType.testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->userPassword->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->userPassword->setPasswordOption(PasswordField::NotRequired);
    }

    const QString groupName = data.value(NM_VPNC_KEY_ID);
    if (!groupName.isEmpty()) {
        m_ui->group->setText(groupName);
    }

    const NetworkManager::Setting::SecretFlags groupPassType =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_VPNC_KEY_SECRET "-flags").toInt());
    if (groupPassType.testFlag(NetworkManager::Setting::None)) {
        m_ui->groupPassword->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (groupPassType.testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->groupPassword->setPasswordOption(PasswordField::StoreForUser);
    } else if (groupPassType.testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->groupPassword->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->groupPassword->setPasswordOption(PasswordField::NotRequired);
    }

    if (data.value(NM_VPNC_KEY_AUTHMODE) == QLatin1String("hybrid")) {
        m_ui->useHybridAuth->setChecked(true);
        m_ui->caFile->setUrl(QUrl::fromLocalFile(data.value(NM_VPNC_KEY_CA_FILE)));
    }

    loadSecrets(setting);
}

void VpncWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();

        const QString userPassword = secrets.value(NM_VPNC_KEY_XAUTH_PASSWORD);
        if (!userPassword.isEmpty()) {
            m_ui->userPassword->setText(userPassword);
        }

        const QString groupPassword = secrets.value(NM_VPNC_KEY_SECRET);
        if (!groupPassword.isEmpty()) {
            m_ui->groupPassword->setText(groupPassword);
        }
    }
}

QVariantMap VpncWidget::setting() const
{
    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_VPNC));
    NMStringMap data;
    NMStringMap secrets;

    if (!m_tmpSetting.isNull()) {
        data = m_tmpSetting->data();
    }

    if (!m_ui->gateway->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_GATEWAY, m_ui->gateway->text());
    }

    if (!m_ui->user->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_XAUTH_USER, m_ui->user->text());
    }

    if (m_ui->userPassword->isEnabled() && !m_ui->userPassword->text().isEmpty()) {
        secrets.insert(NM_VPNC_KEY_XAUTH_PASSWORD, m_ui->userPassword->text());
    }

    if (m_ui->userPassword->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_VPNC_KEY_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::None));
    } else if (m_ui->userPassword->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_VPNC_KEY_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else if (m_ui->userPassword->passwordOption() == PasswordField::AlwaysAsk) { // SettingWidget::EnumPasswordStorageType::Store
        data.insert(NM_VPNC_KEY_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotSaved));
    } else {
        data.insert(NM_VPNC_KEY_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotRequired));
    }

    if (!m_ui->group->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_ID, m_ui->group->text());
    }

    if (m_ui->groupPassword->isEnabled() && !m_ui->groupPassword->text().isEmpty()) {
        secrets.insert(NM_VPNC_KEY_SECRET, m_ui->groupPassword->text());
    }

    if (m_ui->groupPassword->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_VPNC_KEY_SECRET "-flags", QString::number(NetworkManager::Setting::None));
    } else if (m_ui->groupPassword->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_VPNC_KEY_SECRET "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else if (m_ui->groupPassword->passwordOption() == PasswordField::AlwaysAsk) { // SettingWidget::EnumPasswordStorageType::Store
        data.insert(NM_VPNC_KEY_SECRET "-flags", QString::number(NetworkManager::Setting::NotSaved));
    } else {
        data.insert(NM_VPNC_KEY_SECRET "-flags", QString::number(NetworkManager::Setting::NotRequired));
    }

    if (m_ui->useHybridAuth->isChecked() && m_ui->caFile->url().isValid()) {
        data.insert(NM_VPNC_KEY_AUTHMODE, "hybrid");
        data.insert(NM_VPNC_KEY_CA_FILE, m_ui->caFile->url().toLocalFile());
    }

    setting.setData(data);
    setting.setSecrets(secrets);
    return setting.toMap();
}

void VpncWidget::userPasswordTypeChanged(int index)
{
    m_ui->userPassword->setEnabled(index == SettingWidget::EnumPasswordStorageType::Store);
}

void VpncWidget::groupPasswordTypeChanged(int index)
{
    m_ui->groupPassword->setEnabled(index == SettingWidget::EnumPasswordStorageType::Store);
}

void VpncWidget::showAdvanced()
{
    m_advancedWidget->loadConfig(m_tmpSetting);
    connect(m_advancedWidget.data(), &VpncAdvancedWidget::accepted, [this]() {
        NMStringMap advData = m_advancedWidget->setting();
        if (!advData.isEmpty()) {
            m_tmpSetting->setData(advData);
        }
    });
    m_advancedWidget->setModal(true);
    m_advancedWidget->show();
}

bool VpncWidget::isValid() const
{
    return !m_ui->gateway->text().isEmpty();
}

#include "moc_vpncwidget.cpp"
