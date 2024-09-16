// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsecurity8021xsetting_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

DSecurity8021xSettingPrivate::DSecurity8021xSettingPrivate()
    : m_systemCaCerts(false)
    , m_phase1PeapVer(DSecurity8021xSetting::PeapVersion::Unknown)
    , m_phase1PeapLabel(DSecurity8021xSetting::PeapLabel::Unknown)
    , m_phase1FastProvisioning(DSecurity8021xSetting::FastProvisioning::Unknown)
    , m_phase2AuthMethod(DSecurity8021xSetting::AuthMethod::Unknown)
    , m_phase2AuthEapMethod(DSecurity8021xSetting::AuthEapMethod::Unknown)
    , m_pinFlags(DNMSetting::SecretFlagType::None)
    , m_phase2PrivateKeyPasswordFlags(DNMSetting::SecretFlagType::None)
    , m_privateKeyPasswordFlags(DNMSetting::SecretFlagType::None)
    , m_passwordRawFlags(DNMSetting::SecretFlagType::None)
    , m_passwordFlags(DNMSetting::SecretFlagType::None)
    , m_name(NM_SETTING_802_1X_SETTING_NAME)
{
}

DSecurity8021xSetting::DSecurity8021xSetting()
    : DNMSetting(DNMSetting::SettingType::Security8021x)
    , d_ptr(new DSecurity8021xSettingPrivate())
{
}

DSecurity8021xSetting::DSecurity8021xSetting(const QSharedPointer<DSecurity8021xSetting> &other)
    : DNMSetting(other)
    , d_ptr(new DSecurity8021xSettingPrivate())
{
    setEapMethods(other->eapMethods());
    setIdentity(other->identity());
    setDomainSuffixMatch(other->domainSuffixMatch());
    setAnonymousIdentity(other->anonymousIdentity());
    setPacFile(other->pacFile());
    setCaCertificate(other->caCertificate());
    setCaPath(other->caPath());
    setAltSubjectMatches(other->altSubjectMatches());
    setClientCertificate(other->clientCertificate());
    setPhase1PeapVersion(other->phase1PeapVersion());
    setPhase1PeapLabel(other->phase1PeapLabel());
    setPhase1FastProvisioning(other->phase1FastProvisioning());
    setPhase2AuthMethod(other->phase2AuthMethod());
    setPhase2AuthEapMethod(other->phase2AuthEapMethod());
    setPhase2CaCertificate(other->phase2CaCertificate());
    setPhase2CaPath(other->phase2CaPath());
    setPhase2DomainSuffixMatch(other->phase2DomainSuffixMatch());
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

QString DSecurity8021xSetting::name() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_name;
}

void DSecurity8021xSetting::setEapMethods(const QList<DSecurity8021xSetting::EapMethod> &methods)
{
    Q_D(DSecurity8021xSetting);

    d->m_eap = methods;
}

QList<DSecurity8021xSetting::EapMethod> DSecurity8021xSetting::eapMethods() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_eap;
}

void DSecurity8021xSetting::setIdentity(const QString &identity)
{
    Q_D(DSecurity8021xSetting);

    d->m_identity = identity;
}

QString DSecurity8021xSetting::identity() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_identity;
}

void DSecurity8021xSetting::setAnonymousIdentity(const QString &identity)
{
    Q_D(DSecurity8021xSetting);

    d->m_anonymousIdentity = identity;
}

QString DSecurity8021xSetting::anonymousIdentity() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_anonymousIdentity;
}

void DSecurity8021xSetting::setDomainSuffixMatch(const QString &domain)
{
    Q_D(DSecurity8021xSetting);

    d->m_domainSuffixMatch = domain;
}

QString DSecurity8021xSetting::domainSuffixMatch() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_domainSuffixMatch;
}

void DSecurity8021xSetting::setPacFile(const QString &filePath)
{
    Q_D(DSecurity8021xSetting);

    d->m_pacFile = filePath;
}

QString DSecurity8021xSetting::pacFile() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_pacFile;
}

void DSecurity8021xSetting::setCaCertificate(const QByteArray &certificate)
{
    Q_D(DSecurity8021xSetting);

    d->m_caCert = certificate;
}

QByteArray DSecurity8021xSetting::caCertificate() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_caCert;
}

void DSecurity8021xSetting::setCaPath(const QString &path)
{
    Q_D(DSecurity8021xSetting);

    d->m_caPath = path;
}

QString DSecurity8021xSetting::caPath() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_caPath;
}

void DSecurity8021xSetting::setAltSubjectMatches(const QStringList &strings)
{
    Q_D(DSecurity8021xSetting);

    d->m_altSubjectMatches = strings;
}

QStringList DSecurity8021xSetting::altSubjectMatches() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_altSubjectMatches;
}

void DSecurity8021xSetting::setClientCertificate(const QByteArray &certificate)
{
    Q_D(DSecurity8021xSetting);

    d->m_clientCert = certificate;
}

QByteArray DSecurity8021xSetting::clientCertificate() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_clientCert;
}

void DSecurity8021xSetting::setPhase1PeapVersion(DSecurity8021xSetting::PeapVersion version)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase1PeapVer = version;
}

DSecurity8021xSetting::PeapVersion DSecurity8021xSetting::phase1PeapVersion() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase1PeapVer;
}

void DSecurity8021xSetting::setPhase1PeapLabel(DSecurity8021xSetting::PeapLabel label)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase1PeapLabel = label;
}

DSecurity8021xSetting::PeapLabel DSecurity8021xSetting::phase1PeapLabel() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase1PeapLabel;
}

void DSecurity8021xSetting::setPhase1FastProvisioning(DSecurity8021xSetting::FastProvisioning provisioning)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase1FastProvisioning = provisioning;
}

DSecurity8021xSetting::FastProvisioning DSecurity8021xSetting::phase1FastProvisioning() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase1FastProvisioning;
}

void DSecurity8021xSetting::setPhase2AuthMethod(DSecurity8021xSetting::AuthMethod method)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2AuthMethod = method;
}

DSecurity8021xSetting::AuthMethod DSecurity8021xSetting::phase2AuthMethod() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2AuthMethod;
}

void DSecurity8021xSetting::setPhase2AuthEapMethod(DSecurity8021xSetting::AuthEapMethod method)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2AuthEapMethod = method;
}

DSecurity8021xSetting::AuthEapMethod DSecurity8021xSetting::phase2AuthEapMethod() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2AuthEapMethod;
}

void DSecurity8021xSetting::setPhase2CaCertificate(const QByteArray &certificate)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2CaCert = certificate;
}

QByteArray DSecurity8021xSetting::phase2CaCertificate() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2CaCert;
}

void DSecurity8021xSetting::setPhase2CaPath(const QString &path)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2CaPath = path;
}

QString DSecurity8021xSetting::phase2CaPath() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2CaPath;
}

void DSecurity8021xSetting::setPhase2DomainSuffixMatch(const QString &match)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2DomainSuffixMatch = match;
}

QString DSecurity8021xSetting::phase2DomainSuffixMatch() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2DomainSuffixMatch;
}

void DSecurity8021xSetting::setPhase2AltSubjectMatches(const QStringList &strings)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2AltSubjectMatches = strings;
}

QStringList DSecurity8021xSetting::phase2AltSubjectMatches() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2AltSubjectMatches;
}

void DSecurity8021xSetting::setPhase2ClientCertificate(const QByteArray &certificate)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2ClientCert = certificate;
}

QByteArray DSecurity8021xSetting::phase2ClientCertificate() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2ClientCert;
}

void DSecurity8021xSetting::setPassword(const QString &password)
{
    Q_D(DSecurity8021xSetting);

    d->m_password = password;
}

QString DSecurity8021xSetting::password() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_password;
}

void DSecurity8021xSetting::setPasswordFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DSecurity8021xSetting);

    d->m_passwordFlags = flags;
}

DNMSetting::SecretFlags DSecurity8021xSetting::passwordFlags() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_passwordFlags;
}

void DSecurity8021xSetting::setPasswordRaw(const QByteArray &password)
{
    Q_D(DSecurity8021xSetting);

    d->m_passwordRaw = password;
}

QByteArray DSecurity8021xSetting::passwordRaw() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_passwordRaw;
}

void DSecurity8021xSetting::setPasswordRawFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DSecurity8021xSetting);

    d->m_passwordRawFlags = flags;
}

DNMSetting::SecretFlags DSecurity8021xSetting::passwordRawFlags() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_passwordRawFlags;
}

void DSecurity8021xSetting::setPrivateKey(const QByteArray &key)
{
    Q_D(DSecurity8021xSetting);

    d->m_privateKey = key;
}

QByteArray DSecurity8021xSetting::privateKey() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_privateKey;
}

void DSecurity8021xSetting::setPrivateKeyPassword(const QString &password)
{
    Q_D(DSecurity8021xSetting);

    d->m_privateKeyPassword = password;
}

QString DSecurity8021xSetting::privateKeyPassword() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_privateKeyPassword;
}

void DSecurity8021xSetting::setPrivateKeyPasswordFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DSecurity8021xSetting);

    d->m_privateKeyPasswordFlags = flags;
}

DNMSetting::SecretFlags DSecurity8021xSetting::privateKeyPasswordFlags() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_privateKeyPasswordFlags;
}

void DSecurity8021xSetting::setPhase2PrivateKey(const QByteArray &key)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2PrivateKey = key;
}

QByteArray DSecurity8021xSetting::phase2PrivateKey() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2PrivateKey;
}

void DSecurity8021xSetting::setPhase2PrivateKeyPassword(const QString &password)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2PrivateKeyPassword = password;
}

QString DSecurity8021xSetting::phase2PrivateKeyPassword() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2PrivateKeyPassword;
}

void DSecurity8021xSetting::setPhase2PrivateKeyPasswordFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DSecurity8021xSetting);

    d->m_phase2PrivateKeyPasswordFlags = flags;
}

DNMSetting::SecretFlags DSecurity8021xSetting::phase2PrivateKeyPasswordFlags() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_phase2PrivateKeyPasswordFlags;
}

void DSecurity8021xSetting::setSystemCaCertificates(bool use)
{
    Q_D(DSecurity8021xSetting);

    d->m_systemCaCerts = use;
}

void DSecurity8021xSetting::setPin(const QString &pin)
{
    Q_D(DSecurity8021xSetting);

    d->m_pin = pin;
}

QString DSecurity8021xSetting::pin() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_pin;
}

void DSecurity8021xSetting::setPinFlags(DNMSetting::SecretFlags flags)
{
    Q_D(DSecurity8021xSetting);

    d->m_pinFlags = flags;
}

DNMSetting::SecretFlags DSecurity8021xSetting::pinFlags() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_pinFlags;
}

bool DSecurity8021xSetting::systemCaCertificates() const
{
    Q_D(const DSecurity8021xSetting);

    return d->m_systemCaCerts;
}

QStringList DSecurity8021xSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if (eapMethods().contains(EapMethod::Tls) and (privateKeyPassword().isEmpty() or requestNew) and
        !privateKeyPasswordFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD);
    } else if ((eapMethods().contains(EapMethod::Ttls) or eapMethods().contains(EapMethod::Peap) or
                eapMethods().contains(EapMethod::Leap) or eapMethods().contains(EapMethod::Fast) or
                eapMethods().contains(EapMethod::Pwd)) and
               (password().isEmpty() or requestNew) and !passwordFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PASSWORD);
        secrets << QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW);
    } else if (eapMethods().contains(EapMethod::Sim) and (pin().isEmpty() or requestNew) and
               !pinFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PIN);
    }

    if ((phase2AuthMethod() == AuthMethod::Tls or phase2AuthEapMethod() == AuthEapMethod::Tls) and
        (phase2PrivateKeyPassword().isEmpty() or requestNew) and
        !phase2PrivateKeyPasswordFlags().testFlag(DNMSetting::SecretFlagType::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD);
    }

    return secrets;
}

void DSecurity8021xSetting::mapToSecrets(const QVariantMap &secrets)
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

QVariantMap DSecurity8021xSetting::secretsToMap() const
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

void DSecurity8021xSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_802_1X_EAP))) {
        const QStringList methods = setting.value(QLatin1String(NM_SETTING_802_1X_EAP)).toStringList();
        QList<EapMethod> eapMethods;
        for (const QString &method : methods) {
            if (method == "leap") {
                eapMethods << EapMethod::Leap;
            } else if (method == "md5") {
                eapMethods << EapMethod::Md5;
            } else if (method == "tls") {
                eapMethods << EapMethod::Tls;
            } else if (method == "peap") {
                eapMethods << EapMethod::Peap;
            } else if (method == "ttls") {
                eapMethods << EapMethod::Ttls;
            } else if (method == "sim") {
                eapMethods << EapMethod::Sim;
            } else if (method == "fast") {
                eapMethods << EapMethod::Fast;
            } else if (method == "pwd") {
                eapMethods << EapMethod::Pwd;
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

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES))) {
        setAltSubjectMatches(setting.value(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES)).toStringList());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT))) {
        setClientCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER))) {
        const QString version = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER)).toString();

        if (version == "0") {
            setPhase1PeapVersion(PeapVersion::Zero);
        } else if (version == "1") {
            setPhase1PeapVersion(PeapVersion::One);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL))) {
        const QString label = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL)).toString();

        if (label == "1") {
            setPhase1PeapLabel(PeapLabel::Force);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING))) {
        const QString provisioning = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING)).toString();

        if (provisioning == "0") {
            setPhase1FastProvisioning(FastProvisioning::Disabled);
        } else if (provisioning == "1") {
            setPhase1FastProvisioning(FastProvisioning::AllowUnauthenticated);
        } else if (provisioning == "2") {
            setPhase1FastProvisioning(FastProvisioning::AllowAuthenticated);
        } else if (provisioning == "3") {
            setPhase1FastProvisioning(FastProvisioning::AllowBoth);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH))) {
        const QString authMethod = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH)).toString();

        if (authMethod == "pap") {
            setPhase2AuthMethod(AuthMethod::Pap);
        } else if (authMethod == "chap") {
            setPhase2AuthMethod(AuthMethod::Chap);
        } else if (authMethod == "mschap") {
            setPhase2AuthMethod(AuthMethod::Mschap);
        } else if (authMethod == "mschapv2") {
            setPhase2AuthMethod(AuthMethod::Mschapv2);
        } else if (authMethod == "gtc") {
            setPhase2AuthMethod(AuthMethod::Gtc);
        } else if (authMethod == "otp") {
            setPhase2AuthMethod(AuthMethod::Otp);
        } else if (authMethod == "md5") {
            setPhase2AuthMethod(AuthMethod::Md5);
        } else if (authMethod == "tls") {
            setPhase2AuthMethod(AuthMethod::Tls);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP))) {
        const QString authEapMethod = setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTHEAP)).toString();

        if (authEapMethod == "md5") {
            setPhase2AuthEapMethod(AuthEapMethod::Md5);
        } else if (authEapMethod == "mschapv2") {
            setPhase2AuthEapMethod(AuthEapMethod::Mschapv2);
        } else if (authEapMethod == "otp") {
            setPhase2AuthEapMethod(AuthEapMethod::Otp);
        } else if (authEapMethod == "gtc") {
            setPhase2AuthEapMethod(AuthEapMethod::Gtc);
        } else if (authEapMethod == "tls") {
            setPhase2AuthEapMethod(AuthEapMethod::Tls);
        }
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT))) {
        setPhase2CaCertificate(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_CERT)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH))) {
        setPhase2CaPath(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_CA_PATH)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_DOMAIN_SUFFIX_MATCH))) {
        setPhase2DomainSuffixMatch(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_DOMAIN_SUFFIX_MATCH)).toString());
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
        setPasswordFlags((DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW))) {
        setPasswordRaw(setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS))) {
        setPasswordRawFlags((DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY))) {
        setPrivateKey(setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD))) {
        setPrivateKeyPassword(setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS))) {
        setPrivateKeyPasswordFlags(
            (DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY))) {
        setPhase2PrivateKey(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD))) {
        setPhase2PrivateKeyPassword(setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS))) {
        setPhase2PrivateKeyPasswordFlags(
            (DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PIN))) {
        setPin(setting.value(QLatin1String(NM_SETTING_802_1X_PIN)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS))) {
        setPinFlags((DNMSetting::SecretFlags)setting.value(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS))) {
        setSystemCaCertificates(setting.value(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS)).toBool());
    }
}

QVariantMap DSecurity8021xSetting::toMap() const
{
    QVariantMap setting;

    if (!eapMethods().isEmpty()) {
        QStringList methods;

        const auto methodList = eapMethods();
        for (const EapMethod &method : methodList) {
            if (method == EapMethod::Leap) {
                methods << "leap";
            } else if (method == EapMethod::Md5) {
                methods << "md5";
            } else if (method == EapMethod::Tls) {
                methods << "tls";
            } else if (method == EapMethod::Peap) {
                methods << "peap";
            } else if (method == EapMethod::Ttls) {
                methods << "ttls";
            } else if (method == EapMethod::Sim) {
                methods << "sim";
            } else if (method == EapMethod::Fast) {
                methods << "fast";
            } else if (method == EapMethod::Pwd) {
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

    if (!altSubjectMatches().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_ALTSUBJECT_MATCHES), altSubjectMatches());
    }

    if (!clientCertificate().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_CLIENT_CERT), clientCertificate());
    }

    QString version;
    switch (phase1PeapVersion()) {
        case PeapVersion::Zero:
            version = '0';
            break;
        case PeapVersion::One:
            version = '1';
            break;
        case PeapVersion::Unknown:
            break;
    }

    if (!version.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPVER), version);
    }

    QString peapLabel;
    switch (phase1PeapLabel()) {
        case PeapLabel::Force:
            peapLabel = '1';
            break;
        case PeapLabel::Unknown:
            break;
    }

    if (!peapLabel.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_PEAPLABEL), peapLabel);
    }

    QString provisioning;
    switch (phase1FastProvisioning()) {
        case FastProvisioning::Disabled:
            provisioning = '0';
            break;
        case FastProvisioning::AllowUnauthenticated:
            provisioning = '1';
            break;
        case FastProvisioning::AllowAuthenticated:
            provisioning = '2';
            break;
        case FastProvisioning::AllowBoth:
            provisioning = '3';
            break;
        case FastProvisioning::Unknown:
            break;
    }

    if (!provisioning.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE1_FAST_PROVISIONING), provisioning);
    }

    QString authMethod;
    switch (phase2AuthMethod()) {
        case AuthMethod::Pap:
            authMethod = "pap";
            break;
        case AuthMethod::Chap:
            authMethod = "chap";
            break;
        case AuthMethod::Mschap:
            authMethod = "mschap";
            break;
        case AuthMethod::Mschapv2:
            authMethod = "mschapv2";
            break;
        case AuthMethod::Gtc:
            authMethod = "gtc";
            break;
        case AuthMethod::Otp:
            authMethod = "otp";
            break;
        case AuthMethod::Md5:
            authMethod = "md5";
            break;
        case AuthMethod::Tls:
            authMethod = "tls";
            break;
        case AuthMethod::Unknown:
            break;
    }

    if (!authMethod.isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_AUTH), authMethod);
    }

    QString authEapMethod;
    switch (phase2AuthEapMethod()) {
        case AuthEapMethod::Md5:
            authEapMethod = "md5";
            break;
        case AuthEapMethod::Mschapv2:
            authEapMethod = "mschapv2";
            break;
        case AuthEapMethod::Otp:
            authEapMethod = "otp";
            break;
        case AuthEapMethod::Gtc:
            authEapMethod = "gtc";
            break;
        case AuthEapMethod::Tls:
            authEapMethod = "tls";
            break;
        case AuthEapMethod::Unknown:
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

    if (!phase2DomainSuffixMatch().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_DOMAIN_SUFFIX_MATCH), phase2DomainSuffixMatch());
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

    if (!passwordFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_FLAGS), static_cast<int>(passwordFlags()));
    }

    if (!passwordRaw().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW), passwordRaw());
    }

    if (!passwordRawFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PASSWORD_RAW_FLAGS), static_cast<int>(passwordRawFlags()));
    }

    if (!privateKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY), privateKey());
    }

    if (!privateKeyPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD), privateKeyPassword());
    }

    if (!privateKeyPasswordFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PRIVATE_KEY_PASSWORD_FLAGS), static_cast<int>(privateKeyPasswordFlags()));
    }

    if (!phase2PrivateKey().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY), phase2PrivateKey());
    }

    if (!phase2PrivateKeyPassword().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD), phase2PrivateKeyPassword());
    }

    if (!phase2PrivateKeyPasswordFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PHASE2_PRIVATE_KEY_PASSWORD_FLAGS),
                       static_cast<int>(phase2PrivateKeyPasswordFlags()));
    }

    if (!pin().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PIN), pin());
    }

    if (!pinFlags().testFlag(DNMSetting::SecretFlagType::None)) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_PIN_FLAGS), static_cast<int>(pinFlags()));
    }

    if (systemCaCertificates()) {
        setting.insert(QLatin1String(NM_SETTING_802_1X_SYSTEM_CA_CERTS), systemCaCertificates());
    }

    return setting;
}

DNETWORKMANAGER_END_NAMESPACE
