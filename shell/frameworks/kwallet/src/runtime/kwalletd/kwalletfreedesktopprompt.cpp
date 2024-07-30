/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2021 Slava Aseev <nullptrnine@basealt.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kwalletfreedesktopprompt.h"

#include "kwalletd.h"
#include "kwalletfreedesktopcollection.h"
#include "kwalletfreedesktoppromptadaptor.h"

KWalletFreedesktopPrompt::KWalletFreedesktopPrompt(KWalletFreedesktopService *service, QDBusObjectPath objectPath, PromptType type, QString responseBusName)
    : QObject(nullptr)
    , m_service(service)
    , m_objectPath(std::move(objectPath))
    , m_type(type)
    , m_responseBusName(std::move(responseBusName))
{
    (void)new KWalletFreedesktopPromptAdaptor(this);
}

KWalletFreedesktopService *KWalletFreedesktopPrompt::fdoService() const
{
    return m_service;
}

KWalletD *KWalletFreedesktopPrompt::backend() const
{
    return fdoService()->backend();
}

QDBusObjectPath KWalletFreedesktopPrompt::fdoObjectPath() const
{
    return m_objectPath;
}

void KWalletFreedesktopPrompt::Dismiss()
{
    auto msg = QDBusMessage::createTargetedSignal(m_responseBusName,
                                                  fdoObjectPath().path(),
                                                  QStringLiteral("org.freedesktop.Secret.Prompt"),
                                                  QStringLiteral("Completed"));
    QVariantList args;
    args << true << QVariant::fromValue(QDBusVariant(QVariant::fromValue(QList<QDBusObjectPath>())));
    msg.setArguments(args);
    QDBusConnection::sessionBus().send(msg);
    QDBusConnection::sessionBus().unregisterObject(fdoObjectPath().path());
}

void KWalletFreedesktopPrompt::Prompt(const QString &window_id)
{
    if (m_type != PromptType::Open && m_type != PromptType::Create) {
        return;
    }

    const int wId = window_id.toInt();
    for (auto properties : std::as_const(m_propertiesList)) {
        /* When type is "PromptType::Open" the properties.label actually stores
         * the wallet name
         */
        QString walletName = properties.collectionLabel;

        if (m_type == PromptType::Create) {
            walletName = fdoService()->makeUniqueWalletName(properties.collectionLabel);
            properties.collectionLabel = walletName;
        }

        if (!properties.alias.isEmpty()) {
            fdoService()->createCollectionAlias(properties.alias, walletName);
        }

        const int tId = backend()->openAsync(walletName, wId, FDO_APPID, false, connection(), message());
        m_transactionIds.insert(tId);
        m_transactionIdToCollectionProperties.emplace(tId, std::move(properties));
    }
}

void KWalletFreedesktopPrompt::walletAsyncOpened(int transactionId, int walletHandle)
{
    const auto found = m_transactionIds.find(transactionId);

    if (found != m_transactionIds.end()) {
        const auto propertiesPos = m_transactionIdToCollectionProperties.find(transactionId);
        if (walletHandle < 0 || propertiesPos == m_transactionIdToCollectionProperties.end()) {
            Dismiss();
            fdoService()->deletePrompt(fdoObjectPath().path());
            return;
        }
        m_transactionIds.remove(transactionId);

        const QString &walletName = propertiesPos->second.collectionLabel;
        const auto collectionPath = fdoService()->promptUnlockCollection(walletName, walletHandle);
        m_result.push_back(propertiesPos->second.objectPath.path() == QStringLiteral("/") ? collectionPath : propertiesPos->second.objectPath);
    }

    if (m_transactionIds.empty()) {
        /* At this point there is no remaining transactions, so we able to complete prompt */

        auto msg = QDBusMessage::createTargetedSignal(m_responseBusName,
                                                      fdoObjectPath().path(),
                                                      QStringLiteral("org.freedesktop.Secret.Prompt"),
                                                      QStringLiteral("Completed"));
        QVariantList args;
        args << false;

        if (m_type == PromptType::Create && m_result.size() < 2) {
            /* Single object in dbus variant */
            args << QVariant::fromValue(QDBusVariant(QVariant::fromValue(m_result.empty() ? QDBusObjectPath("/") : m_result.front())));
        } else {
            /* Object array in dbus variant */
            args << QVariant::fromValue(QDBusVariant(QVariant::fromValue(m_result)));
        }

        msg.setArguments(args);
        QDBusConnection::sessionBus().send(msg);

        fdoService()->deletePrompt(fdoObjectPath().path());
    }
}

void KWalletFreedesktopPrompt::subscribeForWalletAsyncOpened()
{
    connect(backend(), &KWalletD::walletAsyncOpened, this, &KWalletFreedesktopPrompt::walletAsyncOpened);
    QDBusConnection::sessionBus().registerObject(fdoObjectPath().path(), this);
}

void KWalletFreedesktopPrompt::appendProperties(const QString &label, const QDBusObjectPath &objectPath, const QString &alias)
{
    m_propertiesList.push_back(CollectionProperties{label, objectPath, alias});
}

#include "moc_kwalletfreedesktopprompt.cpp"
