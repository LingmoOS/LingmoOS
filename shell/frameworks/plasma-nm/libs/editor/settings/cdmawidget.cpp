/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cdmawidget.h"
#include "ui_cdma.h"

#include <NetworkManagerQt/CdmaSetting>

CdmaWidget::CdmaWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::CdmaWidget)
{
    m_ui->setupUi(this);

    m_ui->password->setPasswordOptionsEnabled(true);
    m_ui->password->setPasswordNotRequiredEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->number, &KLineEdit::textChanged, this, &CdmaWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::textChanged, this, &CdmaWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::passwordOptionChanged, this, &CdmaWidget::slotWidgetChanged);
    connect(m_ui->username, &KLineEdit::textChanged, this, &CdmaWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

CdmaWidget::~CdmaWidget()
{
    delete m_ui;
}

void CdmaWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::CdmaSetting::Ptr cdmaSetting = setting.staticCast<NetworkManager::CdmaSetting>();
    const QString number = cdmaSetting->number();

    if (!number.isEmpty()) {
        m_ui->number->setText(number);
    }

    m_ui->username->setText(cdmaSetting->username());
    if (cdmaSetting->passwordFlags().testFlag(NetworkManager::Setting::None)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (cdmaSetting->passwordFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForUser);
    } else if (cdmaSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->password->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->password->setPasswordOption(PasswordField::NotRequired);
    }

    loadSecrets(setting);
}

void CdmaWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::CdmaSetting::Ptr cdmaSetting = setting.staticCast<NetworkManager::CdmaSetting>();

    if (cdmaSetting) {
        const QString password = cdmaSetting->password();
        if (!password.isEmpty()) {
            m_ui->password->setText(password);
        }
    }
}

QVariantMap CdmaWidget::setting() const
{
    NetworkManager::CdmaSetting cdmaSetting;
    if (!m_ui->number->text().isEmpty()) {
        cdmaSetting.setNumber(m_ui->number->text());
    }

    if (!m_ui->username->text().isEmpty()) {
        cdmaSetting.setUsername(m_ui->username->text());
    }

    if (!m_ui->password->text().isEmpty()) {
        cdmaSetting.setPassword(m_ui->password->text());
    }

    if (m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        cdmaSetting.setPasswordFlags(NetworkManager::Setting::None);
    } else if (m_ui->password->passwordOption() == PasswordField::StoreForUser) {
        cdmaSetting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        cdmaSetting.setPasswordFlags(NetworkManager::Setting::NotSaved);
    } else {
        cdmaSetting.setPasswordFlags(NetworkManager::Setting::NotRequired);
    }

    return cdmaSetting.toMap();
}

bool CdmaWidget::isValid() const
{
    bool passwordUserValid = true;

    if (m_ui->password->passwordOption() == PasswordField::StoreForUser //
        || m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        passwordUserValid = !m_ui->username->text().isEmpty() && !m_ui->password->text().isEmpty();
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        passwordUserValid = !m_ui->username->text().isEmpty();
    }

    return !m_ui->number->text().isEmpty() && passwordUserValid;
}

#include "moc_cdmawidget.cpp"
