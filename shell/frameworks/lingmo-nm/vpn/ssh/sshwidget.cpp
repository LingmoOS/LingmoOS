/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sshwidget.h"

#include "ui_sshadvanced.h"
#include "ui_sshwidget.h"

#include "simpleipv4addressvalidator.h"
#include "simpleipv6addressvalidator.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QString>

#include "nm-ssh-service.h"

class SshSettingWidgetPrivate
{
public:
    Ui_SshWidget ui;
    Ui_SshAdvanced advUi;
    NetworkManager::VpnSetting::Ptr setting;
    QDialog *advancedDlg = nullptr;
    QWidget *advancedWid = nullptr;
};

SshSettingWidget::SshSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , d_ptr(new SshSettingWidgetPrivate)
{
    Q_D(SshSettingWidget);
    d->ui.setupUi(this);

    d->setting = setting;

    d->ui.le_password->setPasswordOptionsEnabled(true);

    connect(d->ui.cmb_authType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SshSettingWidget::authTypeChanged);
    connect(d->ui.btn_advancedOption, &QPushButton::clicked, this, &SshSettingWidget::doAdvancedDialog);

    d->advancedDlg = new QDialog(this);
    d->advancedDlg->setModal(true);
    d->advancedWid = new QWidget(this);
    d->advUi.setupUi(d->advancedWid);
    auto layout = new QVBoxLayout(d->advancedDlg);
    layout->addWidget(d->advancedWid);
    d->advancedDlg->setLayout(layout);
    auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, d->advancedDlg);
    connect(buttons, &QDialogButtonBox::accepted, d->advancedDlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, d->advancedDlg, &QDialog::reject);

    layout->addWidget(buttons);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(d->ui.le_gateway, &QLineEdit::textChanged, this, &SshSettingWidget::slotWidgetChanged);
    connect(d->ui.le_localIp, &QLineEdit::textChanged, this, &SshSettingWidget::slotWidgetChanged);
    connect(d->ui.le_netmask, &QLineEdit::textChanged, this, &SshSettingWidget::slotWidgetChanged);
    connect(d->ui.le_remoteIp, &QLineEdit::textChanged, this, &SshSettingWidget::slotWidgetChanged);

    auto ipv4Validator = new SimpleIpV4AddressValidator(SimpleIpV4AddressValidator::Base, this);
    d->ui.le_localIp->setValidator(ipv4Validator);
    d->ui.le_remoteIp->setValidator(ipv4Validator);
    d->ui.le_netmask->setValidator(ipv4Validator);

    auto ipv6Validator = new SimpleIpV6AddressValidator(SimpleIpV6AddressValidator::Base, this);
    d->ui.le_localIpv6->setValidator(ipv6Validator);
    d->ui.le_remoteIpv6->setValidator(ipv6Validator);

    d->ui.passwordWidget->setVisible(false);
    d->advUi.sb_useCustomGatewayPort->setValue(NM_SSH_DEFAULT_PORT);
    d->advUi.sb_useCustomTunnelMtu->setValue(NM_SSH_DEFAULT_MTU);
    d->advUi.le_extraSshOptions->setText(QLatin1String(NM_SSH_DEFAULT_EXTRA_OPTS));
    d->advUi.sb_remoteDeviceNumber->setValue(NM_SSH_DEFAULT_REMOTE_DEV);
    d->advUi.le_remoteUsername->setText(QLatin1String(NM_SSH_DEFAULT_REMOTE_USERNAME));

    KAcceleratorManager::manage(this);

    if (d->setting && !d->setting->isNull()) {
        loadConfig(d->setting);
    }
}

SshSettingWidget::~SshSettingWidget()
{
    delete d_ptr;
}

void SshSettingWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(SshSettingWidget);
    Q_UNUSED(setting)

    const NMStringMap dataMap = d->setting->data();

    // General
    const QString gateway = dataMap[QLatin1String(NM_SSH_KEY_REMOTE)];
    if (!gateway.isEmpty()) {
        d->ui.le_gateway->setText(gateway);
    }

    // Network settings
    const QString remoteIp = dataMap[QLatin1String(NM_SSH_KEY_REMOTE_IP)];
    if (!remoteIp.isEmpty()) {
        d->ui.le_remoteIp->setText(remoteIp);
    }

    const QString localIp = dataMap[QLatin1String(NM_SSH_KEY_LOCAL_IP)];
    if (!localIp.isEmpty()) {
        d->ui.le_localIp->setText(localIp);
    }

    const QString netmask = dataMap[QLatin1String(NM_SSH_KEY_NETMASK)];
    if (!netmask.isEmpty()) {
        d->ui.le_netmask->setText(netmask);
    }

    // IPv6 network settings
    const bool ipv6Enabled = dataMap[QLatin1String(NM_SSH_KEY_IP_6)] == QLatin1String("yes");
    d->ui.chk_useIpv6->setChecked(ipv6Enabled);

    if (ipv6Enabled) {
        const QString remoteIpv6 = dataMap[QLatin1String(NM_SSH_KEY_REMOTE_IP_6)];
        if (!remoteIpv6.isEmpty()) {
            d->ui.le_remoteIpv6->setText(remoteIpv6);
        }

        const QString localIpv6 = dataMap[QLatin1String(NM_SSH_KEY_LOCAL_IP_6)];
        if (!localIpv6.isEmpty()) {
            d->ui.le_localIpv6->setText(localIpv6);
        }

        const QString netmaskIpv6 = dataMap[QLatin1String(NM_SSH_KEY_NETMASK_6)];
        if (!netmaskIpv6.isEmpty()) {
            d->ui.le_netmaskIpv6->setText(netmaskIpv6);
        }
    }

    // Authentication
    const QString sshAuthType = dataMap[QLatin1String(NM_SSH_KEY_AUTH_TYPE)];
    if (sshAuthType == QLatin1String(NM_SSH_AUTH_TYPE_SSH_AGENT)) {
        d->ui.cmb_authType->setCurrentIndex(0);
    } else if (sshAuthType == QLatin1String(NM_SSH_AUTH_TYPE_PASSWORD)) {
        d->ui.cmb_authType->setCurrentIndex(1);
        const NetworkManager::Setting::SecretFlags type = (NetworkManager::Setting::SecretFlags)dataMap[NM_SSH_KEY_PASSWORD "-flags"].toInt();
        fillOnePasswordCombo(d->ui.le_password, type);
    } else if (sshAuthType == QLatin1String(NM_SSH_AUTH_TYPE_KEY)) {
        d->ui.cmb_authType->setCurrentIndex(2);
        d->ui.kurl_sshKeyFile->setUrl(QUrl::fromLocalFile(dataMap[QLatin1String(NM_SSH_KEY_KEY_FILE)]));
    }

    // Options below is belongs to "Advanced" dialog
    const QString customGatewayPort = dataMap[QLatin1String(NM_SSH_KEY_PORT)];
    if (!customGatewayPort.isEmpty()) {
        d->advUi.chk_useCustomGatewayPort->setChecked(true);
        d->advUi.sb_useCustomGatewayPort->setValue(customGatewayPort.toInt());
    }

    const QString customMtu = dataMap[QLatin1String(NM_SSH_KEY_TUNNEL_MTU)];
    if (!customMtu.isEmpty()) {
        d->advUi.chk_useCustomTunnelMtu->setChecked(true);
        d->advUi.sb_useCustomTunnelMtu->setValue(customMtu.toInt());
    }

    const QString extraSshOptions = dataMap[QLatin1String(NM_SSH_KEY_EXTRA_OPTS)];
    if (!extraSshOptions.isEmpty()) {
        d->advUi.chk_extraSshOptions->setChecked(true);
        d->advUi.le_extraSshOptions->setText(extraSshOptions);
    }

    const QString remoteDeviceNumber = dataMap[QLatin1String(NM_SSH_KEY_REMOTE_DEV)];
    if (!remoteDeviceNumber.isEmpty()) {
        d->advUi.chk_remoteDeviceNumber->setChecked(true);
        d->advUi.sb_remoteDeviceNumber->setValue(remoteDeviceNumber.toInt());
    }

    const QString useTapDevice = dataMap[QLatin1String(NM_SSH_KEY_TAP_DEV)];
    if (!useTapDevice.isEmpty()) {
        if (useTapDevice == QLatin1String("yes")) {
            d->advUi.chk_useTapDevice->setChecked(true);
        }
    }

    const QString remoteUsername = dataMap[QLatin1String(NM_SSH_KEY_REMOTE_USERNAME)];
    if (!remoteUsername.isEmpty()) {
        d->advUi.chk_remoteUsername->setChecked(true);
        d->advUi.le_remoteUsername->setText(remoteUsername);
    }

    const QString doNotReplaceDefaultRoute = dataMap[QLatin1String(NM_SSH_KEY_NO_DEFAULT_ROUTE)];
    if (!doNotReplaceDefaultRoute.isEmpty()) {
        if (doNotReplaceDefaultRoute == QLatin1String("yes")) {
            d->advUi.chk_doNotReplaceDefaultRoute->setChecked(true);
        }
    }

    loadSecrets(setting);
}

void SshSettingWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(SshSettingWidget);

    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();
        const QString keyPassword = secrets.value(NM_SSH_KEY_PASSWORD);
        if (!keyPassword.isEmpty()) {
            d->ui.le_password->setText(keyPassword);
        }
    }
}

QVariantMap SshSettingWidget::setting() const
{
    Q_D(const SshSettingWidget);

    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_SSH));

    NMStringMap data;
    NMStringMap secretData;

    data.insert(QLatin1String(NM_SSH_KEY_REMOTE), d->ui.le_gateway->text());

    if (!d->ui.le_remoteIp->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSH_KEY_REMOTE_IP), d->ui.le_remoteIp->text());
    }

    if (!d->ui.le_localIp->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSH_KEY_LOCAL_IP), d->ui.le_localIp->text());
    }

    if (!d->ui.le_netmask->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSH_KEY_NETMASK), d->ui.le_netmask->text());
    }

    if (d->ui.chk_useIpv6->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_IP_6), QLatin1String("yes"));

        if (!d->ui.le_remoteIpv6->text().isEmpty()) {
            data.insert(QLatin1String(NM_SSH_KEY_REMOTE_IP_6), d->ui.le_remoteIpv6->text());
        }

        if (!d->ui.le_localIpv6->text().isEmpty()) {
            data.insert(QLatin1String(NM_SSH_KEY_LOCAL_IP_6), d->ui.le_localIpv6->text());
        }

        if (!d->ui.le_netmaskIpv6->text().isEmpty()) {
            data.insert(QLatin1String(NM_SSH_KEY_NETMASK_6), d->ui.le_netmaskIpv6->text());
        }
    }

    switch (d->ui.cmb_authType->currentIndex()) {
    case 0:
        data.insert(QLatin1String(NM_SSH_KEY_AUTH_TYPE), QLatin1String(NM_SSH_AUTH_TYPE_SSH_AGENT));
        break;
    case 1:
        data.insert(QLatin1String(NM_SSH_KEY_AUTH_TYPE), QLatin1String(NM_SSH_AUTH_TYPE_PASSWORD));
        if (!d->ui.le_password->text().isEmpty()) {
            secretData.insert(QLatin1String(NM_SSH_KEY_PASSWORD), d->ui.le_password->text());
        }
        handleOnePasswordType(d->ui.le_password, NM_SSH_KEY_PASSWORD "-flags", data);
        break;
    case 2:
        data.insert(QLatin1String(NM_SSH_KEY_AUTH_TYPE), QLatin1String(NM_SSH_AUTH_TYPE_KEY));
        if (!d->ui.kurl_sshKeyFile->url().isEmpty()) {
            data.insert(QLatin1String(NM_SSH_KEY_KEY_FILE), d->ui.kurl_sshKeyFile->url().toLocalFile());
        }
        break;
    }

    if (d->advUi.chk_useCustomGatewayPort->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_PORT), QString::number(d->advUi.sb_useCustomGatewayPort->value()));
    }

    if (d->advUi.chk_useCustomTunnelMtu->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_TUNNEL_MTU), QString::number(d->advUi.sb_useCustomTunnelMtu->value()));
    }

    if (d->advUi.chk_extraSshOptions->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_EXTRA_OPTS), d->advUi.le_extraSshOptions->text());
    }

    if (d->advUi.chk_remoteDeviceNumber->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_REMOTE_DEV), QString::number(d->advUi.sb_remoteDeviceNumber->value()));
    }

    if (d->advUi.chk_useTapDevice->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_TAP_DEV), QLatin1String("yes"));
    }

    if (d->advUi.chk_remoteUsername->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_REMOTE_USERNAME), d->advUi.le_remoteUsername->text());
    }

    if (d->advUi.chk_doNotReplaceDefaultRoute->isChecked()) {
        data.insert(QLatin1String(NM_SSH_KEY_NO_DEFAULT_ROUTE), QLatin1String("yes"));
    }

    // save it all
    setting.setData(data);
    setting.setSecrets(secretData);

    return setting.toMap();
}

void SshSettingWidget::authTypeChanged(int index)
{
    Q_D(SshSettingWidget);

    if (index == 0) {
        d->ui.stackedWidget->setCurrentIndex(0);
        d->ui.passwordWidget->setVisible(false);
    } else if (index == 1) {
        d->ui.stackedWidget->setCurrentIndex(0);
        d->ui.passwordWidget->setVisible(true);
    } else {
        d->ui.stackedWidget->setCurrentIndex(1);
    }
}

void SshSettingWidget::doAdvancedDialog()
{
    Q_D(SshSettingWidget);
    d->advancedDlg->show();
}

void SshSettingWidget::passwordTypeChanged(int index)
{
    Q_D(SshSettingWidget);
    d->ui.le_password->setEnabled(index == SettingWidget::EnumPasswordStorageType::Store);
}

void SshSettingWidget::fillOnePasswordCombo(PasswordField *passwordField, NetworkManager::Setting::SecretFlags type)
{
    if (type.testFlag(NetworkManager::Setting::None)) {
        passwordField->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (type.testFlag(NetworkManager::Setting::AgentOwned)) {
        passwordField->setPasswordOption(PasswordField::StoreForUser);
    } else if (type.testFlag(NetworkManager::Setting::NotSaved)) {
        passwordField->setPasswordOption(PasswordField::AlwaysAsk);
    } else {
        passwordField->setPasswordOption(PasswordField::PasswordField::NotRequired);
    }
}

void SshSettingWidget::handleOnePasswordType(const PasswordField *passwordField, const QString &key, NMStringMap &data) const
{
    const PasswordField::PasswordOption option = passwordField->passwordOption();
    switch (option) {
    case PasswordField::StoreForAllUsers:
        data.insert(key, QString::number(NetworkManager::Setting::None));
        break;
    case PasswordField::StoreForUser:
        data.insert(key, QString::number(NetworkManager::Setting::AgentOwned));
        break;
    case PasswordField::AlwaysAsk:
        data.insert(key, QString::number(NetworkManager::Setting::NotSaved));
        break;
    case PasswordField::NotRequired:
        data.insert(key, QString::number(NetworkManager::Setting::NotRequired));
        break;
    }
}

bool SshSettingWidget::isValid() const
{
    Q_D(const SshSettingWidget);

    return !d->ui.le_gateway->text().isEmpty() && !d->ui.le_localIp->text().isEmpty() && !d->ui.le_remoteIp->text().isEmpty()
        && !d->ui.le_netmask->text().isEmpty();
}

#include "moc_sshwidget.cpp"
