/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "l2tpipsecwidget.h"
#include "nm-l2tp-service.h"
#include "ui_l2tpipsec.h"
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#include <KAcceleratorManager>
#include <KLocalizedString>

#define DEFAULT_IPSEC_STRONGSWAN_IKELIFETIME_HOURS 3
#define DEFAULT_IPSEC_STRONGSWAN_LIFETIME_HOURS 1

#define DEFAULT_IPSEC_LIBRESWAN_IKELIFETIME_HOURS 1
#define DEFAULT_IPSEC_LIBRESWAN_SALIFETIME_HOURS 8

L2tpIpsecWidget::L2tpIpsecWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::L2tpIpsecWidget)
{
    m_ui->setupUi(this);
    m_ui->machineKeyPassword->setPasswordOptionsEnabled(true);
    m_ui->machineKeyPassword->setPasswordNotRequiredEnabled(true);

    // use requesters' urlSelected signals to set other requester's startDirs to save clicking
    // around the filesystem, also if it is a .p12 file,  set the other URLs to that .p12 file.
    QList<const KUrlRequester *> requesters;
    requesters << m_ui->machineCA << m_ui->machineCert << m_ui->machineKey;
    for (const KUrlRequester *requester : requesters) {
        connect(requester, &KUrlRequester::urlSelected, this, &L2tpIpsecWidget::updateStartDirUrl);
    }

    connect(m_ui->cbIkelifetime, &QCheckBox::toggled, this, &L2tpIpsecWidget::setDefaultIkelifetime);
    connect(m_ui->cbSalifetime, &QCheckBox::toggled, this, &L2tpIpsecWidget::setDefaultSalifetime);
    connect(m_ui->cmbAuthType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &L2tpIpsecWidget::resizeStackedWidget);

    setWindowTitle(i18n("L2TP IPsec Options"));

    KAcceleratorManager::manage(this);

    loadConfig(setting);

    resizeStackedWidget(m_ui->cmbAuthType->currentIndex());
}

L2tpIpsecWidget::~L2tpIpsecWidget()
{
    delete m_ui;
}

void L2tpIpsecWidget::loadConfig(const NetworkManager::VpnSetting::Ptr &setting)
{
    const QString yesString = QLatin1String("yes");
    const QString noString = QLatin1String("no");

    // General settings
    const NMStringMap dataMap = setting->data();

    if (!hasIpsecDaemon()) {
        m_ui->gbEnableTunnelToHost->setChecked(false);
        m_ui->gbEnableTunnelToHost->setDisabled(true);
    } else if (dataMap[NM_L2TP_KEY_IPSEC_ENABLE] == yesString) {
        m_ui->gbEnableTunnelToHost->setChecked(true);

        if (dataMap[NM_L2TP_KEY_MACHINE_AUTH_TYPE].isEmpty() || dataMap[NM_L2TP_KEY_MACHINE_AUTH_TYPE] == QLatin1String(NM_L2TP_AUTHTYPE_PSK)) {
            m_ui->cmbAuthType->setCurrentIndex(AuthType::PSK);
            m_ui->stackedWidget->setCurrentIndex(AuthType::PSK);

            // *SWAN support Base64 encoded PSKs that are prefixed with "0s"
            QString psk = dataMap[NM_L2TP_KEY_IPSEC_PSK];
            if (psk.length() > 2 && psk.startsWith(QLatin1String("0s"))) {
                m_ui->presharedKey->setText(QByteArray::fromBase64(psk.mid(2).toUtf8()));
            } else {
                m_ui->presharedKey->setText(psk);
            }

        } else { // NM_L2TP_AUTHTYPE_TLS
            m_ui->cmbAuthType->setCurrentIndex(AuthType::TLS);
            m_ui->stackedWidget->setCurrentIndex(AuthType::TLS);

            m_ui->machineCA->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_MACHINE_CA]));
            m_ui->machineCert->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_MACHINE_CERT]));
            m_ui->machineKey->setUrl(QUrl::fromLocalFile(dataMap[NM_L2TP_KEY_MACHINE_KEY]));

            const NetworkManager::Setting::SecretFlags machineKeyPassType =
                static_cast<NetworkManager::Setting::SecretFlags>(dataMap[NM_L2TP_KEY_MACHINE_CERTPASS "-flags"].toInt());
            if (machineKeyPassType.testFlag(NetworkManager::Setting::None)) {
                m_ui->machineKeyPassword->setPasswordOption(PasswordField::StoreForAllUsers);
            } else if (machineKeyPassType.testFlag(NetworkManager::Setting::AgentOwned)) {
                m_ui->machineKeyPassword->setPasswordOption(PasswordField::StoreForUser);
            } else if (machineKeyPassType.testFlag(NetworkManager::Setting::NotSaved)) {
                m_ui->machineKeyPassword->setPasswordOption(PasswordField::AlwaysAsk);
            } else if (machineKeyPassType.testFlag(NetworkManager::Setting::NotRequired)) {
                m_ui->machineKeyPassword->setPasswordOption(PasswordField::NotRequired);
            }
        }

        if (!dataMap[NM_L2TP_KEY_IPSEC_GATEWAY_ID].isEmpty()) {
            m_ui->remoteId->setText(dataMap[NM_L2TP_KEY_IPSEC_GATEWAY_ID]);
        } else {
            m_ui->remoteId->setText(dataMap[NM_L2TP_KEY_IPSEC_REMOTE_ID]);
        }
        m_ui->ike->setText(dataMap[NM_L2TP_KEY_IPSEC_IKE]);
        m_ui->esp->setText(dataMap[NM_L2TP_KEY_IPSEC_ESP]);

        if (!dataMap[NM_L2TP_KEY_IPSEC_IKELIFETIME].isEmpty()) {
            const int totalSeconds = dataMap[NM_L2TP_KEY_IPSEC_IKELIFETIME].toInt();
            const int hours = totalSeconds / 3600;
            const int minutes = (totalSeconds % 3600) / 60;
            const int seconds = totalSeconds % 60;
            const QTime lifetime = QTime(hours, minutes, seconds);

            m_ui->ikelifetime->setTime(lifetime);
            m_ui->ikelifetime->setEnabled(true);
            m_ui->cbIkelifetime->setChecked(true);
        } else {
            setDefaultIkelifetime(false);
            m_ui->ikelifetime->setEnabled(false);
            m_ui->cbIkelifetime->setChecked(false);
        }

        if (!dataMap[NM_L2TP_KEY_IPSEC_SALIFETIME].isEmpty()) {
            const int totalSeconds = dataMap[NM_L2TP_KEY_IPSEC_SALIFETIME].toInt();
            const int hours = totalSeconds / 3600;
            const int minutes = (totalSeconds % 3600) / 60;
            const int seconds = totalSeconds % 60;
            const QTime lifetime = QTime(hours, minutes, seconds);

            m_ui->salifetime->setTime(lifetime);
            m_ui->salifetime->setEnabled(true);
            m_ui->cbSalifetime->setChecked(true);
        } else {
            setDefaultSalifetime(false);
            m_ui->salifetime->setEnabled(false);
            m_ui->cbSalifetime->setChecked(false);
        }

        m_ui->cbForceEncaps->setChecked(dataMap[NM_L2TP_KEY_IPSEC_FORCEENCAPS] == yesString);
        m_ui->cbIPComp->setChecked(dataMap[NM_L2TP_KEY_IPSEC_IPCOMP] == yesString);

        if (m_ipsecDaemonType == IpsecDaemonType::Libreswan) {
            m_ui->cbPFS->setChecked(dataMap[NM_L2TP_KEY_IPSEC_PFS] == noString);
        } else {
            m_ui->cbPFS->setChecked(false);
            m_ui->cbPFS->setDisabled(true);
            m_ui->cbPFS->setToolTip("");
        }
    } else {
        m_ui->gbEnableTunnelToHost->setChecked(false);
        setDefaultIkelifetime(false);
        setDefaultSalifetime(false);
        if (m_ipsecDaemonType == IpsecDaemonType::Strongswan) {
            m_ui->cbPFS->setDisabled(true);
            m_ui->cbPFS->setToolTip("");
        }
    }
}

NMStringMap L2tpIpsecWidget::setting() const
{
    NMStringMap result;
    const QString yesString = QLatin1String("yes");
    const QString noString = QLatin1String("no");

    if (m_ui->gbEnableTunnelToHost->isChecked()) {
        result.insert(NM_L2TP_KEY_IPSEC_ENABLE, yesString);

        if (m_ui->cmbAuthType->currentIndex() == AuthType::PSK) {
            // NetworkManager-l2tp < 1.2.12 does not support Base64 PSK
            // For backwards compatibility don't use Base64 PSK for now.
            if (!m_ui->presharedKey->text().isEmpty()) {
                result.insert(NM_L2TP_KEY_IPSEC_PSK, m_ui->presharedKey->text());
                // *SWAN support Base64 encoded PSKs that are prefixed with "0s"
                /*
                QString psk = QLatin1String("0s");
                psk.append(m_ui->presharedKey->text().toUtf8().toBase64());
                result.insert(NM_L2TP_KEY_IPSEC_PSK, psk);
                */
            }
        } else { // AuthType::TLS

            result.insert(NM_L2TP_KEY_MACHINE_AUTH_TYPE, NM_L2TP_AUTHTYPE_TLS);

            result.insert(NM_L2TP_KEY_MACHINE_CA, m_ui->machineCA->url().toLocalFile());
            result.insert(NM_L2TP_KEY_MACHINE_CERT, m_ui->machineCert->url().toLocalFile());
            result.insert(NM_L2TP_KEY_MACHINE_KEY, m_ui->machineKey->url().toLocalFile());

            switch (m_ui->machineKeyPassword->passwordOption()) {
            case PasswordField::StoreForAllUsers:
                result.insert(NM_L2TP_KEY_MACHINE_CERTPASS "-flags", QString::number(NetworkManager::Setting::None));
                break;
            case PasswordField::StoreForUser:
                result.insert(NM_L2TP_KEY_MACHINE_CERTPASS "-flags", QString::number(NetworkManager::Setting::AgentOwned));
                break;
            case PasswordField::AlwaysAsk:
                result.insert(NM_L2TP_KEY_MACHINE_CERTPASS "-flags", QString::number(NetworkManager::Setting::NotSaved));
                break;
            case PasswordField::NotRequired:
                result.insert(NM_L2TP_KEY_MACHINE_CERTPASS "-flags", QString::number(NetworkManager::Setting::NotRequired));
                break;
            };
        }

        // NetworkManager-l2tp 1.2.12 recommends NM_L2TP_KEY_IPSEC_REMOTE_ID
        // instead of deprecated NM_L2TP_KEY_IPSEC_GATEWAY_ID
        // For backwards compatibility use NM_L2TP_KEY_IPSEC_GATEWAY_ID for now.
        if (!m_ui->remoteId->text().isEmpty()) {
            result.insert(NM_L2TP_KEY_IPSEC_GATEWAY_ID, m_ui->remoteId->text());
        }

        if (!m_ui->ike->text().isEmpty()) {
            result.insert(NM_L2TP_KEY_IPSEC_IKE, m_ui->ike->text());
        }

        if (!m_ui->esp->text().isEmpty()) {
            result.insert(NM_L2TP_KEY_IPSEC_ESP, m_ui->esp->text());
        }

        if (m_ui->cbForceEncaps->isChecked()) {
            result.insert(NM_L2TP_KEY_IPSEC_FORCEENCAPS, yesString);
        }

        if (m_ui->cbIkelifetime->isChecked()) {
            const int totalSeconds = m_ui->ikelifetime->time().hour() * 3600 + m_ui->ikelifetime->time().minute() * 60 + m_ui->ikelifetime->time().second();

            result.insert(NM_L2TP_KEY_IPSEC_IKELIFETIME, QString::number(totalSeconds));
        }

        if (m_ui->cbSalifetime->isChecked()) {
            const int totalSeconds = m_ui->salifetime->time().hour() * 3600 + m_ui->salifetime->time().minute() * 60 + m_ui->salifetime->time().second();

            result.insert(NM_L2TP_KEY_IPSEC_SALIFETIME, QString::number(totalSeconds));
        }

        if (m_ui->cbIPComp->isChecked()) {
            result.insert(NM_L2TP_KEY_IPSEC_IPCOMP, yesString);
        }

        if (m_ipsecDaemonType == IpsecDaemonType::Libreswan) {
            if (m_ui->cbPFS->isChecked()) {
                result.insert(NM_L2TP_KEY_IPSEC_PFS, noString);
            }
        }
    }

    return result;
}

NMStringMap L2tpIpsecWidget::secrets() const
{
    NMStringMap result;

    if (m_ui->gbEnableTunnelToHost->isChecked()) {
        if (m_ui->cmbAuthType->currentIndex() == AuthType::TLS) {
            // private key password
            if (!m_ui->machineKeyPassword->text().isEmpty()) {
                result.insert(NM_L2TP_KEY_MACHINE_CERTPASS, m_ui->machineKeyPassword->text());
            }
        }
    }

    return result;
}

void L2tpIpsecWidget::updateStartDirUrl(const QUrl &url)
{
    QList<KUrlRequester *> requesters;
    requesters << m_ui->machineCA << m_ui->machineCert << m_ui->machineKey;
    const bool isP12 = url.toString().endsWith(QLatin1String(".p12"));

    for (KUrlRequester *requester : requesters) {
        requester->setStartDir(url.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash));
        if (isP12) {
            requester->setUrl(url);
        }
    }
}

void L2tpIpsecWidget::setDefaultIkelifetime(bool isChecked)
{
    if (!isChecked) {
        QTime lifetime;
        if (m_ipsecDaemonType == IpsecDaemonType::Libreswan) {
            lifetime = QTime(DEFAULT_IPSEC_LIBRESWAN_IKELIFETIME_HOURS, 0, 0);
        } else {
            lifetime = QTime(DEFAULT_IPSEC_STRONGSWAN_IKELIFETIME_HOURS, 0, 0);
        }
        m_ui->ikelifetime->setTime(lifetime);
    }
}

void L2tpIpsecWidget::setDefaultSalifetime(bool isChecked)
{
    if (!isChecked) {
        QTime lifetime;
        if (m_ipsecDaemonType == IpsecDaemonType::Libreswan) {
            lifetime = QTime(DEFAULT_IPSEC_LIBRESWAN_SALIFETIME_HOURS, 0, 0);
        } else {
            lifetime = QTime(DEFAULT_IPSEC_STRONGSWAN_LIFETIME_HOURS, 0, 0);
        }
        m_ui->salifetime->setTime(lifetime);
    }
}

void L2tpIpsecWidget::resizeStackedWidget(int currentIndex)
{
    m_ui->stackedWidget->setCurrentIndex(currentIndex);
    for (int i = 0; i < m_ui->stackedWidget->count(); ++i) {
        QSizePolicy::Policy policy;

        if (i == currentIndex) {
            policy = QSizePolicy::Preferred;
        } else {
            policy = QSizePolicy::Ignored;
        }
        m_ui->stackedWidget->widget(i)->setSizePolicy(QSizePolicy::Preferred, policy);
    }

    // it takes an event loop iteration before setSizePolicy results in sizeHint being updated
    QTimer::singleShot(0, this, [this] {
        resize(width(), sizeHint().height());
    });
}

L2tpIpsecWidget::IpsecDaemonType L2tpIpsecWidget::m_ipsecDaemonType = IpsecDaemonType::NoIpsecDaemon;

bool L2tpIpsecWidget::hasIpsecDaemon()
{
    // NetworkManager-l2tp currently only supports libreswan and strongswan
    if (m_ipsecDaemonType == IpsecDaemonType::Libreswan || m_ipsecDaemonType == IpsecDaemonType::Strongswan) {
        return true;
    }

    QString ipsecBinary = QStandardPaths::findExecutable(QStringLiteral("ipsec"), QStringList() << QStringLiteral("/sbin") << QStringLiteral("/usr/sbin"));

    // On some Linux distributions, ipsec executable has been renamed strongswan
    if (ipsecBinary.isEmpty()) {
        ipsecBinary = QStandardPaths::findExecutable(QStringLiteral("strongswan"), QStringList() << QStringLiteral("/sbin") << QStringLiteral("/usr/sbin"));
    }

    if (ipsecBinary.isEmpty()) {
        m_ipsecDaemonType = IpsecDaemonType::NoIpsecDaemon;
        return false;
    }

    QProcess ipsecVersionProcess;
    ipsecVersionProcess.setProgram(ipsecBinary);
    ipsecVersionProcess.setArguments(QStringList() << QStringLiteral("--version"));
    ipsecVersionProcess.start();
    ipsecVersionProcess.waitForFinished(-1);

    if (ipsecVersionProcess.exitStatus() == QProcess::NormalExit) {
        QString ipsecStdout = ipsecVersionProcess.readAllStandardOutput();

        if (ipsecStdout.contains("strongSwan", Qt::CaseSensitive)) {
            L2tpIpsecWidget::m_ipsecDaemonType = IpsecDaemonType::Strongswan;
        } else if (ipsecStdout.contains("Libreswan", Qt::CaseSensitive)) {
            L2tpIpsecWidget::m_ipsecDaemonType = IpsecDaemonType::Libreswan;
        } else if (ipsecStdout.contains("Openswan", Qt::CaseSensitive)) {
            L2tpIpsecWidget::m_ipsecDaemonType = IpsecDaemonType::Openswan;
        } else {
            L2tpIpsecWidget::m_ipsecDaemonType = IpsecDaemonType::UnknownIpsecDaemon;
        }
    }

    if (m_ipsecDaemonType == IpsecDaemonType::Libreswan || m_ipsecDaemonType == IpsecDaemonType::Strongswan) {
        return true;
    }
    return false;
}

#include "moc_l2tpipsecwidget.cpp"
