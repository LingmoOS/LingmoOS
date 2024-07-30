/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wifisecurity.h"
#include "ui_wifisecurity.h"

#include <NetworkManagerQt/Device>
#include <NetworkManagerQt/Manager>
#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/WirelessDevice>

WifiSecurity::WifiSecurity(const NetworkManager::Setting::Ptr &setting,
                           const NetworkManager::Security8021xSetting::Ptr &setting8021x,
                           QWidget *parent,
                           Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::WifiSecurity)
    , m_wifiSecurity(setting.staticCast<NetworkManager::WirelessSecuritySetting>())
{
    m_ui->setupUi(this);

    m_ui->leapPassword->setPasswordOptionsEnabled(true);
    m_ui->psk->setPasswordOptionsEnabled(true);
    m_ui->wepKey->setPasswordOptionsEnabled(true);

    m_8021xWidget = new Security8021x(setting8021x, Security8021x::WirelessWpaEap, this); // Dynamic WEP
    m_WPA2Widget = new Security8021x(setting8021x, Security8021x::WirelessWpaEap, this); // WPA(2) Enterprise
    m_WPA3SuiteB192Widget = new Security8021x(setting8021x, Security8021x::WirelessWpaEapSuiteB192, this); // WPA3 Enterprise Suite B 192
    m_ui->stackedWidget->insertWidget(3, m_8021xWidget);
    m_ui->stackedWidget->insertWidget(5, m_WPA2Widget);
    m_ui->stackedWidget->insertWidget(6, m_WPA3SuiteB192Widget);

    // WPA3 Enterprise is available in NM 1.30+
    if (!NetworkManager::checkVersion(1, 30, 0)) {
        m_ui->securityCombo->removeItem(8);
    }

    // WPA3 Personal is available in NM 1.16+
    if (!NetworkManager::checkVersion(1, 16, 0)) {
        m_ui->securityCombo->removeItem(7);
    }

    connect(m_ui->securityCombo, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WifiSecurity::securityChanged);
    connect(m_ui->wepIndex, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WifiSecurity::setWepKey);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->wepKey, &PasswordField::textChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->wepKey, &PasswordField::passwordOptionChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->leapUsername, &KLineEdit::textChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->leapPassword, &PasswordField::textChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->leapPassword, &PasswordField::passwordOptionChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->psk, &PasswordField::textChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->psk, &PasswordField::passwordOptionChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->wepIndex, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WifiSecurity::slotWidgetChanged);
    connect(m_ui->securityCombo, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WifiSecurity::slotWidgetChanged);
    connect(m_8021xWidget, &Security8021x::validChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_WPA2Widget, &Security8021x::validChanged, this, &WifiSecurity::slotWidgetChanged);
    connect(m_WPA3SuiteB192Widget, &Security8021x::validChanged, this, &WifiSecurity::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

WifiSecurity::~WifiSecurity()
{
    delete m_ui;
}

bool WifiSecurity::enabled() const
{
    return m_ui->securityCombo->currentIndex() > 0;
}

bool WifiSecurity::enabled8021x() const
{
    if (m_ui->securityCombo->currentIndex() == 4 || m_ui->securityCombo->currentIndex() == 6 || m_ui->securityCombo->currentIndex() == 8) {
        return true;
    }

    return false;
}

bool WifiSecurity::isValid() const
{
    const int securityIndex = m_ui->securityCombo->currentIndex();

    if (securityIndex == WepHex) { // WEP Hex
        return NetworkManager::wepKeyIsValid(m_ui->wepKey->text(), NetworkManager::WirelessSecuritySetting::Hex)
            || m_ui->wepKey->passwordOption() == PasswordField::AlwaysAsk;
    } else if (securityIndex == WepPassphrase) { // WEP Passphrase
        return NetworkManager::wepKeyIsValid(m_ui->wepKey->text(), NetworkManager::WirelessSecuritySetting::Passphrase)
            || m_ui->wepKey->passwordOption() == PasswordField::AlwaysAsk;
        ;
    } else if (securityIndex == Leap) { // LEAP
        return !m_ui->leapUsername->text().isEmpty()
            && (!m_ui->leapPassword->text().isEmpty() || m_ui->leapPassword->passwordOption() == PasswordField::AlwaysAsk);
    } else if (securityIndex == WpaPsk) { // WPA
        return NetworkManager::wpaPskIsValid(m_ui->psk->text()) || m_ui->psk->passwordOption() == PasswordField::AlwaysAsk;
    } else if (securityIndex == DynamicWep) {
        return m_8021xWidget->isValid();
    } else if (securityIndex == WpaEap) {
        return m_WPA2Widget->isValid();
    } else if (securityIndex == Wpa3SuiteB192) {
        return m_WPA3SuiteB192Widget->isValid();
    } else if (securityIndex == SAE) {
        return !m_ui->psk->text().isEmpty() || m_ui->psk->passwordOption() == PasswordField::AlwaysAsk;
    }

    return true;
}

void WifiSecurity::setStoreSecretsSystemWide(bool system)
{
    if (system) {
        m_ui->wepKey->setPasswordOption(PasswordField::StoreForAllUsers);
        m_ui->leapPassword->setPasswordOption(PasswordField::StoreForAllUsers);
        m_ui->psk->setPasswordOption(PasswordField::StoreForAllUsers);
        m_8021xWidget->setPasswordOption(PasswordField::StoreForAllUsers);
        m_WPA2Widget->setPasswordOption(PasswordField::StoreForAllUsers);
    } else {
        m_ui->wepKey->setPasswordOption(PasswordField::StoreForUser);
        m_ui->leapPassword->setPasswordOption(PasswordField::StoreForUser);
        m_ui->psk->setPasswordOption(PasswordField::StoreForUser);
        m_8021xWidget->setPasswordOption(PasswordField::StoreForUser);
        m_WPA2Widget->setPasswordOption(PasswordField::StoreForUser);
    }
}

void WifiSecurity::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity = setting.staticCast<NetworkManager::WirelessSecuritySetting>();

    const NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = wifiSecurity->keyMgmt();
    const NetworkManager::WirelessSecuritySetting::AuthAlg authAlg = wifiSecurity->authAlg();

    if (keyMgmt == NetworkManager::WirelessSecuritySetting::Unknown) {
        m_ui->securityCombo->setCurrentIndex(None); // None
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Wep) {
        if (wifiSecurity->wepKeyType() == NetworkManager::WirelessSecuritySetting::Hex
            || wifiSecurity->wepKeyType() == NetworkManager::WirelessSecuritySetting::NotSpecified) {
            m_ui->securityCombo->setCurrentIndex(WepHex); // WEP Hex
        } else {
            m_ui->securityCombo->setCurrentIndex(WepPassphrase);
        }
        const int keyIndex = static_cast<int>(wifiSecurity->wepTxKeyindex());
        m_ui->wepIndex->setCurrentIndex(keyIndex);

        if (wifiSecurity->authAlg() == NetworkManager::WirelessSecuritySetting::Open) {
            m_ui->wepAuth->setCurrentIndex(0);
        } else {
            m_ui->wepAuth->setCurrentIndex(1);
        }

        if (wifiSecurity->wepKeyFlags().testFlag(NetworkManager::Setting::None)) {
            m_ui->wepKey->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (wifiSecurity->wepKeyFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->wepKey->setPasswordOption(PasswordField::StoreForUser);
        } else {
            m_ui->wepKey->setPasswordOption(PasswordField::AlwaysAsk);
        }
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x && authAlg == NetworkManager::WirelessSecuritySetting::Leap) {
        m_ui->securityCombo->setCurrentIndex(Leap); // LEAP
        m_ui->leapUsername->setText(wifiSecurity->leapUsername());
        m_ui->leapPassword->setText(wifiSecurity->leapPassword());

        if (wifiSecurity->leapPasswordFlags().testFlag(NetworkManager::Setting::None)) {
            m_ui->leapPassword->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (wifiSecurity->leapPasswordFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->leapPassword->setPasswordOption(PasswordField::StoreForUser);
        } else {
            m_ui->leapPassword->setPasswordOption(PasswordField::AlwaysAsk);
        }
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x) {
        m_ui->securityCombo->setCurrentIndex(DynamicWep); // Dynamic WEP
        // done in the widget

    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaPsk) {
        m_ui->securityCombo->setCurrentIndex(WpaPsk); // WPA

        if (wifiSecurity->pskFlags().testFlag(NetworkManager::Setting::None)) {
            m_ui->psk->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (wifiSecurity->pskFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->psk->setPasswordOption(PasswordField::StoreForUser);
        } else {
            m_ui->psk->setPasswordOption(PasswordField::AlwaysAsk);
        }
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEap) {
        m_ui->securityCombo->setCurrentIndex(WpaEap); // WPA2 Enterprise
        // done in the widget
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::SAE) {
        m_ui->securityCombo->setCurrentIndex(SAE); // WPA3

        if (wifiSecurity->pskFlags().testFlag(NetworkManager::Setting::None)) {
            m_ui->psk->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (wifiSecurity->pskFlags().testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->psk->setPasswordOption(PasswordField::StoreForUser);
        } else {
            m_ui->psk->setPasswordOption(PasswordField::AlwaysAsk);
        }
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192) {
        m_ui->securityCombo->setCurrentIndex(Wpa3SuiteB192); // WPA3 Enterprise Suite B 192
        // done in the widget
    } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::OWE) {
        m_ui->securityCombo->setCurrentIndex(OWE); // Enhanced Open (OWE)
    }

    if (keyMgmt != NetworkManager::WirelessSecuritySetting::Ieee8021x && keyMgmt != NetworkManager::WirelessSecuritySetting::WpaEap
        && keyMgmt != NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192) {
        loadSecrets(setting);
    }
}

void WifiSecurity::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    const NetworkManager::WirelessSecuritySetting::KeyMgmt keyMgmt = m_wifiSecurity->keyMgmt();
    const NetworkManager::WirelessSecuritySetting::AuthAlg authAlg = m_wifiSecurity->authAlg();

    if ((keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x && authAlg != NetworkManager::WirelessSecuritySetting::Leap)
        || keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEap || keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192) {
        NetworkManager::Security8021xSetting::Ptr security8021xSetting = setting.staticCast<NetworkManager::Security8021xSetting>();
        if (security8021xSetting) {
            if (keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x) {
                m_8021xWidget->loadSecrets(security8021xSetting);
            } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaEap) {
                m_WPA2Widget->loadSecrets(security8021xSetting);
            } else {
                m_WPA3SuiteB192Widget->loadSecrets(security8021xSetting);
            }
        }
    } else {
        NetworkManager::WirelessSecuritySetting::Ptr wifiSecurity = setting.staticCast<NetworkManager::WirelessSecuritySetting>();
        if (wifiSecurity) {
            if (keyMgmt == NetworkManager::WirelessSecuritySetting::Wep) {
                m_wifiSecurity->secretsFromMap(wifiSecurity->secretsToMap());
                const int keyIndex = static_cast<int>(m_wifiSecurity->wepTxKeyindex());
                setWepKey(keyIndex);
            } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::Ieee8021x && authAlg == NetworkManager::WirelessSecuritySetting::Leap) {
                const QString leapPassword = wifiSecurity->leapPassword();
                if (!leapPassword.isEmpty()) {
                    m_ui->leapPassword->setText(leapPassword);
                }
            } else if (keyMgmt == NetworkManager::WirelessSecuritySetting::WpaPsk || keyMgmt == NetworkManager::WirelessSecuritySetting::SAE) {
                const QString psk = wifiSecurity->psk();
                if (!psk.isEmpty()) {
                    m_ui->psk->setText(psk);
                }
            }
        }
    }
}

QVariantMap WifiSecurity::setting() const
{
    NetworkManager::WirelessSecuritySetting wifiSecurity;

    const int securityIndex = m_ui->securityCombo->currentIndex();
    if (securityIndex == None) {
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Unknown);
    } else if (securityIndex == WepHex || securityIndex == WepPassphrase) { // WEP
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Wep);
        if (securityIndex == WepHex) {
            wifiSecurity.setWepKeyType(NetworkManager::WirelessSecuritySetting::Hex);
        } else {
            wifiSecurity.setWepKeyType(NetworkManager::WirelessSecuritySetting::Passphrase);
        }
        const int keyIndex = m_ui->wepIndex->currentIndex();
        const QString wepKey = m_ui->wepKey->text();
        wifiSecurity.setWepTxKeyindex(keyIndex);
        if (keyIndex == 0) {
            wifiSecurity.setWepKey0(wepKey);
        } else if (keyIndex == 1) {
            wifiSecurity.setWepKey1(wepKey);
        } else if (keyIndex == 2) {
            wifiSecurity.setWepKey2(wepKey);
        } else if (keyIndex == 3) {
            wifiSecurity.setWepKey3(wepKey);
        }

        if (m_ui->wepKey->passwordOption() == PasswordField::StoreForAllUsers) {
            wifiSecurity.setWepKeyFlags(NetworkManager::Setting::None);
        } else if (m_ui->wepKey->passwordOption() == PasswordField::StoreForUser) {
            wifiSecurity.setWepKeyFlags(NetworkManager::Setting::AgentOwned);
        } else {
            wifiSecurity.setWepKeyFlags(NetworkManager::Setting::NotSaved);
        }

        if (m_ui->wepAuth->currentIndex() == 0) {
            wifiSecurity.setAuthAlg(NetworkManager::WirelessSecuritySetting::Open);
        } else {
            wifiSecurity.setAuthAlg(NetworkManager::WirelessSecuritySetting::Shared);
        }
    } else if (securityIndex == Leap) { // LEAP
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
        wifiSecurity.setAuthAlg(NetworkManager::WirelessSecuritySetting::Leap);
        wifiSecurity.setLeapUsername(m_ui->leapUsername->text());
        wifiSecurity.setLeapPassword(m_ui->leapPassword->text());

        if (m_ui->leapPassword->passwordOption() == PasswordField::StoreForAllUsers) {
            wifiSecurity.setLeapPasswordFlags(NetworkManager::Setting::None);
        } else if (m_ui->leapPassword->passwordOption() == PasswordField::StoreForUser) {
            wifiSecurity.setLeapPasswordFlags(NetworkManager::Setting::AgentOwned);
        } else {
            wifiSecurity.setLeapPasswordFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (securityIndex == DynamicWep) { // Dynamic WEP
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::Ieee8021x);
    } else if (securityIndex == WpaPsk) { // WPA
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaPsk);
        wifiSecurity.setPsk(m_ui->psk->text());

        if (m_ui->psk->passwordOption() == PasswordField::StoreForAllUsers) {
            wifiSecurity.setPskFlags(NetworkManager::Setting::None);
        } else if (m_ui->psk->passwordOption() == PasswordField::StoreForUser) {
            wifiSecurity.setPskFlags(NetworkManager::Setting::AgentOwned);
        } else {
            wifiSecurity.setPskFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (securityIndex == WpaEap) { // WPA2 Enterprise
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEap);
    } else if (securityIndex == SAE) { // WPA3 Personal
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::SAE);
        wifiSecurity.setPsk(m_ui->psk->text());

        if (m_ui->psk->passwordOption() == PasswordField::StoreForAllUsers) {
            wifiSecurity.setPskFlags(NetworkManager::Setting::None);
        } else if (m_ui->psk->passwordOption() == PasswordField::StoreForUser) {
            wifiSecurity.setPskFlags(NetworkManager::Setting::AgentOwned);
        } else {
            wifiSecurity.setPskFlags(NetworkManager::Setting::NotSaved);
        }
    } else if (securityIndex == Wpa3SuiteB192) { // WPA3 Enterprise Suite B 192
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::WpaEapSuiteB192);
        wifiSecurity.setPmf(NetworkManager::WirelessSecuritySetting::RequiredPmf);
    } else if (securityIndex == OWE) { // Enhanced Open (OWE)
        wifiSecurity.setKeyMgmt(NetworkManager::WirelessSecuritySetting::OWE);
    }

    return wifiSecurity.toMap();
}

QVariantMap WifiSecurity::setting8021x() const
{
    if (m_ui->securityCombo->currentIndex() == DynamicWep) { // Dynamic WEP
        return m_8021xWidget->setting();
    } else if (m_ui->securityCombo->currentIndex() == WpaEap) { // WPA2 Enterprise
        return m_WPA2Widget->setting();
    } else if (m_ui->securityCombo->currentIndex() == Wpa3SuiteB192) { // WPA3 Enterprise Suite B 192
        return m_WPA3SuiteB192Widget->setting();
    }

    return {};
}

void WifiSecurity::onSsidChanged(const QString &ssid)
{
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()) {
        if (device->type() == NetworkManager::Device::Wifi) {
            NetworkManager::WirelessDevice::Ptr wifiDevice = device.staticCast<NetworkManager::WirelessDevice>();
            if (wifiDevice) {
                for (const NetworkManager::WirelessNetwork::Ptr &wifiNetwork : wifiDevice->networks()) {
                    if (wifiNetwork && wifiNetwork->ssid() == ssid) {
                        NetworkManager::AccessPoint::Ptr ap = wifiNetwork->referenceAccessPoint();
                        NetworkManager::WirelessSecurityType securityType =
                            NetworkManager::findBestWirelessSecurity(wifiDevice->wirelessCapabilities(),
                                                                     true,
                                                                     (wifiDevice->mode() == NetworkManager::WirelessDevice::Adhoc),
                                                                     ap->capabilities(),
                                                                     ap->wpaFlags(),
                                                                     ap->rsnFlags());
                        switch (securityType) {
                        case NetworkManager::WirelessSecurityType::StaticWep:
                            m_ui->securityCombo->setCurrentIndex(WepHex);
                            break;
                        case NetworkManager::WirelessSecurityType::DynamicWep:
                            m_ui->securityCombo->setCurrentIndex(DynamicWep);
                            break;
                        case NetworkManager::WirelessSecurityType::Leap:
                            m_ui->securityCombo->setCurrentIndex(Leap);
                            break;
                        case NetworkManager::WirelessSecurityType::WpaPsk:
                            m_ui->securityCombo->setCurrentIndex(WpaPsk);
                            break;
                        case NetworkManager::WirelessSecurityType::Wpa2Psk:
                            m_ui->securityCombo->setCurrentIndex(WpaPsk);
                            break;
                        case NetworkManager::WirelessSecurityType::WpaEap:
                            m_ui->securityCombo->setCurrentIndex(WpaEap);
                            break;
                        case NetworkManager::WirelessSecurityType::Wpa2Eap:
                            m_ui->securityCombo->setCurrentIndex(WpaEap);
                            break;
                        case NetworkManager::WirelessSecurityType::SAE:
                            m_ui->securityCombo->setCurrentIndex(SAE);
                            break;
                        case NetworkManager::WirelessSecurityType::Wpa3SuiteB192:
                            m_ui->securityCombo->setCurrentIndex(Wpa3SuiteB192);
                            break;
                        case NetworkManager::WirelessSecurityType::OWE:
                            m_ui->securityCombo->setCurrentIndex(OWE);
                            break;
                        default:
                            m_ui->securityCombo->setCurrentIndex(None);
                        }

                        return;
                    }
                }
            }
        }
    }

    // Reset to none security if we don't find any AP or Wifi device
    m_ui->securityCombo->setCurrentIndex(None);
}

void WifiSecurity::setWepKey(int keyIndex)
{
    if (keyIndex == 0) {
        m_ui->wepKey->setText(m_wifiSecurity->wepKey0());
    } else if (keyIndex == 1) {
        m_ui->wepKey->setText(m_wifiSecurity->wepKey1());
    } else if (keyIndex == 2) {
        m_ui->wepKey->setText(m_wifiSecurity->wepKey2());
    } else if (keyIndex == 3) {
        m_ui->wepKey->setText(m_wifiSecurity->wepKey3());
    }
}

void WifiSecurity::securityChanged(int index)
{
    if (index == None || index == OWE) {
        m_ui->stackedWidget->setCurrentIndex(0);
    } else if (index == WepHex || index == WepPassphrase) {
        m_ui->stackedWidget->setCurrentIndex(1);
    } else if (index == Leap) {
        m_ui->stackedWidget->setCurrentIndex(2);
    } else if (index == DynamicWep) {
        m_ui->stackedWidget->setCurrentIndex(3);
    } else if (index == WpaPsk || index == SAE) {
        m_ui->stackedWidget->setCurrentIndex(4);
    } else if (index == WpaEap) {
        m_ui->stackedWidget->setCurrentIndex(5);
    } else if (index == Wpa3SuiteB192) {
        m_ui->stackedWidget->setCurrentIndex(6);
    }

    KAcceleratorManager::manage(m_ui->stackedWidget->currentWidget());
}

#include "moc_wifisecurity.cpp"
