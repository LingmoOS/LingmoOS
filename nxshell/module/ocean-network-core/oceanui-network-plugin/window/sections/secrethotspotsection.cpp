// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "secrethotspotsection.h"

#include <widgets/comboxwidget.h>
#include <widgets/lineeditwidget.h>

#include <networkmanagerqt/utils.h>

#include <QComboBox>
#include <DLineEdit>

using namespace DCC_NAMESPACE;
using namespace NetworkManager;

static const QList<WirelessSecuritySetting::KeyMgmt> KeyMgmtList {
    WirelessSecuritySetting::KeyMgmt::WpaNone,
    WirelessSecuritySetting::KeyMgmt::WpaPsk,
    WirelessSecuritySetting::KeyMgmt::SAE
};

SecretHotspotSection::SecretHotspotSection(WirelessSecuritySetting::Ptr wsSeting, QFrame *parent)
    : AbstractSection(tr("Security"), parent)
    , m_keyMgmtChooser(new ComboxWidget(this))
    , m_passwdEdit(new LineEditWidget(true))
    , m_currentKeyMgmt(WirelessSecuritySetting::KeyMgmt::WpaNone)
    , m_wsSetting(wsSeting)
{
    initStrMaps();

    // init KeyMgmt
    const WirelessSecuritySetting::KeyMgmt &keyMgmt = m_wsSetting->keyMgmt();
    m_currentKeyMgmt = (keyMgmt == WirelessSecuritySetting::KeyMgmt::Unknown) ?
                       WirelessSecuritySetting::KeyMgmt::WpaNone : keyMgmt;

    initUI();
    initConnection();

    onKeyMgmtChanged(m_currentKeyMgmt);
}

SecretHotspotSection::~SecretHotspotSection()
{
    delete m_passwdEdit;
}

bool SecretHotspotSection::allInputValid()
{
    bool valid = true;

    switch (m_currentKeyMgmt) {
    case WirelessSecuritySetting::KeyMgmt::Wep: {
        valid = wepKeyIsValid(m_passwdEdit->text(), WirelessSecuritySetting::WepKeyType::Passphrase);
        m_passwdEdit->setIsErr(!valid);
        if (!valid && !m_passwdEdit->text().isEmpty())
            m_passwdEdit->dTextEdit()->showAlertMessage(tr("Invalid password"), this);
        break;
    }
    case WirelessSecuritySetting::KeyMgmt::WpaPsk:
    case WirelessSecuritySetting::KeyMgmt::SAE: {
        valid = wpaPskIsValid(m_passwdEdit->text());
        m_passwdEdit->setIsErr(!valid);
        if (!valid && m_passwdEdit->text().length() < 8) {
            m_passwdEdit->dTextEdit()->showAlertMessage(tr("Please enter a password of at least eight digits"), this);
        } else if (!valid) {
            m_passwdEdit->dTextEdit()->showAlertMessage(tr("Invalid password"), this);
        }
        break;
    }
    default: break;
    }

    return valid;
}

void SecretHotspotSection::saveSettings()
{
    if (m_currentKeyMgmt == WirelessSecuritySetting::KeyMgmt::WpaNone) {
        m_wsSetting->setInitialized(false);
        return;
    }

    m_wsSetting->setKeyMgmt(m_currentKeyMgmt);

    switch (m_currentKeyMgmt) {
    case WirelessSecuritySetting::KeyMgmt::Wep: {
        m_wsSetting->setAuthAlg(WirelessSecuritySetting::AuthAlg::Open);
        m_wsSetting->setWepKeyType(WirelessSecuritySetting::WepKeyType::Passphrase);
        m_wsSetting->setWepKey0(m_passwdEdit->text());
        m_wsSetting->setPsk("");
        break;
    }
    case WirelessSecuritySetting::KeyMgmt::WpaPsk: {
        m_wsSetting->setPsk(m_passwdEdit->text());
        m_wsSetting->setPskFlags(NetworkManager::Setting::AgentOwned);
        m_wsSetting->setProto(QList<NetworkManager::WirelessSecuritySetting::WpaProtocolVersion>{NetworkManager::WirelessSecuritySetting::Wpa,NetworkManager::WirelessSecuritySetting::Rsn});
        m_wsSetting->setGroup(QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities>{NetworkManager::WirelessSecuritySetting::Ccmp});
        m_wsSetting->setPairwise(QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities>{NetworkManager::WirelessSecuritySetting::Ccmp});
        m_wsSetting->setPmf(NetworkManager::WirelessSecuritySetting::Pmf::DisablePmf);
        break;
    }
    case WirelessSecuritySetting::KeyMgmt::SAE: {
        m_wsSetting->setPsk(m_passwdEdit->text());
        m_wsSetting->setPskFlags(NetworkManager::Setting::AgentOwned);
        m_wsSetting->setProto(QList<NetworkManager::WirelessSecuritySetting::WpaProtocolVersion>{NetworkManager::WirelessSecuritySetting::Rsn});
        m_wsSetting->setGroup(QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities>{NetworkManager::WirelessSecuritySetting::Ccmp});
        m_wsSetting->setPairwise(QList<NetworkManager::WirelessSecuritySetting::WpaEncryptionCapabilities>{NetworkManager::WirelessSecuritySetting::Ccmp});
        break;
    }
    default: break;
    }

    m_wsSetting->setInitialized(true);
}

void SecretHotspotSection::initStrMaps()
{
    KeyMgmtStrMap = {
        { tr("None"), WirelessSecuritySetting::KeyMgmt::WpaNone },
        { tr("WEP"), WirelessSecuritySetting::KeyMgmt::Wep },
        { tr("WPA/WPA2 Personal"), WirelessSecuritySetting::KeyMgmt::WpaPsk },
        { tr("WPA3 Personal"), WirelessSecuritySetting::KeyMgmt::SAE }
    };
}

void SecretHotspotSection::initUI()
{
    QComboBox *cb = m_keyMgmtChooser->comboBox();
    m_keyMgmtChooser->setTitle(tr("Security"));
    for (auto keyMgmt : KeyMgmtList)
        cb->addItem(KeyMgmtStrMap.key(keyMgmt), keyMgmt);

    cb->setCurrentIndex(cb->findData(m_currentKeyMgmt));

    m_passwdEdit->setPlaceholderText(tr("Required"));

    appendItem(m_keyMgmtChooser);
    appendItem(m_passwdEdit);

    m_passwdEdit->textEdit()->installEventFilter(this);
}

void SecretHotspotSection::initConnection()
{
    connect(m_keyMgmtChooser->comboBox(), static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [ this ] {
        onKeyMgmtChanged(this->m_keyMgmtChooser->comboBox()->currentData().value<WirelessSecuritySetting::KeyMgmt>());
    });

    connect(m_passwdEdit->textEdit(), &QLineEdit::editingFinished, this, &SecretHotspotSection::saveUserInputPassword);
    connect(m_keyMgmtChooser, &ComboxWidget::onIndexChanged, this, &SecretHotspotSection::editClicked);
}

void SecretHotspotSection::onKeyMgmtChanged(WirelessSecuritySetting::KeyMgmt keyMgmt)
{
    if (m_currentKeyMgmt != keyMgmt)
        m_currentKeyMgmt = keyMgmt;

    switch (m_currentKeyMgmt) {
    case WirelessSecuritySetting::KeyMgmt::WpaNone: {
        m_passwdEdit->setVisible(false);
        break;
    }

    case WirelessSecuritySetting::KeyMgmt::Wep: {
        m_passwdEdit->setText(m_wsSetting->wepKey0());
        m_passwdEdit->setTitle(tr("Key"));
        m_passwdEdit->setVisible(true);
        break;
    }

    case WirelessSecuritySetting::KeyMgmt::WpaPsk:
    case WirelessSecuritySetting::KeyMgmt::SAE: {
        m_passwdEdit->setText(m_wsSetting->psk());
        m_passwdEdit->setTitle(tr("Password"));
        m_passwdEdit->setVisible(true);
        break;
    }

    default:
        break;
    }

    if (m_userInputPasswordMap.contains(m_currentKeyMgmt))
        m_passwdEdit->setText(m_userInputPasswordMap.value(m_currentKeyMgmt));
}

void SecretHotspotSection::saveUserInputPassword()
{
    m_userInputPasswordMap.insert(m_currentKeyMgmt, m_passwdEdit->text());
}

bool SecretHotspotSection::eventFilter(QObject *watched, QEvent *event)
{
    // 实现鼠标点击编辑框，确定按钮激活，统一网络模块处理，捕捉FocusIn消息
    if (event->type() == QEvent::FocusIn) {
        if (dynamic_cast<QLineEdit *>(watched))
            Q_EMIT editClicked();
    }

    return QWidget::eventFilter(watched, event);
}
