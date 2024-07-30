/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_SECURITY8021X_SETTING_H
#define NETWORKMANAGERQT_SECURITY8021X_SETTING_H

#include "setting.h"
#include <networkmanagerqt/networkmanagerqt_export.h>

namespace NetworkManager
{
class Security8021xSettingPrivate;

/**
 * Represents 802.1x wireless security setting
 */
class NETWORKMANAGERQT_EXPORT Security8021xSetting : public Setting
{
public:
    typedef QSharedPointer<Security8021xSetting> Ptr;
    typedef QList<Ptr> List;
    enum EapMethod {
        EapMethodUnknown = 0,
        EapMethodLeap,
        EapMethodMd5,
        EapMethodTls,
        EapMethodPeap,
        EapMethodTtls,
        EapMethodSim,
        EapMethodFast,
        EapMethodPwd,
    };
    enum PeapVersion {
        PeapVersionUnknown = -1,
        PeapVersionZero,
        PeapVersionOne,
    };
    enum PeapLabel {
        PeapLabelUnknown = 0,
        PeapLabelForce,
    };
    enum FastProvisioning {
        FastProvisioningUnknown = -1,
        FastProvisioningDisabled,
        FastProvisioningAllowUnauthenticated,
        FastProvisioningAllowAuthenticated,
        FastProvisioningAllowBoth,
    };
    enum AuthMethod {
        AuthMethodUnknown = 0,
        AuthMethodPap,
        AuthMethodChap,
        AuthMethodMschap,
        AuthMethodMschapv2,
        AuthMethodGtc,
        AuthMethodOtp,
        AuthMethodMd5,
        AuthMethodTls,
    };
    enum AuthEapMethod {
        AuthEapMethodUnknown = 0,
        AuthEapMethodMd5,
        AuthEapMethodMschapv2,
        AuthEapMethodOtp,
        AuthEapMethodGtc,
        AuthEapMethodTls,
    };

    Security8021xSetting();
    explicit Security8021xSetting(const Ptr &other);
    ~Security8021xSetting() override;

    QString name() const override;

    void setEapMethods(const QList<EapMethod> &methods);
    QList<EapMethod> eapMethods() const;

    void setIdentity(const QString &identity);
    QString identity() const;

    void setAnonymousIdentity(const QString &identity);
    QString anonymousIdentity() const;

    void setDomainSuffixMatch(const QString &domainSuffixMatch);
    QString domainSuffixMatch() const;

    /**
     * Set UTF-8 encoded file path containing PAC for EAP-FAST.
     *
     * \param filePath file path to be set.
     */
    void setPacFile(const QString &filePath);
    QString pacFile() const;

    /**
     * Set certificate authority (ca)'s certificate for this setting.
     *
     * \param certificate certificate's file path encoded into a byte array.
     *
     * \warning certificate have to be null terminated or NetworkManager will refuse it.
     */
    void setCaCertificate(const QByteArray &certificate);
    QByteArray caCertificate() const;

    void setCaPath(const QString &path);
    QString caPath() const;

    void setSubjectMatch(const QString &substring);
    QString subjectMatch() const;

    void setAltSubjectMatches(const QStringList &strings);
    QStringList altSubjectMatches() const;

    /**
     * Set client certificate for this setting.
     *
     * \param certificate certificate's file path encoded into a byte array.
     *
     * \warning certificate have to be null terminated or NetworkManager will refuse it.
     */
    void setClientCertificate(const QByteArray &certificate);
    QByteArray clientCertificate() const;

    void setPhase1PeapVersion(PeapVersion version);
    PeapVersion phase1PeapVersion() const;

    void setPhase1PeapLabel(PeapLabel label);
    PeapLabel phase1PeapLabel() const;

    void setPhase1FastProvisioning(FastProvisioning provisioning);
    FastProvisioning phase1FastProvisioning() const;

    void setPhase2AuthMethod(AuthMethod method);
    AuthMethod phase2AuthMethod() const;

    void setPhase2AuthEapMethod(AuthEapMethod method);
    AuthEapMethod phase2AuthEapMethod() const;

    /**
     * Contains the CA certificate if used by the EAP method specified in the
     * phase2AuthMethod() or phase2AuthEapMethod() properties.
     *
     * \param certificate certificate's file path encoded into a byte array.
     *
     * \warning certificate have to be null terminated or NetworkManager will refuse it.
     */
    void setPhase2CaCertificate(const QByteArray &certificate);
    QByteArray phase2CaCertificate() const;

    void setPhase2CaPath(const QString &path);
    QString phase2CaPath() const;

    void setPhase2SubjectMatch(const QString &substring);
    QString phase2SubjectMatch() const;

    void setPhase2AltSubjectMatches(const QStringList &strings);
    QStringList phase2AltSubjectMatches() const;

    void setPhase2ClientCertificate(const QByteArray &certificate);
    QByteArray phase2ClientCertificate() const;

    void setPassword(const QString &password);
    QString password() const;

    void setPasswordFlags(Setting::SecretFlags flags);
    Setting::SecretFlags passwordFlags() const;

    void setPasswordRaw(const QByteArray &password);
    QByteArray passwordRaw() const;

    void setPasswordRawFlags(Setting::SecretFlags flags);
    Setting::SecretFlags passwordRawFlags() const;

    /**
     * Set private key for this setting.
     *
     * \param key the key to be set.
     *
     * \warning key have to be null terminated or NetworkManager will refuse it.
     */
    void setPrivateKey(const QByteArray &key);
    QByteArray privateKey() const;

    void setPrivateKeyPassword(const QString &password);
    QString privateKeyPassword() const;

    void setPrivateKeyPasswordFlags(Setting::SecretFlags flags);
    Setting::SecretFlags privateKeyPasswordFlags() const;

    void setPhase2PrivateKey(const QByteArray &key);
    QByteArray phase2PrivateKey() const;

    void setPhase2PrivateKeyPassword(const QString &password);
    QString phase2PrivateKeyPassword() const;

    void setPhase2PrivateKeyPasswordFlags(Setting::SecretFlags flags);
    Setting::SecretFlags phase2PrivateKeyPasswordFlags() const;

    void setPin(const QString &pin);
    QString pin() const;

    void setPinFlags(Setting::SecretFlags flags);
    Setting::SecretFlags pinFlags() const;

    void setSystemCaCertificates(bool use);
    bool systemCaCertificates() const;

    QStringList needSecrets(bool requestNew = false) const override;

    void secretsFromMap(const QVariantMap &secrets) override;

    QVariantMap secretsToMap() const override;

    void fromMap(const QVariantMap &setting) override;

    QVariantMap toMap() const override;

protected:
    Security8021xSettingPrivate *const d_ptr;

private:
    Q_DECLARE_PRIVATE(Security8021xSetting)
};

NETWORKMANAGERQT_EXPORT QDebug operator<<(QDebug dbg, const Security8021xSetting &setting);

}

#endif // NETWORKMANAGERQT_SECURITY802X_SETTING_H
