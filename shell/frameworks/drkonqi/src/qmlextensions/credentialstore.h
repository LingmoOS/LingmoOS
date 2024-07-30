// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>
#include <QWindow>

#include <KWallet>

#include "drkonqi.h"

class CredentialStore : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    Q_INVOKABLE void load();
    Q_INVOKABLE void store();
    Q_INVOKABLE void drop();

    Q_PROPERTY(bool canStore MEMBER m_canStore CONSTANT)
    const bool m_canStore = KWallet::Wallet::isEnabled();

    Q_PROPERTY(QString url MEMBER m_url NOTIFY urlChanged)
    QString m_url;
    Q_SIGNAL void urlChanged();

    Q_PROPERTY(QString email MEMBER m_email NOTIFY emailChanged)
    QString m_email;
    Q_SIGNAL void emailChanged();

    Q_PROPERTY(QString password MEMBER m_password NOTIFY passwordChanged)
    QString m_password;
    Q_SIGNAL void passwordChanged();

    Q_PROPERTY(QWindow *window MEMBER m_window NOTIFY windowChanged)
    QWindow *m_window = nullptr;
    Q_SIGNAL void windowChanged();

private:
    void openWallet();
    static bool kWalletEntryExists(const QString &entryName);

    KWallet::Wallet *m_wallet = nullptr;
    const QString m_walletEntryName = DrKonqi::isTestingBugzilla() ? QStringLiteral("drkonqi_bugzilla_test_mode") : QStringLiteral("drkonqi_bugzilla");
    bool m_walletWasOpenedBefore = false;
};
