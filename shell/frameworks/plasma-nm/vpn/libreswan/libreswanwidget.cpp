/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "libreswanwidget.h"
#include "nm-libreswan-service.h"
#include "ui_libreswan.h"

#include <NetworkManagerQt/Setting>

#include <QDBusMetaType>

LibreswanWidget::LibreswanWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::LibreswanWidget)
    , m_setting(setting)
{
    qDBusRegisterMetaType<NMStringMap>();

    m_ui->setupUi(this);

    m_ui->groupPassword->setPasswordOptionsEnabled(true);
    m_ui->userPassword->setPasswordOptionsEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->gateway, &QLineEdit::textChanged, this, &LibreswanWidget::slotWidgetChanged);
    connect(m_ui->groupname, &QLineEdit::textChanged, this, &LibreswanWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

LibreswanWidget::~LibreswanWidget()
{
    delete m_ui;
}

void LibreswanWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);

    const NMStringMap data = m_setting->data();

    const QString gateway = data.value(NM_LIBRESWAN_RIGHT);
    if (!gateway.isEmpty()) {
        m_ui->gateway->setText(gateway);
    }

    const QString groupName = data.value(NM_LIBRESWAN_LEFTID);
    if (!groupName.isEmpty()) {
        m_ui->groupname->setText(groupName);
    }

    const NetworkManager::Setting::SecretFlags groupPasswordFlag =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_LIBRESWAN_PSK_VALUE "-flags").toInt());
    if (groupPasswordFlag == NetworkManager::Setting::None) {
        m_ui->groupPassword->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (groupPasswordFlag == NetworkManager::Setting::AgentOwned) {
        m_ui->groupPassword->setPasswordOption(PasswordField::StoreForUser);
    } else {
        m_ui->groupPassword->setPasswordOption(PasswordField::AlwaysAsk);
    }

    const NetworkManager::Setting::SecretFlags userPasswordFlag =
        static_cast<NetworkManager::Setting::SecretFlags>(data.value(NM_LIBRESWAN_XAUTH_PASSWORD "-flags").toInt());
    if (userPasswordFlag == NetworkManager::Setting::None) {
        m_ui->userPassword->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (userPasswordFlag == NetworkManager::Setting::AgentOwned) {
        m_ui->userPassword->setPasswordOption(PasswordField::StoreForUser);
    } else {
        m_ui->userPassword->setPasswordOption(PasswordField::AlwaysAsk);
    }

    const QString username = data.value(NM_LIBRESWAN_LEFTXAUTHUSER);
    if (!username.isEmpty()) {
        m_ui->username->setText(username);
    }

    const QString phase1 = data.value(NM_LIBRESWAN_IKE);
    if (!phase1.isEmpty()) {
        m_ui->phase1->setText(phase1);
    }

    const QString phase2 = data.value(NM_LIBRESWAN_ESP);
    if (!phase2.isEmpty()) {
        m_ui->phase2->setText(phase2);
    }

    const QString domain = data.value(NM_LIBRESWAN_DOMAIN);
    if (!domain.isEmpty()) {
        m_ui->domain->setText(domain);
    }

    loadSecrets(setting);
}

void LibreswanWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();

        const QString userPassword = secrets.value(NM_LIBRESWAN_XAUTH_PASSWORD);
        if (!userPassword.isEmpty()) {
            m_ui->userPassword->setText(userPassword);
        }

        const QString groupPassword = secrets.value(NM_LIBRESWAN_PSK_VALUE);
        if (!groupPassword.isEmpty()) {
            m_ui->groupPassword->setText(groupPassword);
        }
    }
}

QVariantMap LibreswanWidget::setting() const
{
    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_LIBRESWAN));
    NMStringMap data;
    NMStringMap secrets;

    if (!m_ui->gateway->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_RIGHT, m_ui->gateway->text());
    }

    if (!m_ui->groupname->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_LEFTID, m_ui->groupname->text());
    }

    if (!m_ui->userPassword->text().isEmpty()) {
        secrets.insert(NM_LIBRESWAN_XAUTH_PASSWORD, m_ui->userPassword->text());
    }

    if (m_ui->userPassword->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_LIBRESWAN_XAUTH_PASSWORD_INPUT_MODES, NM_LIBRESWAN_PW_TYPE_SAVE);
        data.insert(NM_LIBRESWAN_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::None));
    } else if (m_ui->userPassword->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_LIBRESWAN_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else {
        data.insert(NM_LIBRESWAN_XAUTH_PASSWORD_INPUT_MODES, NM_LIBRESWAN_PW_TYPE_ASK);
        data.insert(NM_LIBRESWAN_XAUTH_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotSaved));
    }

    if (!m_ui->groupPassword->text().isEmpty()) {
        secrets.insert(NM_LIBRESWAN_PSK_VALUE, m_ui->groupPassword->text());
    }

    if (m_ui->groupPassword->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_LIBRESWAN_PSK_INPUT_MODES, NM_LIBRESWAN_PW_TYPE_SAVE);
        data.insert(NM_LIBRESWAN_PSK_VALUE "-flags", QString::number(NetworkManager::Setting::None));
    } else if (m_ui->groupPassword->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_LIBRESWAN_PSK_VALUE "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else {
        data.insert(NM_LIBRESWAN_PSK_INPUT_MODES, NM_LIBRESWAN_PW_TYPE_ASK);
        data.insert(NM_LIBRESWAN_PSK_VALUE "-flags", QString::number(NetworkManager::Setting::NotSaved));
    }

    if (!m_ui->username->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_LEFTXAUTHUSER, m_ui->username->text());
    }

    if (!m_ui->phase1->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_IKE, m_ui->phase1->text());
    }

    if (!m_ui->phase2->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_ESP, m_ui->phase2->text());
    }

    if (!m_ui->domain->text().isEmpty()) {
        data.insert(NM_LIBRESWAN_DOMAIN, m_ui->domain->text());
    }

    setting.setData(data);
    setting.setSecrets(secrets);
    return setting.toMap();
}

bool LibreswanWidget::isValid() const
{
    return !m_ui->gateway->text().isEmpty();
}

#include "moc_libreswanwidget.cpp"
