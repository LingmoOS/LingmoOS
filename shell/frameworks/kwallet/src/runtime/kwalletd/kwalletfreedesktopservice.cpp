/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopservice.h"

#include "kwalletd.h"
#include "kwalletd_debug.h"
#include "kwalletfreedesktopcollection.h"
#include "kwalletfreedesktopitem.h"
#include "kwalletfreedesktopprompt.h"
#include "kwalletfreedesktopserviceadaptor.h"
#include "kwalletfreedesktopsession.h"
#include <KConfigGroup>
#include <QWidget>
#include <string.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

[[maybe_unused]] int DBUS_SECRET_SERVICE_META_TYPE_REGISTER = []() {
    qDBusRegisterMetaType<StrStrMap>();
    qDBusRegisterMetaType<QMap<QString, QString>>();
    qDBusRegisterMetaType<FreedesktopSecret>();
    qDBusRegisterMetaType<FreedesktopSecretMap>();
    qDBusRegisterMetaType<PropertiesMap>();
    qDBusRegisterMetaType<QCA::SecureArray>();

    return 0;
}();

namespace
{
QString mangleInvalidObjectPathChars(const QString &str)
{
    const auto utf8Str = str.toUtf8();
    static constexpr char hex[] = "0123456789abcdef";
    static_assert(sizeof(hex) == 17);

    QString mangled;
    mangled.reserve(utf8Str.size());

    for (const auto &c : utf8Str) {
        if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') && c != '_') {
            const auto cp = static_cast<quint8>(c);
            mangled.push_back(QChar::fromLatin1('_'));
            mangled.push_back(QChar::fromLatin1(hex[cp >> 4]));
            mangled.push_back(QChar::fromLatin1(hex[cp & 0x0f]));
        } else {
            mangled.push_back(QChar::fromLatin1(c));
        }
    }

    return mangled;
}
}

#define LABEL_NUMBER_PREFIX "__"
#define LABEL_NUMBER_POSTFIX "_"
#define LABEL_NUMBER_REGEX "(^.*)" LABEL_NUMBER_PREFIX "(\\d+)" LABEL_NUMBER_POSTFIX "$"

EntryLocation EntryLocation::fromUniqueLabel(const FdoUniqueLabel &uniqLabel)
{
    QString dir;
    QString name = uniqLabel.label;

    const int slashPos = uniqLabel.label.indexOf(QChar::fromLatin1('/'));
    if (slashPos == -1 || slashPos == uniqLabel.label.size() - 1) {
        dir = QStringLiteral(FDO_SECRETS_DEFAULT_DIR);
    } else {
        dir = uniqLabel.label.left(slashPos);
        name = uniqLabel.label.right((uniqLabel.label.size() - dir.size()) - 1);
    }

    return EntryLocation{dir, FdoUniqueLabel::makeName(name, uniqLabel.copyId)};
}

FdoUniqueLabel EntryLocation::toUniqueLabel() const
{
    return FdoUniqueLabel::fromEntryLocation(*this);
}

FdoUniqueLabel FdoUniqueLabel::fromEntryLocation(const EntryLocation &entryLocation)
{
    const auto uniqLabel = FdoUniqueLabel::fromName(entryLocation.key);

    if (entryLocation.folder == QStringLiteral(FDO_SECRETS_DEFAULT_DIR)) {
        return uniqLabel;
    } else {
        return {entryLocation.folder + QChar::fromLatin1('/') + uniqLabel.label, uniqLabel.copyId};
    }
}

FdoUniqueLabel FdoUniqueLabel::fromName(const QString &name)
{
    static QRegularExpression regexp(QStringLiteral(LABEL_NUMBER_REGEX));

    const auto match = regexp.match(name);
    if (match.hasMatch()) {
        const QString strNum = match.captured(2);
        bool ok = false;
        const int n = strNum.toInt(&ok);
        if (ok) {
            return FdoUniqueLabel{match.captured(1), n};
        }
    }
    return FdoUniqueLabel{name};
}

QString FdoUniqueLabel::makeName(const QString &label, int n)
{
    if (n == -1) {
        return label;
    } else {
        return label + QStringLiteral(LABEL_NUMBER_PREFIX) + QString::number(n) + QStringLiteral(LABEL_NUMBER_POSTFIX);
    }
}

QString FdoUniqueLabel::toName() const
{
    return makeName(label, copyId);
}

EntryLocation FdoUniqueLabel::toEntryLocation() const
{
    return EntryLocation::fromUniqueLabel(*this);
}

QString KWalletFreedesktopService::wrapToCollectionPath(const QString &itemPath)
{
    /* Take only /org/freedesktop/secrets/collection/collection_name */
    return itemPath.section(QChar::fromLatin1('/'), 0, 5);
}

KWalletFreedesktopService::KWalletFreedesktopService(KWalletD *parent)
    : QObject(nullptr)
    , m_parent(parent)
    , m_kwalletrc(QStringLiteral("kwalletrc"))
{
    (void)new KWalletFreedesktopServiceAdaptor(this);

    /* register */
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.freedesktop.secrets"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral(FDO_SECRETS_SERVICE_OBJECT), this);

    const KConfigGroup walletGroup(&m_kwalletrc, "Wallet");
    if (!parent || !walletGroup.readEntry("Enabled", true)) {
        return;
    }

    connect(m_parent, static_cast<void (KWalletD::*)(const QString &)>(&KWalletD::walletClosed), this, &KWalletFreedesktopService::lockCollection);
    connect(m_parent, &KWalletD::entryUpdated, this, &KWalletFreedesktopService::entryUpdated);
    connect(m_parent, &KWalletD::entryDeleted, this, &KWalletFreedesktopService::entryDeleted);
    connect(m_parent, &KWalletD::entryRenamed, this, &KWalletFreedesktopService::entryRenamed);
    connect(m_parent, &KWalletD::walletDeleted, this, &KWalletFreedesktopService::walletDeleted);
    connect(m_parent, &KWalletD::walletCreated, this, &KWalletFreedesktopService::walletCreated);

    const auto walletNames = backend()->wallets();

    /* Build collections */
    for (const QString &walletName : walletNames) {
        const auto objectPath = makeUniqueObjectPath(walletName);
        auto collection = std::make_unique<KWalletFreedesktopCollection>(this, -1, walletName, objectPath);

        m_collections.emplace(objectPath.path(), std::move(collection));
    }
}

KWalletFreedesktopService::~KWalletFreedesktopService() = default;

QList<QDBusObjectPath> KWalletFreedesktopService::collections() const
{
    QList<QDBusObjectPath> result;
    result.reserve(m_collections.size());

    for (const auto &collectionPair : m_collections) {
        result.push_back(QDBusObjectPath(collectionPair.first));
    }

    return result;
}

QDBusObjectPath KWalletFreedesktopService::CreateCollection(const QVariantMap &properties, const QString &alias, QDBusObjectPath &prompt)
{
    prompt.setPath(QStringLiteral("/"));

    const auto labelIter = properties.find(QStringLiteral("org.freedesktop.Secret.Collection.Label"));
    if (labelIter == properties.end()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Collection.Label property is missing"));
        return QDBusObjectPath("/");
    }
    if (!labelIter->canConvert<QString>()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Type of Collection.Label property is invalid"));
        return QDBusObjectPath("/");
    }

    prompt = nextPromptPath();
    auto fdoPromptPtr = std::make_unique<KWalletFreedesktopPrompt>(this, prompt, PromptType::Create, message().service());
    auto &fdoPrompt = *m_prompts.emplace(prompt.path(), std::move(fdoPromptPtr)).first->second;

    fdoPrompt.appendProperties(labelIter->toString(), QDBusObjectPath("/"), alias);
    fdoPrompt.subscribeForWalletAsyncOpened();

    return QDBusObjectPath("/");
}

FreedesktopSecretMap KWalletFreedesktopService::GetSecrets(const QList<QDBusObjectPath> &items, const QDBusObjectPath &session)
{
    FreedesktopSecretMap result;

    for (const QDBusObjectPath &itemPath : items) {
        const auto item = getItemByObjectPath(itemPath);

        if (item) {
            result.insert(itemPath, item->getSecret(connection(), message(), session));
        } else {
            sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Can't find item at path ") + itemPath.path());
            break;
        }
    }

    return result;
}

QList<QDBusObjectPath> KWalletFreedesktopService::Lock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt)
{
    prompt = QDBusObjectPath("/");
    QList<QDBusObjectPath> result;

    /* Try find in active collections */
    for (const QDBusObjectPath &object : objects) {
        const QString collectionPath = wrapToCollectionPath(resolveIfAlias(object.path()));

        const auto foundCollection = m_collections.find(collectionPath);
        if (foundCollection != m_collections.end()) {
            const int walletHandle = foundCollection->second->walletHandle();
            const int rc = m_parent->close(walletHandle, true, FDO_APPID, message());

            if (rc == 0) {
                result.push_back(QDBusObjectPath(collectionPath));
            } else {
                sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Can't lock object at path ") + collectionPath);
            }
        } else {
            sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Collection at path ") + collectionPath + QStringLiteral(" does not exist"));
        }
    }

    return result;
}

QDBusVariant KWalletFreedesktopService::OpenSession(const QString &algorithm, const QDBusVariant &input, QDBusObjectPath &result)
{
    std::unique_ptr<KWalletFreedesktopSessionAlgorithm> sessionAlgorithm;
    if (algorithm == QStringLiteral("plain")) {
        sessionAlgorithm = createSessionAlgorithmPlain();
    } else if (algorithm == QStringLiteral("dh-ietf1024-sha256-aes128-cbc-pkcs7")) {
        if (!input.variant().canConvert<QByteArray>()) {
            sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Second input argument must be a byte array."));
            return {};
        }

        sessionAlgorithm = createSessionAlgorithmDhAes(input.variant().toByteArray());
    } else {
        sendErrorReply(QDBusError::ErrorType::NotSupported,
                       QStringLiteral("Algorithm ") + algorithm
                           + QStringLiteral(" is not supported. (only plain and dh-ietf1024-sha256-aes128-cbc-pkcs7 are supported)"));
        return {};
    }

    if (!sessionAlgorithm) {
        // Creation of session algorithm failed, createSessionAlgorithm...() method should have sent an error reply.
        return {};
    }

    const QString sessionPath = createSession(std::move(sessionAlgorithm));
    result.setPath(sessionPath);
    return QDBusVariant(QVariant(m_sessions[sessionPath]->negotiationOutput()));
}

QDBusObjectPath KWalletFreedesktopService::ReadAlias(const QString &name)
{
    QString walletName;

    m_kwalletrc.reparseConfiguration();
    if (name == QStringLiteral("default")) {
        KConfigGroup cfg(&m_kwalletrc, "Wallet");
        walletName = defaultWalletName(cfg);

    } else {
        KConfigGroup cfg(&m_kwalletrc, "org.freedesktop.secrets.aliases");
        walletName = cfg.readEntry(name, QString());
    }

    if (!walletName.isEmpty()) {
        const auto *collection = getCollectionByWalletName(walletName);
        if (collection) {
            return collection->fdoObjectPath();
        }
    }

    return QDBusObjectPath("/");
}

QList<QDBusObjectPath> KWalletFreedesktopService::SearchItems(const StrStrMap &attributes, QList<QDBusObjectPath> &locked)
{
    QList<QDBusObjectPath> unlocked;

    for (const auto &collectionPair : m_collections) {
        auto &collection = *collectionPair.second;

        if (collection.locked()) {
            locked += collection.SearchItems(attributes);
        } else {
            unlocked += collection.SearchItems(attributes);
        }
    }

    return unlocked;
}

void KWalletFreedesktopService::SetAlias(const QString &name, const QDBusObjectPath &collectionPath)
{
    const auto foundCollection = m_collections.find(collectionPath.path());
    if (foundCollection == m_collections.end()) {
        return;
    }

    auto *collection = foundCollection->second.get();
    createCollectionAlias(name, collection);
}

QString KWalletFreedesktopService::resolveIfAlias(QString alias)
{
    if (alias.startsWith(QStringLiteral(FDO_ALIAS_PATH))) {
        const auto path = ReadAlias(alias.remove(0, QStringLiteral(FDO_ALIAS_PATH).size())).path();
        if (path != QStringLiteral("/")) {
            alias = path;
        } else {
            sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Alias ") + alias + QStringLiteral(" does not exist"));
            return {};
        }
    }

    if (!alias.startsWith(QStringLiteral(FDO_SECRETS_COLLECTION_PATH))) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Collection object path is invalid"));
        return {};
    }

    return alias;
}

struct UnlockedObject {
    QString walletName;
    QDBusObjectPath objectPath;
};

QList<QDBusObjectPath> KWalletFreedesktopService::Unlock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt)
{
    prompt = QDBusObjectPath("/");

    QList<QDBusObjectPath> result;
    QList<UnlockedObject> needUnlock;

    /* Try find in active collections */
    for (const QDBusObjectPath &object : objects) {
        const QString strPath = object.path();
        const QString collectionPath = wrapToCollectionPath(resolveIfAlias(strPath));

        const auto foundCollection = m_collections.find(collectionPath);
        if (foundCollection != m_collections.end()) {
            if (foundCollection->second->locked()) {
                needUnlock.push_back({foundCollection->second->walletName(), QDBusObjectPath(strPath)});
            } else {
                result.push_back(QDBusObjectPath(strPath));
            }
        } else {
            sendErrorReply(QDBusError::ErrorType::InvalidObjectPath, QStringLiteral("Object ") + strPath + QStringLiteral(" does not exist"));
            return {};
        }
    }

    if (!needUnlock.empty()) {
        const auto promptPath = nextPromptPath();
        auto fdoPromptPtr = std::make_unique<KWalletFreedesktopPrompt>(this, promptPath, PromptType::Open, message().service());
        auto &fdoPrompt = *m_prompts.emplace(promptPath.path(), std::move(fdoPromptPtr)).first->second;

        prompt = QDBusObjectPath(promptPath);

        for (const auto &[walletName, objectPath] : std::as_const(needUnlock)) {
            fdoPrompt.appendProperties(walletName, objectPath);
        }

        fdoPrompt.subscribeForWalletAsyncOpened();
    }
    return result;
}

std::unique_ptr<KWalletFreedesktopSessionAlgorithm> KWalletFreedesktopService::createSessionAlgorithmPlain() const
{
    return std::make_unique<KWalletFreedesktopSessionAlgorithmPlain>();
}

std::unique_ptr<KWalletFreedesktopSessionAlgorithm> KWalletFreedesktopService::createSessionAlgorithmDhAes(const QByteArray &clientKey) const
{
    if (clientKey.size() < FDO_DH_PUBLIC_KEY_SIZE) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Client public key size is invalid"));
        return nullptr;
    }

    QCA::KeyGenerator keygen;
    const auto dlGroup = QCA::DLGroup(keygen.createDLGroup(QCA::IETF_1024));
    if (dlGroup.isNull()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("createDLGroup failed: maybe libqca-ossl is missing"));
        return nullptr;
    }

    auto privateKey = QCA::PrivateKey(keygen.createDH(dlGroup));
    const auto publicKey = QCA::PublicKey(privateKey);
    const auto clientPublicKey = QCA::DHPublicKey(dlGroup, QCA::BigInteger(QCA::SecureArray(clientKey)));
    const auto commonSecret = privateKey.deriveKey(clientPublicKey);
    const auto symmetricKey = QCA::HKDF().makeKey(commonSecret, {}, {}, FDO_SECRETS_CIPHER_KEY_SIZE);

    return std::make_unique<KWalletFreedesktopSessionAlgorithmDhAes>(publicKey, symmetricKey);
}

QString KWalletFreedesktopService::createSession(std::unique_ptr<KWalletFreedesktopSessionAlgorithm> algorithm)
{
    const QString sessionPath = QStringLiteral(FDO_SECRETS_SESSION_PATH) + QString::number(++m_session_counter);
    auto session = std::make_unique<KWalletFreedesktopSession>(this, std::move(algorithm), sessionPath, connection(), message());
    m_sessions[sessionPath] = std::move(session);
    return sessionPath;
}

QString KWalletFreedesktopService::defaultWalletName(KConfigGroup &cfg)
{
    auto walletName = cfg.readEntry("Default Wallet", "kdewallet");
    if (walletName.isEmpty()) {
        walletName = QStringLiteral("kdewallet");
    }
    return walletName;
}

QDBusObjectPath KWalletFreedesktopService::promptUnlockCollection(const QString &walletName, int handle)
{
    auto *collection = getCollectionByWalletName(walletName);
    QString objectPath;

    if (collection) {
        collection->onWalletChangeState(handle);
        onCollectionChanged(collection->fdoObjectPath());
        objectPath = collection->fdoObjectPath().path();
    } else {
        const auto path = makeUniqueObjectPath(walletName);
        objectPath = path.path();
        auto newCollection = std::make_unique<KWalletFreedesktopCollection>(this, handle, walletName, path);
        m_collections[objectPath] = std::move(newCollection);
        onCollectionCreated(path);
    }

    return QDBusObjectPath(objectPath);
}

/* Triggered after KWalletD::walletClosed signal */
void KWalletFreedesktopService::lockCollection(const QString &name)
{
    auto *collection = getCollectionByWalletName(name);
    if (collection) {
        collection->onWalletChangeState(-1);
        onCollectionChanged(collection->fdoObjectPath());
    }
}

/* Triggered after KWalletD::entryUpdated signal */
void KWalletFreedesktopService::entryUpdated(const QString &walletName, const QString &folder, const QString &entryName)
{
    auto *collection = getCollectionByWalletName(walletName);
    if (!collection) {
        return;
    }

    const EntryLocation entryLocation{folder, entryName};
    const auto *item = collection->findItemByEntryLocation(entryLocation);
    if (item) {
        collection->onItemChanged(item->fdoObjectPath());
    } else {
        auto objectPath = collection->nextItemPath();
        collection->pushNewItem(entryLocation.toUniqueLabel(), objectPath);
        collection->onItemCreated(objectPath);
    }
}

/* Triggered after KWalletD::entryDeleted signal */
void KWalletFreedesktopService::entryDeleted(const QString &walletName, const QString &folder, const QString &entryName)
{
    auto *collection = getCollectionByWalletName(walletName);
    if (!collection) {
        return;
    }

    const auto *item = collection->findItemByEntryLocation({folder, entryName});
    if (item) {
        collection->onItemDeleted(item->fdoObjectPath());
    }
}

/* Triggered after KWalletD::entryRenamed signal */
void KWalletFreedesktopService::entryRenamed(const QString &walletName, const QString &folder, const QString &oldName, const QString &newName)
{
    auto *collection = getCollectionByWalletName(walletName);
    if (!collection) {
        return;
    }

    const EntryLocation oldLocation{folder, oldName};
    const EntryLocation newLocation{folder, newName};

    auto *item = collection->findItemByEntryLocation(oldLocation);
    if (!item) {
        /* Warn if label not found and not yet renamed */
        if (!collection->findItemByEntryLocation(newLocation)) {
            qCWarning(KWALLETD_LOG) << "Cannot rename secret service label:" << FdoUniqueLabel::fromEntryLocation(oldLocation).label;
        }
        return;
    }

    if (item) {
        collection->itemAttributes().renameLabel(oldLocation, newLocation);
        item->uniqueLabel(newLocation.toUniqueLabel());
        collection->onItemChanged(item->fdoObjectPath());
    }
}

/* Triggered after KWalletD::walletDeleted signal */
void KWalletFreedesktopService::walletDeleted(const QString &walletName)
{
    auto *collection = getCollectionByWalletName(walletName);
    if (collection) {
        collection->Delete();
    }
}

/* Triggered after KWalletD::walletCreated signal */
void KWalletFreedesktopService::walletCreated(const QString &walletName)
{
    const auto objectPath = makeUniqueObjectPath(walletName);
    auto collection = std::make_unique<KWalletFreedesktopCollection>(this, -1, walletName, objectPath);
    m_collections.emplace(objectPath.path(), std::move(collection));
    onCollectionCreated(objectPath);
}

bool KWalletFreedesktopService::desecret(const QDBusMessage &message, FreedesktopSecret &secret)
{
    const auto foundSession = m_sessions.find(secret.session.path());

    if (foundSession != m_sessions.end()) {
        const KWalletFreedesktopSession &session = *foundSession->second;
        return session.decrypt(message, secret);
    }

    return false;
}

bool KWalletFreedesktopService::ensecret(const QDBusMessage &message, FreedesktopSecret &secret)
{
    const auto foundSession = m_sessions.find(secret.session.path());

    if (foundSession != m_sessions.end()) {
        const KWalletFreedesktopSession &session = *foundSession->second;
        return session.encrypt(message, secret);
    }

    return false;
}

QDBusObjectPath KWalletFreedesktopService::nextPromptPath()
{
    static uint64_t id = 0;
    return QDBusObjectPath(QStringLiteral(FDO_SECRET_SERVICE_PROMPT_PATH) + QStringLiteral("p") + QString::number(id++));
}

QDBusArgument &operator<<(QDBusArgument &arg, const FreedesktopSecret &secret)
{
    arg.beginStructure();
    arg << secret.session;
    arg << secret.parameters;
    arg << secret.value;
    arg << secret.mimeType;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, FreedesktopSecret &secret)
{
    arg.beginStructure();
    arg >> secret.session;
    arg >> secret.parameters;
    arg >> secret.value;
    arg >> secret.mimeType;
    arg.endStructure();
    return arg;
}

QDataStream &operator<<(QDataStream &stream, const QCA::SecureArray &value)
{
    QByteArray bytes = value.toByteArray();
    stream << bytes;
    explicit_zero_mem(bytes.data(), bytes.size());
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QCA::SecureArray &value)
{
    QByteArray bytes;
    stream >> bytes;
    value = QCA::SecureArray(bytes);
    explicit_zero_mem(bytes.data(), bytes.size());
    return stream;
}

QDBusArgument &operator<<(QDBusArgument &arg, const QCA::SecureArray &value)
{
    QByteArray bytes = value.toByteArray();
    arg << bytes;
    explicit_zero_mem(bytes.data(), bytes.size());
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, QCA::SecureArray &buf)
{
    QByteArray byteArray;
    arg >> byteArray;
    buf = QCA::SecureArray(byteArray);
    explicit_zero_mem(byteArray.data(), byteArray.size());
    return arg;
}

KWalletD *KWalletFreedesktopService::backend() const
{
    return m_parent;
}

QDBusObjectPath KWalletFreedesktopService::fdoObjectPath() const
{
    return QDBusObjectPath(FDO_SECRETS_SERVICE_OBJECT);
}

KWalletFreedesktopItem *KWalletFreedesktopService::getItemByObjectPath(const QDBusObjectPath &path) const
{
    const auto str = path.path();
    if (!str.startsWith(QStringLiteral(FDO_SECRETS_COLLECTION_PATH))) {
        return nullptr;
    }

    const QString collectionPath = wrapToCollectionPath(str);
    const auto collectionPos = m_collections.find(collectionPath);
    if (collectionPos == m_collections.end()) {
        return nullptr;
    }

    const auto &collection = collectionPos->second;
    return collection->getItemByObjectPath(str);
}

KWalletFreedesktopPrompt *KWalletFreedesktopService::getPromptByObjectPath(const QDBusObjectPath &path) const
{
    const auto foundPrompt = m_prompts.find(path.path());
    if (foundPrompt != m_prompts.end()) {
        return foundPrompt->second.get();
    } else {
        return nullptr;
    }
}

FdoUniqueLabel KWalletFreedesktopService::makeUniqueCollectionLabel(const QString &label)
{
    int n = -1;
    auto walletName = label;
    const QStringList wallets = backend()->wallets();

    while (wallets.contains(walletName)) {
        walletName = FdoUniqueLabel::makeName(label, ++n);
    }

    return {label, n};
}

QString KWalletFreedesktopService::makeUniqueWalletName(const QString &labelPrefix)
{
    return makeUniqueCollectionLabel(labelPrefix).toName();
}

QDBusObjectPath KWalletFreedesktopService::makeUniqueObjectPath(const QString &walletName) const
{
    auto mangled = mangleInvalidObjectPathChars(walletName);
    mangled.insert(0, QStringLiteral(FDO_SECRETS_COLLECTION_PATH));

    QString result = mangled;
    int postfix = 0;
    while (m_collections.count(result)) {
        result = mangled + QString::number(postfix++);
    }

    return QDBusObjectPath(result);
}

QStringList KWalletFreedesktopService::readAliasesFor(const QString &walletName)
{
    m_kwalletrc.reparseConfiguration();
    KConfigGroup cfg(&m_kwalletrc, "org.freedesktop.secrets.aliases");
    const auto map = cfg.entryMap();
    QStringList aliases;

    for (auto i = map.begin(); i != map.end(); ++i) {
        if (i.value() == walletName) {
            aliases.push_back(i.key());
        }
    }

    KConfigGroup cfgWallet(&m_kwalletrc, "Wallet");
    if (defaultWalletName(cfgWallet) == walletName) {
        aliases.push_back(QStringLiteral("default"));
    }

    return aliases;
}

void KWalletFreedesktopService::updateCollectionAlias(const QString &alias, const QString &walletName)
{
    QString sectName = QStringLiteral("org.freedesktop.secrets.aliases");
    QString sectKey = alias;

    if (alias == QStringLiteral("default")) {
        sectName = QStringLiteral("Wallet");
        sectKey = QStringLiteral("Default Wallet");
    }

    KConfigGroup cfg(&m_kwalletrc, sectName);
    cfg.writeEntry(sectKey, walletName);
    m_kwalletrc.sync();
}

void KWalletFreedesktopService::createCollectionAlias(const QString &alias, const QString &walletName)
{
    QString sectName = QStringLiteral("org.freedesktop.secrets.aliases");
    QString sectKey = alias;

    if (alias == QStringLiteral("default")) {
        sectName = QStringLiteral("Wallet");
        sectKey = QStringLiteral("Default Wallet");
    }

    m_kwalletrc.reparseConfiguration();
    KConfigGroup cfg(&m_kwalletrc, sectName);

    const QString prevWalletName = cfg.readEntry(sectKey, QString());
    if (!prevWalletName.isEmpty()) {
        const auto *prevCollection = getCollectionByWalletName(prevWalletName);
        if (prevCollection) {
            QDBusConnection::sessionBus().unregisterObject(QStringLiteral(FDO_ALIAS_PATH) + alias);
        }
    }

    cfg.writeEntry(sectKey, walletName);
    m_kwalletrc.sync();

    auto *collection = getCollectionByWalletName(walletName);
    if (collection) {
        QDBusConnection::sessionBus().registerObject(QStringLiteral(FDO_ALIAS_PATH) + alias, collection);
    }
}

void KWalletFreedesktopService::createCollectionAlias(const QString &alias, KWalletFreedesktopCollection *collection)
{
    QString sectName = QStringLiteral("org.freedesktop.secrets.aliases");
    QString sectKey = alias;

    if (alias == QStringLiteral("default")) {
        sectName = QStringLiteral("Wallet");
        sectKey = QStringLiteral("Default Wallet");
    }

    m_kwalletrc.reparseConfiguration();
    KConfigGroup cfg(&m_kwalletrc, sectName);

    const QString prevWalletName = cfg.readEntry(sectKey, "");
    if (!prevWalletName.isEmpty()) {
        const auto *prevCollection = getCollectionByWalletName(prevWalletName);
        if (prevCollection) {
            QDBusConnection::sessionBus().unregisterObject(QStringLiteral(FDO_ALIAS_PATH) + alias);
        }
    }

    cfg.writeEntry(sectKey, collection->walletName());
    m_kwalletrc.sync();
    QDBusConnection::sessionBus().registerObject(QStringLiteral(FDO_ALIAS_PATH) + alias, collection);
}

void KWalletFreedesktopService::removeAlias(const QString &alias)
{
    if (alias == QStringLiteral("default")) {
        return;
    }

    KConfigGroup cfg(&m_kwalletrc, "org.freedesktop.secrets.aliases");
    cfg.deleteEntry(alias);
    m_kwalletrc.sync();
    QDBusConnection::sessionBus().unregisterObject(QStringLiteral(FDO_ALIAS_PATH) + alias);
}

KWalletFreedesktopCollection *KWalletFreedesktopService::getCollectionByWalletName(const QString &walletName) const
{
    for (const auto &collectionKeyValue : m_collections) {
        const auto collection = collectionKeyValue.second.get();
        if (collection->walletName() == walletName) {
            return collection;
        }
    }

    return nullptr;
}

void KWalletFreedesktopService::deletePrompt(const QString &objectPath)
{
    const auto foundPrompt = m_prompts.find(objectPath);
    if (foundPrompt == m_prompts.end()) {
        return;
    }

    /* This can be called in the context of the prompt that is currently being
     * deleted. Therefore, we should schedule deletion on the next event loop iteration
     */
    foundPrompt->second->deleteLater();
    foundPrompt->second.release();
    m_prompts.erase(foundPrompt);
}

void KWalletFreedesktopService::deleteSession(const QString &objectPath)
{
    const auto foundSession = m_sessions.find(objectPath);
    if (foundSession == m_sessions.end()) {
        return;
    }

    /* This can be called in the context of the session that is currently being
     * deleted. Therefore, we should schedule deletion on the next event loop iteration
     */
    foundSession->second->deleteLater();
    foundSession->second.release();
    m_sessions.erase(foundSession);
}

void KWalletFreedesktopService::onCollectionCreated(const QDBusObjectPath &path)
{
    Q_EMIT CollectionCreated(path);

    QVariantMap props;
    props.insert(QStringLiteral("Collections"), QVariant::fromValue(collections()));
    onPropertiesChanged(props);
}

void KWalletFreedesktopService::onCollectionChanged(const QDBusObjectPath &path)
{
    Q_EMIT CollectionChanged(path);
}

void KWalletFreedesktopService::onCollectionDeleted(const QDBusObjectPath &path)
{
    const auto collectionMapPos = m_collections.find(path.path());
    if (collectionMapPos == m_collections.end()) {
        return;
    }
    auto &collectionPair = *collectionMapPos;
    collectionPair.second->itemAttributes().deleteFile();

    /* This can be called in the context of the collection that is currently being
     * deleted. Therefore, we should schedule deletion on the next event loop iteration
     */
    collectionPair.second->deleteLater();
    collectionPair.second.release();
    m_collections.erase(collectionMapPos);

    Q_EMIT CollectionDeleted(path);

    QVariantMap props;
    props[QStringLiteral("Collections")] = QVariant::fromValue(collections());
    onPropertiesChanged(props);
}

void KWalletFreedesktopService::onPropertiesChanged(const QVariantMap &properties)
{
    auto msg = QDBusMessage::createSignal(fdoObjectPath().path(), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    auto args = QVariantList();
    args << QStringLiteral("org.freedesktop.Secret.Service") << properties << QStringList();
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

QDataStream &operator<<(QDataStream &stream, const QDBusObjectPath &value)
{
    return stream << value.path();
}

QDataStream &operator>>(QDataStream &stream, QDBusObjectPath &value)
{
    QString str;
    stream >> str;
    value = QDBusObjectPath(str);
    return stream;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, PropertiesMap &value)
{
    arg.beginMap();
    value.map.clear();

    while (!arg.atEnd()) {
        arg.beginMapEntry();
        QString key;
        QVariant val;
        arg >> key >> val;

        /* For org.freedesktop.Secret.Item.Attributes */
        if (val.canConvert<QDBusArgument>()) {
            auto metaArg = val.value<QDBusArgument>();
            StrStrMap metaMap;
            metaArg >> metaMap;
            val = QVariant::fromValue(metaMap);
        }
        value.map.insert(key, val);

        arg.endMapEntry();
    }
    arg.endMap();

    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const PropertiesMap &value)
{
    arg << value.map;
    return arg;
}

void explicit_zero_mem(void *data, size_t size)
{
#if defined(KWALLETD_HAVE_EXPLICIT_BZERO)
    explicit_bzero(data, size);
#elif defined(KWALLETD_HAVE_RTLSECUREZEROMEMORY)
    RtlSecureZeroMemory(data, size);
#else
    auto p = reinterpret_cast<volatile char *>(data);
    for (size_t i = 0; i < size; ++i) {
        p[i] = 0;
    }
#endif
}

#include "moc_kwalletfreedesktopservice.cpp"
