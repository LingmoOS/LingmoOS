/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2001-2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kwalletbackend.h"
#include "kwalletbackend_debug.h"

#include <stdlib.h>

#include <QSaveFile>
#ifdef HAVE_GPGMEPP
#include <gpgme++/key.h>
#endif
#include <gcrypt.h>
#include <KNotification>
#include <KLocalizedString>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QStandardPaths>

#include "blowfish.h"
#include "sha1.h"
#include "cbc.h"

#include <assert.h>
#include <cerrno>

// quick fix to get random numbers on win32
#ifdef Q_OS_WIN //krazy:exclude=cpp
#include <windows.h>
#include <wincrypt.h>
#endif

#define KWALLETSTORAGE_VERSION_MAJOR       0
#define KWALLETSTORAGE_VERSION_MINOR       1

using namespace KWallet;

#define KWMAGIC "KWALLET\n\r\0\r\n"

static const QByteArray walletAllowedChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789^&'@{}[],$=!-#()%.+_\r\n\t\f\v ";

/* The encoding works if the name contains at least one unsupported character.
 * Names that were allowed prior to the Secret Service API patch remain intact.
 */
QString Backend::encodeWalletName(const QString &name) {
    /* Use a semicolon as "percent" because it does not conflict with already allowed characters for wallet names
     * and is allowed for file names
     */
    return QString::fromUtf8(name.toUtf8().toPercentEncoding(walletAllowedChars, {}, ';'));
}

QString Backend::decodeWalletName(const QString &encodedName) {
    return QString::fromUtf8(QByteArray::fromPercentEncoding(encodedName.toUtf8(), ';'));
}

class Backend::BackendPrivate
{
};

// static void initKWalletDir()
// {
//     KGlobal::dirs()->addResourceType("kwallet", 0, "share/apps/kwallet");
// }

Backend::Backend(const QString &name, bool isPath)
    : d(nullptr),
      _name(name),
      _cipherType(KWallet::BACKEND_CIPHER_UNKNOWN)
{
//  initKWalletDir();
    if (isPath) {
        _path = name;
    } else {
        _path = getSaveLocation() + '/' + encodeWalletName(_name) + ".kwl";
    }

    _open = false;
}

Backend::~Backend()
{
    if (_open) {
        close();
    }
    delete d;
}

QString Backend::getSaveLocation()
{
    QString writeLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/kwalletd");
    QDir writeDir(writeLocation);
    if (!writeDir.exists()) {
        if (!writeDir.mkpath(writeLocation)) {
            qFatal("Cannot create wallet save location!");
        }
    }

    // qCDebug(KWALLETBACKEND_LOG) << "Using saveLocation " + writeLocation;
    return writeLocation;
}

void Backend::setCipherType(BackendCipherType ct)
{
    // changing cipher type on already initialed wallets is not permitted
    assert(_cipherType == KWallet::BACKEND_CIPHER_UNKNOWN);
    _cipherType = ct;
}

static int password2PBKDF2_SHA512(const QByteArray &password, QByteArray &hash, const QByteArray &salt)
{
    if (!gcry_check_version("1.5.0")) {
        qCWarning(KWALLETBACKEND_LOG) << "libcrypt version is too old";
        return GPG_ERR_USER_2;
    }

    gcry_error_t error;
    bool static gcry_secmem_init = false;
    if (!gcry_secmem_init) {
        error = gcry_control(GCRYCTL_INIT_SECMEM, 32768, 0);
        if (error != 0) {
            qCWarning(KWALLETBACKEND_LOG) << "Can't get secure memory:" << error;
            return error;
        }
        gcry_secmem_init = true;
    }

    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    error = gcry_kdf_derive(password.constData(), password.size(),
                            GCRY_KDF_PBKDF2, GCRY_MD_SHA512,
                            salt.data(), salt.size(),
                            PBKDF2_SHA512_ITERATIONS, PBKDF2_SHA512_KEYSIZE, hash.data());

    return error;
}

// this should be SHA-512 for release probably
static int password2hash(const QByteArray &password, QByteArray &hash)
{
    SHA1 sha;
    int shasz = sha.size() / 8;

    assert(shasz >= 20);

    QByteArray block1(shasz, 0);

    sha.process(password.data(), qMin(password.size(), 16));

    // To make brute force take longer
    for (int i = 0; i < 2000; i++) {
        memcpy(block1.data(), sha.hash(), shasz);
        sha.reset();
        sha.process(block1.data(), shasz);
    }

    sha.reset();

    if (password.size() > 16) {
        sha.process(password.data() + 16, qMin(password.size() - 16, 16));
        QByteArray block2(shasz, 0);
        // To make brute force take longer
        for (int i = 0; i < 2000; i++) {
            memcpy(block2.data(), sha.hash(), shasz);
            sha.reset();
            sha.process(block2.data(), shasz);
        }

        sha.reset();

        if (password.size() > 32) {
            sha.process(password.data() + 32, qMin(password.size() - 32, 16));

            QByteArray block3(shasz, 0);
            // To make brute force take longer
            for (int i = 0; i < 2000; i++) {
                memcpy(block3.data(), sha.hash(), shasz);
                sha.reset();
                sha.process(block3.data(), shasz);
            }

            sha.reset();

            if (password.size() > 48) {
                sha.process(password.data() + 48, password.size() - 48);

                QByteArray block4(shasz, 0);
                // To make brute force take longer
                for (int i = 0; i < 2000; i++) {
                    memcpy(block4.data(), sha.hash(), shasz);
                    sha.reset();
                    sha.process(block4.data(), shasz);
                }

                sha.reset();
                // split 14/14/14/14
                hash.resize(56);
                memcpy(hash.data(),      block1.data(), 14);
                memcpy(hash.data() + 14, block2.data(), 14);
                memcpy(hash.data() + 28, block3.data(), 14);
                memcpy(hash.data() + 42, block4.data(), 14);
                block4.fill(0);
            } else {
                // split 20/20/16
                hash.resize(56);
                memcpy(hash.data(),      block1.data(), 20);
                memcpy(hash.data() + 20, block2.data(), 20);
                memcpy(hash.data() + 40, block3.data(), 16);
            }
            block3.fill(0);
        } else {
            // split 20/20
            hash.resize(40);
            memcpy(hash.data(),      block1.data(), 20);
            memcpy(hash.data() + 20, block2.data(), 20);
        }
        block2.fill(0);
    } else {
        // entirely block1
        hash.resize(20);
        memcpy(hash.data(), block1.data(), 20);
    }

    block1.fill(0);

    return 0;
}

int Backend::deref()
{
    if (--_ref < 0) {
        qCDebug(KWALLETBACKEND_LOG) << "refCount negative!";
        _ref = 0;
    }
    return _ref;
}

bool Backend::exists(const QString &wallet)
{
    QString saveLocation = getSaveLocation();
    QString path = saveLocation + '/' + encodeWalletName(wallet) + QLatin1String(".kwl");
    // Note: 60 bytes is presently the minimum size of a wallet file.
    //       Anything smaller is junk.
    return QFile::exists(path) && QFileInfo(path).size() >= 60;
}

QString Backend::openRCToString(int rc)
{
    switch (rc) {
    case -255:
        return i18n("Already open.");
    case -2:
        return i18n("Error opening file.");
    case -3:
        return i18n("Not a wallet file.");
    case -4:
        return i18n("Unsupported file format revision.");
    case -41:
        return QStringLiteral("Unknown cipher or hash"); //FIXME: use i18n after string freeze
    case -42:
        return i18n("Unknown encryption scheme.");
    case -43:
        return i18n("Corrupt file?");
    case -8:
        return i18n("Error validating wallet integrity. Possibly corrupted.");
    case -5:
    case -7:
    case -9:
        return i18n("Read error - possibly incorrect password.");
    case -6:
        return i18n("Decryption error.");
    default:
        return QString();
    }
}

int Backend::open(const QByteArray &password, WId w)
{
    if (_open) {
        return -255;  // already open
    }

    setPassword(password);
    return openInternal(w);
}

#ifdef HAVE_GPGMEPP
int Backend::open(const GpgME::Key &key)
{
    if (_open) {
        return -255;  // already open
    }
    _gpgKey = key;
    return openInternal();
}
#endif // HAVE_GPGMEPP

int Backend::openPreHashed(const QByteArray &passwordHash)
{
    if (_open) {
        return -255;  // already open
    }

    // check the password hash for correct size (currently fixed)
    if (passwordHash.size() != 20 && passwordHash.size() != 40 &&
            passwordHash.size() != 56) {
        return -42; // unsupported encryption scheme
    }

    _passhash = passwordHash;
    _newPassHash = passwordHash;
    _useNewHash = true;//Only new hash is supported

    return openInternal();
}

int Backend::openInternal(WId w)
{
    // No wallet existed.  Let's create it.
    // Note: 60 bytes is presently the minimum size of a wallet file.
    //       Anything smaller is junk and should be deleted.
    if (!QFile::exists(_path) || QFileInfo(_path).size() < 60) {
        QFile newfile(_path);
        if (!newfile.open(QIODevice::ReadWrite)) {
            return -2;   // error opening file
        }
        newfile.close();
        _open = true;
        if (sync(w) != 0) {
            return -2;
        }
    }

    QFile db(_path);

    if (!db.open(QIODevice::ReadOnly)) {
        return -2;         // error opening file
    }

    char magicBuf[KWMAGIC_LEN];
    db.read(magicBuf, KWMAGIC_LEN);
    if (memcmp(magicBuf, KWMAGIC, KWMAGIC_LEN) != 0) {
        return -3;         // bad magic
    }

    db.read(magicBuf, 4);

    // First byte is major version, second byte is minor version
    if (magicBuf[0] != KWALLETSTORAGE_VERSION_MAJOR) {
        return -4;         // unknown version
    }

    //0 has been the MINOR version until 4.13, from that point we use it to upgrade the hash
    if (magicBuf[1] == 1) {
        qCDebug(KWALLETBACKEND_LOG) << "Wallet new enough, using new hash";
        swapToNewHash();
    } else if (magicBuf[1] != 0) {
        qCDebug(KWALLETBACKEND_LOG) << "Wallet is old, sad panda :(";
        return -4;  // unknown version
    }

    BackendPersistHandler *phandler = BackendPersistHandler::getPersistHandler(magicBuf);
    if (nullptr == phandler) {
        return -41; // unknown cipher or hash
    }
    int result = phandler->read(this, db, w);
    delete phandler;
    return result;
}

void Backend::swapToNewHash()
{
    //Runtime error happened and we can't use the new hash
    if (!_useNewHash) {
        qCDebug(KWALLETBACKEND_LOG) << "Runtime error on the new hash";
        return;
    }
    _passhash.fill(0);//Making sure the old passhash is not around in memory
    _passhash = _newPassHash;//Use the new hash, means the wallet is modern enough
}

QByteArray Backend::createAndSaveSalt(const QString &path) const
{
    QFile saltFile(path);
    saltFile.remove();

    if (!saltFile.open(QIODevice::WriteOnly)) {
        return QByteArray();
    }
    saltFile.setPermissions(QFile::ReadUser | QFile::WriteUser);

    char *randomData = (char *) gcry_random_bytes(PBKDF2_SHA512_SALTSIZE, GCRY_STRONG_RANDOM);
    QByteArray salt(randomData, PBKDF2_SHA512_SALTSIZE);
    free(randomData);

    if (saltFile.write(salt) != PBKDF2_SHA512_SALTSIZE) {
        return QByteArray();
    }

    saltFile.close();

    return salt;
}

int Backend::sync(WId w)
{
    if (!_open) {
        return -255;  // not open yet
    }

    if (!QFile::exists(_path)) {
        return -3; // File does not exist
    }

    QSaveFile sf(_path);

    if (!sf.open(QIODevice::WriteOnly | QIODevice::Unbuffered)) {
        return -1;      // error opening file
    }
    sf.setPermissions(QFile::ReadUser | QFile::WriteUser);

    if (sf.write(KWMAGIC, KWMAGIC_LEN) != KWMAGIC_LEN) {
        sf.cancelWriting();
        return -4; // write error
    }

    // Write the version number
    QByteArray version(4, 0);
    version[0] = KWALLETSTORAGE_VERSION_MAJOR;
    if (_useNewHash) {
        version[1] = KWALLETSTORAGE_VERSION_MINOR;
        //Use the sync to update the hash to PBKDF2_SHA512
        swapToNewHash();
    } else {
        version[1] = 0; //was KWALLETSTORAGE_VERSION_MINOR before the new hash
    }

    BackendPersistHandler *phandler = BackendPersistHandler::getPersistHandler(_cipherType);
    if (nullptr == phandler) {
        return -4; // write error
    }
    int rc = phandler->write(this, sf, version, w);
    if (rc < 0) {
        // Oops! wallet file sync filed! Display a notification about that
        // TODO: change kwalletd status flags, when status flags will be implemented
        KNotification *notification = new KNotification(QStringLiteral("syncFailed"));
        notification->setText(i18n("Failed to sync wallet <b>%1</b> to disk. Error codes are:\nRC <b>%2</b>\nSF <b>%3</b>. Please file a BUG report using this information to bugs.kde.org", _name, rc, sf.errorString()));
        notification->sendEvent();
    }
    delete phandler;
    return rc;
}

int Backend::closeInternal(bool save)
{
    // save if requested
    if (save) {
        int rc = sync(0);
        if (rc != 0) {
            return rc;
        }
    }

    // do the actual close
    for (FolderMap::ConstIterator i = _entries.constBegin(); i != _entries.constEnd(); ++i) {
        for (EntryMap::ConstIterator j = i.value().constBegin(); j != i.value().constEnd(); ++j) {
            delete j.value();
        }
    }
    _entries.clear();
    _open = false;

    return 0;
}

int Backend::close(bool save)
{
    int rc = closeInternal(save);
    if (rc)
        return rc;

    // empty the password hash
    _passhash.fill(0);
    _newPassHash.fill(0);

    return 0;
}

const QString &Backend::walletName() const
{
    return _name;
}

int Backend::renameWallet(const QString &newName, bool isPath)
{
    QString newPath;
    const auto saveLocation = getSaveLocation();

    if (isPath) {
        newPath = newName;
    } else {
        newPath = saveLocation + QChar::fromLatin1('/') + encodeWalletName(newName) + QStringLiteral(".kwl");
    }

    if (newPath == _path) {
        return 0;
    }

    if (QFile::exists(newPath)) {
        return -EEXIST;
    }

    int rc = closeInternal(true);
    if (rc) {
        return rc;
    }

    QFile::rename(_path, newPath);
    QFile::rename(saveLocation + QChar::fromLatin1('/') + encodeWalletName(_name) + QStringLiteral(".salt"),
                  saveLocation + QChar::fromLatin1('/') + encodeWalletName(newName) + QStringLiteral(".salt"));

    _name = newName;
    _path = newPath;

    rc = openInternal();
    if (rc) {
        return rc;
    }

    return 0;
}

bool Backend::isOpen() const
{
    return _open;
}

QStringList Backend::folderList() const
{
    return _entries.keys();
}

QStringList Backend::entryList() const
{
    return _entries[_folder].keys();
}

Entry *Backend::readEntry(const QString &key)
{
    Entry *rc = nullptr;

    if (_open && hasEntry(key)) {
        rc = _entries[_folder][key];
    }

    return rc;
}

#if KWALLET_BUILD_DEPRECATED_SINCE(5, 72)
QList<Entry *> Backend::readEntryList(const QString &key)
{
    QList<Entry *> rc;

    if (!_open) {
        return rc;
    }

    // HACK: see Wallet::WalletPrivate::forEachItemThatMatches()
    const QString pattern = QRegularExpression::wildcardToRegularExpression(key).replace(
                                                         QLatin1String("[^/]"), QLatin1String("."));
    const QRegularExpression re(pattern);

    const EntryMap &map = _entries[_folder];
    for (EntryMap::ConstIterator i = map.begin(); i != map.end(); ++i) {
        if (re.match(i.key()).hasMatch()) {
            rc.append(i.value());
        }
    }
    return rc;
}
#endif

QList<Entry *> Backend::entriesList() const
{
    if (!_open) {
        return QList<Entry *>();
    }
    const EntryMap &map = _entries[_folder];

    return map.values();
}


bool Backend::createFolder(const QString &f)
{
    if (_entries.contains(f)) {
        return false;
    }

    _entries.insert(f, EntryMap());

    QCryptographicHash folderMd5(QCryptographicHash::Md5);
    folderMd5.addData(f.toUtf8());
    _hashes.insert(MD5Digest(folderMd5.result()), QList<MD5Digest>());

    return true;
}

int Backend::renameEntry(const QString &oldName, const QString &newName)
{
    EntryMap &emap = _entries[_folder];
    EntryMap::Iterator oi = emap.find(oldName);
    EntryMap::Iterator ni = emap.find(newName);

    if (oi != emap.end() && ni == emap.end()) {
        Entry *e = oi.value();
        emap.erase(oi);
        emap[newName] = e;

        QCryptographicHash folderMd5(QCryptographicHash::Md5);
        folderMd5.addData(_folder.toUtf8());

        HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.result()));
        if (i != _hashes.end()) {
            QCryptographicHash oldMd5(QCryptographicHash::Md5);
            QCryptographicHash newMd5(QCryptographicHash::Md5);
            oldMd5.addData(oldName.toUtf8());
            newMd5.addData(newName.toUtf8());
            i.value().removeAll(MD5Digest(oldMd5.result()));
            i.value().append(MD5Digest(newMd5.result()));
        }
        return 0;
    }

    return -1;
}

void Backend::writeEntry(Entry *e)
{
    if (!_open) {
        return;
    }

    if (!hasEntry(e->key())) {
        _entries[_folder][e->key()] = new Entry;
    }
    _entries[_folder][e->key()]->copy(e);

    QCryptographicHash folderMd5(QCryptographicHash::Md5);
    folderMd5.addData(_folder.toUtf8());

    HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.result()));
    if (i != _hashes.end()) {
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(e->key().toUtf8());
        i.value().append(MD5Digest(md5.result()));
    }
}

bool Backend::hasEntry(const QString &key) const
{
    return _entries.contains(_folder) && _entries[_folder].contains(key);
}

bool Backend::removeEntry(const QString &key)
{
    if (!_open) {
        return false;
    }

    FolderMap::Iterator fi = _entries.find(_folder);
    EntryMap::Iterator ei = fi.value().find(key);

    if (fi != _entries.end() && ei != fi.value().end()) {
        delete ei.value();
        fi.value().erase(ei);
        QCryptographicHash folderMd5(QCryptographicHash::Md5);
        folderMd5.addData(_folder.toUtf8());

        HashMap::iterator i = _hashes.find(MD5Digest(folderMd5.result()));
        if (i != _hashes.end()) {
            QCryptographicHash md5(QCryptographicHash::Md5);
            md5.addData(key.toUtf8());
            i.value().removeAll(MD5Digest(md5.result()));
        }
        return true;
    }

    return false;
}

bool Backend::removeFolder(const QString &f)
{
    if (!_open) {
        return false;
    }

    FolderMap::Iterator fi = _entries.find(f);

    if (fi != _entries.end()) {
        if (_folder == f) {
            _folder.clear();
        }

        for (EntryMap::Iterator ei = fi.value().begin(); ei != fi.value().end(); ++ei) {
            delete ei.value();
        }

        _entries.erase(fi);

        QCryptographicHash folderMd5(QCryptographicHash::Md5);
        folderMd5.addData(f.toUtf8());
        _hashes.remove(MD5Digest(folderMd5.result()));
        return true;
    }

    return false;
}

bool Backend::folderDoesNotExist(const QString &folder) const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(folder.toUtf8());
    return !_hashes.contains(MD5Digest(md5.result()));
}

bool Backend::entryDoesNotExist(const QString &folder, const QString &entry) const
{
    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(folder.toUtf8());
    HashMap::const_iterator i = _hashes.find(MD5Digest(md5.result()));
    if (i != _hashes.end()) {
        md5.reset();
        md5.addData(entry.toUtf8());
        return !i.value().contains(MD5Digest(md5.result()));
    }
    return true;
}

void Backend::setPassword(const QByteArray &password)
{
    _passhash.fill(0); // empty just in case
    BlowFish _bf;
    CipherBlockChain bf(&_bf);
    _passhash.resize(bf.keyLen() / 8);
    _newPassHash.resize(bf.keyLen() / 8);
    _newPassHash.fill(0);

    password2hash(password, _passhash);

    QByteArray salt;
    QFile saltFile(getSaveLocation() + '/' + encodeWalletName(_name) + ".salt");
    if (!saltFile.exists() || saltFile.size() == 0) {
        salt = createAndSaveSalt(saltFile.fileName());
    } else {
        if (!saltFile.open(QIODevice::ReadOnly)) {
            salt = createAndSaveSalt(saltFile.fileName());
        } else {
            salt = saltFile.readAll();
        }
    }

    if (!salt.isEmpty() && password2PBKDF2_SHA512(password, _newPassHash,  salt) == 0) {
        qCDebug(KWALLETBACKEND_LOG) << "Setting useNewHash to true";
        _useNewHash = true;
    }
}

#ifdef HAVE_GPGMEPP
const GpgME::Key &Backend::gpgKey() const
{
    return _gpgKey;
}
#endif
