/*
    SPDX-FileCopyrightText: Pranav Gade <pranavgade20@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "macsecsetting.h"
#include "macsecsetting_p.h"

#include <QDebug>

#if !NM_CHECK_VERSION(1, 6, 0)
#define NM_SETTING_MACSEC_SETTING_NAME "macsec"

#define NM_SETTING_MACSEC_PARENT "parent"
#define NM_SETTING_MACSEC_MODE "mode"
#define NM_SETTING_MACSEC_ENCRYPT "encrypt"
#define NM_SETTING_MACSEC_MKA_CAK "mka-cak"
#define NM_SETTING_MACSEC_MKA_CAK_FLAGS "mka-cak-flags"
#define NM_SETTING_MACSEC_MKA_CKN "mka-ckn"
#define NM_SETTING_MACSEC_PORT "port"
#define NM_SETTING_MACSEC_VALIDATION "validation"
#endif

#if !NM_CHECK_VERSION(1, 12, 0)
#define NM_SETTING_MACSEC_SEND_SCI "send-sci"
#endif

NetworkManager::MacsecSettingPrivate::MacsecSettingPrivate()
    : name(NM_SETTING_MACSEC_SETTING_NAME)
    , encrypt(true)
    , mode(NetworkManager::MacsecSetting::Psk)
    , port(1)
    , sendSci(true)
    , validation(NetworkManager::MacsecSetting::Strict)
{
}

NetworkManager::MacsecSetting::MacsecSetting()
    : Setting(Setting::Macsec)
    , d_ptr(new MacsecSettingPrivate())
{
}

NetworkManager::MacsecSetting::MacsecSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new MacsecSettingPrivate())
{
    setEncrypt(other->encrypt());
    setMkaCak(other->mkaCak());
    setMkaCkn(other->mkaCkn());
    setMode(other->mode());
    setParent(other->parent());
    setPort(other->port());
    setSendSci(other->sendSci());
    setValidation(other->validation());
    setMkaCakFlags(other->mkaCakFlags());
}

NetworkManager::MacsecSetting::~MacsecSetting()
{
    delete d_ptr;
}

QString NetworkManager::MacsecSetting::name() const
{
    Q_D(const MacsecSetting);

    return d->name;
}

void NetworkManager::MacsecSetting::setEncrypt(bool encrypt)
{
    Q_D(MacsecSetting);

    d->encrypt = encrypt;
}

bool NetworkManager::MacsecSetting::encrypt() const
{
    Q_D(const MacsecSetting);

    return d->encrypt;
}

void NetworkManager::MacsecSetting::setMkaCak(const QString &mkaCak)
{
    Q_D(MacsecSetting);

    d->mkaCak = mkaCak;
}

QString NetworkManager::MacsecSetting::mkaCak() const
{
    Q_D(const MacsecSetting);

    return d->mkaCak;
}

void NetworkManager::MacsecSetting::setMkaCkn(const QString &mkaCkn)
{
    Q_D(MacsecSetting);

    d->mkaCkn = mkaCkn;
}

QString NetworkManager::MacsecSetting::mkaCkn() const
{
    Q_D(const MacsecSetting);

    return d->mkaCkn;
}

void NetworkManager::MacsecSetting::setMode(Mode mode)
{
    Q_D(MacsecSetting);

    d->mode = mode;
}

NetworkManager::MacsecSetting::Mode NetworkManager::MacsecSetting::mode() const
{
    Q_D(const MacsecSetting);

    return d->mode;
}

void NetworkManager::MacsecSetting::setParent(const QString &parent)
{
    Q_D(MacsecSetting);

    d->parent = parent;
}

QString NetworkManager::MacsecSetting::parent() const
{
    Q_D(const MacsecSetting);

    return d->parent;
}

void NetworkManager::MacsecSetting::setPort(qint32 port)
{
    Q_D(MacsecSetting);

    d->port = port;
}

qint32 NetworkManager::MacsecSetting::port() const
{
    Q_D(const MacsecSetting);

    return d->port;
}

void NetworkManager::MacsecSetting::setSendSci(bool sendSci)
{
    Q_D(MacsecSetting);

    d->sendSci = sendSci;
}

bool NetworkManager::MacsecSetting::sendSci() const
{
    Q_D(const MacsecSetting);

    return d->sendSci;
}

void NetworkManager::MacsecSetting::setValidation(Validation validation)
{
    Q_D(MacsecSetting);

    d->validation = validation;
}

NetworkManager::MacsecSetting::Validation NetworkManager::MacsecSetting::validation() const
{
    Q_D(const MacsecSetting);

    return d->validation;
}

void NetworkManager::MacsecSetting::setMkaCakFlags(NetworkManager::Setting::SecretFlags flags)
{
    Q_D(MacsecSetting);

    d->mkaCakFlags = flags;
}

NetworkManager::Setting::SecretFlags NetworkManager::MacsecSetting::mkaCakFlags() const
{
    Q_D(const MacsecSetting);

    return d->mkaCakFlags;
}

QStringList NetworkManager::MacsecSetting::needSecrets(bool requestNew) const
{
    QStringList secrets;

    if ((mkaCak().isEmpty() || requestNew) && !mkaCakFlags().testFlag(Setting::NotRequired)) {
        secrets << QLatin1String(NM_SETTING_MACSEC_MKA_CAK);
    }

    return secrets;
}

void NetworkManager::MacsecSetting::secretsFromMap(const QVariantMap &secrets)
{
    if (secrets.contains(QLatin1String(NM_SETTING_MACSEC_MKA_CAK))) {
        setMkaCak(secrets.value(QLatin1String(NM_SETTING_MACSEC_MKA_CAK)).toString());
    }
}

QVariantMap NetworkManager::MacsecSetting::secretsToMap() const
{
    QVariantMap secrets;

    if (!mkaCak().isEmpty()) {
        secrets.insert(QLatin1String(NM_SETTING_MACSEC_MKA_CAK), mkaCak());
    }

    return secrets;
}

void NetworkManager::MacsecSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_ENCRYPT))) {
        setEncrypt(setting.value(QLatin1String(NM_SETTING_MACSEC_ENCRYPT)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_MKA_CAK))) {
        setMkaCak(setting.value(QLatin1String(NM_SETTING_MACSEC_MKA_CAK)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_MKA_CKN))) {
        setMkaCkn(setting.value(QLatin1String(NM_SETTING_MACSEC_MKA_CKN)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_MODE))) {
        setMode((Mode)setting.value(QLatin1String(NM_SETTING_MACSEC_MODE)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_PARENT))) {
        setParent(setting.value(QLatin1String(NM_SETTING_MACSEC_PARENT)).toString());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_PORT))) {
        setPort(setting.value(QLatin1String(NM_SETTING_MACSEC_PORT)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_SEND_SCI))) {
        setSendSci(setting.value(QLatin1String(NM_SETTING_MACSEC_SEND_SCI)).toBool());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_VALIDATION))) {
        setValidation((Validation)setting.value(QLatin1String(NM_SETTING_MACSEC_VALIDATION)).toInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_MACSEC_MKA_CAK_FLAGS))) {
        setMkaCakFlags((Setting::SecretFlags)setting.value(QLatin1String(NM_SETTING_MACSEC_MKA_CAK_FLAGS)).toUInt());
    }
}

QVariantMap NetworkManager::MacsecSetting::toMap() const
{
    QVariantMap setting;

    if (!encrypt()) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_ENCRYPT), encrypt());
    }

    if (!mkaCak().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_MKA_CAK), mkaCak());
    }

    if (!mkaCkn().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_MKA_CKN), mkaCkn());
    }

    if (mode() > NetworkManager::MacsecSetting::Psk) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_MODE), (int)mode());
    }

    if (!parent().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_PARENT), parent());
    }

    if (port() > 1) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_PORT), port());
    }

    if (!sendSci()) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_SEND_SCI), sendSci());
    }

    if (validation() != NetworkManager::MacsecSetting::Strict) {
        setting.insert(QLatin1String(NM_SETTING_MACSEC_VALIDATION), (int)validation());
    }

    setting.insert(QLatin1String(NM_SETTING_MACSEC_MKA_CAK_FLAGS), (int)mkaCakFlags());

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::MacsecSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_MACSEC_ENCRYPT << ": " << setting.encrypt() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_MKA_CAK << ": " << setting.mkaCak() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_MKA_CKN << ": " << setting.mkaCkn() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_MODE << ": " << setting.mode() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_PARENT << ": " << setting.parent() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_PORT << ": " << setting.port() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_SEND_SCI << ": " << setting.sendSci() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_VALIDATION << ": " << setting.validation() << '\n';
    dbg.nospace() << NM_SETTING_MACSEC_MKA_CAK_FLAGS << ": " << setting.mkaCakFlags() << '\n';

    return dbg.maybeSpace();
}
