/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPSERVICE_H_
#define _KWALLETFREEDESKTOPSERVICE_H_

#include <KConfig>
#include <QDBusArgument>
#include <QDBusServiceWatcher>
#include <QHash>
#include <QPointer>
#include <QString>
#include <QtCrypto>

#include "kwalletdbuscontext.h"

#define FDO_APPID QString()
#define FDO_SECRETS_SERVICE_OBJECT "/org/freedesktop/secrets"
#define FDO_ALIAS_PATH "/org/freedesktop/secrets/aliases/"

static inline constexpr size_t FDO_SECRETS_CIPHER_KEY_SIZE = 16;
static inline constexpr int FDO_DH_PUBLIC_KEY_SIZE = 128;

class KWalletD;

class FreedesktopSecret
{
public:
    FreedesktopSecret() = default;

    FreedesktopSecret(QDBusObjectPath iSession, const QCA::SecureArray &iValue, QString iMimeType)
        : session(std::move(iSession))
        , value(iValue)
        , mimeType(std::move(iMimeType))
    {
    }

    friend QDBusArgument &operator<<(QDBusArgument &arg, const FreedesktopSecret &secret);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, FreedesktopSecret &secret);

    QDBusObjectPath session;
    QCA::SecureArray parameters;
    QCA::SecureArray value;
    QString mimeType;
};

struct PropertiesMap {
    QVariantMap map;
};

struct EntryLocation {
    static EntryLocation fromUniqueLabel(const struct FdoUniqueLabel &uniqLabel);
    struct FdoUniqueLabel toUniqueLabel() const;

    bool operator==(const EntryLocation &rhs) const
    {
        return folder == rhs.folder && key == rhs.key;
    }

    bool operator!=(const EntryLocation &rhs) const
    {
        return !(*this == rhs);
    }

    QString folder;
    QString key;
};

struct FdoUniqueLabel {
    static FdoUniqueLabel fromEntryLocation(const EntryLocation &entryLocation);
    static FdoUniqueLabel fromName(const QString &name);
    static QString makeName(const QString &label, int copyId);

    bool operator==(const FdoUniqueLabel &rhs) const
    {
        return copyId == rhs.copyId && label == rhs.label;
    }

    bool operator!=(const FdoUniqueLabel &rhs) const
    {
        return !(*this == rhs);
    }

    QString toName() const;
    EntryLocation toEntryLocation() const;

    QString label;
    int copyId = -1;
};

typedef QMap<QDBusObjectPath, FreedesktopSecret> FreedesktopSecretMap;
typedef QMap<QString, QString> StrStrMap;

Q_DECLARE_METATYPE(FreedesktopSecret)
Q_DECLARE_METATYPE(FreedesktopSecretMap)
Q_DECLARE_METATYPE(PropertiesMap)
Q_DECLARE_METATYPE(StrStrMap)
Q_DECLARE_METATYPE(QCA::SecureArray)

class KWalletFreedesktopSession;
class KWalletFreedesktopSessionAlgorithm;
class KWalletFreedesktopCollection;
class KWalletFreedesktopPrompt;
class KWalletFreedesktopItem;

class KWalletFreedesktopService : public QObject, protected FDO_DBUS_CONTEXT
{
    /* org.freedesktop.Secret.Service properties */
public:
    Q_PROPERTY(QList<QDBusObjectPath> Collections READ collections)
    QList<QDBusObjectPath> collections() const;

    Q_OBJECT

public:
    explicit KWalletFreedesktopService(KWalletD *parent);
    ~KWalletFreedesktopService();

    KWalletFreedesktopService(const KWalletFreedesktopService &) = delete;
    KWalletFreedesktopService &operator=(const KWalletFreedesktopService &) = delete;

    KWalletFreedesktopService(KWalletFreedesktopService &&) = delete;
    KWalletFreedesktopService &operator=(KWalletFreedesktopService &&) = delete;

    static QString wrapToCollectionPath(const QString &itemPath);

    static QDBusObjectPath nextPromptPath();
    KWalletD *backend() const;
    QDBusObjectPath fdoObjectPath() const;

    bool desecret(const QDBusMessage &message, FreedesktopSecret &secret);
    bool ensecret(const QDBusMessage &message, FreedesktopSecret &secret);
    KWalletFreedesktopItem *getItemByObjectPath(const QDBusObjectPath &path) const;
    KWalletFreedesktopCollection *getCollectionByWalletName(const QString &walletName) const;
    KWalletFreedesktopPrompt *getPromptByObjectPath(const QDBusObjectPath &path) const;

    FdoUniqueLabel makeUniqueCollectionLabel(const QString &label);
    QString makeUniqueWalletName(const QString &labelPrefix);
    QDBusObjectPath makeUniqueObjectPath(const QString &walletName) const;

    QString resolveIfAlias(QString alias);
    QStringList readAliasesFor(const QString &walletName);
    void createCollectionAlias(const QString &alias, KWalletFreedesktopCollection *collection);
    void createCollectionAlias(const QString &alias, const QString &walletName);
    void updateCollectionAlias(const QString &alias, const QString &walletName);
    void removeAlias(const QString &alias);

    void deletePrompt(const QString &objectPath);
    void deleteSession(const QString &objectPath);
    QDBusObjectPath promptUnlockCollection(const QString &walletName, int handle);

    /* Emitters */
    void onCollectionCreated(const QDBusObjectPath &path);
    void onCollectionChanged(const QDBusObjectPath &path);
    void onCollectionDeleted(const QDBusObjectPath &path);
    void onPropertiesChanged(const QVariantMap &properties);

private Q_SLOTS:
    void lockCollection(const QString &name);
    void entryUpdated(const QString &walletName, const QString &folder, const QString &entryName);
    void entryDeleted(const QString &walletName, const QString &folder, const QString &entryName);
    void entryRenamed(const QString &walletName, const QString &folder, const QString &oldName, const QString &newName);
    void walletDeleted(const QString &walletName);
    void walletCreated(const QString &walletCreated);
    /*
    void slotServiceOwnerChanged(const QString &name, const QString &oldOwner,
                                 const QString &newOwner);
    */

private:
    std::unique_ptr<KWalletFreedesktopSessionAlgorithm> createSessionAlgorithmPlain() const;
    std::unique_ptr<KWalletFreedesktopSessionAlgorithm> createSessionAlgorithmDhAes(const QByteArray &clientKey) const;
    QString createSession(std::unique_ptr<KWalletFreedesktopSessionAlgorithm> algorithm);
    QString defaultWalletName(KConfigGroup &cfg);

private:
    std::map<QString, std::unique_ptr<KWalletFreedesktopSession>> m_sessions;
    std::map<QString, std::unique_ptr<KWalletFreedesktopCollection>> m_collections;
    std::map<QString, std::unique_ptr<KWalletFreedesktopPrompt>> m_prompts;

    uint64_t m_session_counter = 0;

    /*
    QDBusServiceWatcher _serviceWatcher;
    */
    KWalletD *m_parent;
    QCA::Initializer m_init;
    KConfig m_kwalletrc;

    /* Freedesktop API */

    /* org.freedesktop.Secret.Service methods */
public Q_SLOTS:
    QDBusObjectPath CreateCollection(const QVariantMap &properties, const QString &alias, QDBusObjectPath &prompt);
    FreedesktopSecretMap GetSecrets(const QList<QDBusObjectPath> &items, const QDBusObjectPath &session);
    QList<QDBusObjectPath> Lock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &Prompt);
    QDBusVariant OpenSession(const QString &algorithm, const QDBusVariant &input, QDBusObjectPath &result);
    QDBusObjectPath ReadAlias(const QString &name);
    QList<QDBusObjectPath> SearchItems(const StrStrMap &attributes, QList<QDBusObjectPath> &locked);
    void SetAlias(const QString &name, const QDBusObjectPath &collection);
    QList<QDBusObjectPath> Unlock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt);

    /* org.freedesktop.Secret.Service signals */
Q_SIGNALS:
    void CollectionChanged(const QDBusObjectPath &collection);
    void CollectionCreated(const QDBusObjectPath &collection);
    void CollectionDeleted(const QDBusObjectPath &collection);
};

QDataStream &operator<<(QDataStream &stream, const QDBusObjectPath &value);
QDataStream &operator>>(QDataStream &stream, QDBusObjectPath &value);

const QDBusArgument &operator>>(const QDBusArgument &arg, PropertiesMap &value);
QDBusArgument &operator<<(QDBusArgument &arg, const PropertiesMap &value);

QDataStream &operator<<(QDataStream &stream, const QCA::SecureArray &value);
QDataStream &operator>>(QDataStream &stream, QCA::SecureArray &value);
QDBusArgument &operator<<(QDBusArgument &arg, const QCA::SecureArray &value);
const QDBusArgument &operator>>(const QDBusArgument &arg, QCA::SecureArray &buf);

void explicit_zero_mem(void *data, size_t size);

#endif
