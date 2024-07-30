/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPCOLLECTION_H_
#define _KWALLETFREEDESKTOPCOLLECTION_H_
#include "kwalletfreedesktopattributes.h"
#include "kwalletfreedesktopservice.h"

#define FDO_SECRETS_COLLECTION_PATH FDO_SECRETS_SERVICE_OBJECT "/collection/"
#define FDO_SECRETS_DEFAULT_DIR "Secret Service"
#define FDO_KEY_MODIFIED QStringLiteral("$fdo_modified")
#define FDO_KEY_CREATED QStringLiteral("$fdo_created")
#define FDO_KEY_MIME QStringLiteral("$fdo_mime_type")
#define FDO_KEY_XDG_SCHEMA QStringLiteral("xdg:schema")

class KWalletFreedesktopItem;

class KWalletFreedesktopCollection : public QObject, protected QDBusContext
{
    /* org.freedesktop.Secret.Collection properties */
public:
    Q_PROPERTY(qulonglong Created READ created)
    qulonglong created() const;

    Q_PROPERTY(qulonglong Modified READ modified)
    qulonglong modified() const;

    Q_PROPERTY(QList<QDBusObjectPath> Items READ items)
    QList<QDBusObjectPath> items() const;

    Q_PROPERTY(QString Label READ label WRITE setLabel)
    const QString &label() const;
    void setLabel(const QString &value);

    Q_PROPERTY(bool Locked READ locked)
    bool locked() const;

    Q_OBJECT

public:
    KWalletFreedesktopCollection(KWalletFreedesktopService *service, int handle, const QString &walletName, QDBusObjectPath objectPath);

    KWalletFreedesktopCollection(const KWalletFreedesktopCollection &) = delete;
    KWalletFreedesktopCollection &operator=(const KWalletFreedesktopCollection &) = delete;

    KWalletFreedesktopCollection(KWalletFreedesktopCollection &&) = delete;
    KWalletFreedesktopCollection &&operator=(KWalletFreedesktopCollection &&) = delete;

    EntryLocation makeUniqueEntryLocation(const QString &label);
    FdoUniqueLabel makeUniqueItemLabel(const QString &label);

    QDBusObjectPath nextItemPath();

    KWalletFreedesktopService *fdoService() const;
    KWalletD *backend() const;
    QDBusObjectPath fdoObjectPath() const;
    const FdoUniqueLabel &uniqueLabel() const;
    QString walletName() const;
    int walletHandle() const;

    KWalletFreedesktopItem *getItemByObjectPath(const QString &objectPath) const;
    KWalletFreedesktopItem *findItemByEntryLocation(const EntryLocation &entryLocation) const;
    KWalletFreedesktopItem &pushNewItem(FdoUniqueLabel label, const QDBusObjectPath &path);
    KWalletFreedesktopAttributes &itemAttributes();
    const KWalletFreedesktopAttributes &itemAttributes() const;

    /* Emitters */
    void onWalletChangeState(int handle);
    void onItemCreated(const QDBusObjectPath &item);
    void onItemChanged(const QDBusObjectPath &item);
    void onItemDeleted(const QDBusObjectPath &item);
    void onPropertiesChanged(const QVariantMap &properties);

private:
    KWalletFreedesktopItem &pushNewItem(const QString &label, const QDBusObjectPath &path);

private:
    KWalletFreedesktopService *m_service;
    int m_handle;
    FdoUniqueLabel m_uniqueLabel;
    QDBusObjectPath m_objectPath;
    KWalletFreedesktopAttributes m_itemAttribs;
    std::map<QString, std::unique_ptr<KWalletFreedesktopItem>> m_items;
    uint64_t m_itemCounter = 0;

    /* Freedesktop API */

    /* org.freedesktop.Secret.Collection methods */
public Q_SLOTS:
    QDBusObjectPath CreateItem(const PropertiesMap &properties, const FreedesktopSecret &secret, bool replace, QDBusObjectPath &prompt);
    QDBusObjectPath Delete();
    QList<QDBusObjectPath> SearchItems(const StrStrMap &attributes);

    /* org.freedesktop.Secret.Service signals */
Q_SIGNALS:
    void ItemChanged(const QDBusObjectPath &item);
    void ItemCreated(const QDBusObjectPath &item);
    void ItemDeleted(const QDBusObjectPath &item);
};

#endif
