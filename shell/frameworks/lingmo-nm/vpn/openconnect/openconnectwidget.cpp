/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.de>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openconnectwidget.h"
#include <QDialog>
#include <QStringList>
#include <QUrl>

#include "ui_openconnectprop.h"
#include "ui_openconnecttoken.h"

#include "nm-openconnect-service.h"
#include <QDialogButtonBox>
#include <QString>

#include <openconnect.h>

#if !OPENCONNECT_CHECK_VER(2, 1)
#define openconnect_has_stoken_support() 0
#endif
#if !OPENCONNECT_CHECK_VER(2, 2)
#define openconnect_has_oath_support() 0
#endif
#if !OPENCONNECT_CHECK_VER(5, 0)
#define openconnect_has_yubioath_support() 0
#endif

using Token = struct {
    int tokenIndex;
    QString tokenSecret;
};

class OpenconnectSettingWidgetPrivate
{
public:
    Ui_OpenconnectProp ui;
    Ui::OpenConnectToken tokenUi;
    NetworkManager::VpnSetting::Ptr setting;
    QDialog *tokenDlg;
    Token token;
};

OpenconnectSettingWidget::OpenconnectSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , d_ptr(new OpenconnectSettingWidgetPrivate)
{
    Q_D(OpenconnectSettingWidget);

    d->ui.setupUi(this);
    d->setting = setting;

#if !OPENCONNECT_CHECK_VER(5, 8)
    d->ui.leMcaCertLabel->setVisible(false);
    d->ui.leMcaCert->setVisible(false);
    d->ui.leMcaPrivateKeyLabel->setVisible(false);
    d->ui.leMcaPrivateKey->setVisible(false);
#endif

    // Connect for validity check
    connect(d->ui.leGateway, &QLineEdit::textChanged, this, &OpenconnectSettingWidget::slotWidgetChanged);

    connect(d->ui.buTokens, &QPushButton::clicked, this, &OpenconnectSettingWidget::showTokens);

    d->tokenDlg = new QDialog(this);
    d->tokenUi.setupUi(d->tokenDlg);
    d->tokenUi.leTokenSecret->setPasswordModeEnabled(true);
    d->tokenUi.leTokenSecret->setPasswordOptionsEnabled(true);
    auto layout = new QVBoxLayout(d->tokenDlg);
    layout->addWidget(d->tokenDlg);
    d->tokenDlg->setLayout(layout);
    connect(d->tokenUi.buttonBox, &QDialogButtonBox::accepted, d->tokenDlg, &QDialog::accept);
    connect(d->tokenUi.buttonBox, &QDialogButtonBox::rejected, d->tokenDlg, &QDialog::reject);
    connect(d->tokenDlg, &QDialog::rejected, this, &OpenconnectSettingWidget::restoreTokens);
    connect(d->tokenDlg, &QDialog::accepted, this, &OpenconnectSettingWidget::saveTokens);

    connect(d->tokenUi.cmbTokenMode,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            QOverload<int>::of((&OpenconnectSettingWidget::handleTokenSecret)));

    // Connect for setting check
    watchChangedSetting();

    // Remove these from setting check:
    // Just popping up the tokenDlg changes nothing
    disconnect(d->ui.buTokens, &QPushButton::clicked, this, &SettingWidget::settingChanged);
    // User cancels means nothing should change here
    disconnect(d->tokenUi.buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &SettingWidget::settingChanged);

    d->tokenUi.gbToken->setVisible(initTokenGroup());

    KAcceleratorManager::manage(this);

    if (d->setting) {
        loadConfig(d->setting);
    }
}

OpenconnectSettingWidget::~OpenconnectSettingWidget()
{
    delete d_ptr;
}

void OpenconnectSettingWidget::handleTokenSecret(int index)
{
    Q_D(const OpenconnectSettingWidget);

    QVariant mode = d->tokenUi.cmbTokenMode->itemData(index);
    if (mode == QStringLiteral("disabled")) {
        d->tokenUi.leTokenSecret->setEnabled(false);
        d->tokenUi.leTokenSecret->setToolTip("No secrets needed.");
    } else if (mode == QStringLiteral("stokenrc")) {
        d->tokenUi.leTokenSecret->setEnabled(false);
        d->tokenUi.leTokenSecret->setToolTip("No secrets needed; will read them from ~/.stokenrc.");
    } else if (mode == QStringLiteral("manual")) {
        d->tokenUi.leTokenSecret->setToolTip("Insert the secret here. See the openconnect documentation for syntax.");
        d->tokenUi.leTokenSecret->setEnabled(true);
    } else if (mode == QStringLiteral("totp")) {
        d->tokenUi.leTokenSecret->setEnabled(true);
        d->tokenUi.leTokenSecret->setToolTip(
            "Insert the secret here, with a sha specification and a leading '0x' or 'base32:'. See the openconnect documentation for syntax.");
    } else if (mode == QStringLiteral("hotp")) {
        d->tokenUi.leTokenSecret->setEnabled(true);
        d->tokenUi.leTokenSecret->setToolTip(
            "Insert the secret here, with a leading '0x' or 'base32:' and a trailing counter after a comma (','), See the openconnect documentation for "
            "syntax.");
    } else if (mode == QStringLiteral("yubioath")) {
        d->tokenUi.leTokenSecret->setEnabled(true);
        d->tokenUi.leTokenSecret->setToolTip("Insert the token Id here, in the form company:username. Make sure to set your Yubikey in CCID mode");
    } else { // Not really needed now, but who knows?
        d->tokenUi.leTokenSecret->setEnabled(false);
        d->tokenUi.leTokenSecret->setToolTip("");
    }
}

bool OpenconnectSettingWidget::initTokenGroup()
{
    Q_D(const OpenconnectSettingWidget);

    int validRows = 0;
    QStringList tokenLabelList{
        "Disabled",
        "RSA SecurID — read from ~/.stokenrc",
        "RSA SecurID — manually entered",
        "TOTP — manually entered",
        "HOTP — manually entered",
        "Yubikey",
    };
    QStringList tokenModeList{
        "disabled",
        "stokenrc",
        "manual",
        "totp",
        "hotp",
        "yubioath",
    };
    QComboBox *combo = d->tokenUi.cmbTokenMode;

    combo->addItem(tokenLabelList[validRows]);
    combo->setItemData(validRows, tokenModeList[validRows], Qt::UserRole);
    validRows++;
    if (openconnect_has_stoken_support()) {
        for (; validRows < 3; validRows++) {
            combo->addItem(tokenLabelList[validRows]);
            combo->setItemData(validRows, tokenModeList[validRows], Qt::UserRole);
        }
    }
    if (openconnect_has_oath_support()) {
        combo->addItem(tokenLabelList[validRows]);
        combo->setItemData(validRows, tokenModeList[validRows], Qt::UserRole);
        validRows++;
        if (OPENCONNECT_CHECK_VER(3, 4)) {
            combo->addItem(tokenLabelList[validRows]);
            combo->setItemData(validRows, tokenModeList[validRows], Qt::UserRole);
            validRows++;
        }
    }
    if (openconnect_has_yubioath_support()) {
        combo->addItem(tokenLabelList[validRows]);
        combo->setItemData(validRows, tokenModeList[validRows], Qt::UserRole);
    }
    return validRows > 0;
}

void OpenconnectSettingWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(OpenconnectSettingWidget);

    // General settings
    const NMStringMap dataMap = setting.staticCast<NetworkManager::VpnSetting>()->data();

    int cmbProtocolIndex;
    // No value corresponds to "anyconnect", matching GNOME and the openconnect binary itself.
    if (!dataMap.contains(NM_OPENCONNECT_KEY_PROTOCOL) || dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("anyconnect")) {
        cmbProtocolIndex = 0;
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("nc")) {
        cmbProtocolIndex = 1;
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("gp")) {
        cmbProtocolIndex = 2;
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("pulse")) {
        cmbProtocolIndex = 3; // pulse, Pulse Connect Secure
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("f5")) {
        cmbProtocolIndex = 4; // F5 BIG-IP SSL VPN
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("fortinet")) {
        cmbProtocolIndex = 5; // Fortinet SSL VPN
    } else if (dataMap[NM_OPENCONNECT_KEY_PROTOCOL] == QLatin1String("array")) {
        cmbProtocolIndex = 6; // Array SSL VPN
    } else {
        cmbProtocolIndex = 3; // pulse, Pulse Connect Secure is the default
    }

    int cmbReportedOsIndex;
    if (!dataMap.contains(NM_OPENCONNECT_KEY_REPORTED_OS)) {
        cmbReportedOsIndex = 0;
    } else if (dataMap[NM_OPENCONNECT_KEY_REPORTED_OS] == QLatin1String("linux")) {
        cmbReportedOsIndex = 1;
    } else if (dataMap[NM_OPENCONNECT_KEY_REPORTED_OS] == QLatin1String("linux-64")) {
        cmbReportedOsIndex = 2;
    } else if (dataMap[NM_OPENCONNECT_KEY_REPORTED_OS] == QLatin1String("win")) {
        cmbReportedOsIndex = 3;
    } else if (dataMap[NM_OPENCONNECT_KEY_REPORTED_OS] == QLatin1String("mac-intel")) {
        cmbReportedOsIndex = 4;
    } else if (dataMap[NM_OPENCONNECT_KEY_REPORTED_OS] == QLatin1String("android")) {
        cmbReportedOsIndex = 5;
    } else {
        cmbReportedOsIndex = 6; // apple-ios
    }

    d->ui.cmbProtocol->setCurrentIndex(cmbProtocolIndex);
    d->ui.leGateway->setText(dataMap[NM_OPENCONNECT_KEY_GATEWAY]);
    d->ui.leCaCertificate->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_CACERT]));
    d->ui.leProxy->setText(dataMap[NM_OPENCONNECT_KEY_PROXY]);
    d->ui.leUserAgent->setText(dataMap[NM_OPENCONNECT_KEY_USERAGENT]);
    d->ui.leVersionString->setText(dataMap[NM_OPENCONNECT_KEY_VERSION_STRING]);
    d->ui.cmbReportedOs->setCurrentIndex(cmbReportedOsIndex);
    d->ui.chkAllowTrojan->setChecked(dataMap[NM_OPENCONNECT_KEY_CSD_ENABLE] == "yes");
    d->ui.leCsdWrapperScript->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_CSD_WRAPPER]));
    d->ui.leMcaCert->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_MCACERT]));
    d->ui.leMcaPrivateKey->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_MCAKEY]));
    d->ui.leUserCert->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_USERCERT]));
    d->ui.leUserPrivateKey->setUrl(QUrl::fromLocalFile(dataMap[NM_OPENCONNECT_KEY_PRIVKEY]));
    d->ui.chkUseFsid->setChecked(dataMap[NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID] == "yes");
    d->ui.preventInvalidCert->setChecked(dataMap[NM_OPENCONNECT_KEY_PREVENT_INVALID_CERT] == "yes");

    // Token settings
    const NetworkManager::Setting::SecretFlags tokenSecretFlag =
        static_cast<NetworkManager::Setting::SecretFlags>(dataMap.value(NM_OPENCONNECT_KEY_TOKEN_SECRET "-flags").toInt());
    if (tokenSecretFlag == NetworkManager::Setting::None) {
        d->tokenUi.leTokenSecret->setPasswordOption(PasswordField::StoreForAllUsers);
    } else if (tokenSecretFlag == NetworkManager::Setting::AgentOwned) {
        d->tokenUi.leTokenSecret->setPasswordOption(PasswordField::StoreForUser);
    } else {
        d->tokenUi.leTokenSecret->setPasswordOption(PasswordField::AlwaysAsk);
    }
    for (int index = 0; index < d->tokenUi.cmbTokenMode->count(); index++) {
        if (d->tokenUi.cmbTokenMode->itemData(index, Qt::UserRole) == dataMap[NM_OPENCONNECT_KEY_TOKEN_MODE]) {
            d->tokenUi.cmbTokenMode->setCurrentIndex(index);
            d->token.tokenIndex = index;
            if (index > 1) {
                loadSecrets(d->setting);
            }
            break;
        }
    }
}

void OpenconnectSettingWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(OpenconnectSettingWidget);

    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();
        d->tokenUi.leTokenSecret->setText(secrets.value(NM_OPENCONNECT_KEY_TOKEN_SECRET));
        d->token.tokenSecret = secrets.value(NM_OPENCONNECT_KEY_TOKEN_SECRET);
    }
}

QVariantMap OpenconnectSettingWidget::setting() const
{
    Q_D(const OpenconnectSettingWidget);

    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_OPENCONNECT));

    NMStringMap data;
    NMStringMap secrets;
    QString protocol;
    switch (d->ui.cmbProtocol->currentIndex()) {
    case 0:
        protocol = QLatin1String("anyconnect");
        break;
    case 1:
        protocol = QLatin1String("nc");
        break;
    case 2:
        protocol = QLatin1String("gp");
        break;
    case 3:
        protocol = QLatin1String("pulse");
        break;
    case 4:
        protocol = QLatin1String("f5");
        break;
    case 5:
        protocol = QLatin1String("fortinet");
        break;
    case 6:
        protocol = QLatin1String("array");
        break;
    default:
        protocol = QLatin1String("pulse");
    }

    QString reportedOs;
    switch (d->ui.cmbReportedOs->currentIndex()) {
    case 0:
        reportedOs = QString();
        break;
    case 1:
        reportedOs = QLatin1String("linux");
        break;
    case 2:
        reportedOs = QLatin1String("linux-64");
        break;
    case 3:
        reportedOs = QLatin1String("win");
        break;
    case 4:
        reportedOs = QLatin1String("mac-intel");
        break;
    case 5:
        reportedOs = QLatin1String("android");
        break;
    default:
        reportedOs = QLatin1String("apple-ios");
    }

    data.insert(NM_OPENCONNECT_KEY_PROTOCOL, protocol);
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GATEWAY), d->ui.leGateway->text());
    if (d->ui.leCaCertificate->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_CACERT), d->ui.leCaCertificate->url().toLocalFile());
    }
    if (!d->ui.leProxy->text().isEmpty()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_PROXY), d->ui.leProxy->text());
    }
    if (!d->ui.leUserAgent->text().isEmpty()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_USERAGENT), d->ui.leUserAgent->text());
    }
    if (!d->ui.leVersionString->text().isEmpty()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_VERSION_STRING), d->ui.leVersionString->text());
    }
    data.insert(NM_OPENCONNECT_KEY_REPORTED_OS, reportedOs);
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_CSD_ENABLE), d->ui.chkAllowTrojan->isChecked() ? "yes" : "no");
    if (d->ui.leCsdWrapperScript->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_CSD_WRAPPER), d->ui.leCsdWrapperScript->url().toLocalFile());
    }
    if (d->ui.leMcaCert->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_MCACERT), d->ui.leMcaCert->url().toLocalFile());
    }
    if (d->ui.leMcaPrivateKey->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_MCAKEY), d->ui.leMcaPrivateKey->url().toLocalFile());
    }
    if (d->ui.leUserCert->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_USERCERT), d->ui.leUserCert->url().toLocalFile());
    }
    if (d->ui.leUserPrivateKey->url().isValid()) {
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_PRIVKEY), d->ui.leUserPrivateKey->url().toLocalFile());
    }
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID), d->ui.chkUseFsid->isChecked() ? "yes" : "no");
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_PREVENT_INVALID_CERT), d->ui.preventInvalidCert->isChecked() ? "yes" : "no");

    int index = d->tokenUi.cmbTokenMode->currentIndex();
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_TOKEN_MODE), d->tokenUi.cmbTokenMode->itemData(index, Qt::UserRole).toString());
    secrets.insert(QLatin1String(NM_OPENCONNECT_KEY_TOKEN_SECRET), d->tokenUi.leTokenSecret->text());

    // Restore previous flags, this is necessary for keeping secrets stored in KWallet
    for (const QString &key : d->setting->data().keys()) {
        if (key.contains(QLatin1String("-flags"))) {
            data.insert(key, d->setting->data().value(key));
        }
    }

    if (d->tokenUi.leTokenSecret->passwordOption() == PasswordField::StoreForAllUsers) {
        data.insert(NM_OPENCONNECT_KEY_TOKEN_SECRET "-flags", QString::number(NetworkManager::Setting::None));
    } else if (d->tokenUi.leTokenSecret->passwordOption() == PasswordField::StoreForUser) {
        data.insert(NM_OPENCONNECT_KEY_TOKEN_SECRET "-flags", QString::number(NetworkManager::Setting::AgentOwned));
    } else {
        data.insert(NM_OPENCONNECT_KEY_TOKEN_SECRET "-flags", QString::number(NetworkManager::Setting::NotSaved));
    }

    /* These are different for every login session, and should not be stored */
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_COOKIE "-flags"), QString::number(NetworkManager::Setting::NotSaved));
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GWCERT "-flags"), QString::number(NetworkManager::Setting::NotSaved));
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GATEWAY "-flags"), QString::number(NetworkManager::Setting::NotSaved));

    setting.setData(data);
    setting.setSecrets(secrets);

    return setting.toMap();
}

void OpenconnectSettingWidget::restoreTokens()
{
    Q_D(const OpenconnectSettingWidget);

    d->tokenUi.cmbTokenMode->setCurrentIndex(d->token.tokenIndex);
    d->tokenUi.leTokenSecret->setText(d->token.tokenSecret);
}

void OpenconnectSettingWidget::saveTokens()
{
    Q_D(OpenconnectSettingWidget);

    d->token.tokenIndex = d->tokenUi.cmbTokenMode->currentIndex();
    d->token.tokenSecret = d->tokenUi.leTokenSecret->text();
}

void OpenconnectSettingWidget::showTokens()
{
    Q_D(OpenconnectSettingWidget);

    d->tokenDlg->show();
}

bool OpenconnectSettingWidget::isValid() const
{
    Q_D(const OpenconnectSettingWidget);

    return !d->ui.leGateway->text().isEmpty();
}

#include "moc_openconnectwidget.cpp"
