/*
    SPDX-FileCopyrightText: 2013, 2014 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "gsmwidget.h"
#include "ui_gsm.h"

#include <KLocalizedString>

#include <NetworkManagerQt/GsmSetting>

GsmWidget::GsmWidget(const NetworkManager::Setting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::GsmWidget)
{
    m_ui->setupUi(this);

    // Network ID not supported yet in NM
    m_ui->labelNetworkId->setHidden(true);
    m_ui->networkId->setHidden(true);

    m_ui->password->setPasswordOptionsEnabled(true);
    m_ui->password->setPasswordNotRequiredEnabled(true);
    m_ui->pin->setPasswordOptionsEnabled(true);
    m_ui->pin->setPasswordNotRequiredEnabled(true);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->apn, &KLineEdit::textChanged, this, &GsmWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::textChanged, this, &GsmWidget::slotWidgetChanged);
    connect(m_ui->password, &PasswordField::passwordOptionChanged, this, &GsmWidget::slotWidgetChanged);
    connect(m_ui->pin, &PasswordField::textChanged, this, &GsmWidget::slotWidgetChanged);
    connect(m_ui->pin, &PasswordField::passwordOptionChanged, this, &GsmWidget::slotWidgetChanged);
    connect(m_ui->username, &KLineEdit::textChanged, this, &GsmWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting) {
        loadConfig(setting);
    }
}

GsmWidget::~GsmWidget()
{
    delete m_ui;
}

void GsmWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::GsmSetting::Ptr gsmSetting = setting.staticCast<NetworkManager::GsmSetting>();

    const QString number = gsmSetting->number();
    if (!number.isEmpty())
        m_ui->number->setText(number);
    m_ui->username->setText(gsmSetting->username());
    if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::None)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->password->setPasswordOption(PasswordField::StoreForUser);
    } else if (gsmSetting->passwordFlags().testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->password->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->password->setPasswordOption(PasswordField::NotRequired);
    }

    m_ui->apn->setText(gsmSetting->apn());
    m_ui->networkId->setText(gsmSetting->networkId());
    m_ui->roaming->setChecked(!gsmSetting->homeOnly());
    if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::None)) {
        m_ui->pin->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
        m_ui->pin->setPasswordOption(PasswordField::StoreForUser);
    } else if (gsmSetting->pinFlags().testFlag(NetworkManager::Setting::NotSaved)) {
        m_ui->pin->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        m_ui->pin->setPasswordOption(PasswordField::NotRequired);
    }

    loadSecrets(setting);
}

void GsmWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::GsmSetting::Ptr gsmSetting = setting.staticCast<NetworkManager::GsmSetting>();

    if (gsmSetting) {
        const QString password = gsmSetting->password();
        if (!password.isEmpty()) {
            m_ui->password->setText(password);
        }

        const QString pin = gsmSetting->pin();
        if (!pin.isEmpty()) {
            m_ui->pin->setText(pin);
        }
    }
}

QVariantMap GsmWidget::setting() const
{
    NetworkManager::GsmSetting gsmSetting;
    if (!m_ui->number->text().isEmpty())
        gsmSetting.setNumber(m_ui->number->text());
    if (!m_ui->username->text().isEmpty())
        gsmSetting.setUsername(m_ui->username->text());
    if (!m_ui->password->text().isEmpty())
        gsmSetting.setPassword(m_ui->password->text());
    if (m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        gsmSetting.setPasswordFlags(NetworkManager::Setting::None);
    } else if (m_ui->password->passwordOption() == PasswordField::StoreForUser) {
        gsmSetting.setPasswordFlags(NetworkManager::Setting::AgentOwned);
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        gsmSetting.setPasswordFlags(NetworkManager::Setting::NotSaved);
    } else {
        gsmSetting.setPasswordFlags(NetworkManager::Setting::NotRequired);
    }

    if (!m_ui->apn->text().isEmpty())
        gsmSetting.setApn(m_ui->apn->text());
    if (!m_ui->networkId->text().isEmpty())
        gsmSetting.setNetworkId(m_ui->networkId->text());
    gsmSetting.setHomeOnly(!m_ui->roaming->isChecked());
    if (!m_ui->pin->text().isEmpty())
        gsmSetting.setPin(m_ui->pin->text());
    if (m_ui->pin->passwordOption() == PasswordField::StoreForAllUsers) {
        gsmSetting.setPinFlags(NetworkManager::Setting::None);
    } else if (m_ui->pin->passwordOption() == PasswordField::StoreForUser) {
        gsmSetting.setPinFlags(NetworkManager::Setting::AgentOwned);
    } else if (m_ui->pin->passwordOption() == PasswordField::AlwaysAsk) {
        gsmSetting.setPinFlags(NetworkManager::Setting::NotSaved);
    } else {
        gsmSetting.setPinFlags(NetworkManager::Setting::NotRequired);
    }

    return gsmSetting.toMap();
}

bool GsmWidget::isValid() const
{
    bool passwordUserValid = true;
    bool pinValid = true;

    if (m_ui->password->passwordOption() == PasswordField::StoreForUser //
        || m_ui->password->passwordOption() == PasswordField::StoreForAllUsers) {
        passwordUserValid = !m_ui->username->text().isEmpty() && !m_ui->password->text().isEmpty();
    } else if (m_ui->password->passwordOption() == PasswordField::AlwaysAsk) {
        passwordUserValid = !m_ui->username->text().isEmpty();
    }

    if (m_ui->pin->passwordOption() == PasswordField::StoreForUser || m_ui->pin->passwordOption() == PasswordField::StoreForAllUsers) {
        pinValid = !m_ui->pin->text().isEmpty();
    }

    return !m_ui->apn->text().isEmpty() && passwordUserValid && pinValid;
}

#include "moc_gsmwidget.cpp"
