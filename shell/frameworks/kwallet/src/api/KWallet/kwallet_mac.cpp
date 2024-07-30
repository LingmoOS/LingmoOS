/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2002-2004 George Staikos <staikos@kde.org>
    SPDX-FileCopyrightText: 2008 Michael Leupold <lemma@confuego.org>
    SPDX-FileCopyrightText: 2010 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwallet.h"
#include <KConfigGroup>
#include <KSharedConfig>

#include <QApplication>
#include <QDebug>
#include <QPointer>
#include <QWidget>

#include <Carbon/Carbon.h>
#include <Security/SecKeychain.h>
#include <Security/Security.h>

// TODO: OSX_KEYCHAIN_PORT_DISABLED is never defined, all the enclosing code should be removed

using namespace KWallet;

typedef QMap<QString, QString> StringStringMap;
Q_DECLARE_METATYPE(StringStringMap)
typedef QMap<QString, StringStringMap> StringToStringStringMapMap;
Q_DECLARE_METATYPE(StringToStringStringMapMap)
typedef QMap<QString, QByteArray> StringByteArrayMap;
Q_DECLARE_METATYPE(StringByteArrayMap)

namespace
{
template<typename T>
struct CFReleaser {
    explicit CFReleaser(const T &r)
        : ref(r)
    {
    }
    ~CFReleaser()
    {
        CFRelease(ref);
    }
    T ref;
};
}

static QString asQString(CFStringRef sr)
{
    return QString::fromLatin1(CFStringGetCStringPtr(sr, NULL)); // TODO Latin1 correct?
}

static QString errorString(OSStatus s)
{
    const CFReleaser<CFStringRef> ref(SecCopyErrorMessageString(s, NULL));
    return asQString(ref.ref);
}

static bool isError(OSStatus s, QString *errMsg)
{
    if (errMsg) {
        *errMsg = errorString(s);
    }
    return s != 0;
}

static QString appid()
{
    return qApp->applicationName();
}

static OSStatus removeEntryImplementation(const QString &walletName, const QString &key)
{
    const QByteArray serviceName(walletName.toUtf8());
    const QByteArray accountName(key.toUtf8());
    SecKeychainItemRef itemRef;
    QString errMsg;
    OSStatus result =
        SecKeychainFindGenericPassword(NULL, serviceName.size(), serviceName.constData(), accountName.size(), accountName.constData(), NULL, NULL, &itemRef);
    if (isError(result, &errMsg)) {
        qWarning() << "Could not retrieve password:" << qPrintable(errMsg);
        return result;
    }
    const CFReleaser<SecKeychainItemRef> itemReleaser(itemRef);
    result = SecKeychainItemDelete(itemRef);
    if (isError(result, &errMsg)) {
        qWarning() << "Could not delete password:" << qPrintable(errMsg);
        return result;
    }
    return result;
}

const QString Wallet::LocalWallet()
{
    KConfigGroup cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Wallet"));
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
    KConfigGroup cfg(KSharedConfig::openConfig(QStringLiteral("kwalletrc"))->group("Wallet"));

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

void Wallet::WalletPrivate::walletServiceUnregistered()
{
    if (handle >= 0) {
        q->slotWalletClosed(handle);
    }
}

Wallet::Wallet(int handle, const QString &name)
    : QObject(0L)
    , d(new WalletPrivate(this, handle, name))
{
    Q_UNUSED(handle);
}

Wallet::~Wallet()
{
    delete d;
}

QStringList Wallet::walletList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().wallets();
#else
    return QStringList();
#endif
}

void Wallet::changePassword(const QString &name, WId w)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (w == 0) {
        qDebug() << "Pass a valid window to KWallet::Wallet::changePassword().";
    }
    walletLauncher->getInterface().changePassword(name, (qlonglong)w, appid());
#endif
}

bool Wallet::isEnabled()
{
    // PENDING(frank) check
    return true;
}

bool Wallet::isOpen(const QString &name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().isOpen(name); // default is false
#else
    return true;
#endif
}

int Wallet::closeWallet(const QString &name, bool force)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<int> r = walletLauncher->getInterface().close(name, force);
    return r.isValid() ? r : -1;
#else
    return 0;
#endif
}

int Wallet::deleteWallet(const QString &name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<int> r = walletLauncher->getInterface().deleteWallet(name);
    return r.isValid() ? r : -1;
#else
    return -1;
#endif
}

Wallet *Wallet::openWallet(const QString &name, WId w, OpenType ot)
{
    Q_UNUSED(w);
    Q_UNUSED(ot);
    Wallet *wallet = new Wallet(-1, name);
    QMetaObject::invokeMethod(wallet, "emitWalletOpened", Qt::QueuedConnection);
    return wallet;
}

bool Wallet::disconnectApplication(const QString &wallet, const QString &app)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().disconnectApplication(wallet, app); // default is false
#else
    return true;
#endif
}

QStringList Wallet::users(const QString &name)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return walletLauncher->getInterface().users(name); // default is QStringList()
#else
    return QStringList();
#endif
}

int Wallet::sync()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return -1;
    }

    walletLauncher->getInterface().sync(d->handle, appid());
#endif
    return 0;
}

int Wallet::lockWallet()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return -1;
    }

    QDBusReply<int> r = walletLauncher->getInterface().close(d->handle, true, appid());
    d->handle = -1;
    d->folder.clear();
    d->name.clear();
    if (r.isValid()) {
        return r;
    }
#endif
    return -1;
}

const QString &Wallet::walletName() const
{
    return d->name;
}

bool Wallet::isOpen() const
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    return d->handle != -1;
#else
    return true;
#endif
}

void Wallet::requestChangePassword(WId w)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (w == 0) {
        qDebug() << "Pass a valid window to KWallet::Wallet::requestChangePassword().";
    }
    if (d->handle == -1) {
        return;
    }

    walletLauncher->getInterface().changePassword(d->name, (qlonglong)w, appid());
#endif
}

void Wallet::slotWalletClosed(int handle)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == handle) {
        d->handle = -1;
        d->folder.clear();
        d->name.clear();
        Q_EMIT walletClosed();
    }
#endif
}

QStringList Wallet::folderList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().folderList(d->handle, appid());
    return r;
#else
    return QStringList();
#endif
}

QStringList Wallet::entryList()
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return QStringList();
    }

    QDBusReply<QStringList> r = walletLauncher->getInterface().entryList(d->handle, d->folder, appid());
    return r;
#else
    return QStringList();
#endif
}

bool Wallet::hasFolder(const QString &f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().hasFolder(d->handle, f, appid());
    return r; // default is false
#else
    return true;
#endif
}

bool Wallet::createFolder(const QString &f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    if (!hasFolder(f)) {
        QDBusReply<bool> r = walletLauncher->getInterface().createFolder(d->handle, f, appid());
        return r;
    }

    return true; // folder already exists
#else
    return true;
#endif
}

bool Wallet::setFolder(const QString &f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
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
#else
    return true;
#endif
}

bool Wallet::removeFolder(const QString &f)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle == -1) {
        return false;
    }

    QDBusReply<bool> r = walletLauncher->getInterface().removeFolder(d->handle, f, appid());
    if (d->folder == f) {
        setFolder(QString());
    }

    return r; // default is false
#else
    return true;
#endif
}

const QString &Wallet::currentFolder() const
{
    return d->folder;
}

int Wallet::readEntry(const QString &key, QByteArray &value)
{
    const QByteArray serviceName(walletName().toUtf8());
    const QByteArray accountName(key.toUtf8());
    UInt32 passwordSize = 0;
    void *passwordData = 0;
    QString errMsg;
    if (isError(SecKeychainFindGenericPassword(NULL,
                                               serviceName.size(),
                                               serviceName.constData(),
                                               accountName.size(),
                                               accountName.constData(),
                                               &passwordSize,
                                               &passwordData,
                                               NULL),
                &errMsg)) {
        qWarning() << "Could not retrieve password:" << qPrintable(errMsg);
        return -1;
    }

    value = QByteArray(reinterpret_cast<const char *>(passwordData), passwordSize);
    SecKeychainItemFreeContent(NULL, passwordData);
    return 0;
}

int Wallet::readEntryList(const QString &key, QMap<QString, QByteArray> &value)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = walletLauncher->getInterface().readEntryList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        // convert <QString, QVariant> to <QString, QByteArray>
        const QVariantMap val = r.value();
        for (QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it) {
            value.insert(it.key(), it.value().toByteArray());
        }
    }

    return rc;
#else
    return -1;
#endif
}

int Wallet::renameEntry(const QString &oldName, const QString &newName)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<int> r = walletLauncher->getInterface().renameEntry(d->handle, d->folder, oldName, newName, appid());
    if (r.isValid()) {
        rc = r;
    }

    return rc;
#else
    return -1;
#endif
}

int Wallet::readMap(const QString &key, QMap<QString, QString> &value)
{
    QByteArray v;
    const int ret = readEntry(key, v);
    if (ret != 0) {
        return ret;
    }
    if (!v.isEmpty()) {
        QDataStream ds(&v, QIODevice::ReadOnly);
        ds >> value;
    }
    return 0;
}

int Wallet::readMapList(const QString &key, QMap<QString, QMap<QString, QString>> &value)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    registerTypes();

    int rc = -1;

    if (d->handle == -1) {
        return rc;
    }

    QDBusReply<QVariantMap> r = walletLauncher->getInterface().readMapList(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = 0;
        const QVariantMap val = r.value();
        for (QVariantMap::const_iterator it = val.begin(); it != val.end(); ++it) {
            QByteArray mapData = it.value().toByteArray();
            if (!mapData.isEmpty()) {
                QDataStream ds(&mapData, QIODevice::ReadOnly);
                QMap<QString, QString> v;
                ds >> v;
                value.insert(it.key(), v);
            }
        }
    }

    return rc;
#else
    return -1;
#endif
}

int Wallet::readPassword(const QString &key, QString &value)
{
    QByteArray ba;
    const int ret = readEntry(key, ba);
    if (ret == 0) {
        value = QString::fromUtf8(ba.constData());
    }
    return ret;
}

int Wallet::readPasswordList(const QString &key, QMap<QString, QString> &value)
{
    return -1;
}

static OSStatus writeEntryImplementation(const QString &walletName, const QString &key, const QByteArray &value)
{
    const QByteArray serviceName(walletName.toUtf8());
    const QByteArray accountName(key.toUtf8());
    QString errMsg;
    OSStatus err = SecKeychainAddGenericPassword(NULL,
                                                 serviceName.size(),
                                                 serviceName.constData(),
                                                 accountName.size(),
                                                 accountName.constData(),
                                                 value.size(),
                                                 value.constData(),
                                                 NULL);
    if (err == errSecDuplicateItem) {
        err = removeEntryImplementation(walletName, key);
        if (isError(err, &errMsg)) {
            qWarning() << "Could not delete old key in keychain for replacing: " << qPrintable(errMsg);
            return err;
        }
    }
    if (isError(err, &errMsg)) {
        qWarning() << "Could not store password in keychain: " << qPrintable(errMsg);
        return err;
    }
    // qDebug() << "Successfully written out key:" << key;
    return err;
}

int Wallet::writeEntry(const QString &key, const QByteArray &password, EntryType entryType)
{
    Q_UNUSED(entryType)
    return writeEntryImplementation(walletName(), key, password);
}

int Wallet::writeEntry(const QString &key, const QByteArray &value)
{
    return writeEntryImplementation(walletName(), key, value);
}

int Wallet::writeMap(const QString &key, const QMap<QString, QString> &value)
{
    QByteArray mapData;
    QDataStream ds(&mapData, QIODevice::WriteOnly);
    ds << value;
    return writeEntry(key, mapData);
}

int Wallet::writePassword(const QString &key, const QString &value)
{
    return writeEntry(key, value.toUtf8());
}

bool Wallet::hasEntry(const QString &key)
{
    const QByteArray serviceName(walletName().toUtf8());
    const QByteArray accountName(key.toUtf8());
    return !isError(
        SecKeychainFindGenericPassword(NULL, serviceName.size(), serviceName.constData(), accountName.size(), accountName.constData(), NULL, NULL, NULL),
        0);
}

int Wallet::removeEntry(const QString &key)
{
    return removeEntryImplementation(walletName(), key);
}

Wallet::EntryType Wallet::entryType(const QString &key)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    int rc = 0;

    if (d->handle == -1) {
        return Wallet::Unknown;
    }

    QDBusReply<int> r = walletLauncher->getInterface().entryType(d->handle, d->folder, key, appid());
    if (r.isValid()) {
        rc = r;
    }

    return static_cast<EntryType>(rc);
#else
    return Wallet::Unknown;
#endif
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
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    if (d->handle >= 0 && d->name == wallet && application == appid()) {
        slotWalletClosed(d->handle);
    }
#endif
}

void Wallet::walletAsyncOpened(int tId, int handle)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    // ignore responses to calls other than ours
    if (d->transactionId != tId || d->handle != -1) {
        return;
    }

    // disconnect the async signal
    disconnect(this, SLOT(walletAsyncOpened(int, int)));

    d->handle = handle;
    Q_EMIT walletOpened(handle > 0);
#endif
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
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<bool> r = walletLauncher->getInterface().folderDoesNotExist(wallet, folder);
    return r;
#else
    return false;
#endif
}

bool Wallet::keyDoesNotExist(const QString &wallet, const QString &folder, const QString &key)
{
#ifdef OSX_KEYCHAIN_PORT_DISABLED
    QDBusReply<bool> r = walletLauncher->getInterface().keyDoesNotExist(wallet, folder, key);
    return r;
#else
    return false;
#endif
}

void Wallet::slotCollectionStatusChanged(int status)
{
}

void Wallet::slotCollectionDeleted()
{
    d->folder.clear();
    d->name.clear();
    Q_EMIT walletClosed();
}

void Wallet::virtual_hook(int, void *)
{
    // BASE::virtual_hook( id, data );
}

#include "moc_kwallet.cpp"
