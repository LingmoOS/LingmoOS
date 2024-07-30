/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPITEM_H_
#define _KWALLETFREEDESKTOPITEM_H_
#include "kwalletfreedesktopservice.h"

static inline constexpr auto FDO_SS_MAGICK = 0x4950414f44465353ULL;

class KWalletD;
class KWalletFreedesktopCollection;

class KWalletFreedesktopItem : public QObject, protected FDO_DBUS_CONTEXT
{
    /* org.freedesktop.Secret.Item properties */
public:
    Q_PROPERTY(StrStrMap Attributes READ attributes WRITE setAttributes)
    StrStrMap attributes() const;
    void setAttributes(const StrStrMap &value);

    Q_PROPERTY(qulonglong Created READ created)
    qulonglong created() const;

    Q_PROPERTY(QString Label READ label WRITE setLabel)
    QString label() const;
    void setLabel(const QString &value);

    Q_PROPERTY(bool Locked READ locked)
    bool locked() const;

    Q_PROPERTY(qulonglong Modified READ modified)
    qulonglong modified() const;

    Q_PROPERTY(QString Type READ type WRITE setType)
    QString type() const;
    void setType(const QString &value);

    Q_OBJECT

public:
    KWalletFreedesktopItem(KWalletFreedesktopCollection *collection, FdoUniqueLabel uniqLabel, QDBusObjectPath path);
    ~KWalletFreedesktopItem();

    KWalletFreedesktopItem(const KWalletFreedesktopItem &) = delete;
    KWalletFreedesktopItem &operator=(const KWalletFreedesktopItem &) = delete;

    KWalletFreedesktopItem(KWalletFreedesktopItem &&) = delete;
    KWalletFreedesktopItem &operator=(KWalletFreedesktopItem &&) = delete;

    KWalletFreedesktopCollection *fdoCollection() const;
    KWalletFreedesktopService *fdoService() const;
    KWalletD *backend() const;
    QDBusObjectPath fdoObjectPath() const;
    const FdoUniqueLabel &uniqueLabel() const;
    void uniqueLabel(const FdoUniqueLabel &uniqLabel);

    /*
    QVariantMap readMap() const;
    void writeMap(const QVariantMap &data);
    */

    FreedesktopSecret getSecret(const QDBusConnection &connection, const QDBusMessage &message, const QDBusObjectPath &session);
    void setDeleted();

    /* Emitters */
    void onPropertiesChanged(const QVariantMap &properties);
    void enableFdoFormat();

private:
    KWalletFreedesktopCollection *m_collection;
    FdoUniqueLabel m_uniqueLabel;
    QDBusObjectPath m_path;
    bool m_wasDeleted = false;

    /* Freedesktop API */

    /* org.freedesktop.Secret.Item methods */
public Q_SLOTS:
    QDBusObjectPath Delete();
    FreedesktopSecret GetSecret(const QDBusObjectPath &session);
    void SetSecret(const FreedesktopSecret &secret);
};

#endif
