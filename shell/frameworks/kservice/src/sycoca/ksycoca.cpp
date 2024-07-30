/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999-2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2005-2009 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2020-2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksycoca.h"
#include "ksycoca_p.h"
#include "ksycocafactory_p.h"
#include "ksycocatype.h"
#include "ksycocautils_p.h"
#include "sycocadebug.h"
#include <KConfigGroup>
#include <KSandbox>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QMetaMethod>
#include <QStandardPaths>
#include <QThread>
#include <QThreadStorage>

#include <QCryptographicHash>
#include <fcntl.h>
#include <kmimetypefactory_p.h>
#include <kservicefactory_p.h>
#include <kservicegroupfactory_p.h>

#include "kbuildsycoca_p.h"
#include "ksycocadevices_p.h"

#ifdef Q_OS_UNIX
#include <sys/time.h>
#include <utime.h>
#endif

/**
 * Sycoca file version number.
 * If the existing file is outdated, it will not get read
 * but instead we'll regenerate a new one.
 * However running apps should still be able to read it, so
 * only add to the data, never remove/modify.
 */
#define KSYCOCA_VERSION 306

#if HAVE_MADVISE || HAVE_MMAP
#include <sys/mman.h> // This #include was checked when looking for posix_madvise
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

QDataStream &operator>>(QDataStream &in, KSycocaHeader &h)
{
    in >> h.prefixes >> h.timeStamp >> h.language >> h.updateSignature;
    return in;
}

// The following limitations are in place:
// Maximum length of a single string: 8192 bytes
// Maximum length of a string list: 1024 strings
// Maximum number of entries: 8192
//
// The purpose of these limitations is to limit the impact
// of database corruption.

Q_DECLARE_OPERATORS_FOR_FLAGS(KSycocaPrivate::BehaviorsIfNotFound)

KSycocaPrivate::KSycocaPrivate(KSycoca *qq)
    : databaseStatus(DatabaseNotOpen)
    , readError(false)
    , timeStamp(0)
    , m_databasePath()
    , updateSig(0)
    , m_fileWatcher(new KDirWatch)
    , m_haveListeners(false)
    , q(qq)
    , sycoca_size(0)
    , sycoca_mmap(nullptr)
    , m_mmapFile(nullptr)
    , m_device(nullptr)
    , m_mimeTypeFactory(nullptr)
    , m_serviceFactory(nullptr)
    , m_serviceGroupFactory(nullptr)
{
#ifdef Q_OS_WIN
    /*
      on windows we use KMemFile (QSharedMemory) to avoid problems
      with mmap (can't delete a mmap'd file)
    */
    m_sycocaStrategy = StrategyMemFile;
#else
    m_sycocaStrategy = StrategyMmap;
#endif
    KConfigGroup config(KSharedConfig::openConfig(), QStringLiteral("KSycoca"));
    setStrategyFromString(config.readEntry("strategy"));
}

void KSycocaPrivate::setStrategyFromString(const QString &strategy)
{
    if (strategy == QLatin1String("mmap")) {
        m_sycocaStrategy = StrategyMmap;
    } else if (strategy == QLatin1String("file")) {
        m_sycocaStrategy = StrategyFile;
    } else if (strategy == QLatin1String("sharedmem")) {
        m_sycocaStrategy = StrategyMemFile;
    } else if (!strategy.isEmpty()) {
        qCWarning(SYCOCA) << "Unknown sycoca strategy:" << strategy;
    }
}

bool KSycocaPrivate::tryMmap()
{
#if HAVE_MMAP
    Q_ASSERT(!m_databasePath.isEmpty());
    m_mmapFile = new QFile(m_databasePath);
    const bool canRead = m_mmapFile->open(QIODevice::ReadOnly);
    Q_ASSERT(canRead);
    if (!canRead) {
        return false;
    }
    fcntl(m_mmapFile->handle(), F_SETFD, FD_CLOEXEC);
    sycoca_size = m_mmapFile->size();
    void *mmapRet = mmap(nullptr, sycoca_size, PROT_READ, MAP_SHARED, m_mmapFile->handle(), 0);
    /* POSIX mandates only MAP_FAILED, but we are paranoid so check for
       null pointer too.  */
    if (mmapRet == MAP_FAILED || mmapRet == nullptr) {
        qCDebug(SYCOCA).nospace() << "mmap failed. (length = " << sycoca_size << ")";
        sycoca_mmap = nullptr;
        return false;
    } else {
        sycoca_mmap = static_cast<const char *>(mmapRet);
#if HAVE_MADVISE
        (void)posix_madvise(mmapRet, sycoca_size, POSIX_MADV_WILLNEED);
#endif // HAVE_MADVISE
        return true;
    }
#else
    return false;
#endif // HAVE_MMAP
}

int KSycoca::version()
{
    return KSYCOCA_VERSION;
}

class KSycocaSingleton
{
public:
    KSycocaSingleton()
    {
    }
    ~KSycocaSingleton()
    {
    }

    bool hasSycoca() const
    {
        return m_threadSycocas.hasLocalData();
    }
    KSycoca *sycoca()
    {
        if (!m_threadSycocas.hasLocalData()) {
            m_threadSycocas.setLocalData(new KSycoca);
        }
        return m_threadSycocas.localData();
    }
    void setSycoca(KSycoca *s)
    {
        m_threadSycocas.setLocalData(s);
    }

private:
    QThreadStorage<KSycoca *> m_threadSycocas;
};

Q_GLOBAL_STATIC(KSycocaSingleton, ksycocaInstance)

QString KSycocaPrivate::findDatabase()
{
    Q_ASSERT(databaseStatus == DatabaseNotOpen);

    const QString path = KSycoca::absoluteFilePath();
    const QFileInfo info(path);
    if (info.isReadable()) {
        if (m_haveListeners && m_fileWatcher) {
            m_fileWatcher->addFile(path);
        }
        return path;
    }
    // Let's be notified when it gets created - by another process or by ourselves
    if (m_fileWatcher) {
        m_fileWatcher->addFile(path);
    }
    return QString();
}

// Read-only constructor
// One instance per thread
KSycoca::KSycoca()
    : d(new KSycocaPrivate(this))
{
    if (d->m_fileWatcher) {
        // We always delete and recreate the DB, so KDirWatch normally emits created
        connect(d->m_fileWatcher.get(), &KDirWatch::created, this, [this]() {
            d->slotDatabaseChanged();
        });
        // In some cases, KDirWatch only thinks the file was modified though
        connect(d->m_fileWatcher.get(), &KDirWatch::dirty, this, [this]() {
            d->slotDatabaseChanged();
        });
    }
}

bool KSycocaPrivate::openDatabase()
{
    Q_ASSERT(databaseStatus == DatabaseNotOpen);

    delete m_device;
    m_device = nullptr;

    if (m_databasePath.isEmpty()) {
        m_databasePath = findDatabase();
    }

    bool result = true;
    if (!m_databasePath.isEmpty()) {
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            if (KSandbox::isFlatpak()) {
                // We're running inside flatpak, which sets all times to 1970
                // So the first very time, don't use an existing database, recreate it
                qCDebug(SYCOCA) << "flatpak detected, ignoring" << m_databasePath;
                return false;
            }
        }

        qCDebug(SYCOCA) << "Opening ksycoca from" << m_databasePath;
        m_dbLastModified = QFileInfo(m_databasePath).lastModified();
        result = checkVersion();
    } else { // No database file
        // qCDebug(SYCOCA) << "Could not open ksycoca";
        result = false;
    }
    return result;
}

KSycocaAbstractDevice *KSycocaPrivate::device()
{
    if (m_device) {
        return m_device;
    }

    KSycocaAbstractDevice *device = m_device;
    Q_ASSERT(!m_databasePath.isEmpty());
#if HAVE_MMAP
    if (m_sycocaStrategy == StrategyMmap && tryMmap()) {
        device = new KSycocaMmapDevice(sycoca_mmap, sycoca_size);
        if (!device->device()->open(QIODevice::ReadOnly)) {
            delete device;
            device = nullptr;
        }
    }
#endif
#ifndef QT_NO_SHAREDMEMORY
    if (!device && m_sycocaStrategy == StrategyMemFile) {
        device = new KSycocaMemFileDevice(m_databasePath);
        if (!device->device()->open(QIODevice::ReadOnly)) {
            delete device;
            device = nullptr;
        }
    }
#endif
    if (!device) {
        device = new KSycocaFileDevice(m_databasePath);
        if (!device->device()->open(QIODevice::ReadOnly)) {
            qCWarning(SYCOCA) << "Couldn't open" << m_databasePath << "even though it is readable? Impossible.";
            // delete device; device = 0; // this would crash in the return statement...
        }
    }
    if (device) {
        m_device = device;
    }
    return m_device;
}

QDataStream *&KSycocaPrivate::stream()
{
    if (!m_device) {
        if (databaseStatus == DatabaseNotOpen) {
            checkDatabase(KSycocaPrivate::IfNotFoundRecreate);
        }

        device(); // create m_device
    }

    return m_device->stream();
}

void KSycocaPrivate::slotDatabaseChanged()
{
    qCDebug(SYCOCA) << QThread::currentThread() << "got a notifyDatabaseChanged signal";
    // In case we have changed the database outselves, we have already notified the application
    if (!m_dbLastModified.isValid() || m_dbLastModified != QFileInfo(m_databasePath).lastModified()) {
        // KDirWatch tells us the database file changed
        // We would have found out in the next call to ensureCacheValid(), but for
        // now keep the call to closeDatabase, to help refcounting to 0 the old mmapped file earlier.
        closeDatabase();
        // Start monitoring the new file right away
        m_databasePath = findDatabase();

        // Now notify applications
        Q_EMIT q->databaseChanged();
    }
}

KMimeTypeFactory *KSycocaPrivate::mimeTypeFactory()
{
    if (!m_mimeTypeFactory) {
        m_mimeTypeFactory = new KMimeTypeFactory(q);
    }
    return m_mimeTypeFactory;
}

KServiceFactory *KSycocaPrivate::serviceFactory()
{
    if (!m_serviceFactory) {
        m_serviceFactory = new KServiceFactory(q);
    }
    return m_serviceFactory;
}

KServiceGroupFactory *KSycocaPrivate::serviceGroupFactory()
{
    if (!m_serviceGroupFactory) {
        m_serviceGroupFactory = new KServiceGroupFactory(q);
    }
    return m_serviceGroupFactory;
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca(bool /* dummy */)
    : d(new KSycocaPrivate(this))
{
}

KSycoca *KSycoca::self()
{
    KSycoca *s = ksycocaInstance()->sycoca();
    Q_ASSERT(s);
    return s;
}

KSycoca::~KSycoca()
{
    d->closeDatabase();
    delete d;
    // if (ksycocaInstance.exists()
    //    && ksycocaInstance->self == this)
    //    ksycocaInstance->self = 0;
}

bool KSycoca::isAvailable() // TODO KF6: make it non-static (mostly useful for unittests)
{
    return self()->d->checkDatabase(KSycocaPrivate::IfNotFoundDoNothing);
}

void KSycocaPrivate::closeDatabase()
{
    delete m_device;
    m_device = nullptr;

    // It is very important to delete all factories here
    // since they cache information about the database file
    // But other threads might be using them, so this class is
    // refcounted, and deleted when the last thread is done with them
    qDeleteAll(m_factories);
    m_factories.clear();

    m_mimeTypeFactory = nullptr;
    m_serviceFactory = nullptr;
    m_serviceGroupFactory = nullptr;

#if HAVE_MMAP
    if (sycoca_mmap) {
        // Solaris has munmap(char*, size_t) and everything else should
        // be happy with a char* for munmap(void*, size_t)
        munmap(const_cast<char *>(sycoca_mmap), sycoca_size);
        sycoca_mmap = nullptr;
    }
    delete m_mmapFile;
    m_mmapFile = nullptr;
#endif

    databaseStatus = DatabaseNotOpen;
    m_databasePath.clear();
    timeStamp = 0;
}

void KSycoca::addFactory(KSycocaFactory *factory)
{
    d->addFactory(factory);
}

QDataStream *KSycoca::findEntry(int offset, KSycocaType &type)
{
    QDataStream *str = stream();
    Q_ASSERT(str);
    // qCDebug(SYCOCA) << QString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16);
    str->device()->seek(offset);
    qint32 aType;
    *str >> aType;
    type = KSycocaType(aType);
    // qCDebug(SYCOCA) << QString("KSycoca::found type %1").arg(aType);
    return str;
}

KSycocaFactoryList *KSycoca::factories()
{
    return d->factories();
}

// Warning, checkVersion rewinds to the beginning of stream().
bool KSycocaPrivate::checkVersion()
{
    QDataStream *m_str = device()->stream();
    Q_ASSERT(m_str);
    m_str->device()->seek(0);
    qint32 aVersion;
    *m_str >> aVersion;
    if (aVersion < KSYCOCA_VERSION) {
        qCDebug(SYCOCA) << "Found version" << aVersion << ", expecting version" << KSYCOCA_VERSION << "or higher.";
        databaseStatus = BadVersion;
        return false;
    } else {
        databaseStatus = DatabaseOK;
        return true;
    }
}

// If it returns true, we have a valid database and the stream has rewinded to the beginning
// and past the version number.
bool KSycocaPrivate::checkDatabase(BehaviorsIfNotFound ifNotFound)
{
    if (databaseStatus == DatabaseOK) {
        if (checkVersion()) { // we know the version is ok, but we must rewind the stream anyway
            return true;
        }
    }

    closeDatabase(); // close the dummy one

    // Check if new database already available
    if (openDatabase()) {
        // Database exists, and version is ok, we can read it.

        if (qAppName() != QLatin1String(KBUILDSYCOCA_EXENAME) && ifNotFound != IfNotFoundDoNothing) {
            // Ensure it's up-to-date, rebuild if needed
            checkDirectories();

            // Don't check again for some time
            m_lastCheck.start();
        }

        return true;
    }

    if (ifNotFound & IfNotFoundRecreate) {
        return buildSycoca();
    }

    return false;
}

QDataStream *KSycoca::findFactory(KSycocaFactoryId id)
{
    // Ensure we have a valid database (right version, and rewinded to beginning)
    if (!d->checkDatabase(KSycocaPrivate::IfNotFoundRecreate)) {
        return nullptr;
    }

    QDataStream *str = stream();
    Q_ASSERT(str);

    qint32 aId;
    qint32 aOffset;
    while (true) {
        *str >> aId;
        if (aId == 0) {
            qCWarning(SYCOCA) << "Error, KSycocaFactory (id =" << int(id) << ") not found!";
            break;
        }
        *str >> aOffset;
        if (aId == id) {
            // qCDebug(SYCOCA) << "KSycoca::findFactory(" << id << ") offset " << aOffset;
            str->device()->seek(aOffset);
            return str;
        }
    }
    return nullptr;
}

bool KSycoca::needsRebuild()
{
    return d->needsRebuild();
}

KSycocaHeader KSycocaPrivate::readSycocaHeader()
{
    KSycocaHeader header;
    // do not try to launch kbuildsycoca from here; this code is also called by kbuildsycoca.
    if (!checkDatabase(KSycocaPrivate::IfNotFoundDoNothing)) {
        return header;
    }
    QDataStream *str = stream();
    qint64 oldPos = str->device()->pos();

    Q_ASSERT(str);
    qint32 aId;
    qint32 aOffset;
    // skip factories offsets
    while (true) {
        *str >> aId;
        if (aId) {
            *str >> aOffset;
        } else {
            break; // just read 0
        }
    }
    // We now point to the header
    QStringList directoryList;
    *str >> header >> directoryList;
    allResourceDirs.clear();
    for (int i = 0; i < directoryList.count(); ++i) {
        qint64 mtime;
        *str >> mtime;
        allResourceDirs.insert(directoryList.at(i), mtime);
    }

    QStringList fileList;
    *str >> fileList;
    extraFiles.clear();
    for (const auto &fileName : std::as_const(fileList)) {
        qint64 mtime;
        *str >> mtime;
        extraFiles.insert(fileName, mtime);
    }

    str->device()->seek(oldPos);

    timeStamp = header.timeStamp;

    // for the useless public accessors. KF6: remove these two lines, the accessors and the vars.
    language = header.language;
    updateSig = header.updateSignature;

    return header;
}

class TimestampChecker
{
public:
    TimestampChecker()
        : m_now(QDateTime::currentDateTime())
    {
    }

    // Check times of last modification of all directories on which ksycoca depends,
    // If none of them is newer than the mtime we stored for that directory at the
    // last rebuild, this means that there's no need to rebuild ksycoca.
    bool checkDirectoriesTimestamps(const QMap<QString, qint64> &dirs) const
    {
        Q_ASSERT(!dirs.isEmpty());
        // qCDebug(SYCOCA) << "checking file timestamps";
        for (auto it = dirs.begin(); it != dirs.end(); ++it) {
            const QString dir = it.key();
            const qint64 lastStamp = it.value();

            auto visitor = [&](const QFileInfo &fi) {
                const QDateTime mtime = fi.lastModified();
                if (mtime.toMSecsSinceEpoch() > lastStamp) {
                    if (mtime > m_now) {
                        qCDebug(SYCOCA) << fi.filePath() << "has a modification time in the future" << mtime;
                    }
                    qCDebug(SYCOCA) << "dir timestamp changed:" << fi.filePath() << mtime << ">" << QDateTime::fromMSecsSinceEpoch(lastStamp);
                    // no need to continue search
                    return false;
                }

                return true;
            };

            if (!KSycocaUtilsPrivate::visitResourceDirectory(dir, visitor)) {
                return false;
            }
        }
        return true;
    }

    bool checkFilesTimestamps(const QMap<QString, qint64> &files) const
    {
        for (auto it = files.begin(); it != files.end(); ++it) {
            const QString fileName = it.key();
            const qint64 lastStamp = it.value();

            QFileInfo fi(fileName);
            if (!fi.exists()) {
                return false;
            }
            const QDateTime mtime = fi.lastModified();
            if (mtime.toMSecsSinceEpoch() > lastStamp) {
                if (mtime > m_now) {
                    qCDebug(SYCOCA) << fi.filePath() << "has a modification time in the future" << mtime;
                }
                qCDebug(SYCOCA) << "file timestamp changed:" << fi.filePath() << mtime << ">" << QDateTime::fromMSecsSinceEpoch(lastStamp);
                return false;
            }
        }
        return true;
    }

private:
    QDateTime m_now;
};

void KSycocaPrivate::checkDirectories()
{
    if (needsRebuild()) {
        buildSycoca();
    }
}

bool KSycocaPrivate::needsRebuild()
{
    // In case it is not open, it might be due to another process/thread having rebuild it. Thus we read the header for both the not open and ok state
    if (!timeStamp && databaseStatus != BadVersion) {
        (void)readSycocaHeader();
    }
    // these days timeStamp is really a "bool headerFound", the value itself doesn't matter...
    // KF6: replace it with bool.
    const auto timestampChecker = TimestampChecker();
    bool ret = timeStamp != 0
        && (!timestampChecker.checkDirectoriesTimestamps(allResourceDirs) //
            || !timestampChecker.checkFilesTimestamps(extraFiles));
    if (ret) {
        return true;
    }
    auto files = KBuildSycoca::factoryExtraFiles();
    // ensure files are ordered so next comparison works
    files.sort();
    // to cover cases when extra files were added
    return extraFiles.keys() != files;
}

bool KSycocaPrivate::buildSycoca()
{
    KBuildSycoca builder;
    if (!builder.recreate()) {
        return false; // error
    }

    closeDatabase(); // close the dummy one

    // Ok, the new database should be here now, open it.
    if (!openDatabase()) {
        qCDebug(SYCOCA) << "Still no database...";
        return false;
    }
    Q_EMIT q->databaseChanged();
    return true;
}

QString KSycoca::absoluteFilePath()
{
    const QStringList paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    QString suffix = QLatin1Char('_') + QLocale().bcp47Name();

    const QByteArray ksycoca_env = qgetenv("KDESYCOCA");
    if (ksycoca_env.isEmpty()) {
        const QByteArray pathHash = QCryptographicHash::hash(paths.join(QLatin1Char(':')).toUtf8(), QCryptographicHash::Sha1);
        suffix += QLatin1Char('_') + QString::fromLatin1(pathHash.toBase64());
        suffix.replace(QLatin1Char('/'), QLatin1Char('_'));
#ifdef Q_OS_WIN
        suffix.replace(QLatin1Char(':'), QLatin1Char('_'));
#endif
        const QString fileName = QLatin1String("ksycoca6") + suffix;
        return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1Char('/') + fileName;
    } else {
        return QFile::decodeName(ksycoca_env);
    }
}

QStringList KSycoca::allResourceDirs()
{
    if (!d->timeStamp) {
        (void)d->readSycocaHeader();
    }
    return d->allResourceDirs.keys();
}

void KSycoca::flagError()
{
    qCWarning(SYCOCA) << "ERROR: KSycoca database corruption!";
    KSycoca *sycoca = self();
    if (sycoca->d->readError) {
        return;
    }
    sycoca->d->readError = true;
    if (qAppName() != QLatin1String(KBUILDSYCOCA_EXENAME) && !sycoca->isBuilding()) {
        // Rebuild the damned thing.
        KBuildSycoca builder;
        (void)builder.recreate();
    }
}

bool KSycoca::isBuilding()
{
    return false;
}

void KSycoca::disableAutoRebuild()
{
    ksycocaInstance->sycoca()->d->m_fileWatcher = nullptr;
}

QDataStream *&KSycoca::stream()
{
    return d->stream();
}

void KSycoca::connectNotify(const QMetaMethod &signal)
{
    if (signal.name() == "databaseChanged" && !d->m_haveListeners) {
        d->m_haveListeners = true;
        if (d->m_databasePath.isEmpty()) {
            d->m_databasePath = d->findDatabase();
        } else if (d->m_fileWatcher) {
            d->m_fileWatcher->addFile(d->m_databasePath);
        }
    }
}

void KSycoca::clearCaches()
{
    if (ksycocaInstance.exists() && ksycocaInstance()->hasSycoca()) {
        ksycocaInstance()->sycoca()->d->closeDatabase();
    }
}

extern KSERVICE_EXPORT int ksycoca_ms_between_checks;
KSERVICE_EXPORT int ksycoca_ms_between_checks = 1500;

void KSycoca::ensureCacheValid()
{
    if (qAppName() == QLatin1String(KBUILDSYCOCA_EXENAME)) {
        return;
    }

    if (d->databaseStatus != KSycocaPrivate::DatabaseOK) {
        if (!d->checkDatabase(KSycocaPrivate::IfNotFoundRecreate)) {
            return;
        }
    }

    if (d->m_lastCheck.isValid() && d->m_lastCheck.elapsed() < ksycoca_ms_between_checks) {
        return;
    }
    d->m_lastCheck.start();

    // Check if the file on disk was modified since we last checked it.
    QFileInfo info(d->m_databasePath);
    if (info.lastModified() == d->m_dbLastModified) {
        // Check if the watched directories were modified, then the cache needs a rebuild.
        d->checkDirectories();
        return;
    }

    // Close the database and forget all about what we knew.
    // The next call to any public method will recreate
    // everything that's needed.
    d->closeDatabase();
}

void KSycoca::setupTestMenu()
{
    const QByteArray content = R"(<?xml version="1.0"?>
<!DOCTYPE Menu PUBLIC "-//freedesktop//DTD Menu 1.0//EN" "http://www.freedesktop.org/standards/menu-spec/menu-1.0.dtd">
<Menu>
  <Name>Applications</Name>
  <Directory>Applications.directory</Directory>
  <DefaultAppDirs/>
  <DefaultDirectoryDirs/>
  <MergeDir>applications-merged</MergeDir>
  <LegacyDir>/usr/share/applnk</LegacyDir>
  <DefaultLayout>
    <Merge type="menus"/>
    <Merge type="files"/>
    <Separator/>
    <Menuname>More</Menuname>
  </DefaultLayout>
</Menu>
)";

    const QString destDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/menus");
    QDir(destDir).mkpath(QStringLiteral("."));
    QFile output(destDir + QLatin1String("/applications.menu"));
    output.open(QIODevice::ReadWrite | QIODevice::Truncate);
    output.write(content);
}

#include "moc_ksycoca.cpp"
