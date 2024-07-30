/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "sstpwidget.h"

#include "ui_sstpadvanced.h"
#include "ui_sstpwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QString>

#include "nm-sstp-service.h"

class SstpSettingWidgetPrivate
{
public:
    Ui_SstpWidget ui;
    Ui_SstpAdvanced advUi;
    NetworkManager::VpnSetting::Ptr setting;
    QDialog *advancedDlg = nullptr;
    QWidget *advancedWid = nullptr;
};

SstpSettingWidget::SstpSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , d_ptr(new SstpSettingWidgetPrivate)
{
    Q_D(SstpSettingWidget);
    d->ui.setupUi(this);

    d->setting = setting;

    d->ui.le_password->setPasswordOptionsEnabled(true);

    connect(d->ui.btn_advancedOption, &QPushButton::clicked, this, &SstpSettingWidget::doAdvancedDialog);

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
    connect(d->ui.le_gateway, &QLineEdit::textChanged, this, &SstpSettingWidget::slotWidgetChanged);
    connect(d->ui.le_username, &QLineEdit::textChanged, this, &SstpSettingWidget::slotWidgetChanged);
    connect(d->ui.le_password, &PasswordField::textChanged, this, &SstpSettingWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (d->setting && !d->setting->isNull()) {
        loadConfig(d->setting);
    }
}

SstpSettingWidget::~SstpSettingWidget()
{
    delete d_ptr;
}

void SstpSettingWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(SstpSettingWidget);
    Q_UNUSED(setting)

    const QString yesString = QLatin1String("yes");
    const NMStringMap dataMap = d->setting->data();

    // General
    const QString gateway = dataMap[QLatin1String(NM_SSTP_KEY_GATEWAY)];
    if (!gateway.isEmpty()) {
        d->ui.le_gateway->setText(gateway);
    }

    // Optional setting
    const QString username = dataMap[QLatin1String(NM_SSTP_KEY_USER)];
    if (!username.isEmpty()) {
        d->ui.le_username->setText(username);
    }

    // Authentication
    const NetworkManager::Setting::SecretFlags type = (NetworkManager::Setting::SecretFlags)dataMap[NM_SSTP_KEY_PASSWORD_FLAGS].toInt();
    fillOnePasswordCombo(d->ui.le_password, type);

    const QString ntDomain = dataMap[QLatin1String(NM_SSTP_KEY_DOMAIN)];
    if (!ntDomain.isEmpty()) {
        d->ui.le_ntDomain->setText(ntDomain);
    }

    const QString caCert = dataMap[QLatin1String(NM_SSTP_KEY_CA_CERT)];
    if (!caCert.isEmpty()) {
        d->ui.kurl_caCert->setUrl(QUrl::fromLocalFile(caCert));
    }

    const bool ignoreCertWarnings = (dataMap[QLatin1String(NM_SSTP_KEY_IGN_CERT_WARN)] == yesString);
    d->ui.chk_ignoreCertWarnings->setChecked(ignoreCertWarnings);

    // Advanced options - Point-to-Point
    bool refuse_pap = (dataMap[QLatin1String(NM_SSTP_KEY_REFUSE_PAP)] == yesString);
    bool refuse_chap = (dataMap[QLatin1String(NM_SSTP_KEY_REFUSE_CHAP)] == yesString);
    bool refuse_mschap = (dataMap[QLatin1String(NM_SSTP_KEY_REFUSE_MSCHAP)] == yesString);
    bool refuse_mschapv2 = (dataMap[QLatin1String(NM_SSTP_KEY_REFUSE_MSCHAPV2)] == yesString);
    bool refuse_eap = (dataMap[QLatin1String(NM_SSTP_KEY_REFUSE_EAP)] == yesString);

    QListWidgetItem *item = nullptr;
    item = d->advUi.listWidget->item(0); // PAP
    item->setCheckState(refuse_pap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(1); // CHAP
    item->setCheckState(refuse_chap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(2); // MSCHAP
    item->setCheckState(refuse_mschap ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(3); // MSCHAPv2
    item->setCheckState(refuse_mschapv2 ? Qt::Unchecked : Qt::Checked);
    item = d->advUi.listWidget->item(4); // EAP
    item->setCheckState(refuse_eap ? Qt::Unchecked : Qt::Checked);

    // Cryptography and compression
    const bool mppe = (dataMap[QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE)] == yesString);
    const bool mppe40 = (dataMap[QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE_40)] == yesString);
    const bool mppe128 = (dataMap[QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE_128)] == yesString);
    const bool mppe_stateful = (dataMap[QLatin1String(NM_SSTP_KEY_MPPE_STATEFUL)] == yesString);

    if (mppe || mppe40 || mppe128) { // If MPPE is use
        d->advUi.gb_MPPE->setChecked(mppe || mppe40 || mppe128);
        if (mppe128) {
            d->advUi.cb_MPPECrypto->setCurrentIndex(1); // 128 bit
        } else if (mppe40) {
            d->advUi.cb_MPPECrypto->setCurrentIndex(2); // 40 bit
        } else {
            d->advUi.cb_MPPECrypto->setCurrentIndex(0); // Any
        }
        d->advUi.cb_statefulEncryption->setChecked(mppe_stateful);
    }

    const bool nobsd = (dataMap[QLatin1String(NM_SSTP_KEY_NOBSDCOMP)] == yesString);
    d->advUi.cb_BSD->setChecked(!nobsd);

    const bool nodeflate = (dataMap[QLatin1String(NM_SSTP_KEY_NODEFLATE)] == yesString);
    d->advUi.cb_deflate->setChecked(!nodeflate);

    const bool novjcomp = (dataMap[QLatin1String(NM_SSTP_KEY_NO_VJ_COMP)] == yesString);
    d->advUi.cb_TCPheaders->setChecked(!novjcomp);

    // Echo
    const int lcp_echo_interval = QString(dataMap[QLatin1String(NM_SSTP_KEY_LCP_ECHO_INTERVAL)]).toInt();
    d->advUi.cb_sendEcho->setChecked(lcp_echo_interval > 0);

    if (dataMap.contains(QLatin1String(NM_SSTP_KEY_UNIT_NUM))) {
        d->advUi.chk_useCustomUnitNumber->setChecked(true);
        d->advUi.sb_customUnitNumber->setValue(dataMap[QLatin1String(NM_SSTP_KEY_UNIT_NUM)].toInt());
    }

    // Advanced options - Proxy
    const QString address = dataMap[QLatin1String(NM_SSTP_KEY_PROXY_SERVER)];
    if (!address.isEmpty()) {
        d->advUi.le_address->setText(address);
    }

    const int port = dataMap[QLatin1String(NM_SSTP_KEY_PROXY_PORT)].toInt();
    if (port >= 0) {
        d->advUi.sb_port->setValue(port);
    }

    const QString proxyUsername = dataMap[QLatin1String(NM_SSTP_KEY_PROXY_USER)];
    if (!proxyUsername.isEmpty()) {
        d->advUi.le_username->setText(proxyUsername);
    }

    const QString proxyPassword = dataMap[QLatin1String(NM_SSTP_KEY_PROXY_PASSWORD)];
    if (!proxyPassword.isEmpty()) {
        d->advUi.le_password->setText(proxyPassword);
    }

    loadSecrets(setting);
}

void SstpSettingWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_D(SstpSettingWidget);

    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const NMStringMap secrets = vpnSetting->secrets();

        const QString keyPassword = secrets.value(NM_SSTP_KEY_PASSWORD);
        if (!keyPassword.isEmpty()) {
            d->ui.le_password->setText(keyPassword);
        }
    }
}

QVariantMap SstpSettingWidget::setting() const
{
    Q_D(const SstpSettingWidget);

    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_SSTP));

    const QString yesString = QLatin1String("yes");

    NMStringMap data;
    NMStringMap secretData;

    data.insert(QLatin1String(NM_SSTP_KEY_GATEWAY), d->ui.le_gateway->text());

    if (!d->ui.le_username->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_USER), d->ui.le_username->text());
    }

    if (!d->ui.le_password->text().isEmpty()) {
        secretData.insert(QLatin1String(NM_SSTP_KEY_PASSWORD), d->ui.le_password->text());
    }
    handleOnePasswordType(d->ui.le_password, NM_SSTP_KEY_PASSWORD_FLAGS, data);

    if (!d->ui.le_ntDomain->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_DOMAIN), d->ui.le_ntDomain->text());
    }

    if (!d->ui.kurl_caCert->url().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_CA_CERT), d->ui.kurl_caCert->url().toLocalFile());
    }

    if (d->ui.chk_ignoreCertWarnings->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_IGN_CERT_WARN), yesString);
    }

    // Advanced configuration
    QListWidgetItem *item = nullptr;
    item = d->advUi.listWidget->item(0); // PAP
    if (item->checkState() == Qt::Unchecked) {
        data.insert(QLatin1String(NM_SSTP_KEY_REFUSE_PAP), yesString);
    }
    item = d->advUi.listWidget->item(1); // CHAP
    if (item->checkState() == Qt::Unchecked) {
        data.insert(QLatin1String(NM_SSTP_KEY_REFUSE_CHAP), yesString);
    }
    item = d->advUi.listWidget->item(2); // MSCHAP
    if (item->checkState() == Qt::Unchecked) {
        data.insert(QLatin1String(NM_SSTP_KEY_REFUSE_MSCHAP), yesString);
    }
    item = d->advUi.listWidget->item(3); // MSCHAPv2
    if (item->checkState() == Qt::Unchecked) {
        data.insert(QLatin1String(NM_SSTP_KEY_REFUSE_MSCHAPV2), yesString);
    }
    item = d->advUi.listWidget->item(4); // EAP
    if (item->checkState() == Qt::Unchecked) {
        data.insert(QLatin1String(NM_SSTP_KEY_REFUSE_EAP), yesString);
    }

    // Cryptography and compression
    if (d->advUi.gb_MPPE->isChecked()) {
        int index = d->advUi.cb_MPPECrypto->currentIndex();

        switch (index) {
        case 0:
            data.insert(QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE), yesString);
            break;
        case 1:
            data.insert(QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE_128), yesString);
            break;
        case 2:
            data.insert(QLatin1String(NM_SSTP_KEY_REQUIRE_MPPE_40), yesString);
            break;
        }

        if (d->advUi.cb_statefulEncryption->isChecked()) {
            data.insert(NM_SSTP_KEY_MPPE_STATEFUL, yesString);
        }
    }

    if (!d->advUi.cb_BSD->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_NOBSDCOMP), yesString);
    }

    if (!d->advUi.cb_deflate->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_NODEFLATE), yesString);
    }

    if (!d->advUi.cb_TCPheaders->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_NO_VJ_COMP), yesString);
    }

    // Echo
    if (d->advUi.cb_sendEcho->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_LCP_ECHO_FAILURE), "5");
        data.insert(QLatin1String(NM_SSTP_KEY_LCP_ECHO_INTERVAL), "30");
    }

    if (d->advUi.chk_useCustomUnitNumber->isChecked()) {
        data.insert(QLatin1String(NM_SSTP_KEY_UNIT_NUM), QString::number(d->advUi.sb_customUnitNumber->value()));
    }

    if (!d->advUi.le_address->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_PROXY_SERVER), d->advUi.le_address->text());
    }

    if (d->advUi.sb_port->value() >= 0) {
        data.insert(QLatin1String(NM_SSTP_KEY_PROXY_PORT), QString::number(d->advUi.sb_port->value()));
    }

    if (!d->advUi.le_username->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_PROXY_USER), d->advUi.le_username->text());
    }

    if (!d->advUi.le_password->text().isEmpty()) {
        data.insert(QLatin1String(NM_SSTP_KEY_PROXY_PASSWORD), d->advUi.le_password->text());
    }
    handleOnePasswordType(d->advUi.le_password, NM_SSTP_KEY_PROXY_PASSWORD_FLAGS, data);

    // save it all
    setting.setData(data);
    setting.setSecrets(secretData);

    return setting.toMap();
}

void SstpSettingWidget::doAdvancedDialog()
{
    Q_D(SstpSettingWidget);
    d->advancedDlg->show();
}

void SstpSettingWidget::fillOnePasswordCombo(PasswordField *passwordField, NetworkManager::Setting::SecretFlags type)
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

void SstpSettingWidget::handleOnePasswordType(const PasswordField *passwordField, const QString &key, NMStringMap &data) const
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

bool SstpSettingWidget::isValid() const
{
    Q_D(const SstpSettingWidget);

    return !d->ui.le_gateway->text().isEmpty();
}

#include "moc_sstpwidget.cpp"
