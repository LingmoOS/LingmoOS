/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef _KWALLETFREEDESKTOPPROMPT_H_
#define _KWALLETFREEDESKTOPPROMPT_H_

#include "kwalletfreedesktopservice.h"

#define FDO_SECRET_SERVICE_PROMPT_PATH FDO_SECRETS_SERVICE_OBJECT "/prompt/"

class KWalletD;

enum class PromptType {
    Open,
    Create,
};

class KWalletFreedesktopPrompt : public QObject, protected FDO_DBUS_CONTEXT
{
    Q_OBJECT

    struct CollectionProperties {
        QString collectionLabel;
        QDBusObjectPath objectPath;
        QString alias;
    };

public:
    KWalletFreedesktopPrompt(KWalletFreedesktopService *service, QDBusObjectPath objectPath, PromptType type, QString responseBusName);

    KWalletFreedesktopPrompt(const KWalletFreedesktopPrompt &) = delete;
    KWalletFreedesktopPrompt &operator=(const KWalletFreedesktopPrompt &) = delete;

    KWalletFreedesktopPrompt(KWalletFreedesktopPrompt &&) = delete;
    KWalletFreedesktopPrompt &operator=(KWalletFreedesktopPrompt &&) = delete;

    KWalletFreedesktopService *fdoService() const;
    KWalletD *backend() const;
    QDBusObjectPath fdoObjectPath() const;

    void subscribeForWalletAsyncOpened();
    void appendProperties(const QString &label, const QDBusObjectPath &objectPath = QDBusObjectPath("/"), const QString &alias = {});

public Q_SLOTS:
    void walletAsyncOpened(int transactionId, int walletHandle);

private:
    KWalletFreedesktopService *m_service;
    QDBusObjectPath m_objectPath;
    PromptType m_type;
    QSet<int> m_transactionIds;
    QList<QDBusObjectPath> m_result;
    QList<CollectionProperties> m_propertiesList;
    std::map<int, CollectionProperties> m_transactionIdToCollectionProperties;
    QString m_responseBusName;

    /* Freedesktop API */

    /* org.freedesktop.Secret.Prompt methods */
public Q_SLOTS:
    void Dismiss();
    void Prompt(const QString &window_id);

    /* org.freedesktop.Secret.Prompt signals */
Q_SIGNALS:
    /* Emitted manually now */
    void Completed(bool dismissed, const QDBusVariant &result);
};

#endif
