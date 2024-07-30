/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "security8021xsetting.h"
#include "security8021xsetting_p.h"

#include <QDebug>

NetworkManager::Security8021xSettingPrivate::Security8021xSettingPrivate()
    : name(NM_SETTING_802_1X_SETTING_NAME)
    , phase1PeapVer(Security8021xSetting::PeapVersionUnknown)
    , phase1PeapLabel(Security8021xSetting::PeapLabelUnknown)
    , phase1FastProvisioning(Security8021xSetting::FastProvisioningUnknown)
    , phase2AuthMethod(Security8021xSetting::AuthMethodUnknown)
    , phase2AuthEapMethod(Security8021xSetting::AuthEapMethodUnknown)
    , passwordFlags(NetworkManager::Setting::None)
    , passwordRawFlags(NetworkManager::Setting::None)
    , privateKeyPasswordFlags(NetworkManager::Setting::None)
    , phase2PrivateKeyPasswordFlags(NetworkManager::Setting::None)
    , pinFlags(NetworkManager::Setting::None)
    , systemCaCerts(false)
{
}

NetworkManager::Security8021xSetting::Security8021xSetting()
    : Setting(Setting::Security8021x)
    , d_ptr(new Security8021xSettingPrivate())
{
}

NetworkManager::Security8021xSetting::Security8021xSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new Security8021xSettingPrivate())
{
    setEapMethods(other->eapMethods());
    setIdentity(other->identity());
    setDomainSuffixMatch(other->domainSuffixMatch());
    setAnonymousIdentity(other->anonymousIdentity());
    setPacFile(other->pacFile());
    setCaCertificate(other->caCertificate());
    setCaPath(other->caPath());
    setSubjectMatch(other->subjectMatch());
    setAltSubjectMatches(other->altSubjectMatches());
    setClientCertificate(other->clientCertificate());
    setPhase1PeapVersion(other->phase1PeapVersion());
    setPhase1PeapLabel(other->phase1PeapLabel());
    setPhase1FastProvisioning(other->phase1FastProvisioning());
    setPhase2AuthMethod(other->phase2AuthMethod());
    setPhase2AuthEapMethod(other->phase2AuthEapMethod());
    setPhase2CaCertificate(other->phase2CaCertificate());
    setPhase2CaPath(other->phase2CaPath());
    setPhase2SubjectMatch(other->phase2SubjectMatch());
    setPhase2AltSubjectMatches(other->phase2AltSubjectMatches());
    setPassword(other->password());
    setPasswordFlags(other->passwordFlags());
    setPasswordRaw(other->passwordRaw());
    setPasswordRawFlags(other->passwordRawFlags());
    setPrivateKey(other->privateKey());
    setPrivateKeyPassword(other->privateKeyPassword());
    setPrivateKeyPasswordFlags(other->privateKeyPasswordFlags());
    setPhase2PrivateKey(other->phase2PrivateKey());
    setPhase2PrivateKeyPassword(other->phase2PrivateKeyPassword());
    setPhase2PrivateKeyPasswordFlags(other->phase2PrivateKeyPasswordFlags());
    setSystemCaCertificates(other->systemCaCertificates());
}

NetworkManager::Security8021xSetting::~Security8021xSetting()
{
    delete d_ptr;
}

QString NetworkManager::Security8021xSetting::name() const
{
    Q_D(const Security8021xSetting);

    return d->name;
}

void NetworkManager::Security8021xSetting::setEapMethods(const QList<NetworkManager::Security8021xSetting::EapMethod> &methods)
{
    Q_D(Security8021xSetting);

    d->eap = methods;
}

QList<NetworkManager::Security8021xSetting::EapMethod> NetworkManager::Security8021xSetting::eapMethods() const
{
    Q_D(const Security8021xSetting);

    return d->eap;
}

void NetworkManager::Security8021xSetting::setIdentity(const QString &identity)
{
    Q_D(Security8021xSetting);

    d->identity = identity;
}

QString NetworkManager::Security8021xSetting::identity() const
{
    Q_D(const Security8021xSetting);

    return d->identity;
}

void NetworkManager::Security8021xSetting::setAnonymousIdentity(const QString &identity)
{
    Q_D(Security8021xSetting);

    d->anonymousIdentity = identity;
}

QString NetworkManager::Security8021xSetting::anonymousIdentity() const
{
    Q_D(const Security8021xSetting);

    return d->anonymousIdentity;
}

void NetworkManager::Security8021xSetting::setDomainSuffixMatch(const QString &domain)
{
    Q_D(Security8021xSetting);

    d->domainSuffixMatch = domain;
}

QString NetworkManager::Security8021xSetting::domainSuffixMatch() const
{
    Q_D(const Security8021xSetting);

    return d->domainSuffixMatch;
}

void NetworkManager::Security8021xSetting::setPacFile(const QString &filePath)
{
    Q_D(Security8021xSetting);

    d->pacFile = filePath;
}

QString NetworkManager::Security8021xSetting::pacFile() const
{
    Q_D(const Security8021xSetting);

    return d->pacFile;
}

void NetworkManager::Security8021xSetting::setCaCertificate(const QByteArray &certificate)
{
    Q_D(Security8021xSetting);

    d->caCert = certificate;
}

QByteArray NetworkManager::Security8021xSetting::caCertificate() const
{
    Q_D(const Security8021xSetting);

    return d->caCert;
}

void NetworkManager::Security8021xSetting::setCaPath(const QString &path)
{
    Q_D(Security8021xSetting);

    d->caPath = path;
}

QString NetworkManager::Security8021xSetting::caPath() const
{
    Q_D(const Security8021xSetting);

    return d->caPath;
}

void NetworkManager::Security8021xSetting::setSubjectMatch(const QString &substring)
{
    Q_D(Security8021xSetting);

    d->subjectMatch = substring;
}

QString NetworkManager::Security8021xSetting::subjectMatch() const
{
    Q_D(const Security8021xSetting);

    return d->subjectMatch;
}

void NetworkManager::Security8021xSetting::setAltSubjectMatches(const QStringList &strings)
{
    Q_D(Security8021xSetting);

    d->altSubjectMatches = strings;
}

QStringList NetworkManager::Security8021xSetting::altSubjectMatches() const
{
    Q_D(const Security8021xSetting);

    return d->altSubjectMatches;
}

void NetworkManager::Security8021xSetting::setClientCertificate(const QByteArray &certificate)
{
    Q_D(Security8021xSetting);

    d->clientCert = certificate;
}

QByteArray NetworkManager::Security8021xSetting::clientCertificate() const
{
    Q_D(const Security8021xSetting);

    return d->clientCert;
}

void NetworkManager::Security8021xSetting::setPhase1PeapVersion(NetworkManager::Security8021xSetting::PeapVersion version)
{
    Q_D(Security8021xSetting);

    d->phase1PeapVer = version;
}

NetworkManager::Security8021xSetting::PeapVersion NetworkManager::Security8021xSetting::phase1PeapVersion() const
{
    Q_D(const Security8021xSetting);

    return d->phase1PeapVer;
}

void NetworkManager::Security8021xSetting::setPhase1PeapLabel(NetworkManager::Security8021xSetting::PeapLabel label)
{
    Q_D(Security8021xSetting);

    d->phase1PeapLabel = label;
}

NetworkManager::Security8021xSetting::PeapLabel NetworkManager::Security8021xSetting::phase1PeapLabel() const
{
    Q_D(const Security8021xSetting);

    return d->phase1PeapLabel;
}

void NetworkManager::Security8021xSetting::setPhase1FastProvisioning(NetworkManager::Security8021xSetting::FastProvisioning provisioning)
{
    Q_D(Security8021xSetting);

    d->phase1FastProvisioning = provisioning;
}

NetworkManager::Security8021xSetting::FastProvisioning NetworkManager::Security8021xSetting::phase1FastProvisioning() const
{
    Q_D(const Security8021xSetting);

    return d->phase1FastProvisioning;
}

void NetworkManager::Security8021xSetting::setPhase2AuthMethod(NetworkManager::Security8021xSetting::AuthMethod method)
{
    Q_D(Security8021xSetting);

    d->phase2AuthMethod = method;
}

NetworkManager::Security8021xSetting::AuthMethod NetworkManager::Security8021xSetting::phase2AuthMethod() const
{
    Q_D(const Security8021xSetting);

    return d->phase2AuthMethod;
}

void NetworkManager::Security8021xSetting::setPhase2AuthEapMethod(NetworkManager::Security8021xSetting::AuthEapMethod method)
{
    Q_D(Security8021xSetting);

    d->phase2AuthEapMethod = method;
}

NetworkManager::Security8021xSetting::AuthEapMethod NetworkManager::Security8021xSetting::phase2AuthEapMethod() const
{
    Q_D(const Security8021xSetting);

    return d->phase2AuthEapMethod;
}

void NetworkManager::Security8021xSetting::setPhase2CaCertificate(const QByteArray &certificate)
{
    Q_D(Security8021xSetting);

    d->phase2CaCert = certificate;
}

QByteArray NetworkManager::Security8021xSetting::phase2CaCertificate() const
{
    Q_D(const Security8021xSetting);

    return d->phase2CaCert;
}

void NetworkManager::Security8021xSetting::setPhase2CaPath(const QString &path)
{
    Q_D(Security8021xSetting);

    d->phase2CaPath = path;
}

QString NetworkManager::Security8021xSetting::phase2CaPath() const
{
    Q_D(const Security8021xSetting);

    return d->phase2CaPath;
}

void NetworkManager::Security8021xSetting::setPhase2SubjectMatch(const QString &substring)
{
    Q_D(Security8021xSetting);

    d->phase2SubjectMatch = substring;
}

QString NetworkManager::Security8021xSetting::phase2SubjectMatch() const
{
    Q_D(const Security8021xSetting);

    return d->phase2SubjectMatch;
}

void NetworkManager::Security8021xSetting::setPhase2AltSubjectMatches(const QStringList &strings)
{
    Q_D(Security8021xSetting);

    d->phase2AltSubjectMatches = strings;
}

QStringList NetworkManager::Security8021xSetting::phase2AltSubjectMatches() const
{
    Q_D(const Security8021xSetting);

    return d->phase2AltSubjectMatches;
}

void NetworkManager::Security8021xSetting::setPhase2ClientCertificate(const QByteArray &certificate)
{
    Q_D(Security8021xSetting);

    d->phase2ClientCert = certificate;
}

QByteArray NetworkManager::Security8021xSetting::phase2ClientCertificate() const
{
    Q_D(const Security8021xSetting);

    return d->phase2ClientCert;
}

void NetworkManager::Security8021xSetting::setPassword(const QString &password)
{
    Q_D(Security8021xSetting);

    d->password = password;
}

QString NetworkManager::Security8021xSetting::password() const
{
    Q_D(const Security8021xSetting);

    return d->password;
}

void NetworkManager::Security8021xSetting::setPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(Security8021xSetting);

    d->passwordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::Security8021xSetting::passwordFlags() const
{
    Q_D(const Security8021xSetting);

    return d->passwordFlags;
}

void NetworkManager::Security8021xSetting::setPasswordRaw(const QByteArray &password)
{
    Q_D(Security8021xSetting);

    d->passwordRaw = password;
}

QByteArray NetworkManager::Security8021xSetting::passwordRaw() const
{
    Q_D(const Security8021xSetting);

    return d->passwordRaw;
}

void NetworkManager::Security8021xSetting::setPasswordRawFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(Security8021xSetting);

    d->passwordRawFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::Security8021xSetting::passwordRawFlags() const
{
    Q_D(const Security8021xSetting);

    return d->passwordRawFlags;
}

void NetworkManager::Security8021xSetting::setPrivateKey(const QByteArray &key)
{
    Q_D(Security8021xSetting);

    d->privateKey = key;
}

QByteArray NetworkManager::Security8021xSetting::privateKey() const
{
    Q_D(const Security8021xSetting);

    return d->privateKey;
}

void NetworkManager::Security8021xSetting::setPrivateKeyPassword(const QString &password)
{
    Q_D(Security8021xSetting);

    d->privateKeyPassword = password;
}

QString NetworkManager::Security8021xSetting::privateKeyPassword() const
{
    Q_D(const Security8021xSetting);

    return d->privateKeyPassword;
}

void NetworkManager::Security8021xSetting::setPrivateKeyPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(Security8021xSetting);

    d->privateKeyPasswordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::Security8021xSetting::privateKeyPasswordFlags() const
{
    Q_D(const Security8021xSetting);

    return d->privateKeyPasswordFlags;
}

void NetworkManager::Security8021xSetting::setPhase2PrivateKey(const QByteArray &key)
{
    Q_D(Security8021xSetting);

    d->phase2PrivateKey = key;
}

QByteArray NetworkManager::Security8021xSetting::phase2PrivateKey() const
{
    Q_D(const Security8021xSetting);

    return d->phase2PrivateKey;
}

void NetworkManager::Security8021xSetting::setPhase2PrivateKeyPassword(const QString &password)
{
    Q_D(Security8021xSetting);

    d->phase2PrivateKeyPassword = password;
}

QString NetworkManager::Security8021xSetting::phase2PrivateKeyPassword() const
{
    Q_D(const Security8021xSetting);

    return d->phase2PrivateKeyPassword;
}

void NetworkManager::Security8021xSetting::setPhase2PrivateKeyPasswordFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(Security8021xSetting);

    d->phase2PrivateKeyPasswordFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::Security8021xSetting::phase2PrivateKeyPasswordFlags() const
{
    Q_D(const Security8021xSetting);

    return d->phase2PrivateKeyPasswordFlags;
}

void NetworkManager::Security8021xSetting::setSystemCaCertificates(bool use)
{
    Q_D(Security8021xSetting);

    d->systemCaCerts = use;
}

void NetworkManager::Security8021xSetting::setPin(const QString &pin)
{
    Q_D(Security8021xSetting);

    d->pin = pin;
}

QString NetworkManager::Security8021xSetting::pin() const
{
    Q_D(const Security8021xSetting);

    return d->pin;
}

void NetworkManager::Security8021xSetting::setPinFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(Security8021xSetting);

    d->pinFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::Security8021xSetting::pinFlags() const
{
    Q_D(const Security8021xSetting);

    return d->pinFlags;
}

bool NetworkManager::Security8021xSetting::systemCaCertificates() const
{
    Q_D(const Security8021xSetting);

    return d->systemCaCerts;
}

QStringList NetworkManager::Security8021xSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    /* clang-format off */
    if (eapMethods().contains(EapMethodTls)
        && (privateKeyPassword().isEmpty() || requestNew)
        && !privateKeyPasswordFlags().testFlag(NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD);
    } else if ((eapMethods().contains(EapMethodTtls)
                    || eapMethods().contains(EapMethodPeap)
                    || eapMethods().contains(EapMethodLeap)
                    || eapMethods().contains(EapMethodFast)
                    || eapMethods().contains(EapMethodPwd))
               && (password().isEmpty() || requestNew)
               && !passwordFlags().testFlag(NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PASSWORD);
        secrets << QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW);
    } else if (eapMethods().contains(EapMethodSim)
               && (pin().isEmpty() || requestNew)
               && !pinFlags().testFlag(NotRequired)) { /* clang-format on */
        secrets << QLatin1String(NM_SETTING_802_1X_PIN);
    }

    /* clang-format off */
    if ((phase2AuthMethod() == AuthMethodTls || phase2AuthEapMethod() == AuthEapMethodTls)
        && (phase2PrivateKeyPassword().isEmpty() || requestNew)
        && !phase2PrivateKeyPasswordFlags().testFlag(NotRequired)) { /* clang-format on */
        secrets << QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD);
    }

    return secrets;
}

void NetworkManager::Security8021xSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD))) {
        setPassword(secrets.value(QLatin1String(NM_SETTING_802_1X_PASSWORD)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW))) {
        setPasswordRaw(secrets.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW)).toByteArray());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD))) {
        setPrivateKeyPassword(secrets.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD))) {
        setPhase2PrivateKeyPassword(secrets.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (secrets.contains(QLatin1String(NM_SETTING_802_1X_PIN))) {
        setPin(secrets.value(QLatin1String(NM_SETTING_802_1X_PIN)).toString());
    }
}

QVariantMap NetworkManager::Security8021xSetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!password().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD), password());
    }

    if (!passwordRaw().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW), passwordRaw());
    }

    if (!privateKeyPassword().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD), privateKeyPassword());
    }

    if (!phase2PrivateKeyPassword().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD), phase2PrivateKeyPassword());
    }

    if (!pin().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_802_1X_PIN), pin());
    }

    return secrets;
}

void NetworkManager::Security8021xSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_802_1X_EAP))) {
        const QStringList methods = setting.value(QLatin1String(NM_SETTING_802_1X_EAP)).toStringList();
        QList<EapMethod> eapMethods;
        for (const QString &method : methods) {
            if (method == "leap") {
                eapMethods << EapMethodLeap;
            } else if (method == "md5") {
                eapMethods << EapMethodMd5;
            } else if (method == "tls") {
                eapMethods << EapMethodTls;
            } else if (method == "peap") {
                eapMethods << EapMethodPeap;
            } else if (method == "ttls") {
                eapMethods << EapMethodTtls;
            } else if (method == "sim") {
                eapMethods << EapMethodSim;
            } else if (method == "fast") {
                eapMethods << EapMethodFast;
            } else if (method == "pwd") {
                eapMethods << EapMethodPwd;
            }
        }

        setEapMethods(eapMethods);
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_IDENTITY))) {
        setIdentity(setting.value(QLatin1String(NM_SETTING_802_1X_IDENTITY)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY))) {
        setAnonymousIdentity(setting.value(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_DOMAIN_SUFFIX_MATCH))) {
        setDomainSuffixMatch(setting.value(QLatin1String(NM_SETTING_802_1X_DOMAIN_SUFFIX_MATCH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PAC_FILE))) {
        setPacFile(setting.value(QLatin1String(NM_SETTING_802_1X_PAC_FILE)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_CA_CERT))) {
        setCaCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_CA_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_CA_PATH))) {
        setCaPath(setting.value(QLatin1String(NM_SETTING_802_1X_CA_PATH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_SUBJECT_MATCH))) {
        setSubjectMatch(setting.value(QLatin1String(NM_SETTING_802_1X_SUBJECT_MATCH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES))) {
        setAltSubjectMatches(setting.value(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT))) {
        setClientCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER))) {
        const QString version = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER)).toString();

        if (version == "0") {
            setPhase1PeapVersion(PeapVersionZero);
        } else if (version == "1") {
            setPhase1PeapVersion(PeapVersionOne);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL))) {
        const QString label = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL)).toString();

        if (label == "1") {
            setPhase1PeapLabel(PeapLabelForce);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING))) {
        const QString provisioning = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING)).toString();

        if (provisioning == "0") {
            setPhase1FastProvisioning(FastProvisioningDisabled);
        } else if (provisioning == "1") {
            setPhase1FastProvisioning(FastProvisioningAllowUnauthenticated);
        } else if (provisioning == "2") {
            setPhase1FastProvisioning(FastProvisioningAllowAuthenticated);
        } else if (provisioning == "3") {
            setPhase1FastProvisioning(FastProvisioningAllowBoth);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH))) {
        const QString authMethod = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH)).toString();

        if (authMethod == "pap") {
            setPhase2AuthMethod(AuthMethodPap);
        } else if (authMethod == "chap") {
            setPhase2AuthMethod(AuthMethodChap);
        } else if (authMethod == "mschap") {
            setPhase2AuthMethod(AuthMethodMschap);
        } else if (authMethod == "mschapv2") {
            setPhase2AuthMethod(AuthMethodMschapv2);
        } else if (authMethod == "gtc") {
            setPhase2AuthMethod(AuthMethodGtc);
        } else if (authMethod == "otp") {
            setPhase2AuthMethod(AuthMethodOtp);
        } else if (authMethod == "md5") {
            setPhase2AuthMethod(AuthMethodMd5);
        } else if (authMethod == "tls") {
            setPhase2AuthMethod(AuthMethodTls);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP))) {
        const QString authEapMethod = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP)).toString();

        if (authEapMethod == "md5") {
            setPhase2AuthEapMethod(AuthEapMethodMd5);
        } else if (authEapMethod == "mschapv2") {
            setPhase2AuthEapMethod(AuthEapMethodMschapv2);
        } else if (authEapMethod == "otp") {
            setPhase2AuthEapMethod(AuthEapMethodOtp);
        } else if (authEapMethod == "gtc") {
            setPhase2AuthEapMethod(AuthEapMethodGtc);
        } else if (authEapMethod == "tls") {
            setPhase2AuthEapMethod(AuthEapMethodTls);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT))) {
        setPhase2CaCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH))) {
        setPhase2CaPath(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_SUBJECT_MATCH))) {
        setPhase2SubjectMatch(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_SUBJECT_MATCH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_ALTSUBJECT_MATCHES))) {
        setPhase2AltSubjectMatches(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_ALTSUBJECT_MATCHES)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT))) {
        setPhase2ClientCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD))) {
        setPassword(setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS))) {
        setPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW))) {
        setPasswordRaw(setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS))) {
        setPasswordRawFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY))) {
        setPrivateKey(setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD))) {
        setPrivateKeyPassword(setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS))) {
        setPrivateKeyPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY))) {
        setPhase2PrivateKey(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD))) {
        setPhase2PrivateKeyPassword(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS))) {
        setPhase2PrivateKeyPasswordFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PIN))) {
        setPin(setting.value(QLatin1String(NM_SETTING_802_1X_PIN)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS))) {
        setPinFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS))) {
        setSystemCaCertificates(setting.value(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS)).toBool());
    }
}

QVariantMap NetworkManager::Security8021xSetting::toMap() const
{
    QVariantMap setting;

    if (!eapMethods().isEmpty()) {
        QStringList methods;

        const auto methodList = eapMethods();
        for (const EapMethod &method : methodList) {
            if (method == EapMethodLeap) {
                methods << "leap";
            } else if (method == EapMethodMd5) {
                methods << "md5";
            } else if (method == EapMethodTls) {
                methods << "tls";
            } else if (method == EapMethodPeap) {
                methods << "peap";
            } else if (method == EapMethodTtls) {
                methods << "ttls";
            } else if (method == EapMethodSim) {
                methods << "sim";
            } else if (method == EapMethodFast) {
                methods << "fast";
            } else if (method == EapMethodPwd) {
                methods << "pwd";
            }
        }

        setting.insert(QLatin1String(NM_SETTING_802_1X_EAP), methods);
    }

    if (!identity().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_IDENTITY), identity());
    }

    if (!anonymousIdentity().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_ANONYMOUS_IDENTITY), anonymousIdentity());
    }

    if (!domainSuffixMatch().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_DOMAIN_SUFFIX_MATCH), domainSuffixMatch());
    }

    if (!pacFile().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PAC_FILE), pacFile());
    }

    if (!caCertificate().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_CA_CERT), caCertificate());
    }

    if (!caPath().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_CA_PATH), caPath());
    }

    if (!subjectMatch().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_SUBJECT_MATCH), subjectMatch());
    }

    if (!altSubjectMatches().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES), altSubjectMatches());
    }

    if (!clientCertificate().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT), clientCertificate());
    }

    QString version;
    switch (phase1PeapVersion()) {
    case PeapVersionZero:
        version = '0';
        break;
    case PeapVersionOne:
        version = '1';
        break;
    case PeapVersionUnknown:
        break;
    }

    if (!version.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), version);
    }

    QString peapLabel;
    switch (phase1PeapLabel()) {
    case PeapLabelForce:
        peapLabel = '1';
        break;
    case PeapLabelUnknown:
        break;
    }

    if (!peapLabel.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL), peapLabel);
    }

    QString provisioning;
    switch (phase1FastProvisioning()) {
    case FastProvisioningDisabled:
        provisioning = '0';
        break;
    case FastProvisioningAllowUnauthenticated:
        provisioning = '1';
        break;
    case FastProvisioningAllowAuthenticated:
        provisioning = '2';
        break;
    case FastProvisioningAllowBoth:
        provisioning = '3';
        break;
    case FastProvisioningUnknown:
        break;
    }

    if (!provisioning.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING), provisioning);
    }

    QString authMethod;
    switch (phase2AuthMethod()) {
    case AuthMethodPap:
        authMethod = "pap";
        break;
    case AuthMethodChap:
        authMethod = "chap";
        break;
    case AuthMethodMschap:
        authMethod = "mschap";
        break;
    case AuthMethodMschapv2:
        authMethod = "mschapv2";
        break;
    case AuthMethodGtc:
        authMethod = "gtc";
        break;
    case AuthMethodOtp:
        authMethod = "otp";
        break;
    case AuthMethodMd5:
        authMethod = "md5";
        break;
    case AuthMethodTls:
        authMethod = "tls";
        break;
    case AuthMethodUnknown:
        break;
    }

    if (!authMethod.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH), authMethod);
    }

    QString authEapMethod;
    switch (phase2AuthEapMethod()) {
    case AuthEapMethodMd5:
        authEapMethod = "md5";
        break;
    case AuthEapMethodMschapv2:
        authEapMethod = "mschapv2";
        break;
    case AuthEapMethodOtp:
        authEapMethod = "otp";
        break;
    case AuthEapMethodGtc:
        authEapMethod = "gtc";
        break;
    case AuthEapMethodTls:
        authEapMethod = "tls";
        break;
    case AuthEapMethodUnknown:
        break;
    }

    if (!authEapMethod.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP), authEapMethod);
    }

    if (!phase2CaCertificate().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT), phase2CaCertificate());
    }

    if (!phase2CaPath().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH), phase2CaPath());
    }

    if (!phase2SubjectMatch().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_SUBJECT_MATCH), phase2SubjectMatch());
    }

    if (!phase2AltSubjectMatches().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_ALTSUBJECT_MATCHES), phase2AltSubjectMatches());
    }

    if (!phase2ClientCertificate().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_CLIENT_CERT), phase2ClientCertificate());
    }

    if (!password().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD), password());
    }

    if (passwordFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS), (int)passwordFlags());
    }

    if (!passwordRaw().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW), passwordRaw());
    }

    if (passwordRawFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS), (int)passwordRawFlags());
    }

    if (!privateKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY), privateKey());
    }

    if (!privateKeyPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD), privateKeyPassword());
    }

    if (privateKeyPasswordFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS), (int)privateKeyPasswordFlags());
    }

    if (!phase2PrivateKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY), phase2PrivateKey());
    }

    if (!phase2PrivateKeyPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD), phase2PrivateKeyPassword());
    }

    if (phase2PrivateKeyPasswordFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS), (int)phase2PrivateKeyPasswordFlags());
    }

    if (!pin().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PIN), pin());
    }

    if (pinFlags() != None) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS), (int)pinFlags());
    }

    if (systemCaCertificates()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS), systemCaCertificates());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::Security8021xSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_802_1X_EAP << ": " << setting.eapMethods() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_IDENTITY << ": " << setting.identity() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_ANONYMOUS_IDENTITY << ": " << setting.anonymousIdentity() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PAC_FILE << ": " << setting.pacFile() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_CA_CERT << ": " << setting.caCertificate() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_CA_PATH << ": " << setting.caPath() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_DOMAIN_SUFFIX_MATCH << ": " << setting.domainSuffixMatch() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_SUBJECT_MATCH << ": " << setting.subjectMatch() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_ALTSUBJECT_MATCHES << ": " << setting.altSubjectMatches() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_CLIENT_CERT << ": " << setting.clientCertificate() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE1_PEAPVER << ": " << setting.phase1PeapVersion() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE1_PEAPLABEL << ": " << setting.phase1PeapLabel() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING << ": " << setting.phase1FastProvisioning() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_AUTH << ": " << setting.phase2AuthMethod() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_AUTHEAP << ": " << setting.phase2AuthEapMethod() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_CA_CERT << ": " << setting.phase2CaCertificate() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_CA_PATH << ": " << setting.phase2CaPath() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_SUBJECT_MATCH << ": " << setting.phase2SubjectMatch() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_ALTSUBJECT_MATCHES << ": " << setting.phase2AltSubjectMatches() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_CLIENT_CERT << ": " << setting.phase2ClientCertificate() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PASSWORD << ": " << setting.password() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PASSWORD_FLAGS << ": " << setting.passwordFlags() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PASSWORD_RAW << ": " << setting.passwordRaw() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PASSWORD_RAW_FLAGS << ": " << setting.passwordRawFlags() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PRIVATE_KEY << ": " << setting.privateKey() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD << ": " << setting.privateKeyPassword() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS << ": " << setting.privateKeyPasswordFlags() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_PRIVATE_KEY << ": " << setting.phase2PrivateKey() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD << ": " << setting.phase2PrivateKeyPassword() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS << ": " << setting.phase2PrivateKeyPasswordFlags() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PIN << ": " << setting.pin() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_PIN_FLAGS << ": " << setting.pinFlags() << '\n';
    dbg.nospace() << NM_SETTING_802_1X_SYSTEM_CA_CERTS << ": " << setting.systemCaCertificates() << '\n';

    return dbg.maybeSpace();
}
