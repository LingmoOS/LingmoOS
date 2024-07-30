/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001-2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KWALLETBACKEND_H
#define _KWALLETBACKEND_H

#include "backendpersisthandler.h"
#include "kwalletbackend_export.h"
#include "kwalletentry.h"
#include <QMap>
#include <QString>
#include <QStringList>

#ifdef HAVE_GPGMEPP
#include <gpgme++/key.h>
#endif // HAVE_GPGMEPP

#define PBKDF2_SHA512_KEYSIZE 56
#define PBKDF2_SHA512_SALTSIZE 56
#define PBKDF2_SHA512_ITERATIONS 50000

namespace KWallet
{
/**
 * @internal
 */
class MD5Digest : public QByteArray
{
public:
    MD5Digest()
        : QByteArray(16, 0)
    {
    }
    MD5Digest(const char *data)
        : QByteArray(data, 16)
    {
    }
    MD5Digest(const QByteArray &digest)
        : QByteArray(digest)
    {
    }
    virtual ~MD5Digest()
    {
    }

    int operator<(const MD5Digest &r) const
    {
        int i = 0;
        char x, y;
        for (; i < 16; ++i) {
            x = at(i);
            y = r.at(i);
            if (x != y) {
                break;
            }
        }
        if (i < 16 && x < y) {
            return 1;
        }
        return 0;
    }
};

/* @internal
 */
class KWALLETBACKEND_EXPORT Backend
{
public:
    explicit Backend(const QString &name = QStringLiteral("kdewallet"), bool isPath = false);
    ~Backend();

    // Open and unlock the wallet.
    // If opening succeeds, the password's hash will be remembered.
    // If opening fails, the password's hash will be cleared.
    int open(const QByteArray &password, WId w = 0);
#ifdef HAVE_GPGMEPP
    int open(const GpgME::Key &key);
#endif

    // Open and unlock the wallet using a pre-hashed password.
    // If opening succeeds, the password's hash will be remembered.
    // If opening fails, the password's hash will be cleared.
    int openPreHashed(const QByteArray &passwordHash);

    // Close the wallet, losing any changes.
    // if save is true, the wallet is saved prior to closing it.
    int close(bool save = false);

    // Write the wallet to disk
    int sync(WId w);

    // Returns true if the current wallet is open.
    bool isOpen() const;

    // Returns the current wallet name.
    const QString &walletName() const;

    // Rename the wallet
    int renameWallet(const QString &newName, bool isPath = false);

    // The list of folders.
    QStringList folderList() const;

    // Force creation of a folder.
    bool createFolder(const QString &f);

    // Change the folder.
    void setFolder(const QString &f)
    {
        _folder = f;
    }

    // Current folder.  If empty, it's the global folder.
    const QString &folder() const
    {
        return _folder;
    }

    // Does it have this folder?
    bool hasFolder(const QString &f) const
    {
        return _entries.contains(f);
    }

    // Look up an entry.  Returns null if it doesn't exist.
    Entry *readEntry(const QString &key);

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
    // Look up a list of entries.  Supports wildcards.
    // You delete the list.
    // Deprecated since 5.72, use entriesList()
    QList<Entry *> readEntryList(const QString &key);
#endif

    // Get a list of all the entries in the current folder.
    // @since 5.72
    QList<Entry *> entriesList() const;

    // Store an entry.
    void writeEntry(Entry *e);

    // Does this folder contain this entry?
    bool hasEntry(const QString &key) const;

    // Returns true if the entry was removed
    bool removeEntry(const QString &key);

    // Returns true if the folder was removed
    bool removeFolder(const QString &f);

    // The list of entries in this folder.
    QStringList entryList() const;

    // Rename an entry in this folder.
    int renameEntry(const QString &oldName, const QString &newName);

    // Set the password used for opening/closing the wallet.
    // This does not sync the wallet to disk!
    void setPassword(const QByteArray &password);

    int ref()
    {
        return ++_ref;
    }

    int deref();

    int refCount() const
    {
        return _ref;
    }

    static bool exists(const QString &wallet);

    bool folderDoesNotExist(const QString &folder) const;

    bool entryDoesNotExist(const QString &folder, const QString &entry) const;

    static QString openRCToString(int rc);

    void setCipherType(BackendCipherType ct);
    BackendCipherType cipherType() const
    {
        return _cipherType;
    }
#ifdef HAVE_GPGMEPP
    const GpgME::Key &gpgKey() const;
#endif

    static QString getSaveLocation();
    static QString encodeWalletName(const QString &name);
    static QString decodeWalletName(const QString &encodedName);

private:
    Q_DISABLE_COPY(Backend)
    class BackendPrivate;
    BackendPrivate *const d;
    QString _name;
    QString _path;
    bool _open;
    bool _useNewHash = false;
    QString _folder;
    int _ref = 0;
    // Map Folder->Entries
    typedef QMap<QString, Entry *> EntryMap;
    typedef QMap<QString, EntryMap> FolderMap;
    FolderMap _entries;
    typedef QMap<MD5Digest, QList<MD5Digest>> HashMap;
    HashMap _hashes;
    QByteArray _passhash; // password hash used for saving the wallet
    QByteArray _newPassHash; // Modern hash using KWALLET_HASH_PBKDF2_SHA512
    BackendCipherType _cipherType; // the kind of encryption used for this wallet

#ifdef HAVE_GPGMEPP
    GpgME::Key _gpgKey;
#endif
    friend class BlowfishPersistHandler;
    friend class GpgPersistHandler;

    // open the wallet with the password already set. This is
    // called internally by both open and openPreHashed.
    int openInternal(WId w = 0);
    int closeInternal(bool save);
    void swapToNewHash();
    QByteArray createAndSaveSalt(const QString &path) const;
};

}

#endif
