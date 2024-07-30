// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2009, 2010, 2011 Dario Andres Rodriguez <andresbajotierra@gmail.com>
// SPDX-FileCopyrightText: 2019-2022 Harald Sitter <sitter@kde.org>

#include "credentialstore.h"

#include <QVariant>

#include "drkonqi_debug.h"
#include "drkonqi_globals.h"

static const char kWalletEntryUsername[] = "username";
static const char kWalletEntryPassword[] = "password";

static const QString konquerorKWalletEntryName = KDE_BUGZILLA_URL + QStringLiteral("index.cgi#login");
static const char konquerorKWalletEntryUsername[] = "Bugzilla_login";
static const char konquerorKWalletEntryPassword[] = "Bugzilla_password";

void CredentialStore::openWallet()
{
    // Store if the wallet was previously opened so we can know if we should close it later
    m_walletWasOpenedBefore = KWallet::Wallet::isOpen(KWallet::Wallet::NetworkWallet());
    qCDebug(DRKONQI_LOG) << "Wallet was open?" << m_walletWasOpenedBefore;

    // Request open the wallet
    const WId windowId = m_window ? m_window->winId() : 0;

    m_wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), windowId);
    qCDebug(DRKONQI_LOG) << "wallet?" << m_wallet;
}

bool CredentialStore::kWalletEntryExists(const QString &entryName)
{
    return !KWallet::Wallet::keyDoesNotExist(KWallet::Wallet::NetworkWallet(), KWallet::Wallet::FormDataFolder(), entryName);
}

void CredentialStore::load()
{
    if (m_wallet) { // already open, nothing to do
        return;
    }
    qCDebug(DRKONQI_LOG) << "Wallet not open";
    QString username;
    QString password;
    if (kWalletEntryExists(m_walletEntryName)) { // Key exists!
        qCDebug(DRKONQI_LOG) << "wallet entry exists";
        openWallet();
        // Was the wallet opened?
        if (m_wallet) {
            m_wallet->setFolder(KWallet::Wallet::FormDataFolder());

            // Use wallet data to try login
            QMap<QString, QString> values;
            m_wallet->readMap(m_walletEntryName, values);
            username = values.value(QLatin1String(kWalletEntryUsername));
            password = values.value(QLatin1String(kWalletEntryPassword));
        }
    } else if (kWalletEntryExists(konquerorKWalletEntryName)) {
        qCDebug(DRKONQI_LOG) << "wallet entry does not exist";
        // If the DrKonqi entry is empty, but a Konqueror entry exists, use and copy it.
        openWallet();
        if (m_wallet) {
            m_wallet->setFolder(KWallet::Wallet::FormDataFolder());

            // Fetch Konqueror data
            QMap<QString, QString> values;
            m_wallet->readMap(konquerorKWalletEntryName, values);
            username = values.value(QLatin1String(konquerorKWalletEntryUsername));
            password = values.value(QLatin1String(konquerorKWalletEntryPassword));
        }
    }
    if (!username.isEmpty() && !password.isEmpty()) {
        setProperty("email", username);
        setProperty("password", password);
    }
}

void CredentialStore::store()
{
    qCDebug(DRKONQI_LOG) << "trying to store credentials";
    if (!m_wallet) {
        openWallet();
    }
    qCDebug(DRKONQI_LOG) << "Wallet opened?" << m_wallet;
    // Got wallet open ?
    if (!m_wallet) {
        return;
    }

    if (!m_wallet->hasFolder(KWallet::Wallet::FormDataFolder())) {
        m_wallet->createFolder(KWallet::Wallet::FormDataFolder());
    }

    QMap<QString, QString> values;
    values.insert(QLatin1String(kWalletEntryUsername), m_email);
    values.insert(QLatin1String(kWalletEntryPassword), m_password);
    m_wallet->writeMap(m_walletEntryName, values);
}

void CredentialStore::drop()
{
    if (!kWalletEntryExists(m_walletEntryName)) {
        return;
    }

    if (!m_wallet) {
        openWallet();
    }
    if (m_wallet && m_wallet->hasFolder(KWallet::Wallet::FormDataFolder())) {
        m_wallet->setFolder(KWallet::Wallet::FormDataFolder());
        m_wallet->removeEntry(m_walletEntryName);
    }
}

#include "moc_credentialstore.cpp"
