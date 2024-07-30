/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopcollection.h"

#include "kwalletd.h"
#include "kwalletfreedesktopcollectionadaptor.h"
#include "kwalletfreedesktopitem.h"

KWalletFreedesktopCollection::KWalletFreedesktopCollection(KWalletFreedesktopService *service,
                                                           int handle,
                                                           const QString &walletName,
                                                           QDBusObjectPath objectPath)
    : m_service(service)
    , m_handle(handle)
    , m_uniqueLabel(FdoUniqueLabel::fromName(walletName))
    , m_objectPath(std::move(objectPath))
    , m_itemAttribs(walletName)
{
    (void)new KWalletFreedesktopCollectionAdaptor(this);
    QDBusConnection::sessionBus().registerObject(fdoObjectPath().path(), this);

    const QStringList aliases = fdoService()->readAliasesFor(walletName);
    for (const auto &alias : aliases) {
        QDBusConnection::sessionBus().registerObject(QStringLiteral(FDO_ALIAS_PATH) + alias, this);
    }

    onWalletChangeState(handle);

    /* Create items described in the attributes file */
    if (m_handle == -1) {
        const auto items = itemAttributes().listItems();
        for (const auto &entryLocation : items) {
            if (!findItemByEntryLocation(entryLocation)) {
                pushNewItem(entryLocation.toUniqueLabel(), nextItemPath());
            }
        }
    }
}

QDBusObjectPath KWalletFreedesktopCollection::nextItemPath()
{
    return QDBusObjectPath(fdoObjectPath().path() + QChar::fromLatin1('/') + QString::number(m_itemCounter++));
}

const QString &KWalletFreedesktopCollection::label() const
{
    return m_uniqueLabel.label;
}

void KWalletFreedesktopCollection::setLabel(const QString &newLabel)
{
    if (newLabel == label()) {
        return;
    }

    const auto oldName = m_uniqueLabel.toName();
    const auto newUniqLabel = fdoService()->makeUniqueCollectionLabel(newLabel);
    const auto newName = newUniqLabel.toName();

    int rc = backend()->renameWallet(oldName, newName);
    if (rc == 0) {
        const QStringList aliases = fdoService()->readAliasesFor(walletName());
        m_uniqueLabel = newUniqLabel;
        const QString newName = walletName();
        for (const auto &alias : aliases) {
            fdoService()->updateCollectionAlias(alias, newName);
        }

        itemAttributes().renameWallet(newName);
    }
}

bool KWalletFreedesktopCollection::locked() const
{
    return m_handle < 0 || !backend()->isOpen(m_handle);
}

QList<QDBusObjectPath> KWalletFreedesktopCollection::items() const
{
    QList<QDBusObjectPath> items;

    for (const auto &item : m_items) {
        items.push_back(item.second->fdoObjectPath());
    }

    return items;
}

qulonglong KWalletFreedesktopCollection::created() const
{
    return itemAttributes().birthTime();
}

qulonglong KWalletFreedesktopCollection::modified() const
{
    return itemAttributes().lastModified();
}

QDBusObjectPath
KWalletFreedesktopCollection::CreateItem(const PropertiesMap &properties, const FreedesktopSecret &secret, bool replace, QDBusObjectPath &prompt)
{
    prompt = QDBusObjectPath("/");

    if (m_handle == -1) {
        sendErrorReply(QStringLiteral("org.freedesktop.Secret.Error.IsLocked"),
                       QStringLiteral("Collection ") + fdoObjectPath().path() + QStringLiteral(" is locked"));
        return QDBusObjectPath("/");
    }

    const auto labelFound = properties.map.find(QStringLiteral("org.freedesktop.Secret.Item.Label"));
    if (labelFound == properties.map.end()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Item label is missing (org.freedesktop.Secret.Item.Label)"));
        return QDBusObjectPath("/");
    }
    if (!labelFound->canConvert<QString>()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Item label is not a string (org.freedesktop.Secret.Item.Label)"));
        return QDBusObjectPath("/");
    }

    const QString fdoLabel = labelFound->toString();
    QString dir, label;
    QDBusObjectPath itemPath;

    StrStrMap attribs;
    const auto attribsFound = properties.map.find(QStringLiteral("org.freedesktop.Secret.Item.Attributes"));
    if (attribsFound != properties.map.end() && attribsFound->canConvert<StrStrMap>()) {
        attribs = attribsFound->value<StrStrMap>();
    }

    if (replace) {
        /* Try find item with same attributes */
        const auto matchedItems = itemAttributes().matchAttributes(attribs);

        if (!matchedItems.empty()) {
            const auto &entryLoc = matchedItems.constFirst();
            const auto item = findItemByEntryLocation(entryLoc);
            if (item) {
                itemPath = item->fdoObjectPath();
                dir = entryLoc.folder;
                label = entryLoc.key;
            }
        }
    }

    if (dir.isEmpty() && label.isEmpty()) {
        const auto entryLocation = makeUniqueEntryLocation(fdoLabel);
        dir = entryLocation.folder;
        label = entryLocation.key;
        itemPath = nextItemPath();
    }

    if (label.isEmpty()) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QStringLiteral("Item label is invalid (org.freedesktop.Secret.Item.Label)"));
        return QDBusObjectPath("/");
    }

    const qulonglong createTime = QDateTime::currentSecsSinceEpoch();
    const EntryLocation entryLoc{dir, label};
    itemAttributes().newItem(entryLoc);
    itemAttributes().setParam(entryLoc, FDO_KEY_MIME, secret.mimeType);
    itemAttributes().setParam(entryLoc, FDO_KEY_CREATED, createTime);
    itemAttributes().setParam(entryLoc, FDO_KEY_MODIFIED, createTime);
    itemAttributes().setAttributes(entryLoc, attribs);

    pushNewItem(entryLoc.toUniqueLabel(), itemPath);

    {
        auto decrypted = secret;
        if (!fdoService()->desecret(message(), decrypted)) {
            sendErrorReply(QDBusError::ErrorType::InvalidObjectPath, QStringLiteral("Can't find session ") + secret.session.path());
            return QDBusObjectPath("/");
        }

        QString xdgSchema = QStringLiteral("org.kde.KWallet.Stream");
        const auto found = attribs.find(FDO_KEY_XDG_SCHEMA);
        if (found != attribs.end()) {
            xdgSchema = found.value();
        }

        if (xdgSchema == QStringLiteral("org.kde.KWallet.Password") || secret.mimeType.startsWith(QStringLiteral("text/"))) {
            auto bytes = decrypted.value.toByteArray();
            auto str = QString::fromUtf8(bytes);
            backend()->writePassword(walletHandle(), dir, label, str, FDO_APPID);
            explicit_zero_mem(bytes.data(), bytes.size());
            explicit_zero_mem(str.data(), str.size() * sizeof(QChar));
        } else {
            auto bytes = decrypted.value.toByteArray();
            backend()->writeEntry(walletHandle(), dir, label, bytes, KWallet::Wallet::Stream, FDO_APPID);
            explicit_zero_mem(bytes.data(), bytes.size());
        }
    }

    onItemCreated(itemPath);

    return itemPath;
}

QDBusObjectPath KWalletFreedesktopCollection::Delete()
{
    const auto name = walletName();

    const QStringList aliases = fdoService()->readAliasesFor(name);
    for (const QString &alias : aliases) {
        fdoService()->removeAlias(alias);
    }

    backend()->deleteWallet(name);
    QDBusConnection::sessionBus().unregisterObject(fdoObjectPath().path());
    m_service->onCollectionDeleted(fdoObjectPath());

    return QDBusObjectPath("/");
}

QList<QDBusObjectPath> KWalletFreedesktopCollection::SearchItems(const StrStrMap &attributes)
{
    QList<QDBusObjectPath> result;

    for (const auto &entryLoc : m_itemAttribs.matchAttributes(attributes)) {
        auto *itm = findItemByEntryLocation(entryLoc);
        if (itm) {
            result.push_back(itm->fdoObjectPath());
        }
    }

    return result;
}

int KWalletFreedesktopCollection::walletHandle() const
{
    return m_handle;
}

KWalletFreedesktopItem *KWalletFreedesktopCollection::getItemByObjectPath(const QString &objectPath) const
{
    const auto found = m_items.find(objectPath);
    if (found != m_items.end()) {
        return found->second.get();
    } else {
        return nullptr;
    }
}

KWalletFreedesktopItem *KWalletFreedesktopCollection::findItemByEntryLocation(const EntryLocation &entryLocation) const
{
    const auto uniqLabel = FdoUniqueLabel::fromEntryLocation(entryLocation);

    for (const auto &itemPair : m_items) {
        auto *item = itemPair.second.get();
        if (item->uniqueLabel() == uniqLabel) {
            return item;
        }
    }

    return nullptr;
}

EntryLocation KWalletFreedesktopCollection::makeUniqueEntryLocation(const QString &label)
{
    QString dir, name;

    const int slashPos = label.indexOf(QChar::fromLatin1('/'));
    if (slashPos == -1 || slashPos == label.size() - 1) {
        dir = QStringLiteral(FDO_SECRETS_DEFAULT_DIR);
        name = label;
    } else {
        dir = label.left(slashPos);
        name = label.mid(slashPos + 1);
    }

    int suffix = 0;
    QString resultName = name;
    while (backend()->hasEntry(m_handle, dir, resultName, FDO_APPID)) {
        resultName = FdoUniqueLabel::makeName(name, suffix++);
    }

    return {dir, resultName};
}

FdoUniqueLabel KWalletFreedesktopCollection::makeUniqueItemLabel(const QString &label)
{
    return makeUniqueEntryLocation(label).toUniqueLabel();
}

KWalletFreedesktopItem &KWalletFreedesktopCollection::pushNewItem(FdoUniqueLabel uniqLabel, const QDBusObjectPath &path)
{
    m_items.erase(path.path());
    auto item = std::make_unique<KWalletFreedesktopItem>(this, std::move(uniqLabel), path);
    return *m_items.emplace(path.path(), std::move(item)).first->second;
}

KWalletFreedesktopItem &KWalletFreedesktopCollection::pushNewItem(const QString &label, const QDBusObjectPath &path)
{
    return pushNewItem(makeUniqueItemLabel(label), path);
}

KWalletFreedesktopService *KWalletFreedesktopCollection::fdoService() const
{
    return m_service;
}

KWalletD *KWalletFreedesktopCollection::backend() const
{
    return fdoService()->backend();
}

QDBusObjectPath KWalletFreedesktopCollection::fdoObjectPath() const
{
    return m_objectPath;
}

const FdoUniqueLabel &KWalletFreedesktopCollection::uniqueLabel() const
{
    return m_uniqueLabel;
}

QString KWalletFreedesktopCollection::walletName() const
{
    return m_uniqueLabel.toName();
}

void KWalletFreedesktopCollection::onWalletChangeState(int handle)
{
    if (handle == m_handle) {
        return;
    }

    if (handle >= 0 && m_handle >= 0) {
        m_handle = handle;
        return;
    }

    m_handle = handle;

    if (m_handle < 0 || !m_items.empty()) {
        return;
    }

    const QStringList folderList = backend()->folderList(m_handle, FDO_APPID);
    for (const QString &folder : folderList) {
        const QStringList entries = backend()->entryList(m_handle, folder, FDO_APPID);

        for (const auto &entry : entries) {
            const EntryLocation entryLoc{folder, entry};
            const auto itm = findItemByEntryLocation(entryLoc);
            if (!itm) {
                auto &newItem = pushNewItem(entryLoc.toUniqueLabel(), nextItemPath());
                Q_EMIT ItemChanged(newItem.fdoObjectPath());
            } else {
                Q_EMIT ItemChanged(itm->fdoObjectPath());
            }
        }
    }
}

void KWalletFreedesktopCollection::onItemCreated(const QDBusObjectPath &item)
{
    itemAttributes().updateLastModified();
    Q_EMIT ItemCreated(item);

    QVariantMap props;
    props[QStringLiteral("Items")] = QVariant::fromValue(items());
    onPropertiesChanged(props);
}

void KWalletFreedesktopCollection::onItemChanged(const QDBusObjectPath &item)
{
    itemAttributes().updateLastModified();
    Q_EMIT ItemChanged(item);
}

void KWalletFreedesktopCollection::onItemDeleted(const QDBusObjectPath &item)
{
    itemAttributes().updateLastModified();
    const auto itemMapPos = m_items.find(item.path());
    if (itemMapPos == m_items.end()) {
        return;
    }
    auto *itemPtr = itemMapPos->second.get();

    /* This can be called in the context of the item that is currently being
     * deleted. Therefore we should schedule deletion on the next event loop iteration
     */
    itemPtr->setDeleted();
    itemPtr->deleteLater();
    itemMapPos->second.release();
    m_items.erase(itemMapPos);

    Q_EMIT ItemDeleted(item);

    QVariantMap props;
    props[QStringLiteral("Items")] = QVariant::fromValue(items());
    onPropertiesChanged(props);
}

void KWalletFreedesktopCollection::onPropertiesChanged(const QVariantMap &properties)
{
    auto msg = QDBusMessage::createSignal(fdoObjectPath().path(), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    auto args = QVariantList();
    args << QStringLiteral("org.freedesktop.Secret.Collection") << properties << QStringList();
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

KWalletFreedesktopAttributes &KWalletFreedesktopCollection::itemAttributes()
{
    return m_itemAttribs;
}

const KWalletFreedesktopAttributes &KWalletFreedesktopCollection::itemAttributes() const
{
    return m_itemAttribs;
}

#include "moc_kwalletfreedesktopcollection.cpp"
