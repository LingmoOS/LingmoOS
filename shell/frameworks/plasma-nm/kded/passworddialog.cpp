/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "passworddialog.h"
#include "plasma_nm_kded.h"
#include "ui_passworddialog.h"
#include "uiutils.h"

#include <vpnuiplugin.h>

#include <NetworkManagerQt/Utils>
#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/WirelessSetting>

#include <KLocalizedString>

#include <QIcon>
#include <QPushButton>

using namespace NetworkManager;

PasswordDialog::PasswordDialog(const NetworkManager::ConnectionSettings::Ptr &connectionSettings,
                               SecretAgent::GetSecretsFlags flags,
                               const QString &setting_name,
                               const QStringList &hints,
                               QWidget *parent)
    : QDialog(parent)
    , m_settingName(setting_name)
    , m_connectionSettings(connectionSettings)
    , m_error(SecretAgent::NoSecrets)
    , m_flags(flags)
    , m_hints(hints)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("dialog-password")));

    initializeUi();
}

PasswordDialog::~PasswordDialog()
{
    delete m_ui;
}

void PasswordDialog::initializeUi()
{
    m_ui = new Ui::PasswordDialog;
    m_ui->setupUi(this);
    // TODO fix this for high DPI
    m_ui->labelIcon->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-password")).pixmap(64));
    m_ui->labelHeadline->setText(i18n("Authenticate %1", m_connectionSettings->id()));

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &PasswordDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &PasswordDialog::reject);
    connect(m_ui->password, &PasswordField::textChanged, [this](const QString &text) {
        if (m_connectionSettings->connectionType() == NetworkManager::ConnectionSettings::Wireless) {
            NetworkManager::WirelessSecuritySetting::Ptr wirelessSecuritySetting =
                m_connectionSettings->setting(NetworkManager::Setting::WirelessSecurity).staticCast<NetworkManager::WirelessSecuritySetting>();
            bool valid = true;

            if (wirelessSecuritySetting) {
                switch (wirelessSecuritySetting->keyMgmt()) {
                case NetworkManager::WirelessSecuritySetting::WpaPsk:
                    valid = wpaPskIsValid(text);
                    break;
                case NetworkManager::WirelessSecuritySetting::Wep:
                    valid = wepKeyIsValid(text, wirelessSecuritySetting->wepKeyType());
                    break;
                default:
                    break;
                }
            }

            // disable button if key is not valid
            m_ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(!valid);
        }
    });

    if (m_connectionSettings->connectionType() != NetworkManager::ConnectionSettings::Vpn) {
        NetworkManager::Setting::Ptr setting = m_connectionSettings->setting(m_settingName);
        m_neededSecrets = setting->needSecrets(m_flags & SecretAgent::RequestNew);

        if (m_neededSecrets.isEmpty()) {
            qCWarning(PLASMA_NM_KDED_LOG) << "list of secrets is empty!!!";
            m_hasError = true;
            m_error = SecretAgent::InternalError;
            m_errorMessage = i18nc("@info:status 'secrets' means the authentication needed for a VPN", "No secrets were requested");
            return;
        }

        WirelessSetting::Ptr wifi = m_connectionSettings->setting(Setting::Wireless).dynamicCast<WirelessSetting>();
        Setting::SettingType connectionType = setting->type();
        if (wifi && (connectionType == Setting::WirelessSecurity || connectionType == Setting::Security8021x)) {
            const QString ssid = QString::fromUtf8(wifi->ssid());
            if (m_flags & NetworkManager::SecretAgent::RequestNew) {
                m_ui->labelText->setText(i18n("Incorrect password for the wireless network \"%1\". Please try again.", ssid));
            } else {
                m_ui->labelText->setText(i18n("Enter the password for the wireless network \"%1\":", ssid));
            }
        } else {
            if (m_flags & NetworkManager::SecretAgent::RequestNew) {
                m_ui->labelText->setText(i18n("Incorrect password for the connection \"%1\". Please try again.", m_connectionSettings->id()));
            } else {
                m_ui->labelText->setText(i18n("Enter the password for the connection \"%1\":", m_connectionSettings->id()));
            }
        }

        QString connectionLabel;
        UiUtils::iconAndTitleForConnectionSettingsType(m_connectionSettings->connectionType(), connectionLabel);
        setFocusProxy(m_ui->password);
        setWindowTitle(i18n("%1 password dialog", connectionLabel));
    } else {
        NetworkManager::VpnSetting::Ptr vpnSetting = m_connectionSettings->setting(Setting::Vpn).dynamicCast<VpnSetting>();
        if (!vpnSetting) {
            qCWarning(PLASMA_NM_KDED_LOG) << "Missing VPN setting!";
            m_hasError = true;
            m_error = SecretAgent::InternalError;
            m_errorMessage = i18nc("@info:status", "VPN settings are missing");
        } else {
            const QString serviceType = vpnSetting->serviceType();
            const auto result = VpnUiPlugin::loadPluginForType(this, serviceType);

            if (result) {
                VpnUiPlugin *vpnUiPlugin = result.plugin;
                const QString shortName = serviceType.section('.', -1);
                NMStringMap data = vpnSetting->data();
                m_vpnWidget = vpnUiPlugin->askUser(vpnSetting, m_hints, this);
                auto layout = new QVBoxLayout();
                layout->addWidget(m_vpnWidget);
                m_ui->vpnWidget->setLayout(layout);
                m_ui->labelText->setText(i18n("Provide the secrets for the VPN connection \"%1\":", m_connectionSettings->id()));
                setWindowTitle(i18n("VPN secrets (%1) dialog", shortName));

                // Hide generic password field and OK button in case of openconnect dialog
                m_ui->labelPass->setVisible(false);
                m_ui->password->setVisible(false);
                if (shortName == QLatin1String("openconnect")) {
                    QAbstractButton *button = m_ui->buttonBox->button(QDialogButtonBox::Ok);
                    m_ui->buttonBox->removeButton(button);
                }

                setFocusProxy(m_vpnWidget);
                m_vpnWidget->setFocus(Qt::OtherFocusReason);
            } else {
                qCWarning(PLASMA_NM_KDED_LOG) << "Could not load VPN UI plugin" << result.errorText;
                m_hasError = true;
                m_error = SecretAgent::InternalError;
                m_errorMessage = result.errorString;
            }
        }
    }

    // Workaround to force m_ui->password to get focus.
    focusNextChild();
}

bool PasswordDialog::hasError() const
{
    return m_hasError;
}

SecretAgent::Error PasswordDialog::error() const
{
    return m_error;
}

QString PasswordDialog::errorMessage() const
{
    return m_errorMessage;
}

NMVariantMapMap PasswordDialog::secrets() const
{
    NMVariantMapMap ret = m_connectionSettings->toMap();
    QVariantMap result;
    if (m_vpnWidget) {
        result = m_vpnWidget->setting();
    } else if (!m_ui->password->text().isEmpty() && !m_neededSecrets.isEmpty()) {
        result.insert(m_neededSecrets.first(), m_ui->password->text());
    }

    ret.insert(m_settingName, result);

    return ret;
}

#include "moc_passworddialog.cpp"
