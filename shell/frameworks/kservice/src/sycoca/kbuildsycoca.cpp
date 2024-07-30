/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2002-2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbuildsycoca_p.h"
#include "ksycoca_p.h"
#include "ksycocaresourcelist_p.h"
#include "ksycocautils_p.h"
#include "sycocadebug.h"
#include "vfolder_menu_p.h"

#include "kbuildmimetypefactory_p.h"
#include "kbuildservicefactory_p.h"
#include "kbuildservicegroupfactory_p.h"
#include "kctimefactory_p.h"
#include <QDataStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QLocale>
#include <QSaveFile>
#include <QTimer>
#include <config-ksycoca.h>
#include <kservice.h>
#include <kservicegroup.h>

#include <kmemfile_p.h>

#include <QLockFile>
#include <QStandardPaths>
#include <qplatformdefs.h>

static const char *s_cSycocaPath = nullptr;

KBuildSycocaInterface::~KBuildSycocaInterface()
{
}

KBuildSycoca::KBuildSycoca()
    : KSycoca(true)
    , m_allEntries(nullptr)
    , m_ctimeFactory(nullptr)
    , m_ctimeDict(nullptr)
    , m_currentEntryDict(nullptr)
    , m_serviceGroupEntryDict(nullptr)
    , m_vfolder(nullptr)
    , m_newTimestamp(0)
    , m_menuTest(false)
    , m_changed(false)
{
}

KBuildSycoca::~KBuildSycoca()
{
    // Delete the factories while we exist, so that the virtual isBuilding() still works
    qDeleteAll(*factories());
    factories()->clear();
}

KSycocaEntry::Ptr KBuildSycoca::createEntry(KSycocaFactory *currentFactory, const QString &file)
{
    quint32 timeStamp = m_ctimeFactory->dict()->ctime(file, m_resource);
    if (!timeStamp) {
        timeStamp = calcResourceHash(m_resourceSubdir, file);
        if (!timeStamp) { // file disappeared meanwhile
            return {};
        }
    }
    KSycocaEntry::Ptr entry;
    if (m_allEntries) {
        Q_ASSERT(m_ctimeDict);
        quint32 oldTimestamp = m_ctimeDict->ctime(file, m_resource);
        if (file.contains(QLatin1String("fake"))) {
            qCDebug(SYCOCA) << "m_ctimeDict->ctime(" << file << ") = " << oldTimestamp << "compared with" << timeStamp;
        }

        if (timeStamp && (timeStamp == oldTimestamp)) {
            // Re-use old entry
            if (currentFactory == d->m_serviceFactory) { // Strip .directory from service-group entries
                entry = m_currentEntryDict->value(file.left(file.length() - 10));
            } else {
                entry = m_currentEntryDict->value(file);
            }
            // remove from m_ctimeDict; if m_ctimeDict is not empty
            // after all files have been processed, it means
            // some files were removed since last time
            if (file.contains(QLatin1String("fake"))) {
                qCDebug(SYCOCA) << "reusing (and removing) old entry for:" << file << "entry=" << entry;
            }
            m_ctimeDict->remove(file, m_resource);
        } else if (oldTimestamp) {
            m_changed = true;
            m_ctimeDict->remove(file, m_resource);
            qCDebug(SYCOCA) << "modified:" << file;
        } else {
            m_changed = true;
            qCDebug(SYCOCA) << "new:" << file;
        }
    }
    m_ctimeFactory->dict()->addCTime(file, m_resource, timeStamp);
    if (!entry) {
        // Create a new entry
        entry = currentFactory->createEntry(file);
    }
    if (entry && entry->isValid()) {
        return entry;
    }
    return KSycocaEntry::Ptr();
}

KService::Ptr KBuildSycoca::createService(const QString &path)
{
    KSycocaEntry::Ptr entry = createEntry(d->m_serviceFactory, path);
    if (entry) {
        m_tempStorage.append(entry);
    }
    return KService::Ptr(static_cast<KService *>(entry.data()));
}

// returns false if the database is up to date, true if it needs to be saved
bool KBuildSycoca::build()
{
    using KBSEntryDictList = QList<KBSEntryDict *>;
    KBSEntryDictList entryDictList;
    KBSEntryDict *serviceEntryDict = nullptr;

    // Convert for each factory the entryList to a Dict.
    entryDictList.reserve(factories()->size());
    int i = 0;
    // For each factory
    const auto &factoryList = *factories();
    for (KSycocaFactory *factory : factoryList) {
        KBSEntryDict *entryDict = new KBSEntryDict;
        if (m_allEntries) { // incremental build
            for (const KSycocaEntry::Ptr &entry : std::as_const((*m_allEntries).at(i++))) {
                // if (entry->entryPath().contains("fake"))
                //    qCDebug(SYCOCA) << "inserting into entryDict:" << entry->entryPath() << entry;
                entryDict->insert(entry->entryPath(), entry);
            }
        }
        if (factory == d->m_serviceFactory) {
            serviceEntryDict = entryDict;
        } else if (factory == m_buildServiceGroupFactory) {
            m_serviceGroupEntryDict = entryDict;
        }
        entryDictList.append(entryDict);
    }

    // Save the mtime of each dir, just before we list them
    // ## should we convert to UTC to avoid surprises when summer time kicks in?
    const auto lstDirs = factoryResourceDirs();
    for (const QString &dir : lstDirs) {
        qint64 stamp = 0;
        KSycocaUtilsPrivate::visitResourceDirectory(dir, [&stamp](const QFileInfo &info) {
            stamp = qMax(stamp, info.lastModified().toMSecsSinceEpoch());
            return true;
        });
        m_allResourceDirs.insert(dir, stamp);
    }

    const auto lstFiles = factoryExtraFiles();
    for (const QString &file : lstFiles) {
        m_extraFiles.insert(file, QFileInfo(file).lastModified().toMSecsSinceEpoch());
    }

    QMap<QString, QByteArray> allResourcesSubDirs; // dirs, kstandarddirs-resource-name
    // For each factory
    for (KSycocaFactory *factory : factoryList) {
        // For each resource the factory deals with
        const KSycocaResourceList resourceList = factory->resourceList();
        for (const KSycocaResource &res : resourceList) {
            // With this we would get dirs, but not a unique list of relative files (for global+local merging to work)
            // const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, res.subdir, QStandardPaths::LocateDirectory);
            // allResourcesSubDirs[res.resource] += dirs;
            allResourcesSubDirs.insert(res.subdir, res.resource);
        }
    }

    m_ctimeFactory = new KCTimeFactory(this); // This is a build factory too, don't delete!!
    for (auto it1 = allResourcesSubDirs.cbegin(); it1 != allResourcesSubDirs.cend(); ++it1) {
        m_changed = false;
        m_resourceSubdir = it1.key();
        m_resource = it1.value();

        QSet<QString> relFiles;
        const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, m_resourceSubdir, QStandardPaths::LocateDirectory);
        qCDebug(SYCOCA) << "Looking for subdir" << m_resourceSubdir << "=>" << dirs;
        for (const QString &dir : dirs) {
            QDirIterator it(dir, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                const QString filePath = it.next();
                Q_ASSERT(filePath.startsWith(dir)); // due to the line below...
                const QString relPath = filePath.mid(dir.length() + 1);
                relFiles.insert(relPath);
            }
        }
        // Now find all factories that use this resource....
        // For each factory -- and its corresponding entryDict (iterate over both lists in parallel)
        for (int f = 0; f < factoryList.count(); ++f) {
            KSycocaFactory *currentFactory = factoryList.at(f);
            // m_ctimeInfo gets created after the initial loop, so it has no entryDict.
            m_currentEntryDict = f == entryDictList.size() ? nullptr : entryDictList.at(f);
            // For each resource the factory deals with
            const KSycocaResourceList &resourceList = currentFactory->resourceList();
            for (const KSycocaResource &res : resourceList) {
                if (res.resource != m_resource) {
                    continue;
                }

                // For each file in the resource
                for (const QString &entryPath : std::as_const(relFiles)) {
                    // Check if file matches filter
                    if (entryPath.endsWith(res.extension)) {
                        KSycocaEntry::Ptr entry = createEntry(currentFactory, entryPath);
                        if (entry) {
                            currentFactory->addEntry(entry);
                        }
                    }
                }
            }
        }
    }

    bool result = true;
    const bool createVFolder = true; // we need to always run the VFolderMenu code
    if (createVFolder || m_menuTest) {
        m_resource = "apps";
        m_resourceSubdir = QStringLiteral("applications");
        m_currentEntryDict = serviceEntryDict;
        m_changed = false;

        m_vfolder = new VFolderMenu(d->m_serviceFactory, this);
        if (!m_trackId.isEmpty()) {
            m_vfolder->setTrackId(m_trackId);
        }

        VFolderMenu::SubMenu *kdeMenu = m_vfolder->parseMenu(QStringLiteral("applications.menu"));

        KServiceGroup::Ptr entry = m_buildServiceGroupFactory->addNew(QStringLiteral("/"), kdeMenu->directoryFile, KServiceGroup::Ptr(), false);
        entry->setLayoutInfo(kdeMenu->layoutList);
        createMenu(QString(), QString(), kdeMenu);

        // Storing the mtime *after* looking at these dirs is a tiny race condition,
        // but I'm not sure how to get the vfolder dirs upfront...
        const auto allDirectories = m_vfolder->allDirectories();
        for (QString dir : allDirectories) {
            if (dir.endsWith(QLatin1Char('/'))) {
                dir.chop(1); // remove trailing slash, to avoid having ~/.local/share/applications twice
            }
            if (!m_allResourceDirs.contains(dir)) {
                qint64 stamp = 0;
                KSycocaUtilsPrivate::visitResourceDirectory(dir, [&stamp](const QFileInfo &info) {
                    stamp = qMax(stamp, info.lastModified().toMSecsSinceEpoch());
                    return true;
                });
                m_allResourceDirs.insert(dir, stamp);
            }
        }

        if (m_menuTest) {
            result = false;
        }
    }

    if (m_ctimeDict && !m_ctimeDict->isEmpty()) {
        qCDebug(SYCOCA) << "Still in time dict:";
        m_ctimeDict->dump();
    }

    qDeleteAll(entryDictList);
    return result;
}

void KBuildSycoca::createMenu(const QString &caption_, const QString &name_, VFolderMenu::SubMenu *menu)
{
    QString caption = caption_;
    QString name = name_;
    for (VFolderMenu::SubMenu *subMenu : std::as_const(menu->subMenus)) {
        QString subName = name + subMenu->name + QLatin1Char('/');

        QString directoryFile = subMenu->directoryFile;
        if (directoryFile.isEmpty()) {
            directoryFile = subName + QLatin1String(".directory");
        }
        quint32 timeStamp = m_ctimeFactory->dict()->ctime(directoryFile, m_resource);
        if (!timeStamp) {
            timeStamp = calcResourceHash(m_resourceSubdir, directoryFile);
        }

        KServiceGroup::Ptr entry;
        if (m_allEntries) {
            const quint32 oldTimestamp = m_ctimeDict->ctime(directoryFile, m_resource);

            if (timeStamp && (timeStamp == oldTimestamp)) {
                KSycocaEntry::Ptr group = m_serviceGroupEntryDict->value(subName);
                if (group) {
                    entry = KServiceGroup::Ptr(static_cast<KServiceGroup *>(group.data()));
                    if (entry->directoryEntryPath() != directoryFile) {
                        entry = nullptr; // Can't reuse this one!
                    }
                }
            }
        }
        if (timeStamp) { // bug? (see calcResourceHash). There might not be a .directory file...
            m_ctimeFactory->dict()->addCTime(directoryFile, m_resource, timeStamp);
        }

        entry = m_buildServiceGroupFactory->addNew(subName, subMenu->directoryFile, entry, subMenu->isDeleted);
        entry->setLayoutInfo(subMenu->layoutList);
        if (!(m_menuTest && entry->noDisplay())) {
            createMenu(caption + entry->caption() + QLatin1Char('/'), subName, subMenu);
        }
    }
    if (caption.isEmpty()) {
        caption += QLatin1Char('/');
    }
    if (name.isEmpty()) {
        name += QLatin1Char('/');
    }
    for (const KService::Ptr &p : std::as_const(menu->items)) {
        if (m_menuTest) {
            if (!menu->isDeleted && !p->noDisplay()) {
                printf("%s\t%s\t%s\n",
                       qPrintable(caption),
                       qPrintable(p->menuId()),
                       qPrintable(QStandardPaths::locate(QStandardPaths::ApplicationsLocation, p->entryPath())));
            }
        } else {
            m_buildServiceGroupFactory->addNewEntryTo(name, p);
        }
    }
}

bool KBuildSycoca::recreate(bool incremental)
{
    QFileInfo fi(KSycoca::absoluteFilePath());
    if (!QDir().mkpath(fi.absolutePath())) {
        qCWarning(SYCOCA) << "Couldn't create" << fi.absolutePath();
        return false;
    }
    QString path(fi.absoluteFilePath());

    QLockFile lockFile(path + QLatin1String(".lock"));
    if (!lockFile.tryLock()) {
        qCDebug(SYCOCA) << "Waiting for already running" << KBUILDSYCOCA_EXENAME << "to finish.";
        if (!lockFile.lock()) {
            qCWarning(SYCOCA) << "Couldn't lock" << path + QLatin1String(".lock");
            return false;
        }
        if (!needsRebuild()) {
            // qCDebug(SYCOCA) << "Up-to-date, skipping.";
            return true;
        }
    }

    QByteArray qSycocaPath = QFile::encodeName(path);
    s_cSycocaPath = qSycocaPath.data();

    m_allEntries = nullptr;
    m_ctimeDict = nullptr;
    if (incremental && checkGlobalHeader()) {
        qCDebug(SYCOCA) << "Reusing existing ksycoca";
        KSycoca *oldSycoca = KSycoca::self();
        m_allEntries = new KSycocaEntryListList;
        m_ctimeDict = new KCTimeDict;

        // Must be in same order as in KBuildSycoca::recreate()!
        m_allEntries->append(KSycocaPrivate::self()->mimeTypeFactory()->allEntries());
        m_allEntries->append(KSycocaPrivate::self()->serviceGroupFactory()->allEntries());
        m_allEntries->append(KSycocaPrivate::self()->serviceFactory()->allEntries());

        KCTimeFactory *ctimeInfo = new KCTimeFactory(oldSycoca);
        *m_ctimeDict = ctimeInfo->loadDict();
    }
    s_cSycocaPath = nullptr;

    QSaveFile database(path);
    bool openedOK = database.open(QIODevice::WriteOnly);

    if (!openedOK && database.error() == QFile::WriteError && QFile::exists(path)) {
        QFile::remove(path);
        openedOK = database.open(QIODevice::WriteOnly);
    }
    if (!openedOK) {
        qCWarning(SYCOCA) << "ERROR creating database" << path << ":" << database.errorString();
        return false;
    }

    QDataStream *str = new QDataStream(&database);
    str->setVersion(QDataStream::Qt_5_3);

    m_newTimestamp = QDateTime::currentMSecsSinceEpoch();
    qCDebug(SYCOCA).nospace() << "Recreating ksycoca file (" << path << ", version " << KSycoca::version() << ")";

    KBuildMimeTypeFactory *buildMimeTypeFactory = new KBuildMimeTypeFactory(this);
    d->m_mimeTypeFactory = buildMimeTypeFactory;
    m_buildServiceGroupFactory = new KBuildServiceGroupFactory(this);
    d->m_serviceGroupFactory = m_buildServiceGroupFactory;
    d->m_serviceFactory = new KBuildServiceFactory(buildMimeTypeFactory);

    if (build()) { // Parse dirs
        save(str); // Save database
        if (str->status() != QDataStream::Ok) { // Probably unnecessary now in Qt5, since QSaveFile detects write errors
            database.cancelWriting(); // Error
        }
        delete str;
        str = nullptr;

        // if we are currently via sudo, preserve the original owner
        // as $HOME may also be that of another user rather than /root
#ifdef Q_OS_UNIX
        if (qEnvironmentVariableIsSet("SUDO_UID")) {
            const int uid = qEnvironmentVariableIntValue("SUDO_UID");
            const int gid = qEnvironmentVariableIntValue("SUDO_GID");
            if (uid && gid) {
                fchown(database.handle(), uid, gid);
            }
        }
#endif

        if (!database.commit()) {
            qCWarning(SYCOCA) << "ERROR writing database" << database.fileName() << database.errorString();
            return false;
        }
    } else {
        delete str;
        str = nullptr;
        database.cancelWriting();
        if (m_menuTest) {
            return true;
        }
        qCDebug(SYCOCA) << "Database is up to date";
    }

#ifndef QT_NO_SHAREDMEMORY
    if (d->m_sycocaStrategy == KSycocaPrivate::StrategyMemFile) {
        KMemFile::fileContentsChanged(path);
    }
#endif

    delete m_ctimeDict;
    delete m_allEntries;
    delete m_vfolder;

    return true;
}

void KBuildSycoca::save(QDataStream *str)
{
    // Write header (#pass 1)
    str->device()->seek(0);

    (*str) << qint32(KSycoca::version());
    // KSycocaFactory * servicetypeFactory = 0;
    // KBuildMimeTypeFactory * mimeTypeFactory = 0;
    KBuildServiceFactory *serviceFactory = nullptr;
    auto lst = *factories();
    for (KSycocaFactory *factory : std::as_const(lst)) {
        qint32 aId;
        qint32 aOffset;
        aId = factory->factoryId();
        // if ( aId == KST_KServiceTypeFactory )
        //   servicetypeFactory = factory;
        // else if ( aId == KST_KMimeTypeFactory )
        //   mimeTypeFactory = static_cast<KBuildMimeTypeFactory *>( factory );
        if (aId == KST_KServiceFactory) {
            serviceFactory = static_cast<KBuildServiceFactory *>(factory);
        }
        aOffset = factory->offset(); // not set yet, so always 0
        (*str) << aId;
        (*str) << aOffset;
    }
    (*str) << qint32(0); // No more factories.
    // Write XDG_DATA_DIRS
    (*str) << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).join(QString(QLatin1Char(':')));
    (*str) << m_newTimestamp;
    (*str) << QLocale().bcp47Name();
    // This makes it possible to trigger a ksycoca update for all users (KIOSK feature)
    (*str) << calcResourceHash(QStringLiteral("kservices6"), QStringLiteral("update_ksycoca"));
    (*str) << m_allResourceDirs.keys();
    for (auto it = m_allResourceDirs.constBegin(); it != m_allResourceDirs.constEnd(); ++it) {
        (*str) << it.value();
    }
    (*str) << m_extraFiles.keys();
    for (auto it = m_extraFiles.constBegin(); it != m_extraFiles.constEnd(); ++it) {
        (*str) << it.value();
    }

    // Calculate per-servicetype/MIME type data
    if (serviceFactory) {
        serviceFactory->postProcessServices();
    }

    // Here so that it's the last debug message
    qCDebug(SYCOCA) << "Saving";

    // Write factory data....
    lst = *factories();
    for (KSycocaFactory *factory : std::as_const(lst)) {
        factory->save(*str);
        if (str->status() != QDataStream::Ok) { // ######## TODO: does this detect write errors, e.g. disk full?
            return; // error
        }
    }

    qint64 endOfData = str->device()->pos();

    // Write header (#pass 2)
    str->device()->seek(0);

    (*str) << qint32(KSycoca::version());
    lst = *factories();
    for (KSycocaFactory *factory : std::as_const(lst)) {
        qint32 aId;
        qint32 aOffset;
        aId = factory->factoryId();
        aOffset = factory->offset();
        (*str) << aId;
        (*str) << aOffset;
    }
    (*str) << qint32(0); // No more factories.

    // Jump to end of database
    str->device()->seek(endOfData);
}

QStringList KBuildSycoca::factoryResourceDirs()
{
    static QStringList *dirs = nullptr;
    if (dirs != nullptr) {
        return *dirs;
    }
    dirs = new QStringList;
    // these are all resource dirs cached by ksycoca
    *dirs += KMimeTypeFactory::resourceDirs();
    *dirs += KServiceFactory::resourceDirs();

    return *dirs;
}

QStringList KBuildSycoca::factoryExtraFiles()
{
    QStringList files;
    // these are the extra files cached by ksycoca
    // and whose timestamps are checked
    files += KMimeAssociations::mimeAppsFiles();

    return files;
}

QStringList KBuildSycoca::existingResourceDirs()
{
    static QStringList *dirs = nullptr;
    if (dirs != nullptr) {
        return *dirs;
    }
    dirs = new QStringList(factoryResourceDirs());

    auto checkDir = [](const QString &str) {
        QFileInfo info(str);
        return !info.exists() || !info.isReadable();
    };
    dirs->erase(std::remove_if(dirs->begin(), dirs->end(), checkDir), dirs->end());

    return *dirs;
}

static quint32 updateHash(const QString &file, quint32 hash)
{
    QFileInfo fi(file);
    if (fi.isReadable() && fi.isFile()) {
        // This was using buff.st_ctime (in Waldo's initial commit to kstandarddirs.cpp in 2001), but that looks wrong?
        // Surely we want to catch manual editing, while a chmod doesn't matter much?
        qint64 timestamp = fi.lastModified().toSecsSinceEpoch();
        // On some systems (i.e. Fedora Kinoite), all files in /usr have a last
        // modified timestamp of 0 (UNIX Epoch). In this case, always assume
        // the file as been changed.
        if (timestamp == 0) {
            static qint64 now = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
            timestamp = now;
        }
        hash += timestamp;
    }
    return hash;
}

quint32 KBuildSycoca::calcResourceHash(const QString &resourceSubDir, const QString &filename)
{
    quint32 hash = 0;
    if (!QDir::isRelativePath(filename)) {
        return updateHash(filename, hash);
    }
    const QString filePath = resourceSubDir + QLatin1Char('/') + filename;
    const QString qrcFilePath = QStringLiteral(":/") + filePath;
    const QStringList files =
        QFileInfo::exists(qrcFilePath) ? QStringList{qrcFilePath} : QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, filePath);
    for (const QString &file : files) {
        hash = updateHash(file, hash);
    }
    if (hash == 0 && !filename.endsWith(QLatin1String("update_ksycoca"))
        && !filename.endsWith(QLatin1String(".directory")) // bug? needs investigation from someone who understands the VFolder spec
    ) {
        if (files.isEmpty()) {
            // This can happen if the file was deleted between directory listing and the above locateAll
            qCDebug(SYCOCA) << "File not found anymore:" << filename << " -- probably deleted meanwhile";
        } else {
            // This can happen if the file was deleted between locateAll and QFileInfo
            qCDebug(SYCOCA) << "File(s) found but not readable (or disappeared meanwhile)" << files;
        }
    }
    return hash;
}

bool KBuildSycoca::checkGlobalHeader()
{
    // Since it's part of the filename, we are 99% sure that the locale and prefixes will match.
    const QString current_language = QLocale().bcp47Name();
    const quint32 current_update_sig = KBuildSycoca::calcResourceHash(QStringLiteral("kservices6"), QStringLiteral("update_ksycoca"));
    const QString current_prefixes = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).join(QString(QLatin1Char(':')));

    const KSycocaHeader header = KSycocaPrivate::self()->readSycocaHeader();
    Q_ASSERT(!header.prefixes.split(QLatin1Char(':')).contains(QDir::homePath()));

    return (current_update_sig == header.updateSignature) //
        && (current_language == header.language) //
        && (current_prefixes == header.prefixes) //
        && (header.timeStamp != 0);
}

const char *KBuildSycoca::sycocaPath()
{
    return s_cSycocaPath;
}

#include "moc_kbuildsycoca_p.cpp"
