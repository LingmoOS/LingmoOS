/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopitem.h"

#include "kwalletd.h"
#include "kwalletd_debug.h"
#include "kwalletfreedesktopcollection.h"
#include "kwalletfreedesktopitemadaptor.h"

KWalletFreedesktopItem::KWalletFreedesktopItem(KWalletFreedesktopCollection *collection, FdoUniqueLabel uniqLabel, QDBusObjectPath path)
    : m_collection(collection)
    , m_uniqueLabel(std::move(uniqLabel))
    , m_path(std::move(path))
{
    (void)new KWalletFreedesktopItemAdaptor(this);
    QDBusConnection::sessionBus().registerObject(fdoObjectPath().path(), this);
}

KWalletFreedesktopItem::~KWalletFreedesktopItem()
{
    onPropertiesChanged(QVariantMap());

    QDBusConnection::sessionBus().unregisterObject(fdoObjectPath().path());

    if (!m_wasDeleted) {
        m_collection->onItemChanged(fdoObjectPath());
    }
}

StrStrMap KWalletFreedesktopItem::attributes() const
{
    return fdoCollection()->itemAttributes().getAttributes(m_uniqueLabel);
}

void KWalletFreedesktopItem::setAttributes(const StrStrMap &value)
{
    fdoCollection()->itemAttributes().setAttributes(m_uniqueLabel, value);
}

qulonglong KWalletFreedesktopItem::created() const
{
    return fdoCollection()->itemAttributes().getULongLongParam(m_uniqueLabel, FDO_KEY_CREATED, fdoCollection()->modified());
}

qulonglong KWalletFreedesktopItem::modified() const
{
    return fdoCollection()->itemAttributes().getULongLongParam(m_uniqueLabel, FDO_KEY_MODIFIED, fdoCollection()->modified());
}

QString KWalletFreedesktopItem::label() const
{
    return m_uniqueLabel.label;
}

void KWalletFreedesktopItem::setLabel(const QString &value)
{
    const auto entryLocation = m_uniqueLabel.toEntryLocation();
    m_uniqueLabel = fdoCollection()->makeUniqueItemLabel(value);
    const auto newEntryLocation = m_uniqueLabel.toEntryLocation();

    if (newEntryLocation.folder != entryLocation.folder) {
        const auto data = backend()->readEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
        backend()->writeEntry(fdoCollection()->walletHandle(), newEntryLocation.folder, newEntryLocation.key, data, FDO_APPID);
        backend()->removeEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
    } else if (newEntryLocation.key != entryLocation.key) {
        backend()->renameEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, newEntryLocation.key, FDO_APPID);
    }

    fdoCollection()->itemAttributes().setParam(entryLocation, FDO_KEY_MODIFIED, static_cast<qulonglong>(QDateTime::currentSecsSinceEpoch()));
    fdoCollection()->itemAttributes().renameLabel(entryLocation, newEntryLocation);

    fdoCollection()->onItemChanged(fdoObjectPath());
}

bool KWalletFreedesktopItem::locked() const
{
    return m_collection->locked();
}

QString KWalletFreedesktopItem::type() const
{
    const auto attribs = fdoCollection()->itemAttributes().getAttributes(m_uniqueLabel);
    const auto found = attribs.find(FDO_KEY_XDG_SCHEMA);
    if (found != attribs.end()) {
        return found.value();
    } else {
        return QStringLiteral("org.freedesktop.Secret.Generic");
    }
}

void KWalletFreedesktopItem::setType(const QString &value)
{
    auto attribs = fdoCollection()->itemAttributes().getAttributes(m_uniqueLabel);
    attribs[FDO_KEY_XDG_SCHEMA] = value;
    fdoCollection()->itemAttributes().setAttributes(m_uniqueLabel, attribs);
}

QDBusObjectPath KWalletFreedesktopItem::Delete()
{
    const auto entryLocation = m_uniqueLabel.toEntryLocation();

    backend()->removeEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
    QDBusConnection::sessionBus().unregisterObject(fdoObjectPath().path());

    m_collection->onItemDeleted(fdoObjectPath());

    return QDBusObjectPath("/");
}

FreedesktopSecret KWalletFreedesktopItem::getSecret(const QDBusConnection &connection, const QDBusMessage &message, const QDBusObjectPath &session)
{
    const auto entryLocation = m_uniqueLabel.toEntryLocation();
    const auto mimeType = fdoCollection()->itemAttributes().getStringParam(entryLocation, FDO_KEY_MIME, QStringLiteral("application/octet-stream"));

    FreedesktopSecret fdoSecret;

    const auto entryType = backend()->entryType(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
    if (entryType == KWallet::Wallet::Password) {
        auto password = backend()->readPassword(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
        auto bytes = password.toUtf8();
        fdoSecret = FreedesktopSecret(session, bytes, mimeType);
        explicit_zero_mem(bytes.data(), bytes.size());
        explicit_zero_mem(password.data(), password.size() * sizeof(QChar));
    } else {
        auto bytes = backend()->readEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, FDO_APPID);
        fdoSecret = FreedesktopSecret(session, bytes, mimeType);
        explicit_zero_mem(bytes.data(), bytes.size());
    }

    if (!fdoService()->ensecret(message, fdoSecret)) {
        message.setDelayedReply(true);
        connection.send(message.createErrorReply(QDBusError::ErrorType::UnknownObject, QStringLiteral("Can't find session ") + session.path()));
    }

    return fdoSecret;
}

FreedesktopSecret KWalletFreedesktopItem::GetSecret(const QDBusObjectPath &session)
{
    return getSecret(connection(), message(), session);
}

void KWalletFreedesktopItem::SetSecret(const FreedesktopSecret &secret)
{
    const auto entryLocation = m_uniqueLabel.toEntryLocation();

    fdoCollection()->itemAttributes().setParam(entryLocation, FDO_KEY_MIME, secret.mimeType);
    fdoCollection()->itemAttributes().setParam(entryLocation, FDO_KEY_MODIFIED, static_cast<qulonglong>(QDateTime::currentSecsSinceEpoch()));

    auto decrypted = secret;
    if (!fdoService()->desecret(message(), decrypted)) {
        sendErrorReply(QDBusError::ErrorType::UnknownObject, QStringLiteral("Can't find session ") + secret.session.path());
        return;
    }

    QString xdgSchema = QStringLiteral("org.kde.KWallet.Stream");
    const auto attribs = fdoCollection()->itemAttributes().getAttributes(entryLocation);
    const auto found = attribs.find(FDO_KEY_XDG_SCHEMA);
    if (found != attribs.end()) {
        xdgSchema = found.value();
    }

    if (xdgSchema == QStringLiteral("org.kde.KWallet.Password") || secret.mimeType.startsWith(QStringLiteral("text/"))) {
        auto bytes = decrypted.value.toByteArray();
        auto str = QString::fromUtf8(bytes);
        backend()->writePassword(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, str, FDO_APPID);
        explicit_zero_mem(bytes.data(), bytes.size());
        explicit_zero_mem(str.data(), str.size() * sizeof(QChar));
    } else {
        auto bytes = decrypted.value.toByteArray();
        backend()->writeEntry(fdoCollection()->walletHandle(), entryLocation.folder, entryLocation.key, bytes, KWallet::Wallet::Stream, FDO_APPID);
    }
}

KWalletFreedesktopCollection *KWalletFreedesktopItem::fdoCollection() const
{
    return m_collection;
}

KWalletFreedesktopService *KWalletFreedesktopItem::fdoService() const
{
    return fdoCollection()->fdoService();
}

KWalletD *KWalletFreedesktopItem::backend() const
{
    return fdoCollection()->fdoService()->backend();
}

QDBusObjectPath KWalletFreedesktopItem::fdoObjectPath() const
{
    return m_path;
}

const FdoUniqueLabel &KWalletFreedesktopItem::uniqueLabel() const
{
    return m_uniqueLabel;
}

void KWalletFreedesktopItem::uniqueLabel(const FdoUniqueLabel &uniqueLabel)
{
    m_uniqueLabel = uniqueLabel;
}

void KWalletFreedesktopItem::setDeleted()
{
    m_wasDeleted = true;
    fdoCollection()->itemAttributes().remove(m_uniqueLabel);
}

void KWalletFreedesktopItem::onPropertiesChanged(const QVariantMap &properties)
{
    auto msg = QDBusMessage::createSignal(fdoObjectPath().path(), QStringLiteral("org.freedesktop.DBus.Properties"), QStringLiteral("PropertiesChanged"));
    auto args = QVariantList();
    args << QStringLiteral("org.freedesktop.Secret.Item") << properties << QStringList();
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
}

#include "moc_kwalletfreedesktopitem.cpp"
