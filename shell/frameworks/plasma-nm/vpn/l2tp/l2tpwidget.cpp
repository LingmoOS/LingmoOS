/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "l2tpwidget.h"
#include "l2tpipsecwidget.h"
#include "l2tppppwidget.h"
#include "nm-l2tp-service.h"
#include "ui_l2tp.h"

#include <NetworkManagerQt/Setting>

#include <QDBusMetaType>
#include <QPointer>

L2tpWidget::L2tpWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(setting, parent, f)
    , m_ui(new Ui::L2tpWidget)
    , m_setting(setting)
{
    qDBusRegisterMetaType<NMStringMap>();

    m_ui->setupUi(this);

    m_ui->password->setPasswordOptionsEnabled(true);
    m_ui->userKeyPassword->setPasswordOptionsEnabled(true);
    m_ui->userKeyPassword->setPasswordNotRequiredEnabled(true);

    // use requesters' urlSelected signals to set other requester's startDirs to save clicking
    // around the filesystem, also if it is a .p12 file,  set the other URLs to that .p12 file.
    QList<const KUrlRequester *> requesters;
    requesters << m_ui->userCA << m_ui->userCert << m_ui->userKey;
    for (const KUrlRequester *requester : std::as_const(requesters)) {
        connect(requester, &KUrlRequester::urlSelected, this, &L2tpWidget::updateStartDirUrl);
    }

    if (L2tpIpsecWidget::hasIpsecDaemon()) {
        connect(m_ui->btnIPSecSettings, &QPushButton::clicked, this, &L2tpWidget::showIpsec);
    } else {
        m_ui->btnIPSecSettings->setDisabled(true);
    }
    connect(m_ui->btnPPPSettings, &QPushButton::clicked, this, &L2tpWidget::showPpp);

    // Connect for setting check
    watchChangedSetting();

    // Connect for validity check
    connect(m_ui->gateway, &QLineEdit::textChanged, this, &L2tpWidget::slotWidgetChanged);

    KAcceleratorManager::manage(this);

    if (setting && !setting->isNull()) {
        loadConfig(setting);
    }
}

L2tpWidget::~L2tpWidget()
{
    m_tmpIpsecSetting.clear();
    m_tmpPppSetting.clear();
    delete m_ui;
}

void L2tpWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);

    const NMStringMap dataMap = m_setting->data();

    m_ui->gateway->setText(dataMap[NM_L2TP_KEY_GATEWAY]);

    if (dataMap[NM_L2TP_KEY_USER_AUTH_TYPE].isEmpty() || dataMap[NM_L2TP_KEY_USER_AUTH_TYPE] == QLatin1String(NM_L2TP_AUTHTYPE_PASSWORD)) {
        m_ui->cmbAuthType->setCurrentIndex(AuthType::Password);
        m_ui->stackedWidget->setCurrentIndex(AuthType::Password);
        m_ui->username->setText(dataMap[NM_L2TP_KEY_USER]);

        const NetworkManager::Setting::SecretFlags userPassType =
            static_cast<NetworkManager::Setting::SecretFlags>(dataMap[NM_L2TP_KEY_PASSWORD "-flags"].toInt());
        if (userPassType.testFlag(NetworkManager::Setting::None)) {
            m_ui->password->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (userPassType.testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->password->setPasswordOption(PasswordField::StoreForUser);
        } else {
            m_ui->password->setPasswordOption(PasswordField::AlwaysAsk);
        }

        m_ui->domain->setText(dataMap[NM_L2TP_KEY_DOMAIN]);

    } else { // NM_L2TP_AUTHTYPE_TLS
        m_ui->cmbAuthType->setCurrentIndex(AuthType::TLS);
        m_ui->stackedWidget->setCurrentIndex(AuthType::TLS);

        m_ui->userCA->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_USER_CA]));
        m_ui->userCert->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_USER_CERT]));
        m_ui->userKey->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_USER_KEY]));

        const NetworkManager::Setting::SecretFlags userKeyPassType =
            static_cast<NetworkManager::Setting::SecretFlags>(dataMap[NM_L2TP_KEY_USER_CERTPASS "-flags"].toInt());
        if (userKeyPassType.testFlag(NetworkManager::Setting::None)) {
            m_ui->userKeyPassword->setPasswordOption(PasswordField::StoreForAllUsers);
        } else if (userKeyPassType.testFlag(NetworkManager::Setting::AgentOwned)) {
            m_ui->userKeyPassword->setPasswordOption(PasswordField::StoreForUser);
        } else if (userKeyPassType.testFlag(NetworkManager::Setting::NotSaved)) {
            m_ui->userKeyPassword->setPasswordOption(PasswordField::AlwaysAsk);
        } else if (userKeyPassType.testFlag(NetworkManager::Setting::NotRequired)) {
            m_ui->userKeyPassword->setPasswordOption(PasswordField::NotRequired);
        }
    }

    loadSecrets(setting);
}

void L2tpWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::VpnSetting::Ptr vpnSetting = setting.staticCast<NetworkManager::VpnSetting>();

    if (vpnSetting) {
        const QString authType = m_setting->data().value(NM_L2TP_KEY_USER_AUTH_TYPE);
        const NMStringMap secrets = vpnSetting->secrets();

        if (authType == QLatin1String(NM_L2TP_AUTHTYPE_TLS)) {
            m_ui->userKeyPassword->setText(secrets.value(NM_L2TP_KEY_USER_CERTPASS));
        } else { // NM_L2TP_AUTHTYPE_PASSWORD
            m_ui->password->setText(secrets.value(NM_L2TP_KEY_PASSWORD));
        }
    }
}

QVariantMap L2tpWidget::setting() const
{
    NetworkManager::VpnSetting setting;
    setting.setServiceType(QLatin1String(NM_DBUS_SERVICE_L2TP));
    NMStringMap data;
    NMStringMap secrets;

    if (!m_tmpIpsecSetting.isNull()) {
        data = m_tmpIpsecSetting->data();
        secrets = m_tmpIpsecSetting->secrets();
    } else {
        // retrieve the settings if the dialog has not been opened
        QScopedPointer<L2tpIpsecWidget> ipsec(new L2tpIpsecWidget(m_setting, nullptr));
        data = ipsec->setting();
        secrets = ipsec->secrets();
    }

    if (!m_tmpPppSetting.isNull()) {
        data.insert(m_tmpPppSetting->data());
    } else {
        const bool need_peer_eap = m_ui->cmbAuthType->currentIndex() != AuthType::Password;

        // retrieve the settings if the dialog has not been opened
        QScopedPointer<L2tpPPPWidget> ppp(new L2tpPPPWidget(m_setting, nullptr, need_peer_eap));
        data.insert(ppp->setting());
    }

    if (!m_ui->gateway->text().isEmpty()) {
        data.insert(NM_L2TP_KEY_GATEWAY, m_ui->gateway->text());
    }

    if (m_ui->cmbAuthType->currentIndex() == AuthType::Password) {
        if (!m_ui->username->text().isEmpty()) {
            data.insert(NM_L2TP_KEY_USER, m_ui->username->text());
        }

        if (!m_ui->password->text().isEmpty()) {
            secrets.insert(NM_L2TP_KEY_PASSWORD, m_ui->password->text());
        } else {
            secrets.remove(NM_L2TP_KEY_PASSWORD);
        }

        switch (m_ui->password->passwordOption()) {
        case PasswordField::StoreForAllUsers:
            data.insert(NM_L2TP_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::None));
            break;
        case PasswordField::StoreForUser:
            data.insert(NM_L2TP_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::AgentOwned));
            break;
        default:
            data.insert(NM_L2TP_KEY_PASSWORD "-flags", QString::number(NetworkManager::Setting::NotSaved));
        };

        if (!m_ui->domain->text().isEmpty()) {
            data.insert(NM_L2TP_KEY_DOMAIN, m_ui->domain->text());
        }

    } else { // EnumAuthType::TLS

        data.insert(NM_L2TP_KEY_USER_AUTH_TYPE, NM_L2TP_AUTHTYPE_TLS);

        data.insert(NM_L2TP_KEY_USER_CA, m_ui->userCA->url().toLocalFile());
        data.insert(NM_L2TP_KEY_USER_CERT, m_ui->userCert->url().toLocalFile());
        data.insert(NM_L2TP_KEY_USER_KEY, m_ui->userKey->url().toLocalFile());

        // key password
        if (!m_ui->userKeyPassword->text().isEmpty()) {
            secrets.insert(NM_L2TP_KEY_USER_CERTPASS, m_ui->userKeyPassword->text());
        } else {
            secrets.remove(NM_L2TP_KEY_USER_CERTPASS);
        }

        switch (m_ui->userKeyPassword->passwordOption()) {
        case PasswordField::StoreForAllUsers:
            data.insert(NM_L2TP_KEY_USER_CERTPASS "-flags", QString::number(NetworkManager::Setting::None));
            break;
        case PasswordField::StoreForUser:
            data.insert(NM_L2TP_KEY_USER_CERTPASS "-flags", QString::number(NetworkManager::Setting::AgentOwned));
            break;
        case PasswordField::AlwaysAsk:
            data.insert(NM_L2TP_KEY_USER_CERTPASS "-flags", QString::number(NetworkManager::Setting::NotSaved));
            break;
        case PasswordField::NotRequired:
            data.insert(NM_L2TP_KEY_USER_CERTPASS "-flags", QString::number(NetworkManager::Setting::NotRequired));
            break;
        };
    }

    setting.setData(data);
    setting.setSecrets(secrets);
    return setting.toMap();
}

void L2tpWidget::updateStartDirUrl(const QUrl &url)
{
    QList<KUrlRequester *> requesters;
    requesters << m_ui->userCA << m_ui->userCert << m_ui->userKey;
    bool isP12 = url.toString().endsWith(QLatin1String(".p12"));

    for (KUrlRequester *requester : std::as_const(requesters)) {
        requester->setStartDir(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash));
        if (isP12) {
            requester->setUrl(url);
        }
    }
}

void L2tpWidget::showIpsec()
{
    QPointer<L2tpIpsecWidget> ipsec;
    if (m_tmpIpsecSetting.isNull()) {
        ipsec = new L2tpIpsecWidget(m_setting, this);
    } else {
        ipsec = new L2tpIpsecWidget(m_tmpIpsecSetting, this);
    }
    ipsec->setAttribute(Qt::WA_DeleteOnClose);
    connect(ipsec.data(), &L2tpIpsecWidget::accepted, [ipsec, this]() {
        NMStringMap ipsecData = ipsec->setting();
        if (!ipsecData.isEmpty()) {
            if (m_tmpIpsecSetting.isNull()) {
                m_tmpIpsecSetting = NetworkManager::VpnSetting::Ptr(new NetworkManager::VpnSetting);
            }
            m_tmpIpsecSetting->setData(ipsecData);
        }
    });
    ipsec->setModal(true);
    ipsec->show();
}

void L2tpWidget::showPpp()
{
    QPointer<L2tpPPPWidget> ipsec;
    bool need_peer_eap = m_ui->cmbAuthType->currentIndex() != AuthType::Password;
    if (m_tmpPppSetting.isNull()) {
        ipsec = new L2tpPPPWidget(m_setting, this, need_peer_eap);
    } else {
        ipsec = new L2tpPPPWidget(m_tmpPppSetting, this, need_peer_eap);
    }
    ipsec->setAttribute(Qt::WA_DeleteOnClose);
    connect(ipsec.data(), &L2tpPPPWidget::accepted, [ipsec, this]() {
        NMStringMap ipsecData = ipsec->setting();
        if (!ipsecData.isEmpty()) {
            if (m_tmpPppSetting.isNull()) {
                m_tmpPppSetting = NetworkManager::VpnSetting::Ptr(new NetworkManager::VpnSetting);
            }
            m_tmpPppSetting->setData(ipsecData);
        }
    });
    ipsec->setModal(true);
    ipsec->show();
}

bool L2tpWidget::isValid() const
{
    return !m_ui->gateway->text().isEmpty();
}

#include "moc_l2tpwidget.cpp"
