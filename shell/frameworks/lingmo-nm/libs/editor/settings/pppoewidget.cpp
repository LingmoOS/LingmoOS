/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pppoewidget.h"
#include "ui_pppoe.h"

#include <NetworkManagerQt/PppoeSetting>

PppoeWidget::PppoeWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::PppoeWidget)
{
    m_ui->setupUi(this);

    KAcceleratorManager::manage(this);

    m_ui->password->setPasswordOptionsEnabled(true);
    m_ui->password->setPasswordNotRequiredEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->service, &KLineEdit::textChanged, this, &PppoeWidget::slotWidgetChanged);
    connect(m_ui->username, &KLineEdit::textChanged, this, &PppoeWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::textChanged, this, &PppoeWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::passwordOptionChanged, this, &PppoeWidget::slotWidgetChanged);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

PppoeWidget::~PppoeWidget()
{
    delete m_ui;
}

void PppoeWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::PppoeSetting::Ptr pppoeSetting = setting.staticCast<NetworkManager::PppoeSetting>();

    m_ui->service->setText(pppoeSetting->service());
    m_ui->username->setText(pppoeSetting->username());
    if (pppoeSetting->passwordFlags().testFlag(NetworkManager::Setting::None)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (pppoeSetting->passwordFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForUser);
    } else if (pppoeSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->password->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->password->setPasswordOption(PasswordField::NotRequired);
    }

    loadSecrets(setting);
}

void PppoeWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::PppoeSetting::Ptr pppoeSetting = setting.staticCast<NetworkManager::PppoeSetting>();

    if (pppoeSetting) {
        const QString password = pppoeSetting->password();
        if (!password.isEmpty()) {
            m_ui->password->setText(password);
        }
    }
}

QVariantMap PppoeWidget::setting() const
{
    NetworkManager::PppoeSetting pppoeSetting;
    if (!m_ui->service->text().isEmpty()) {
        pppoeSetting.setService(m_ui->service->text());
    }

    if (!m_ui->username->text().isEmpty()) {
        pppoeSetting.setUsername(m_ui->username->text());
    }

    if (!m_ui->password->text().isEmpty()) {
        pppoeSetting.setPassword(m_ui->password->text());
    }

    if (m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        pppoeSetting.setPasswordFlags(NetworkManager::Setting::None);
    } else if (m_ui->password->passwordOption() == PasswordField::StoreForUser) {
        pppoeSetting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        pppoeSetting.setPasswordFlags(NetworkManager::Setting::NotSaved);
    } else {
        pppoeSetting.setPasswordFlags(NetworkManager::Setting::NotRequired);
    }

    return pppoeSetting.toMap();
}

bool PppoeWidget::isValid() const
{
    bool passwordUserValid = true;

    if (m_ui->password->passwordOption() == PasswordField::StoreForUser //
        || m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        passwordUserValid = !m_ui->username->text().isEmpty() && !m_ui->password->text().isEmpty();
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        passwordUserValid = !m_ui->username->text().isEmpty();
    }

    return passwordUserValid;
}

#include "moc_pppoewidget.cpp"
