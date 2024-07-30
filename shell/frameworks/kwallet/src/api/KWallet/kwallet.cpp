/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2002-2004 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2011 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwallet.h"
#include "kwallet_api_debug.h"

#include <QApplication>
#include <QDBusConnection>
#include <QRegularExpression>

#include <KConfigGroup>
#include <KSharedConfig>

#include "kwallet_interface.h"

typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

namespace KWallet
{
class KWalletDLauncher
{
public:
    KWalletDLauncher();
    ~KWalletDLauncher();
    KWalletDLauncher(const KWalletDLauncher &) = delete;
    KWalletDLauncher &operator=(const KWalletDLauncher &) = delete;
    org::kde::KWallet &getInterface();

    org::kde::KWallet *m_wallet_deamon;
    KConfigGroup m_cgroup;
    bool m_walletEnabled;
};

Q_GLOBAL_STATIC(KWalletDLauncher, walletLauncher)

static QString appid()
{
    return qApp->applicationName();
}

static void registerTypes()
{
    static bool registered = false;
    if (!registered) {
        qDBusRegisterMetaType<StringByteArrayMap>();
        registered = true;
    }
}

const QString Wallet::LocalWallet()
{
    KConfigGroup cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group(QStringLiteral("Wallet")));
    if (!cfg.readEntry("Use One Wallet", true)) {
        QString tmp = cfg.readEntry("Local Wallet", "localwallet");
        if (tmp.isEmpty()) {
            return QStringLiteral("localwallet");
        }
        return tmp;
    }

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return QStringLiteral("kdewallet");
    }
    return tmp;
}

const QString Wallet::NetworkWallet()
{
    KConfigGroup cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group(QStringLiteral("Wallet")));

    QString tmp = cfg.readEntry("Default Wallet", "kdewallet");
    if (tmp.isEmpty()) {
        return QStringLiteral("kdewallet");
    }
    return tmp;
}

const QString Wallet::PasswordFolder()
{
    return QStringLiteral("Passwords");
}

const QString Wallet::FormDataFolder()
{
    return QStringLiteral("Form Data");
}

class Q_DECL_HIDDEN Wallet::WalletPrivate
{
public:
    WalletPrivate(Wallet *wallet, int h, const QString &n)
        : q(wallet)
        , name(n)
        , handle(h)
    {
    }

    void walletServiceUnregistered();

    Wallet *q;
    QString name;
    QString folder;
    int handle;
    int transactionId;
};

static const char s_kwalletdServiceName[] = "org.kde.kwalletd6";

Wallet::Wallet(int handle, const QString &name)
    : QObject(nullptr)
    , d(new WalletPrivate(this, handle, name))
{
    QDBusServiceWatcher *watcher =
        new QDBusServiceWatcher(QString::fromLatin1(s_kwalletdServiceName), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration, this);
    connect(watcher, &QDBusServiceWatcher::serviceUnregistered, this, [this]() {
        d->walletServiceUnregistered();
    });

    connect(&walletLauncher()->getInterface(), &org::kde::KWallet::walletClosedId, this, &KWallet::Wallet::slotWalletClosed);
    connect(&walletLauncher()->getInterface(), &org::kde::KWallet::folderListUpdated, this, &KWallet::Wallet::slotFolderListUpdated);
    connect(&walletLauncher()->getInterface(), &org::kde::KWallet::folderUpdated, this, &KWallet::Wallet::slotFolderUpdated);
    connect(&walletLauncher()->getInterface(), &org::kde::KWallet::applicationDisconnected, this, &KWallet::Wallet::slotApplicationDisconnected);

    // Verify that the wallet is still open
    if (d->handle != -1) {
        QDBusReply<bool> r = walletLauncher()->getInterface().isOpen(d->handle);
        if (r.isValid() && !r) {
            d->handle = -1;
            d->name.clear();
        }
    }
}

Wallet::~Wallet()
{
    if (d->handle != -1) {
        if (!walletLauncher.isDestroyed()) {
            walletLauncher()->getInterface().close(d->handle, false, appid());
        } else {
            qCDebug(KWALLET_API_LOG) << "Problem with static destruction sequence."
                                        "Destroy any static Wallet before the event-loop exits.";
        }
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
    }
    delete d;
}

QStringList Wallet::walletList()
{
    QStringList result;
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<QStringList> r = walletLauncher()->getInterface().wallets();

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        } else {
            result = r;
        }
    }
    return result;
}

void Wallet::changePassword(const QString &name, WId w)
{
    if (w == 0) {
        qCDebug(KWALLET_API_LOG) << "Pass a valid window to KWallet::Wallet::changePassword().";
    }

    if (walletLauncher()->m_walletEnabled) {
        walletLauncher()->getInterface().changePassword(name, (qlonglong)w, appid());
    }
}

bool Wallet::isEnabled()
{
    return walletLauncher()->m_walletEnabled;
}

bool Wallet::isOpen(const QString &name)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<bool> r = walletLauncher()->getInterface().isOpen(name);

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return false;
        } else {
            return r;
        }
    } else {
        return false;
    }
}

int Wallet::closeWallet(const QString &name, bool force)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<int> r = walletLauncher()->getInterface().close(name, force);

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return -1;
        } else {
            return r;
        }
    } else {
        return -1;
    }
}

int Wallet::deleteWallet(const QString &name)
{
    if (walletLauncher->m_walletEnabled) {
        QDBusReply<int> r = walletLauncher()->getInterface().deleteWallet(name);

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return -1;
        } else {
            return r;
        }
    } else {
        return -1;
    }
}

Wallet *Wallet::openWallet(const QString &name, WId w, OpenType ot)
{
    if (w == 0) {
        qCDebug(KWALLET_API_LOG) << "Pass a valid window to KWallet::Wallet::openWallet().";
    }

    if (!walletLauncher()->m_walletEnabled) {
        qCDebug(KWALLET_API_LOG) << "User disabled the wallet system so returning 0 here.";
        return nullptr;
    }

    Wallet *wallet = new Wallet(-1, name);

    // connect the daemon's opened signal to the slot filtering the
    // signals we need
    connect(&walletLauncher()->getInterface(), &org::kde::KWallet::walletAsyncOpened, wallet, &KWallet::Wallet::walletAsyncOpened);

    org::kde::KWallet &interface = walletLauncher->getInterface();

    // do the call
    QDBusReply<int> r;
    if (ot == Synchronous) {
        interface.setTimeout(0x7FFFFFFF); // Don't timeout after 25s, but 24 days
        r = interface.open(name, (qlonglong)w, appid());
        interface.setTimeout(-1); // Back to the default 25s
        // after this call, r would contain a transaction id >0 if OK or -1 if NOK
        // if OK, the slot walletAsyncOpened should have been received, but the transaction id
        // will not match. We'll get that handle from the reply - see below
    } else if (ot == Asynchronous) {
        r = interface.openAsync(name, (qlonglong)w, appid(), true);
    } else if (ot == Path) {
        r = interface.openPathAsync(name, (qlonglong)w, appid(), true);
    } else {
        delete wallet;
        return nullptr;
    }
    // error communicating with the daemon (maybe not running)
    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        delete wallet;
        return nullptr;
    }
    wallet->d->transactionId = r.value();

    if (ot == Synchronous || ot == Path) {
        // check for an immediate error
        if (wallet->d->transactionId < 0) {
            delete wallet;
            wallet = nullptr;
        } else {
            wallet->d->handle = r.value();
        }
    } else if (ot == Asynchronous) {
        if (wallet->d->transactionId < 0) {
            QTimer::singleShot(0, wallet, SLOT(emitWalletAsyncOpenError()));
            // client code is responsible for deleting the wallet
        }
    }

    return wallet;
}

void Wallet::slotCollectionStatusChanged(int status)
{
    Q_UNUSED(status)
}

void Wallet::slotCollectionDeleted()
{
    d->folder.clear();
    d->name.clear();
    Q_EMIT walletClosed();
}

bool Wallet::disconnectApplication(const QString &wallet, const QString &app)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<bool> r = walletLauncher()->getInterface().disconnectApplication(wallet, app);

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return false;
        } else {
            return r;
        }
    } else {
        return true;
    }
}

QStringList Wallet::users(const QString &name)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<QStringList> r = walletLauncher()->getInterface().users(name);
        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return QStringList();
        } else {
            return r;
        }
    } else {
        return QStringList();
    }
}

int Wallet::sync()
{
    if (d->handle == -1) {
        return -1;
    }

    walletLauncher()->getInterface().sync(d->handle, appid());
    return 0;
}

int Wallet::lockWallet()
{
    if (d->handle == -1) {
        return -1;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().close(d->handle, true, appid());
    d->handle = -1;
    d->folder.clear();
    d->name.clear();
    if (r.isValid()) {
        return r;
    } else {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return -1;
    }
}

const QString &Wallet::walletName() const
{
    return d->name;
}

bool Wallet::isOpen() const
{
    return d->handle != -1;
}

void Wallet::requestChangePassword(WId w)
{
    if (w == 0) {
        qCDebug(KWALLET_API_LOG) << "Pass a valid window to KWallet::Wallet::requestChangePassword().";
    }

    if (d->handle == -1) {
        return;
    }

    walletLauncher()->getInterface().changePassword(d->name, (qlonglong)w, appid());
}

void Wallet::slotWalletClosed(int handle)
{
    if (d->handle == handle) {
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        Q_EMIT walletClosed();
    }
}

QStringList Wallet::folderList()
{
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher()->getInterface().folderList(d->handle, appid());
    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return QStringList();
    } else {
        return r;
    }
}

QStringList Wallet::entryList()
{
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher()->getInterface().entryList(d->handle, d->folder, appid());
    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return QStringList();
    } else {
        return r;
    }
}

bool Wallet::hasFolder(const QString &f)
{
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher()->getInterface().hasFolder(d->handle, f, appid());
    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}

bool Wallet::createFolder(const QString &f)
{
    if (d->handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = walletLauncher()->getInterface().createFolder(d->handle, f, appid());

        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return false;
        } else {
            return r;
        }
    }

    return true; // folder already exists
}

bool Wallet::setFolder(const QString &f)
{
    bool rc = false;

    if (d->handle == -1) {
        return rc;
    }

    // Don't do this - the folder could have disappeared?
#if 0
        if (f == d->folder) {
            return true;
        }
#endif

    if (hasFolder(f)) {
        d->folder = f;
        rc = true;
    }

    return rc;
}

bool Wallet::removeFolder(const QString &f)
{
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher()->getInterface().removeFolder(d->handle, f, appid());
    if (d->folder == f) {
        setFolder(QString());
    }

    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}

const QString &Wallet::currentFolder() const
{
    return d->folder;
}

int Wallet::readEntry(const QString &key, QByteArray &value)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = walletLauncher()->getInterface().readEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}

QMap<QString, QByteArray> Wallet::entriesList(bool *ok) const
{
    QMap<QString, QByteArray> entries;

    registerTypes();

    if (d->handle == -1) {
        if (ok) {
            *ok = false;
        }
        return entries;
    }

    QDBusReply<QVariantMap> reply = walletLauncher()->getInterface().entriesList(d->handle, d->folder, appid());
    if (reply.isValid()) {
        if (ok) {
            *ok = true;
        }
        // convert <QString, QVariant> to <QString, QByteArray>
        const QVariantMap val = reply.value();
        for (QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it) {
            entries.insert(it.key(), it.value().toByteArray());
        }
    }

    return entries;
}

int Wallet::renameEntry(const QString &oldName, const QString &newName)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    QDBusReply<int> r = walletLauncher()->getInterface().renameEntry(d->handle, d->folder, oldName, newName, appid());
    QT_WARNING_POP
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

int Wallet::readMap(const QString &key, QMap<QString, QString> &value)
{
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QByteArray> r = walletLauncher()->getInterface().readMap(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        QByteArray v = r;
        if (!v.isEmpty()) {
            QDataStream ds(&v, QIODevice::ReadOnly);
            ds >> value;
        }
    }

    return rc;
}

QMap<QString, QMap<QString, QString>> Wallet::mapList(bool *ok) const
{
    QMap<QString, QMap<QString, QString>> list;

    registerTypes();

    if (d->handle == -1) {
        if (ok) {
            *ok = false;
        }
        return list;
    }

    QDBusReply<QVariantMap> reply = walletLauncher()->getInterface().mapList(d->handle, d->folder, appid());
    if (reply.isValid()) {
        if (ok) {
            *ok = true;
        }
        const QVariantMap val = reply.value();
        for (QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it) {
            QByteArray mapData = it.value().toByteArray();
            if (!mapData.isEmpty()) {
                QDataStream ds(&mapData, QIODevice::ReadOnly);
                QMap<QString, QString> v;
                ds >> v;
                list.insert(it.key(), v);
            }
        }
    }

    return list;
}

int Wallet::readPassword(const QString &key, QString &value)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QString> r = walletLauncher()->getInterface().readPassword(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        value = r;
        rc = 0;
    }

    return rc;
}

QMap<QString, QString> Wallet::passwordList(bool *ok) const
{
    QMap<QString, QString> passList;

    registerTypes();

    if (d->handle == -1) {
        if (ok) {
            *ok = false;
        }
        return passList;
    }

    QDBusReply<QVariantMap> reply = walletLauncher()->getInterface().passwordList(d->handle, d->folder, appid());
    if (reply.isValid()) {
        if (ok) {
            *ok = true;
        }
        const QVariantMap val = reply.value();
        for (QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it) {
            passList.insert(it.key(), it.value().toString());
        }
    }

    return passList;
}

int Wallet::writeEntry(const QString &key, const QByteArray &value, EntryType entryType)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().writeEntry(d->handle, d->folder, key, value, int(entryType), appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

int Wallet::writeEntry(const QString &key, const QByteArray &value)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().writeEntry(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

int Wallet::writeMap(const QString &key, const QMap<QString, QString> &value)
{
    int rc = -1;

    registerTypes();

    if (d->handle == -1) {
        return rc;
    }

    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    QDBusReply<int> r = walletLauncher()->getInterface().writeMap(d->handle, d->folder, key, mapData, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

int Wallet::writePassword(const QString &key, const QString &value)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().writePassword(d->handle, d->folder, key, value, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

bool Wallet::hasEntry(const QString &key)
{
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher()->getInterface().hasEntry(d->handle, d->folder, key, appid());
    if (!r.isValid()) {
        qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
        return false;
    } else {
        return r;
    }
}

int Wallet::removeEntry(const QString &key)
{
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().removeEntry(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
}

Wallet::EntryType Wallet::entryType(const QString &key)
{
    int rc = 0;

    if (d->handle == -1) {
        return Wallet::Unknown;
    }

    QDBusReply<int> r = walletLauncher()->getInterface().entryType(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }
    return static_cast<EntryType>(rc);
}

void Wallet::WalletPrivate::walletServiceUnregistered()
{
    if (handle >= 0) {
        q->slotWalletClosed(handle);
    }
}

void Wallet::slotFolderUpdated(const QString &wallet, const QString &folder)
{
    if (d->name == wallet) {
        Q_EMIT folderUpdated(folder);
    }
}

void Wallet::slotFolderListUpdated(const QString &wallet)
{
    if (d->name == wallet) {
        Q_EMIT folderListUpdated();
    }
}

void Wallet::slotApplicationDisconnected(const QString &wallet, const QString &application)
{
    if (d->handle >= 0 && d->name == wallet && application == appid()) {
        slotWalletClosed(d->handle);
    }
}

void Wallet::walletAsyncOpened(int tId, int handle)
{
    // ignore responses to calls other than ours
    if (d->transactionId != tId || d->handle != -1) {
        return;
    }

    // disconnect the async signal
    disconnect(this, SLOT(walletAsyncOpened(int, int)));

    d->handle = handle;
    Q_EMIT walletOpened(handle > 0);
}

void Wallet::emitWalletAsyncOpenError()
{
    Q_EMIT walletOpened(false);
}

void Wallet::emitWalletOpened()
{
    Q_EMIT walletOpened(true);
}

bool Wallet::folderDoesNotExist(const QString &wallet, const QString &folder)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<bool> r = walletLauncher()->getInterface().folderDoesNotExist(wallet, folder);
        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return false;
        } else {
            return r;
        }
    } else {
        return false;
    }
}

bool Wallet::keyDoesNotExist(const QString &wallet, const QString &folder, const QString &key)
{
    if (walletLauncher()->m_walletEnabled) {
        QDBusReply<bool> r = walletLauncher()->getInterface().keyDoesNotExist(wallet, folder, key);
        if (!r.isValid()) {
            qCDebug(KWALLET_API_LOG) << "Invalid DBus reply: " << r.error();
            return false;
        } else {
            return r;
        }
    } else {
        return false;
    }
}

void Wallet::virtual_hook(int, void *)
{
    // BASE::virtual_hook( id, data );
}

KWalletDLauncher::KWalletDLauncher()
    : m_wallet_deamon(nullptr)
    , m_cgroup(KSharedConfig::openConfig(QStringLiteral("kwalletrc"), KConfig::NoGlobals)->group(QStringLiteral("Wallet")))
    , m_walletEnabled(false)
{
    m_walletEnabled = m_cgroup.readEntry("Enabled", true);
    if (!m_walletEnabled) {
        qCDebug(KWALLET_API_LOG) << "The wallet service was disabled by the user";
        return;
    }
    m_wallet_deamon = new org::kde::KWallet(QString::fromLatin1(s_kwalletdServiceName), QStringLiteral("/modules/kwalletd6"), QDBusConnection::sessionBus());
}

KWalletDLauncher::~KWalletDLauncher()
{
    delete m_wallet_deamon;
}

org::kde::KWallet &KWalletDLauncher::getInterface()
{
    Q_ASSERT(m_wallet_deamon != nullptr);

    // check if kwalletd is already running
    QDBusConnectionInterface *bus = QDBusConnection::sessionBus().interface();
    if (!bus->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName))) {
        // not running! check if it is enabled.
        if (m_walletEnabled) {
            // wallet is enabled! try launching it
            QDBusReply<void> reply = bus->startService(QString::fromLatin1(s_kwalletdServiceName));
            if (!reply.isValid()) {
                qCritical() << "Couldn't start kwalletd: " << reply.error();
            }

            if (!bus->isServiceRegistered(QString::fromLatin1(s_kwalletdServiceName))) {
                qCDebug(KWALLET_API_LOG) << "The kwalletd service is still not registered";
            } else {
                qCDebug(KWALLET_API_LOG) << "The kwalletd service has been registered";
            }
        } else {
            qCritical() << "The kwalletd service has been disabled";
        }
    }

    return *m_wallet_deamon;
}

} // namespace KWallet

#include "moc_kwallet.cpp"
