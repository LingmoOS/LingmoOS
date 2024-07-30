/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wiredsecurity.h"
#include "ui_wiredsecurity.h"

WiredSecurity::WiredSecurity(const NetworkManager::Security8021xSetting::Ptr &setting8021x, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting8021x, parent, f)
    , m_ui(new Ui::WiredSecurity)
    , m_8021xSetting(setting8021x)
{
    m_ui->setupUi(this);

    m_8021xWidget = new Security8021x(m_8021xSetting, Security8021x::Ethernet, this);
    m_8021xWidget->setDisabled(true);

    m_ui->verticalLayout->addWidget(m_8021xWidget);

    connect(m_ui->use8021X, &QCheckBox::toggled, m_8021xWidget, &Security8021x::setEnabled);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);

    loadConfig(setting8021x);
}

WiredSecurity::~WiredSecurity()
{
    delete m_ui;
}

bool WiredSecurity::enabled8021x() const
{
    if (m_ui->use8021X->checkState() == Qt::Checked) {
        return true;
    }

    return false;
}

void WiredSecurity::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    if (!setting->isNull()) {
        m_8021xWidget->setEnabled(true);
        m_ui->use8021X->setChecked(true);
    }
}

void WiredSecurity::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::Security8021xSetting::Ptr securitySetting = setting.staticCast<NetworkManager::Security8021xSetting>();

    if (securitySetting) {
        m_8021xWidget->loadSecrets(securitySetting);
    }
}

QVariantMap WiredSecurity::setting() const
{
    if (m_ui->use8021X->isChecked()) {
        return m_8021xWidget->setting();
    }

    return {};
}

#include "moc_wiredsecurity.cpp"
