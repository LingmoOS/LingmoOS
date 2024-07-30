/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "openvpnadvancedwidget.h"
#include "nm-openvpn-service.h"
#include "settingwidget.h"
#include "ui_openvpnadvanced.h"

#include <QComboBox>
#include <QStandardPaths>
#include <QUrl>

#include <KAcceleratorManager>
#include <KLocalizedString>
#include <KProcess>

class OpenVpnAdvancedWidget::Private
{
public:
    NetworkManager::VpnSetting::Ptr setting;
    KProcess *openvpnCipherProcess = nullptr;
    KProcess *openvpnVersionProcess = nullptr;
    QByteArray openvpnCiphers;
    QByteArray openVpnVersion;
    bool gotOpenVpnCiphers = false;
    bool gotOpenVpnVersion = false;
    bool readConfig = false;
    int versionX = 0;
    int versionY = 0;
    int versionZ = 0;

    class EnumProxyType
    {
    public:
        enum ProxyType { NotRequired = 0, HTTP = 1, SOCKS = 2 };
    };
    class EnumHashingAlgorithms
    {
    public:
        enum HashingAlgorithms { Default = 0, None, Md4, Md5, Sha1, Sha224, Sha256, Sha384, Sha512, Ripemd160 };
    };
    class EnumCompression
    {
    public:
        enum Compression { None = 0, LZO, LZ4, LZ4v2, Adaptive, Automatic };
    };
};

OpenVpnAdvancedWidget::OpenVpnAdvancedWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::OpenVpnAdvancedWidget)
    , d(new Private)
{
    m_ui->setupUi(this);

    setWindowTitle(i18nc("@title: window advanced openvpn properties", "Advanced OpenVPN properties"));

    d->setting = setting;

    m_ui->proxyPassword->setPasswordOptionsEnabled(true);
    m_ui->proxyPassword->setPasswordNotRequiredEnabled(true);

    connect(m_ui->cbCertCheck, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OpenVpnAdvancedWidget::certCheckTypeChanged);
    connect(m_ui->cmbProxyType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OpenVpnAdvancedWidget::proxyTypeChanged);
    connect(m_ui->cboTLSMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index == 0) {
            m_ui->kurlTlsAuthKey->setDisabled(true);
            m_ui->cboDirection->setDisabled(true);
        } else if (index == 1) { // TLS-Auth
            m_ui->kurlTlsAuthKey->setEnabled(true);
            m_ui->cboDirection->setEnabled(true);
        } else { // TLS-Crypt
            m_ui->kurlTlsAuthKey->setEnabled(true);
            m_ui->cboDirection->setDisabled(true);
        }
    });

    // start openVPN process and get its cipher list
    const QString openVpnBinary = QStandardPaths::findExecutable("openvpn", QStringList{"/sbin", "/usr/sbin"});
    const QStringList ciphersArgs(QLatin1String("--show-ciphers"));
    const QStringList versionArgs(QLatin1String("--version"));

    d->openvpnCipherProcess = new KProcess(this);
    d->openvpnCipherProcess->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    d->openvpnCipherProcess->setReadChannel(QProcess::StandardOutput);
    connect(d->openvpnCipherProcess, &KProcess::errorOccurred, this, &OpenVpnAdvancedWidget::openVpnCipherError);
    connect(d->openvpnCipherProcess, &KProcess::readyReadStandardOutput, this, &OpenVpnAdvancedWidget::gotOpenVpnCipherOutput);
    connect(d->openvpnCipherProcess, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), this, &OpenVpnAdvancedWidget::openVpnCipherFinished);
    d->openvpnCipherProcess->setProgram(openVpnBinary, ciphersArgs);

    d->openvpnVersionProcess = new KProcess(this);
    d->openvpnVersionProcess->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    d->openvpnVersionProcess->setReadChannel(QProcess::StandardOutput);
    connect(d->openvpnVersionProcess, &KProcess::errorOccurred, this, &OpenVpnAdvancedWidget::openVpnVersionError);
    connect(d->openvpnVersionProcess, &KProcess::readyReadStandardOutput, this, &OpenVpnAdvancedWidget::gotOpenVpnVersionOutput);
    connect(d->openvpnVersionProcess, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), this, &OpenVpnAdvancedWidget::openVpnVersionFinished);
    d->openvpnVersionProcess->setProgram(openVpnBinary, versionArgs);

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &OpenVpnAdvancedWidget::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &OpenVpnAdvancedWidget::reject);

    KAcceleratorManager::manage(this);

    if (d->setting) {
        loadConfig();
    }
}

OpenVpnAdvancedWidget::~OpenVpnAdvancedWidget()
{
    delete d;
}

void OpenVpnAdvancedWidget::init()
{
    d->openvpnCipherProcess->start();
    d->openvpnVersionProcess->start();
}

void OpenVpnAdvancedWidget::gotOpenVpnCipherOutput()
{
    d->openvpnCiphers.append(d->openvpnCipherProcess->readAll());
}

void OpenVpnAdvancedWidget::openVpnCipherError(QProcess::ProcessError)
{
    m_ui->cboCipher->removeItem(0);
    m_ui->cboCipher->addItem(i18nc("@item:inlistbox Item added when OpenVPN cipher lookup failed", "OpenVPN cipher lookup failed"));
}

void OpenVpnAdvancedWidget::openVpnCipherFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_ui->cboCipher->removeItem(0);
    if (!exitCode && exitStatus == QProcess::NormalExit) {
        m_ui->cboCipher->addItem(i18nc("@item::inlist Default openvpn cipher item", "Default"));
        const QList<QByteArray> rawOutputLines = d->openvpnCiphers.split('\n');
        bool foundFirstSpace = false;
        for (const QByteArray &cipher : rawOutputLines) {
            if (cipher.length() == 0) {
                foundFirstSpace = true;
            } else if (foundFirstSpace) {
                m_ui->cboCipher->addItem(QString::fromLocal8Bit(cipher.left(cipher.indexOf(' '))));
            }
        }

        if (m_ui->cboCipher->count()) {
            m_ui->cboCipher->setEnabled(true);
        } else {
            m_ui->cboCipher->addItem(i18nc("@item:inlistbox Item added when OpenVPN cipher lookup failed", "No OpenVPN ciphers found"));
        }
    } else {
        m_ui->cboCipher->addItem(i18nc("@item:inlistbox Item added when OpenVPN cipher lookup failed", "OpenVPN cipher lookup failed"));
    }
    delete d->openvpnCipherProcess;
    d->openvpnCipherProcess = nullptr;
    d->openvpnCiphers = QByteArray();
    d->gotOpenVpnCiphers = true;

    if (d->readConfig) {
        const NMStringMap dataMap = d->setting->data();
        if (dataMap.contains(NM_OPENVPN_KEY_CIPHER)) {
            m_ui->cboCipher->setCurrentIndex(m_ui->cboCipher->findText(dataMap.value(NM_OPENVPN_KEY_CIPHER)));
        }
    }
}

void OpenVpnAdvancedWidget::gotOpenVpnVersionOutput()
{
    d->openVpnVersion.append(d->openvpnVersionProcess->readAll());
}

void OpenVpnAdvancedWidget::openVpnVersionError(QProcess::ProcessError)
{
    // We couldn't identify OpenVPN version so disable tls-remote
    disableLegacySubjectMatch();
}

void OpenVpnAdvancedWidget::openVpnVersionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    // OpenVPN returns 1 when you use "--help" and unfortunately returns 1 even when some error occurs
    if (exitCode == 1 && exitStatus == QProcess::NormalExit) {
        QStringList list = QString(d->openVpnVersion).split(QLatin1Char(' '));
        if (list.count() > 2) {
            const QStringList versionList = list.at(1).split(QLatin1Char('.'));
            if (versionList.count() == 3) {
                d->versionX = versionList.at(0).toInt();
                d->versionY = versionList.at(1).toInt();
                d->versionZ = versionList.at(2).toInt();

                if (compareVersion(2, 4, 0) >= 0) {
                    disableLegacySubjectMatch();
                }
            }
        }
    } else {
        disableLegacySubjectMatch();
    }

    delete d->openvpnVersionProcess;
    d->openvpnVersionProcess = nullptr;
    d->openVpnVersion = QByteArray();
    d->gotOpenVpnVersion = true;

    if (d->readConfig) {
        const NMStringMap dataMap = d->setting->data();
        if (dataMap.contains(NM_OPENVPN_KEY_TLS_REMOTE)) {
            m_ui->subjectMatch->setText(dataMap.value(NM_OPENVPN_KEY_TLS_REMOTE));
        }
    }
}

int OpenVpnAdvancedWidget::compareVersion(const int x, const int y, const int z) const
{
    if (d->versionX == 0) {
        // Not valid version
        return -2;
    }

    if (d->versionX > x) {
        return 1;
    } else if (d->versionX < x) {
        return -1;
    } else if (d->versionY > y) {
        return 1;
    } else if (d->versionY < y) {
        return -1;
    } else if (d->versionZ > z) {
        return 1;
    } else if (d->versionZ < z) {
        return -1;
    }
    return 0;
}

void OpenVpnAdvancedWidget::disableLegacySubjectMatch()
{
    m_ui->cbCertCheck->removeItem(CertCheckType::VerifySubjectPartially);
}

void OpenVpnAdvancedWidget::loadConfig()
{
    const NMStringMap dataMap = d->setting->data();
    const NMStringMap secrets = d->setting->secrets();

    // Optional Settings
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PORT))) {
        m_ui->chkCustomPort->setChecked(true);
        m_ui->sbCustomPort->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_PORT)].toUInt());
    } else {
        m_ui->chkCustomPort->setChecked(false);
        m_ui->sbCustomPort->setValue(1194); // Default value
    }
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_TUNNEL_MTU))) {
        m_ui->chkMtu->setChecked(true);
        m_ui->sbMtu->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_TUNNEL_MTU)].toUInt());
    } else {
        m_ui->chkMtu->setChecked(false);
        m_ui->sbMtu->setValue(1500); // Default value
    }
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_FRAGMENT_SIZE))) {
        m_ui->chkCustomFragmentSize->setChecked(true);
        m_ui->sbCustomFragmentSize->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_FRAGMENT_SIZE)].toUInt());
    } else {
        m_ui->chkCustomFragmentSize->setChecked(false);
        m_ui->sbCustomFragmentSize->setValue(1300);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_RENEG_SECONDS))) {
        m_ui->chkUseCustomReneg->setChecked(true);
        m_ui->sbCustomReneg->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_RENEG_SECONDS)].toUInt());
    } else {
        m_ui->chkUseCustomReneg->setChecked(false);
        m_ui->sbCustomReneg->setValue(0);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_COMP_LZO))) {
        const QString compLzo = dataMap[QLatin1String(NM_OPENVPN_KEY_COMP_LZO)];
        if (compLzo == QLatin1String("no-by-default")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::None);
        } else if (compLzo == QLatin1String("yes")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::LZO);
        } else {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::Adaptive);
        }
        m_ui->chkUseCompression->setChecked(true);
    }
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_COMPRESS))) {
        const QString compress = dataMap[QLatin1String(NM_OPENVPN_KEY_COMPRESS)];
        if (compress == QLatin1String("lz4")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::LZ4);
        } else if (compress == QLatin1String("lz4-v2")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::LZ4v2);
        } else if (compress == QLatin1String("lzo")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::LZO);
        } else if (compress == QLatin1String("yes")) {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::Automatic);
        } else {
            m_ui->cmbUseCompression->setCurrentIndex(Private::EnumCompression::Automatic);
        }
        m_ui->chkUseCompression->setChecked(true);
    }
    m_ui->chkUseTCP->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_PROTO_TCP)] == QLatin1String("yes"));
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_DEV_TYPE))) {
        m_ui->chkUseVirtualDeviceType->setChecked(true);
        if (dataMap[QLatin1String(NM_OPENVPN_KEY_DEV_TYPE)] == QLatin1String("tap")) {
            m_ui->cmbDeviceType->setCurrentIndex(1);
        }
    }
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_DEV))) {
        m_ui->chkUseVirtualDeviceName->setChecked(true);
        m_ui->leVirtualDeviceName->setText(dataMap[QLatin1String(NM_OPENVPN_KEY_DEV)]);
    }
    m_ui->chkMssRestrict->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_MSSFIX)] == QLatin1String("yes"));
    m_ui->chkRandRemHosts->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_REMOTE_RANDOM)] == QLatin1String("yes"));

    m_ui->chkIpv6TunLink->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_TUN_IPV6)] == QLatin1String("yes"));

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PING))) {
        m_ui->chkPingInterval->setChecked(true);
        m_ui->sbPingInterval->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_PING)].toInt());
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PING_EXIT)) || dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PING_RESTART))) {
        m_ui->chkSpecifyExitRestartPing->setChecked(true);
        if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PING_EXIT))) {
            m_ui->cbSpecifyExitRestartPing->setCurrentIndex(0); // Exit
            m_ui->sbSpecifyExitRestartPing->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_PING_EXIT)].toInt());
        } else {
            m_ui->cbSpecifyExitRestartPing->setCurrentIndex(1); // Restart
            m_ui->sbSpecifyExitRestartPing->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_PING_RESTART)].toInt());
        }
    }

    m_ui->chkAcceptAuthenticatedPackets->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_FLOAT)] == QLatin1String("yes"));

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_MAX_ROUTES))) {
        m_ui->chkMaxRoutes->setChecked(true);
        m_ui->sbMaxRoutes->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_MAX_ROUTES)].toInt());
    }

    // Optional Security Settings
    const QString hmacKeyAuth = dataMap[QLatin1String(NM_OPENVPN_KEY_AUTH)];
    if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_NONE)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::None);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_RSA_MD4)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Md4);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_MD5)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Md5);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_SHA1)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Sha1);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_SHA224)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Sha224);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_SHA256)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Sha256);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_SHA384)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Sha384);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_SHA512)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Sha512);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_RIPEMD160)) {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Ripemd160);
    } else {
        m_ui->cboHmac->setCurrentIndex(Private::EnumHashingAlgorithms::Default);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_KEYSIZE))) {
        m_ui->chkUseCustomCipherKey->setChecked(true);
        m_ui->sbCustomCipherKey->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_KEYSIZE)].toUInt());
    }

    // ciphers populated above?
    if (d->gotOpenVpnCiphers && dataMap.contains(QLatin1String(NM_OPENVPN_KEY_CIPHER))) {
        m_ui->cboCipher->setCurrentIndex(m_ui->cboCipher->findText(dataMap[QLatin1String(NM_OPENVPN_KEY_CIPHER)]));
    }

    // Optional TLS
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_TLS_REMOTE))) {
        m_ui->cbCertCheck->setCurrentIndex(CertCheckType::VerifySubjectPartially);
        m_ui->subjectMatch->setText(dataMap[QLatin1String(NM_OPENVPN_KEY_TLS_REMOTE)]);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_VERIFY_X509_NAME))) {
        const QString x509Value = dataMap.value(QLatin1String(NM_OPENVPN_KEY_VERIFY_X509_NAME));
        const QStringList x509List = x509Value.split(QLatin1Char(':'));
        if (x509List.size() == 2) {
            if (x509List.at(0) == QLatin1String(NM_OPENVPN_VERIFY_X509_NAME_TYPE_SUBJECT)) {
                m_ui->cbCertCheck->setCurrentIndex(CertCheckType::VerifyWholeSubjectExactly);
            } else if (x509List.at(0) == QLatin1String(NM_OPENVPN_VERIFY_X509_NAME_TYPE_NAME)) {
                m_ui->cbCertCheck->setCurrentIndex(CertCheckType::VerifyNameExactly);
            } else if (x509List.at(0) == QLatin1String(NM_OPENVPN_VERIFY_X509_NAME_TYPE_NAME_PREFIX)) {
                m_ui->cbCertCheck->setCurrentIndex(CertCheckType::VerifyNameByPrefix);
            }
            m_ui->subjectMatch->setText(x509List.at(1));
        }
    } else {
        m_ui->cbCertCheck->setCurrentIndex(CertCheckType::DontVerify);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_REMOTE_CERT_TLS))) {
        const QString remoteCertTls = dataMap[QLatin1String(NM_OPENVPN_KEY_REMOTE_CERT_TLS)];
        m_ui->chkRemoteCertTls->setChecked(true);
        m_ui->labelRemoteCertTls->setEnabled(true);
        m_ui->cmbRemoteCertTls->setEnabled(true);
        m_ui->cmbRemoteCertTls->setCurrentIndex(remoteCertTls == QLatin1String(NM_OPENVPN_REM_CERT_TLS_SERVER) ? 0 : 1);
    }

    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_NS_CERT_TYPE))) {
        const QString remoteCertTls = dataMap[QLatin1String(NM_OPENVPN_KEY_NS_CERT_TYPE)];
        m_ui->chkNsCertType->setChecked(true);
        m_ui->lblNsCertType->setEnabled(true);
        m_ui->cmbNsCertType->setEnabled(true);
        m_ui->cmbNsCertType->setCurrentIndex(remoteCertTls == QLatin1String(NM_OPENVPN_NS_CERT_TYPE_SERVER) ? 0 : 1);
    }

    const QString openvpnKeyTa = dataMap[QLatin1String(NM_OPENVPN_KEY_TA)];
    const QString openvpnKeyTlsCrypt = dataMap[QLatin1String(NM_OPENVPN_KEY_TLS_CRYPT)];

    if (!openvpnKeyTlsCrypt.isEmpty()) {
        m_ui->cboTLSMode->setCurrentIndex(2); // TLS-Crypt
        m_ui->kurlTlsAuthKey->setUrl(QUrl::fromLocalFile(openvpnKeyTlsCrypt));
    } else if (!openvpnKeyTa.isEmpty()) {
        m_ui->cboTLSMode->setCurrentIndex(1); // TLS-Auth
        m_ui->kurlTlsAuthKey->setUrl(QUrl::fromLocalFile(openvpnKeyTa));
        if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_TA_DIR))) {
            const uint tlsAuthDirection = dataMap[QLatin1String(NM_OPENVPN_KEY_TA_DIR)].toUInt();
            m_ui->cboDirection->setCurrentIndex(tlsAuthDirection + 1);
        }
    }

    // Proxies
    if (dataMap[QLatin1String(NM_OPENVPN_KEY_PROXY_TYPE)] == QLatin1String("http")) {
        m_ui->cmbProxyType->setCurrentIndex(Private::EnumProxyType::HTTP);
    } else if (dataMap[QLatin1String(NM_OPENVPN_KEY_PROXY_TYPE)] == QLatin1String("socks")) {
        m_ui->cmbProxyType->setCurrentIndex(Private::EnumProxyType::SOCKS);
    } else {
        m_ui->cmbProxyType->setCurrentIndex(Private::EnumProxyType::NotRequired);
    }
    proxyTypeChanged(m_ui->cmbProxyType->currentIndex());
    m_ui->proxyServerAddress->setText(dataMap[QLatin1String(NM_OPENVPN_KEY_PROXY_SERVER)]);
    if (dataMap.contains(QLatin1String(NM_OPENVPN_KEY_PROXY_PORT))) {
        m_ui->sbProxyPort->setValue(dataMap[QLatin1String(NM_OPENVPN_KEY_PROXY_PORT)].toUInt());
    } else {
        m_ui->sbProxyPort->setValue(0);
    }
    m_ui->chkProxyRetry->setChecked(dataMap[QLatin1String(NM_OPENVPN_KEY_PROXY_RETRY)] == QLatin1String("yes"));
    m_ui->proxyUsername->setText(dataMap[QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_USERNAME)]);
    d->readConfig = true;

    NetworkManager::Setting::SecretFlags type;
    type = (NetworkManager::Setting::SecretFlags)dataMap[NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD "-flags"].toInt();
    if (!(type & NetworkManager::Setting::NotSaved || type & NetworkManager::Setting::NotRequired)) {
        m_ui->proxyPassword->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD)));
    }
    fillOnePasswordCombo(m_ui->proxyPassword, type);
}

void OpenVpnAdvancedWidget::fillOnePasswordCombo(PasswordField *passwordField, NetworkManager::Setting::SecretFlags type)
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

NetworkManager::VpnSetting::Ptr OpenVpnAdvancedWidget::setting() const
{
    NMStringMap data;
    NMStringMap secretData;

    // optional settings
    if (m_ui->chkCustomPort->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_PORT), QString::number(m_ui->sbCustomPort->value()));
    }
    if (m_ui->chkMtu->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_TUNNEL_MTU), QString::number(m_ui->sbMtu->value()));
    }
    if (m_ui->chkCustomFragmentSize->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_FRAGMENT_SIZE), QString::number(m_ui->sbCustomFragmentSize->value()));
    }
    if (m_ui->chkUseCustomReneg->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_RENEG_SECONDS), QString::number(m_ui->sbCustomReneg->value()));
    }
    data.insert(QLatin1String(NM_OPENVPN_KEY_PROTO_TCP), m_ui->chkUseTCP->isChecked() ? QLatin1String("yes") : QLatin1String("no"));

    if (m_ui->chkUseCompression->isChecked()) {
        switch (m_ui->cmbUseCompression->currentIndex()) {
        case Private::EnumCompression::None:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMP_LZO), QLatin1String("no-by-default"));
            break;
        case Private::EnumCompression::LZO:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMPRESS), QLatin1String("lzo"));
            break;
        case Private::EnumCompression::LZ4:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMPRESS), QLatin1String("lz4"));
            break;
        case Private::EnumCompression::LZ4v2:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMPRESS), QLatin1String("lz4-v2"));
            break;
        case Private::EnumCompression::Adaptive:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMP_LZO), QLatin1String("adaptive"));
            break;
        case Private::EnumCompression::Automatic:
            data.insert(QLatin1String(NM_OPENVPN_KEY_COMPRESS), QLatin1String("yes"));
            break;
        }
    }

    if (m_ui->chkUseVirtualDeviceType->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_DEV_TYPE), m_ui->cmbDeviceType->currentIndex() == 0 ? QLatin1String("tun") : QLatin1String("tap"));
    }
    if (m_ui->chkUseVirtualDeviceName->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_DEV), m_ui->leVirtualDeviceName->text());
    }

    data.insert(QLatin1String(NM_OPENVPN_KEY_MSSFIX), m_ui->chkMssRestrict->isChecked() ? QLatin1String("yes") : QLatin1String("no"));
    data.insert(QLatin1String(NM_OPENVPN_KEY_REMOTE_RANDOM), m_ui->chkRandRemHosts->isChecked() ? QLatin1String("yes") : QLatin1String("no"));
    data.insert(QLatin1String(NM_OPENVPN_KEY_TUN_IPV6), m_ui->chkIpv6TunLink->isChecked() ? QLatin1String("yes") : QLatin1String("no"));

    if (m_ui->chkPingInterval->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_PING), QString::number(m_ui->sbPingInterval->value()));
    }

    if (m_ui->chkSpecifyExitRestartPing->isChecked()) {
        if (m_ui->cbSpecifyExitRestartPing->currentIndex() == 0) { // Exit
            data.insert(QLatin1String(NM_OPENVPN_KEY_PING_EXIT), QString::number(m_ui->sbSpecifyExitRestartPing->value()));
        } else { // Restart
            data.insert(QLatin1String(NM_OPENVPN_KEY_PING_RESTART), QString::number(m_ui->sbSpecifyExitRestartPing->value()));
        }
    }

    data.insert(QLatin1String(NM_OPENVPN_KEY_FLOAT), m_ui->chkAcceptAuthenticatedPackets->isChecked() ? QLatin1String("yes") : QLatin1String("no"));

    if (m_ui->chkMaxRoutes->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_MAX_ROUTES), QString::number(m_ui->sbMaxRoutes->value()));
    }

    // Optional Security
    switch (m_ui->cboHmac->currentIndex()) {
    case Private::EnumHashingAlgorithms::Default:
        break;
    case Private::EnumHashingAlgorithms::None:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_NONE));
        break;
    case Private::EnumHashingAlgorithms::Md4:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_RSA_MD4));
        break;
    case Private::EnumHashingAlgorithms::Md5:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_MD5));
        break;
    case Private::EnumHashingAlgorithms::Sha1:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_SHA1));
        break;
    case Private::EnumHashingAlgorithms::Sha224:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_SHA224));
        break;
    case Private::EnumHashingAlgorithms::Sha256:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_SHA256));
        break;
    case Private::EnumHashingAlgorithms::Sha384:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_SHA384));
        break;
    case Private::EnumHashingAlgorithms::Sha512:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_SHA512));
        break;
    case Private::EnumHashingAlgorithms::Ripemd160:
        data.insert(QLatin1String(NM_OPENVPN_KEY_AUTH), QLatin1String(NM_OPENVPN_AUTH_RIPEMD160));
        break;
    }

    if (m_ui->chkUseCustomCipherKey->isChecked()) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_KEYSIZE), QString::number(m_ui->sbCustomCipherKey->value()));
    }

    if (m_ui->cboCipher->currentIndex() != 0) {
        data.insert(QLatin1String(NM_OPENVPN_KEY_CIPHER), m_ui->cboCipher->currentText());
    }

    // optional tls authentication
    switch (m_ui->cbCertCheck->currentIndex()) {
    case CertCheckType::DontVerify:
        break;
    case CertCheckType::VerifyWholeSubjectExactly:
        data.insert(QLatin1String(NM_OPENVPN_KEY_VERIFY_X509_NAME),
                    QStringLiteral("%1:%2").arg(NM_OPENVPN_VERIFY_X509_NAME_TYPE_SUBJECT, m_ui->subjectMatch->text()));
        break;
    case CertCheckType::VerifyNameExactly:
        data.insert(QLatin1String(NM_OPENVPN_KEY_VERIFY_X509_NAME),
                    QStringLiteral("%1:%2").arg(NM_OPENVPN_VERIFY_X509_NAME_TYPE_NAME, m_ui->subjectMatch->text()));
        break;
    case CertCheckType::VerifyNameByPrefix:
        data.insert(QLatin1String(NM_OPENVPN_KEY_VERIFY_X509_NAME),
                    QStringLiteral("%1:%2").arg(NM_OPENVPN_VERIFY_X509_NAME_TYPE_NAME_PREFIX, m_ui->subjectMatch->text()));
        break;
    case CertCheckType::VerifySubjectPartially:
        data.insert(QLatin1String(NM_OPENVPN_KEY_TLS_REMOTE), m_ui->subjectMatch->text());
        break;
    }

    if (m_ui->chkRemoteCertTls->isChecked()) {
        if (m_ui->cmbRemoteCertTls->currentIndex() == 0) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_REMOTE_CERT_TLS), NM_OPENVPN_REM_CERT_TLS_SERVER);
        } else {
            data.insert(QLatin1String(NM_OPENVPN_KEY_REMOTE_CERT_TLS), NM_OPENVPN_REM_CERT_TLS_CLIENT);
        }
    }

    if (m_ui->chkNsCertType->isChecked()) {
        if (m_ui->cmbNsCertType->currentIndex() == 0) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_NS_CERT_TYPE), NM_OPENVPN_NS_CERT_TYPE_SERVER);
        } else {
            data.insert(QLatin1String(NM_OPENVPN_KEY_NS_CERT_TYPE), NM_OPENVPN_NS_CERT_TYPE_CLIENT);
        }
    }

    if (m_ui->cboTLSMode->currentIndex() == 1) { // TLS-Auth
        QUrl tlsAuthKeyUrl = m_ui->kurlTlsAuthKey->url();
        if (!tlsAuthKeyUrl.isEmpty()) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_TA), tlsAuthKeyUrl.path());
        }
        if (m_ui->cboDirection->currentIndex() > 0) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_TA_DIR), QString::number(m_ui->cboDirection->currentIndex() - 1));
        }
    } else if (m_ui->cboTLSMode->currentIndex() == 2) { // TLS-Crypt
        QUrl tlsCryptKeyUrl = m_ui->kurlTlsAuthKey->url();
        if (!tlsCryptKeyUrl.isEmpty()) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_TLS_CRYPT), tlsCryptKeyUrl.path());
        }
    }

    // Proxies
    switch (m_ui->cmbProxyType->currentIndex()) {
    case Private::EnumProxyType::NotRequired:
        break;
    case Private::EnumProxyType::HTTP:
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_TYPE), QLatin1String("http"));
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_SERVER), m_ui->proxyServerAddress->text());
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_PORT), QString::number(m_ui->sbProxyPort->value()));
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_RETRY), m_ui->chkProxyRetry->isChecked() ? QLatin1String("yes") : QLatin1String("no"));
        if (!m_ui->proxyUsername->text().isEmpty()) {
            data.insert(QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_USERNAME), m_ui->proxyUsername->text());
            secretData.insert(QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD), m_ui->proxyPassword->text());
            handleOnePasswordType(m_ui->proxyPassword, QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD "-flags"), data);
        }
        break;
    case Private::EnumProxyType::SOCKS:
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_TYPE), QLatin1String("socks"));
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_SERVER), m_ui->proxyServerAddress->text());
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_PORT), QString::number(m_ui->sbProxyPort->value()));
        data.insert(QLatin1String(NM_OPENVPN_KEY_PROXY_RETRY), m_ui->chkProxyRetry->isChecked() ? QLatin1String("yes") : QLatin1String("no"));
        break;
    }

    d->setting->setData(data);
    d->setting->setSecrets(secretData);

    return d->setting;
}

void OpenVpnAdvancedWidget::certCheckTypeChanged(int type)
{
    if (type == CertCheckType::DontVerify) {
        m_ui->lbSubjectMatch->setEnabled(false);
        m_ui->subjectMatch->setEnabled(false);
    } else {
        m_ui->lbSubjectMatch->setEnabled(true);
        m_ui->subjectMatch->setEnabled(true);
    }
}

void OpenVpnAdvancedWidget::proxyTypeChanged(int type)
{
    switch (type) {
    case Private::EnumProxyType::NotRequired:
        m_ui->proxyServerAddress->setEnabled(false);
        m_ui->sbProxyPort->setEnabled(false);
        m_ui->chkProxyRetry->setEnabled(false);
        m_ui->proxyUsername->setEnabled(false);
        m_ui->proxyPassword->setEnabled(false);
        break;
    case Private::EnumProxyType::HTTP:
        m_ui->proxyServerAddress->setEnabled(true);
        m_ui->sbProxyPort->setEnabled(true);
        m_ui->chkProxyRetry->setEnabled(true);
        m_ui->proxyUsername->setEnabled(true);
        m_ui->proxyPassword->setEnabled(true);
        break;
    case Private::EnumProxyType::SOCKS:
        m_ui->proxyServerAddress->setEnabled(true);
        m_ui->sbProxyPort->setEnabled(true);
        m_ui->chkProxyRetry->setEnabled(true);
        m_ui->proxyUsername->setEnabled(false);
        m_ui->proxyPassword->setEnabled(false);
        break;
    }
}

void OpenVpnAdvancedWidget::handleOnePasswordType(const PasswordField *passwordField, const QString &key, NMStringMap &data) const
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

#include "moc_openvpnadvancedwidget.cpp"
