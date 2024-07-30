/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "backendpersisthandler.h"
#include "kwalletbackend_debug.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <QCryptographicHash>
#include <QFile>
#include <QIODevice>
#include <QSaveFile>
#include <assert.h>
#ifdef HAVE_GPGMEPP
#include <gpgme++/context.h>
#include <gpgme++/data.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/encryptionresult.h>
#include <gpgme++/key.h>
#include <gpgme++/keylistresult.h>
#endif
#include "blowfish.h"
#include "cbc.h"
#include "kwalletbackend.h"
#include "sha1.h"

#ifdef Q_OS_WIN
#include <windows.h> // Must be included before wincrypt.h

#include <wincrypt.h>
#endif

#define KWALLET_CIPHER_BLOWFISH_ECB 0 // this was the old KWALLET_CIPHER_BLOWFISH_CBC
#define KWALLET_CIPHER_3DES_CBC 1 // unsupported
#define KWALLET_CIPHER_GPG 2
#define KWALLET_CIPHER_BLOWFISH_CBC 3

#define KWALLET_HASH_SHA1 0
#define KWALLET_HASH_MD5 1 // unsupported
#define KWALLET_HASH_PBKDF2_SHA512 2 // used when using kwallet with pam or since 4.13 version

namespace KWallet
{
typedef char Digest[16];

static int getRandomBlock(QByteArray &randBlock)
{
#ifdef Q_OS_WIN // krazy:exclude=cpp

    // Use windows crypto API to get randomness on win32
    // HACK: this should be done using qca
    HCRYPTPROV hProv;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        return -1; // couldn't get random data
    }

    if (!CryptGenRandom(hProv, static_cast<DWORD>(randBlock.size()), (BYTE *)randBlock.data())) {
        return -3; // read error
    }

    // release the crypto context
    CryptReleaseContext(hProv, 0);

    return randBlock.size();

#else

    // First try /dev/urandom
    if (QFile::exists(QStringLiteral("/dev/urandom"))) {
        QFile devrand(QStringLiteral("/dev/urandom"));
        if (devrand.open(QIODevice::ReadOnly)) {
            int rc = devrand.read(randBlock.data(), randBlock.size());

            if (rc != randBlock.size()) {
                return -3; // not enough data read
            }

            return 0;
        }
    }

    // If that failed, try /dev/random
    // FIXME: open in noblocking mode!
    if (QFile::exists(QStringLiteral("/dev/random"))) {
        QFile devrand(QStringLiteral("/dev/random"));
        if (devrand.open(QIODevice::ReadOnly)) {
            int rc = 0;
            int cnt = 0;

            do {
                int rc2 = devrand.read(randBlock.data() + rc, randBlock.size());

                if (rc2 < 0) {
                    return -3; // read error
                }

                rc += rc2;
                cnt++;
                if (cnt > randBlock.size()) {
                    return -4; // reading forever?!
                }
            } while (rc < randBlock.size());

            return 0;
        }
    }

    // EGD method
    QString randFilename = QString::fromLocal8Bit(qgetenv("RANDFILE"));
    if (!randFilename.isEmpty()) {
        if (QFile::exists(randFilename)) {
            QFile devrand(randFilename);
            if (devrand.open(QIODevice::ReadOnly)) {
                int rc = devrand.read(randBlock.data(), randBlock.size());
                if (rc != randBlock.size()) {
                    return -3; // not enough data read
                }
                return 0;
            }
        }
    }

    // Couldn't get any random data!!
    return -1;

#endif
}

BackendPersistHandler *BackendPersistHandler::getPersistHandler(BackendCipherType cipherType)
{
    switch (cipherType) {
    case BACKEND_CIPHER_BLOWFISH:
        return new BlowfishPersistHandler;
#ifdef HAVE_GPGMEPP
    case BACKEND_CIPHER_GPG:
        return new GpgPersistHandler;
#endif // HAVE_GPGMEPP
    default:
        Q_ASSERT(0);
        return nullptr;
    }
}

BackendPersistHandler *BackendPersistHandler::getPersistHandler(char magicBuf[12])
{
    if ((magicBuf[2] == KWALLET_CIPHER_BLOWFISH_ECB || magicBuf[2] == KWALLET_CIPHER_BLOWFISH_CBC)
        && (magicBuf[3] == KWALLET_HASH_SHA1 || magicBuf[3] == KWALLET_HASH_PBKDF2_SHA512)) {
        bool useECBforReading = magicBuf[2] == KWALLET_CIPHER_BLOWFISH_ECB;
        if (useECBforReading) {
            qCDebug(KWALLETBACKEND_LOG) << "this wallet uses ECB encryption. It'll be converted to CBC on next save.";
        }
        return new BlowfishPersistHandler(useECBforReading);
    }
#ifdef HAVE_GPGMEPP
    if (magicBuf[2] == KWALLET_CIPHER_GPG && magicBuf[3] == 0) {
        return new GpgPersistHandler;
    }
#endif // HAVE_GPGMEPP
    return nullptr; // unknown cipher or hash
}

int BlowfishPersistHandler::write(Backend *wb, QSaveFile &sf, QByteArray &version, WId)
{
    assert(wb->_cipherType == BACKEND_CIPHER_BLOWFISH);

    if (_useECBforReading) {
        qCDebug(KWALLETBACKEND_LOG) << "This wallet used ECB and is now saved using CBC";
        _useECBforReading = false;
    }

    version[2] = KWALLET_CIPHER_BLOWFISH_CBC;
    if (!wb->_useNewHash) {
        version[3] = KWALLET_HASH_SHA1;
    } else {
        version[3] = KWALLET_HASH_PBKDF2_SHA512; // Since 4.13 we always use PBKDF2_SHA512
    }

    if (sf.write(version) != 4) {
        sf.cancelWriting();
        return -4; // write error
    }

    // Holds the hashes we write out
    QByteArray hashes;
    QDataStream hashStream(&hashes, QIODevice::WriteOnly);
    QCryptographicHash md5(QCryptographicHash::Md5);
    hashStream << static_cast<quint32>(wb->_entries.count());

    // Holds decrypted data prior to encryption
    QByteArray decrypted;

    // FIXME: we should estimate the amount of data we will write in each
    // buffer and resize them approximately in order to avoid extra
    // resizes.

    // populate decrypted
    QDataStream dStream(&decrypted, QIODevice::WriteOnly);
    for (Backend::FolderMap::ConstIterator i = wb->_entries.constBegin(); i != wb->_entries.constEnd(); ++i) {
        dStream << i.key();
        dStream << static_cast<quint32>(i.value().count());

        md5.reset();
        md5.addData(i.key().toUtf8());
        hashStream.writeRawData(md5.result().constData(), 16);
        hashStream << static_cast<quint32>(i.value().count());

        for (Backend::EntryMap::ConstIterator j = i.value().constBegin(); j != i.value().constEnd(); ++j) {
            dStream << j.key();
            dStream << static_cast<qint32>(j.value()->type());
            dStream << j.value()->value();

            md5.reset();
            md5.addData(j.key().toUtf8());
            hashStream.writeRawData(md5.result().constData(), 16);
        }
    }

    if (sf.write(hashes) != hashes.size()) {
        sf.cancelWriting();
        return -4; // write error
    }

    // calculate the hash of the file
    SHA1 sha;
    BlowFish _bf;
    CipherBlockChain bf(&_bf);

    sha.process(decrypted.data(), decrypted.size());

    // prepend and append the random data
    QByteArray wholeFile;
    long blksz = bf.blockSize();
    long newsize = decrypted.size() + blksz + // encrypted block
        4 + // file size
        20; // size of the SHA hash

    int delta = (blksz - (newsize % blksz));
    newsize += delta;
    wholeFile.resize(newsize);

    QByteArray randBlock;
    randBlock.resize(blksz + delta);
    if (getRandomBlock(randBlock) < 0) {
        sha.reset();
        decrypted.fill(0);
        sf.cancelWriting();
        return -3; // Fatal error: can't get random
    }

    for (int i = 0; i < blksz; i++) {
        wholeFile[i] = randBlock[i];
    }

    for (int i = 0; i < 4; i++) {
        wholeFile[(int)(i + blksz)] = (decrypted.size() >> 8 * (3 - i)) & 0xff;
    }

    for (int i = 0; i < decrypted.size(); i++) {
        wholeFile[(int)(i + blksz + 4)] = decrypted[i];
    }

    for (int i = 0; i < delta; i++) {
        wholeFile[(int)(i + blksz + 4 + decrypted.size())] = randBlock[(int)(i + blksz)];
    }

    const char *hash = (const char *)sha.hash();
    for (int i = 0; i < 20; i++) {
        wholeFile[(int)(newsize - 20 + i)] = hash[i];
    }

    sha.reset();
    decrypted.fill(0);

    // encrypt the data
    if (!bf.setKey(wb->_passhash.data(), wb->_passhash.size() * 8)) {
        wholeFile.fill(0);
        sf.cancelWriting();
        return -2; // encrypt error
    }

    int rc = bf.encrypt(wholeFile.data(), wholeFile.size());
    if (rc < 0) {
        wholeFile.fill(0);
        sf.cancelWriting();
        return -2; // encrypt error
    }

    // write the file
    auto written = sf.write(wholeFile);
    if (written != wholeFile.size()) {
        wholeFile.fill(0);
        sf.cancelWriting();
        return -4; // write error
    }
    if (!sf.commit()) {
        qCDebug(KWALLETBACKEND_LOG) << "WARNING: wallet sync to disk failed! QSaveFile status was " << sf.errorString();
        wholeFile.fill(0);
        return -4; // write error
    }

    wholeFile.fill(0);

    return 0;
}

int BlowfishPersistHandler::read(Backend *wb, QFile &db, WId)
{
    wb->_cipherType = BACKEND_CIPHER_BLOWFISH;
    wb->_hashes.clear();
    // Read in the hashes
    QDataStream hds(&db);
    quint32 n;
    hds >> n;
    if (n > 0xffff) { // sanity check
        return -43;
    }

    for (size_t i = 0; i < n; ++i) {
        Digest d;
        Digest d2; // judgment day
        MD5Digest ba;
        QMap<MD5Digest, QList<MD5Digest>>::iterator it;
        quint32 fsz;
        if (hds.atEnd()) {
            return -43;
        }
        hds.readRawData(d, 16);
        hds >> fsz;
        ba = MD5Digest(reinterpret_cast<char *>(d));
        it = wb->_hashes.insert(ba, QList<MD5Digest>());
        for (size_t j = 0; j < fsz; ++j) {
            hds.readRawData(d2, 16);
            ba = MD5Digest(d2);
            (*it).append(ba);
        }
    }

    // Read in the rest of the file.
    QByteArray encrypted = db.readAll();
    assert(encrypted.size() < db.size());

    BlowFish _bf;
    CipherBlockChain bf(&_bf, _useECBforReading);
    int blksz = bf.blockSize();
    if ((encrypted.size() % blksz) != 0) {
        return -5; // invalid file structure
    }

    bf.setKey((void *)wb->_passhash.data(), wb->_passhash.size() * 8);

    if (!encrypted.data()) {
        wb->_passhash.fill(0);
        encrypted.fill(0);
        return -7; // file structure error
    }

    int rc = bf.decrypt(encrypted.data(), encrypted.size());
    if (rc < 0) {
        wb->_passhash.fill(0);
        encrypted.fill(0);
        return -6; // decrypt error
    }

    const char *t = encrypted.data();

    // strip the leading data
    t += blksz; // one block of random data

    // strip the file size off
    long fsize = 0;

    fsize |= (long(*t) << 24) & 0xff000000;
    t++;
    fsize |= (long(*t) << 16) & 0x00ff0000;
    t++;
    fsize |= (long(*t) << 8) & 0x0000ff00;
    t++;
    fsize |= long(*t) & 0x000000ff;
    t++;

    if (fsize < 0 || fsize > long(encrypted.size()) - blksz - 4) {
        qCDebug(KWALLETBACKEND_LOG) << "fsize: " << fsize << " encrypted.size(): " << encrypted.size() << " blksz: " << blksz;
        encrypted.fill(0);
        return -9; // file structure error.
    }

    // compute the hash ourself
    SHA1 sha;
    sha.process(t, fsize);
    const char *testhash = (const char *)sha.hash();

    // compare hashes
    int sz = encrypted.size();
    for (int i = 0; i < 20; i++) {
        if (testhash[i] != encrypted[sz - 20 + i]) {
            encrypted.fill(0);
            sha.reset();
            return -8; // hash error.
        }
    }

    sha.reset();

    // chop off the leading blksz+4 bytes
    QByteArray tmpenc(encrypted.data() + blksz + 4, fsize);
    encrypted = tmpenc;
    tmpenc.fill(0);

    // Load the data structures up
    QDataStream eStream(encrypted);

    while (!eStream.atEnd()) {
        QString folder;
        quint32 n;

        eStream >> folder;
        eStream >> n;

        // Force initialisation
        wb->_entries[folder].clear();

        for (size_t i = 0; i < n; ++i) {
            QString key;
            KWallet::Wallet::EntryType et = KWallet::Wallet::Unknown;
            Entry *e = new Entry;
            eStream >> key;
            qint32 x = 0; // necessary to read properly
            eStream >> x;
            et = static_cast<KWallet::Wallet::EntryType>(x);

            switch (et) {
            case KWallet::Wallet::Password:
            case KWallet::Wallet::Stream:
            case KWallet::Wallet::Map:
                break;
            default: // Unknown entry
                delete e;
                continue;
            }

            QByteArray a;
            eStream >> a;
            e->setValue(a);
            e->setType(et);
            e->setKey(key);
            wb->_entries[folder][key] = e;
        }
    }

    wb->_open = true;
    encrypted.fill(0);
    return 0;
}

#ifdef HAVE_GPGMEPP
GpgME::Error initGpgME()
{
    GpgME::Error err;
    static bool alreadyInitialized = false;
    if (!alreadyInitialized) {
        GpgME::initializeLibrary();
        err = GpgME::checkEngine(GpgME::OpenPGP);
        if (err) {
            qCDebug(KWALLETBACKEND_LOG) << "OpenPGP not supported!";
        }
        alreadyInitialized = true;
    }
    return err;
}

int GpgPersistHandler::write(Backend *wb, QSaveFile &sf, QByteArray &version, WId w)
{
    version[2] = KWALLET_CIPHER_GPG;
    version[3] = 0;
    if (sf.write(version) != 4) {
        sf.cancelWriting();
        return -4; // write error
    }

    GpgME::Error err = initGpgME();
    if (err) {
        qCDebug(KWALLETBACKEND_LOG) << "initGpgME returned " << err.code();
        KMessageBox::errorWId(w,
                              i18n("<qt>Error when attempting to initialize OpenPGP while attempting to save the wallet <b>%1</b>. Error code is <b>%2</b>. "
                                   "Please fix your system configuration, then try again.</qt>",
                                   wb->_name.toHtmlEscaped(),
                                   err.code()));
        sf.cancelWriting();
        return -5;
    }

    std::shared_ptr<GpgME::Context> ctx(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (!ctx) {
        qCDebug(KWALLETBACKEND_LOG) << "Cannot setup OpenPGP context!";
        KMessageBox::errorWId(w,
                              i18n("<qt>Error when attempting to initialize OpenPGP while attempting to save the wallet <b>%1</b>. Please fix your system "
                                   "configuration, then try again.</qt>"),
                              wb->_name.toHtmlEscaped());
        return -6;
    }

    assert(wb->_cipherType == BACKEND_CIPHER_GPG);

    QByteArray hashes;
    QDataStream hashStream(&hashes, QIODevice::WriteOnly);
    QCryptographicHash md5(QCryptographicHash::Md5);
    hashStream << static_cast<quint32>(wb->_entries.count());

    QByteArray values;
    QDataStream valueStream(&values, QIODevice::WriteOnly);
    Backend::FolderMap::ConstIterator i = wb->_entries.constBegin();
    Backend::FolderMap::ConstIterator ie = wb->_entries.constEnd();
    for (; i != ie; ++i) {
        valueStream << i.key();
        valueStream << static_cast<quint32>(i.value().count());

        md5.reset();
        md5.addData(i.key().toUtf8());
        hashStream.writeRawData(md5.result().constData(), 16);
        hashStream << static_cast<quint32>(i.value().count());

        Backend::EntryMap::ConstIterator j = i.value().constBegin();
        Backend::EntryMap::ConstIterator je = i.value().constEnd();
        for (; j != je; ++j) {
            valueStream << j.key();
            valueStream << static_cast<qint32>(j.value()->type());
            valueStream << j.value()->value();

            md5.reset();
            md5.addData(j.key().toUtf8());
            hashStream.writeRawData(md5.result().constData(), 16);
        }
    }

    QByteArray dataBuffer;
    QDataStream dataStream(&dataBuffer, QIODevice::WriteOnly);
    QString keyID(wb->_gpgKey.keyID());
    dataStream << keyID;
    dataStream << hashes;
    dataStream << values;

    GpgME::Data decryptedData(dataBuffer.data(), size_t(dataBuffer.size()), false);
    GpgME::Data encryptedData;
    std::vector<GpgME::Key> keys;
    keys.push_back(wb->_gpgKey);
    const GpgME::EncryptionResult res = ctx->encrypt(keys, decryptedData, encryptedData, GpgME::Context::None);
    if (res.error()) {
        const int gpgerr = res.error().code();
        KMessageBox::errorWId(w,
                              i18n("<qt>Encryption error while attempting to save the wallet <b>%1</b>. Error code is <b>%2 (%3)</b>. Please fix your system "
                                   "configuration, then try again. This error may occur if you are not using a full trust GPG key. Please ensure you have the "
                                   "secret key for the key you are using.</qt>",
                                   wb->_name.toHtmlEscaped(),
                                   gpgerr,
                                   res.error().asString()));
        qCDebug(KWALLETBACKEND_LOG) << "GpgME encryption error: " << gpgerr;
        sf.cancelWriting();
        return -7;
    }

    char buffer[4096];
    ssize_t bytes = 0;
    encryptedData.seek(0, SEEK_SET);
    while ((bytes = encryptedData.read(buffer, sizeof(buffer) / sizeof(buffer[0]))) > 0) {
        if (sf.write(buffer, bytes) != bytes) {
            KMessageBox::errorWId(w,
                                  i18n("<qt>File handling error while attempting to save the wallet <b>%1</b>. Error was <b>%2</b>. Please fix your system "
                                       "configuration, then try again.</qt>",
                                       wb->_name.toHtmlEscaped(),
                                       sf.errorString()));
            sf.cancelWriting();
            return -4; // write error
        }
    }

    if (!sf.commit()) {
        qCDebug(KWALLETBACKEND_LOG) << "WARNING: wallet sync to disk failed! QSaveFile status was " << sf.errorString();
        return -4; // write error
    }

    return 0;
}

int GpgPersistHandler::read(Backend *wb, QFile &sf, WId w)
{
    GpgME::Error err = initGpgME();
    if (err) {
        KMessageBox::errorWId(w,
                              i18n("<qt>Error when attempting to initialize OpenPGP while attempting to open the wallet <b>%1</b>. Error code is <b>%2</b>. "
                                   "Please fix your system configuration, then try again.</qt>",
                                   wb->_name.toHtmlEscaped(),
                                   err.code()));
        return -1;
    }

    wb->_cipherType = BACKEND_CIPHER_GPG;
    wb->_hashes.clear();

    // the remainder of the file is GPG encrypted. Let's decrypt it
    GpgME::Data encryptedData;
    char buffer[4096];
    ssize_t bytes = 0;
    while ((bytes = sf.read(buffer, sizeof(buffer) / sizeof(buffer[0])))) {
        encryptedData.write(buffer, bytes);
    }

retry_label:
    std::shared_ptr<GpgME::Context> ctx(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (nullptr == ctx) {
        KMessageBox::errorWId(w,
                              i18n("<qt>Error when attempting to initialize OpenPGP while attempting to open the wallet <b>%1</b>. Please fix your system "
                                   "configuration, then try again.</qt>",
                                   wb->_name.toHtmlEscaped()));
        qCDebug(KWALLETBACKEND_LOG) << "Cannot setup OpenPGP context!";
        return -1;
    }

    GpgME::Data decryptedData;
    encryptedData.seek(0, SEEK_SET);
    GpgME::DecryptionResult res = ctx->decrypt(encryptedData, decryptedData);
    if (res.error()) {
        qCDebug(KWALLETBACKEND_LOG) << "Error decrypting message: " << res.error().asString() << ", code " << res.error().code() << ", source "
                                    << res.error().source();
        KGuiItem btnRetry(i18n("Retry"));
        // FIXME the logic here should be a little more elaborate; a dialog box should be used with "retry", "cancel", but also "troubleshoot" with options to
        // show card status and to kill scdaemon
        int userChoice =
            KMessageBox::warningTwoActionsWId(w,
                                              i18n("<qt>Error when attempting to decrypt the wallet <b>%1</b> using GPG. If you're using a SmartCard, "
                                                   "please ensure it's inserted then try again.<br><br>GPG error was <b>%2</b></qt>",
                                                   wb->_name.toHtmlEscaped(),
                                                   res.error().asString()),
                                              i18n("kwalletd GPG backend"),
                                              btnRetry,
                                              KStandardGuiItem::cancel());
        if (userChoice == KMessageBox::PrimaryAction) {
            decryptedData.seek(0, SEEK_SET);
            goto retry_label;
        }
        return -1;
    }

    decryptedData.seek(0, SEEK_SET);
    QByteArray dataBuffer;
    while ((bytes = decryptedData.read(buffer, sizeof(buffer) / sizeof(buffer[0])))) {
        dataBuffer.append(buffer, bytes);
    }

    // load the wallet from the decrypted data
    QDataStream dataStream(dataBuffer);
    QString keyID;
    QByteArray hashes;
    QByteArray values;
    dataStream >> keyID;
    dataStream >> hashes;
    dataStream >> values;

    // locate the GPG key having the ID found inside the file. This will be needed later, when writing changes to disk.
    QDataStream fileStream(&sf);
    fileStream.setDevice(nullptr);
    qCDebug(KWALLETBACKEND_LOG) << "This wallet was encrypted using GPG key with ID " << keyID;

    ctx->setKeyListMode(GpgME::KeyListMode::Local);
    err = ctx->startKeyListing();
    while (!err) {
        GpgME::Key k = ctx->nextKey(err);
        if (err) {
            break;
        }
        if (keyID == k.keyID()) {
            qCDebug(KWALLETBACKEND_LOG) << "The key was found.";
            wb->_gpgKey = k;
            break;
        }
    }
    ctx->endKeyListing();
    if (wb->_gpgKey.isNull()) {
        KMessageBox::errorWId(w,
                              i18n("<qt>Error when attempting to open the wallet <b>%1</b>. The wallet was encrypted using the GPG Key ID <b>%2</b> but this "
                                   "key was not found on your system.</qt>",
                                   wb->_name.toHtmlEscaped(),
                                   keyID));
        return -1;
    }

    QDataStream hashStream(hashes);
    QDataStream valueStream(values);

    quint32 hashCount;
    hashStream >> hashCount;
    if (hashCount > 0xFFFF) {
        return -43;
    }

    quint32 folderCount = hashCount;
    while (hashCount--) {
        Digest d;
        hashStream.readRawData(d, 16);

        quint32 folderSize;
        hashStream >> folderSize;

        MD5Digest ba = MD5Digest(reinterpret_cast<char *>(d));
        QMap<MD5Digest, QList<MD5Digest>>::iterator it = wb->_hashes.insert(ba, QList<MD5Digest>());
        while (folderSize--) {
            Digest d2;
            hashStream.readRawData(d2, 16);
            ba = MD5Digest(d2);
            (*it).append(ba);
        }
    }

    while (folderCount--) {
        QString folder;
        valueStream >> folder;

        quint32 entryCount;
        valueStream >> entryCount;

        wb->_entries[folder].clear();

        while (entryCount--) {
            KWallet::Wallet::EntryType et = KWallet::Wallet::Unknown;
            Entry *e = new Entry;

            QString key;
            valueStream >> key;

            qint32 x = 0; // necessary to read properly
            valueStream >> x;
            et = static_cast<KWallet::Wallet::EntryType>(x);

            switch (et) {
            case KWallet::Wallet::Password:
            case KWallet::Wallet::Stream:
            case KWallet::Wallet::Map:
                break;
            default: // Unknown entry
                delete e;
                continue;
            }

            QByteArray a;
            valueStream >> a;
            e->setValue(a);
            e->setType(et);
            e->setKey(key);
            wb->_entries[folder][key] = e;
        }
    }

    wb->_open = true;

    return 0;
}
#endif // HAVE_GPGMEPP

} // namespace
