/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfig.h"
#include "kconfig_p.h"

#include "config-kconfig.h"
#include "kconfig_core_log_settings.h"

#include <cstdlib>
#include <fcntl.h>

#include "kconfigbackend_p.h"
#include "kconfiggroup.h"

#include <QBasicMutex>
#include <QByteArray>
#include <QCache>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QMutexLocker>
#include <QProcess>
#include <QSet>
#include <QThreadStorage>

#include <algorithm>
#include <iterator>
#include <set>
#include <string_view>
#include <unordered_set>

#if KCONFIG_USE_DBUS
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusMetaType>
#endif

bool KConfigPrivate::mappingsRegistered = false;

// For caching purposes
static bool s_wasTestModeEnabled = false;

Q_GLOBAL_STATIC(QStringList, s_globalFiles) // For caching purposes.
static QBasicMutex s_globalFilesMutex;
Q_GLOBAL_STATIC_WITH_ARGS(QString, sGlobalFileName, (QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kdeglobals")))

using ParseCacheKey = std::pair<QStringList, QString>;
struct ParseCacheValue {
    KEntryMap entries;
    QDateTime parseTime;
};
using ParseCache = QThreadStorage<QCache<ParseCacheKey, ParseCacheValue>>;
Q_GLOBAL_STATIC(ParseCache, sGlobalParse)

#ifndef Q_OS_WIN
static const Qt::CaseSensitivity sPathCaseSensitivity = Qt::CaseSensitive;
#else
static const Qt::CaseSensitivity sPathCaseSensitivity = Qt::CaseInsensitive;
#endif

KConfigPrivate::KConfigPrivate(KConfig::OpenFlags flags, QStandardPaths::StandardLocation resourceType)
    : openFlags(flags)
    , resourceType(resourceType)
    , mBackend(nullptr)
    , bDynamicBackend(true)
    , bDirty(false)
    , bReadDefaults(false)
    , bFileImmutable(false)
    , bForceGlobal(false)
    , bSuppressGlobal(false)
    , configState(KConfigBase::NoAccess)
{
    const bool isTestMode = QStandardPaths::isTestModeEnabled();
    // If sGlobalFileName was initialised and testMode has been toggled,
    // sGlobalFileName may need to be updated to point to the correct kdeglobals file
    if (sGlobalFileName.exists() && s_wasTestModeEnabled != isTestMode) {
        s_wasTestModeEnabled = isTestMode;
        *sGlobalFileName = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/kdeglobals");
    }

    static QBasicAtomicInt use_etc_kderc = Q_BASIC_ATOMIC_INITIALIZER(-1);
    if (use_etc_kderc.loadRelaxed() < 0) {
        use_etc_kderc.storeRelaxed(!qEnvironmentVariableIsSet("KDE_SKIP_KDERC")); // for unit tests
    }
    if (use_etc_kderc.loadRelaxed()) {
        etc_kderc =
#ifdef Q_OS_WIN
            QFile::decodeName(qgetenv("WINDIR") + "/kde5rc");
#else
            QStringLiteral("/etc/kde5rc");
#endif
        if (!QFileInfo(etc_kderc).isReadable()) {
            use_etc_kderc.storeRelaxed(false);
            etc_kderc.clear();
        }
    }

    //    if (!mappingsRegistered) {
    //        KEntryMap tmp;
    //        if (!etc_kderc.isEmpty()) {
    //            QExplicitlySharedDataPointer<KConfigBackend> backend = KConfigBackend::create(etc_kderc, QLatin1String("INI"));
    //            backend->parseConfig( "en_US", tmp, KConfigBackend::ParseDefaults);
    //        }
    //        const QString kde5rc(QDir::home().filePath(".kde5rc"));
    //        if (KStandardDirs::checkAccess(kde5rc, R_OK)) {
    //            QExplicitlySharedDataPointer<KConfigBackend> backend = KConfigBackend::create(kde5rc, QLatin1String("INI"));
    //            backend->parseConfig( "en_US", tmp, KConfigBackend::ParseOptions());
    //        }
    //        KConfigBackend::registerMappings(tmp);
    //        mappingsRegistered = true;
    //    }

    setLocale(QLocale().name());
}

bool KConfigPrivate::lockLocal()
{
    if (mBackend) {
        return mBackend->lock();
    }
    // anonymous object - pretend we locked it
    return true;
}

static bool isGroupOrSubGroupMatch(KEntryMapConstIterator entryMapIt, const QByteArray &group)
{
    const QByteArray &entryGroup = entryMapIt.key().mGroup;
    Q_ASSERT_X(entryGroup.startsWith(group), Q_FUNC_INFO, "Precondition");
    return entryGroup.size() == group.size() || entryGroup[group.size()] == '\x1d';
}

void KConfigPrivate::copyGroup(const QByteArray &source, const QByteArray &destination, KConfigGroup *otherGroup, KConfigBase::WriteConfigFlags flags) const
{
    KEntryMap &otherMap = otherGroup->config()->d_ptr->entryMap;
    const bool sameName = (destination == source);

    // we keep this bool outside the for loop so that if
    // the group is empty, we don't end up marking the other config
    // as dirty erroneously
    bool dirtied = false;

    entryMap.forEachEntryWhoseGroupStartsWith(source, [&source, &destination, flags, &otherMap, sameName, &dirtied](KEntryMapConstIterator entryMapIt) {
        // don't copy groups that start with the same prefix, but are not sub-groups
        if (!isGroupOrSubGroupMatch(entryMapIt, source)) {
            return;
        }

        KEntryKey newKey = entryMapIt.key();

        if (flags & KConfigBase::Localized) {
            newKey.bLocal = true;
        }

        if (!sameName) {
            newKey.mGroup.replace(0, source.size(), destination);
        }

        KEntry entry = entryMapIt.value();
        dirtied = entry.bDirty = flags & KConfigBase::Persistent;

        if (flags & KConfigBase::Global) {
            entry.bGlobal = true;
        }

        if (flags & KConfigBase::Notify) {
            entry.bNotify = true;
        }

        otherMap[newKey] = entry;
    });

    if (dirtied) {
        otherGroup->config()->d_ptr->bDirty = true;
    }
}

QString KConfigPrivate::expandString(const QString &value)
{
    QString aValue = value;

    // check for environment variables and make necessary translations
    int nDollarPos = aValue.indexOf(QLatin1Char('$'));
    while (nDollarPos != -1 && nDollarPos + 1 < aValue.length()) {
        // there is at least one $
        if (aValue.at(nDollarPos + 1) != QLatin1Char('$')) {
            int nEndPos = nDollarPos + 1;
            // the next character is not $
            QStringView aVarName;
            if (aValue.at(nEndPos) == QLatin1Char('{')) {
                while ((nEndPos <= aValue.length()) && (aValue[nEndPos] != QLatin1Char('}'))) {
                    ++nEndPos;
                }
                ++nEndPos;
                aVarName = QStringView(aValue).mid(nDollarPos + 2, nEndPos - nDollarPos - 3);
            } else {
                while (nEndPos < aValue.length() && (aValue[nEndPos].isNumber() || aValue[nEndPos].isLetter() || aValue[nEndPos] == QLatin1Char('_'))) {
                    ++nEndPos;
                }
                aVarName = QStringView(aValue).mid(nDollarPos + 1, nEndPos - nDollarPos - 1);
            }
            QString env;
            if (!aVarName.isEmpty()) {
#ifdef Q_OS_WIN
                if (aVarName == QLatin1String("HOME")) {
                    env = QDir::homePath();
                } else
#endif
                {
                    QByteArray pEnv = qgetenv(aVarName.toLatin1().constData());
                    if (!pEnv.isEmpty()) {
                        env = QString::fromLocal8Bit(pEnv.constData());
                    } else {
                        if (aVarName == QLatin1String("QT_DATA_HOME")) {
                            env = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
                        } else if (aVarName == QLatin1String("QT_CONFIG_HOME")) {
                            env = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
                        } else if (aVarName == QLatin1String("QT_CACHE_HOME")) {
                            env = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation);
                        }
                    }
                }
                aValue.replace(nDollarPos, nEndPos - nDollarPos, env);
                nDollarPos += env.length();
            } else {
                aValue.remove(nDollarPos, nEndPos - nDollarPos);
            }
        } else {
            // remove one of the dollar signs
            aValue.remove(nDollarPos, 1);
            ++nDollarPos;
        }
        nDollarPos = aValue.indexOf(QLatin1Char('$'), nDollarPos);
    }

    return aValue;
}

KConfig::KConfig(const QString &file, OpenFlags mode, QStandardPaths::StandardLocation resourceType)
    : d_ptr(new KConfigPrivate(mode, resourceType))
{
    d_ptr->changeFileName(file); // set the local file name

    // read initial information off disk
    reparseConfiguration();
}

KConfig::KConfig(const QString &file, const QString &backend, QStandardPaths::StandardLocation resourceType)
    : d_ptr(new KConfigPrivate(SimpleConfig, resourceType))
{
    d_ptr->mBackend = KConfigBackend::create(file, backend);
    d_ptr->bDynamicBackend = false;
    d_ptr->changeFileName(file); // set the local file name

    // read initial information off disk
    reparseConfiguration();
}

KConfig::KConfig(KConfigPrivate &d)
    : d_ptr(&d)
{
}

KConfig::~KConfig()
{
    Q_D(KConfig);
    if (d->bDirty && (d->mBackend && d->mBackend->ref.loadRelaxed() == 1)) {
        sync();
    }
    delete d;
}

static bool isNonDeletedKey(KEntryMapConstIterator entryMapIt)
{
    return !entryMapIt.key().mKey.isNull() && !entryMapIt->bDeleted;
}

static int findFirstGroupEndPos(const QByteArray &groupFullName, int from = 0)
{
    const auto index = groupFullName.indexOf('\x1d', from);
    return index == -1 ? groupFullName.size() : index;
}

// std::string_view is used because QByteArrayView does not exist in Qt 5.
// std::unordered_set rather than QSet is used because there is no qHash() overload for std::string_view in Qt 5.
using ByteArrayViewSet = std::unordered_set<std::string_view>;

static QStringList stringListFromUtf8Collection(const ByteArrayViewSet &source)
{
    QStringList list;
    list.reserve(source.size());
    std::transform(source.cbegin(), source.cend(), std::back_inserter(list), [](std::string_view view) {
        return QString::fromUtf8(view.data(), view.size());
    });
    return list;
}

QStringList KConfig::groupList() const
{
    Q_D(const KConfig);
    ByteArrayViewSet groups;

    for (auto entryMapIt = d->entryMap.cbegin(); entryMapIt != d->entryMap.cend(); ++entryMapIt) {
        const QByteArray &group = entryMapIt.key().mGroup;
        if (isNonDeletedKey(entryMapIt) && !group.isEmpty() && group != "<default>" && group != "$Version") {
            groups.emplace(group.constData(), findFirstGroupEndPos(group));
        }
    }

    return stringListFromUtf8Collection(groups);
}

QStringList KConfigPrivate::groupList(const QByteArray &group) const
{
    const QByteArray theGroup = group + '\x1d';
    ByteArrayViewSet groups;

    entryMap.forEachEntryWhoseGroupStartsWith(theGroup, [&theGroup, &groups](KEntryMapConstIterator entryMapIt) {
        if (isNonDeletedKey(entryMapIt)) {
            const QByteArray &entryGroup = entryMapIt.key().mGroup;
            const auto subgroupStartPos = theGroup.size();
            const auto subgroupEndPos = findFirstGroupEndPos(entryGroup, subgroupStartPos);
            groups.emplace(entryGroup.constData() + subgroupStartPos, subgroupEndPos - subgroupStartPos);
        }
    });

    return stringListFromUtf8Collection(groups);
}

/// Returns @p parentGroup itself, all its subgroups, subsubgroups, and so on, including deleted groups.
QSet<QByteArray> KConfigPrivate::allSubGroups(const QByteArray &parentGroup) const
{
    QSet<QByteArray> groups;

    entryMap.forEachEntryWhoseGroupStartsWith(parentGroup, [&parentGroup, &groups](KEntryMapConstIterator entryMapIt) {
        const KEntryKey &key = entryMapIt.key();
        if (key.mKey.isNull() && isGroupOrSubGroupMatch(entryMapIt, parentGroup)) {
            groups << key.mGroup;
        }
    });

    return groups;
}

bool KConfigPrivate::hasNonDeletedEntries(const QByteArray &group) const
{
    return entryMap.anyEntryWhoseGroupStartsWith(group, [&group](KEntryMapConstIterator entryMapIt) {
        return isGroupOrSubGroupMatch(entryMapIt, group) && isNonDeletedKey(entryMapIt);
    });
}

QStringList KConfigPrivate::keyListImpl(const QByteArray &theGroup) const
{
    QStringList keys;

    const auto theEnd = entryMap.constEnd();
    auto it = entryMap.constFindEntry(theGroup);
    if (it != theEnd) {
        ++it; // advance past the special group entry marker

        std::set<QString> tmp; // unique set, sorted for unittests
        for (; it != theEnd && it.key().mGroup == theGroup; ++it) {
            if (isNonDeletedKey(it)) {
                tmp.insert(QString::fromUtf8(it.key().mKey));
            }
        }
        keys = QList<QString>(tmp.begin(), tmp.end());
    }

    return keys;
}

QStringList KConfig::keyList(const QString &aGroup) const
{
    Q_D(const KConfig);
    const QByteArray theGroup(aGroup.isEmpty() ? "<default>" : aGroup.toUtf8());
    return d->keyListImpl(theGroup);
}

QMap<QString, QString> KConfig::entryMap(const QString &aGroup) const
{
    Q_D(const KConfig);
    QMap<QString, QString> theMap;
    const QByteArray theGroup(aGroup.isEmpty() ? "<default>" : aGroup.toUtf8());

    const auto theEnd = d->entryMap.constEnd();
    auto it = d->entryMap.constFindEntry(theGroup, {}, {});
    if (it != theEnd) {
        ++it; // advance past the special group entry marker

        for (; it != theEnd && it.key().mGroup == theGroup; ++it) {
            // leave the default values and deleted entries out
            if (!it->bDeleted && !it.key().bDefault) {
                const QString key = QString::fromUtf8(it.key().mKey.constData());
                // the localized entry should come first, so don't overwrite it
                // with the non-localized entry
                if (!theMap.contains(key)) {
                    if (it->bExpand) {
                        theMap.insert(key, KConfigPrivate::expandString(QString::fromUtf8(it->mValue.constData())));
                    } else {
                        theMap.insert(key, QString::fromUtf8(it->mValue.constData()));
                    }
                }
            }
        }
    }

    return theMap;
}

bool KConfig::sync()
{
    Q_D(KConfig);

    if (isImmutable() || name().isEmpty()) {
        // can't write to an immutable or anonymous file.
        return false;
    }

    QHash<QString, QByteArrayList> notifyGroupsLocal;
    QHash<QString, QByteArrayList> notifyGroupsGlobal;

    if (d->bDirty && d->mBackend) {
        const QByteArray utf8Locale(locale().toUtf8());

        // Create the containing dir, maybe it wasn't there
        d->mBackend->createEnclosing();

        // lock the local file
        if (d->configState == ReadWrite && !d->lockLocal()) {
            qCWarning(KCONFIG_CORE_LOG) << "couldn't lock local file";
            return false;
        }

        // Rewrite global/local config only if there is a dirty entry in it.
        bool writeGlobals = false;
        bool writeLocals = false;

        for (auto it = d->entryMap.constBegin(); it != d->entryMap.constEnd(); ++it) {
            auto e = it.value();
            if (e.bDirty) {
                if (e.bGlobal) {
                    writeGlobals = true;
                    if (e.bNotify) {
                        notifyGroupsGlobal[QString::fromUtf8(it.key().mGroup)] << it.key().mKey;
                    }
                } else {
                    writeLocals = true;
                    if (e.bNotify) {
                        notifyGroupsLocal[QString::fromUtf8(it.key().mGroup)] << it.key().mKey;
                    }
                }
            }
        }

        d->bDirty = false; // will revert to true if a config write fails

        if (d->wantGlobals() && writeGlobals) {
            QExplicitlySharedDataPointer<KConfigBackend> tmp = KConfigBackend::create(*sGlobalFileName);
            if (d->configState == ReadWrite && !tmp->lock()) {
                qCWarning(KCONFIG_CORE_LOG) << "couldn't lock global file";

                // unlock the local config if we're returning early
                if (d->mBackend->isLocked()) {
                    d->mBackend->unlock();
                }

                d->bDirty = true;
                return false;
            }
            if (!tmp->writeConfig(utf8Locale, d->entryMap, KConfigBackend::WriteGlobal)) {
                d->bDirty = true;
            }
            if (tmp->isLocked()) {
                tmp->unlock();
            }
        }

        if (writeLocals) {
            if (!d->mBackend->writeConfig(utf8Locale, d->entryMap, KConfigBackend::WriteOptions())) {
                d->bDirty = true;
            }
        }
        if (d->mBackend->isLocked()) {
            d->mBackend->unlock();
        }
    }

    if (!notifyGroupsLocal.isEmpty()) {
        d->notifyClients(notifyGroupsLocal, QLatin1Char('/') + name());
    }
    if (!notifyGroupsGlobal.isEmpty()) {
        d->notifyClients(notifyGroupsGlobal, QStringLiteral("/kdeglobals"));
    }

    return !d->bDirty;
}

void KConfigPrivate::notifyClients(const QHash<QString, QByteArrayList> &changes, const QString &path)
{
#if KCONFIG_USE_DBUS
    qDBusRegisterMetaType<QByteArrayList>();

    qDBusRegisterMetaType<QHash<QString, QByteArrayList>>();

    QDBusMessage message = QDBusMessage::createSignal(path, QStringLiteral("org.kde.kconfig.notify"), QStringLiteral("ConfigChanged"));
    message.setArguments({QVariant::fromValue(changes)});
    QDBusConnection::sessionBus().send(message);
#else
    Q_UNUSED(changes)
    Q_UNUSED(path)
#endif
}

void KConfig::markAsClean()
{
    Q_D(KConfig);
    d->bDirty = false;

    // clear any dirty flags that entries might have set
    const KEntryMapIterator theEnd = d->entryMap.end();
    for (KEntryMapIterator it = d->entryMap.begin(); it != theEnd; ++it) {
        it->bDirty = false;
        it->bNotify = false;
    }
}

bool KConfig::isDirty() const
{
    Q_D(const KConfig);
    return d->bDirty;
}

void KConfig::checkUpdate(const QString &id, const QString &updateFile)
{
    const KConfigGroup cg(this, "$Version");
    const QString cfg_id = updateFile + QLatin1Char(':') + id;
    const QStringList ids = cg.readEntry("update_info", QStringList());
    if (!ids.contains(cfg_id)) {
        QProcess::execute(QStringLiteral(KCONF_UPDATE_INSTALL_LOCATION), QStringList{QStringLiteral("--check"), updateFile});
        reparseConfiguration();
    }
}

KConfig *KConfig::copyTo(const QString &file, KConfig *config) const
{
    Q_D(const KConfig);
    if (!config) {
        config = new KConfig(QString(), SimpleConfig, d->resourceType);
    }
    config->d_func()->changeFileName(file);
    config->d_func()->entryMap = d->entryMap;
    config->d_func()->bFileImmutable = false;

    const KEntryMapIterator theEnd = config->d_func()->entryMap.end();
    for (KEntryMapIterator it = config->d_func()->entryMap.begin(); it != theEnd; ++it) {
        it->bDirty = true;
    }
    config->d_ptr->bDirty = true;

    return config;
}

QString KConfig::name() const
{
    Q_D(const KConfig);
    return d->fileName;
}

KConfig::OpenFlags KConfig::openFlags() const
{
    Q_D(const KConfig);
    return d->openFlags;
}

struct KConfigStaticData {
    QString globalMainConfigName;
    // Keep a copy so we can use it in global dtors, after qApp is gone
    QStringList appArgs;
};
Q_GLOBAL_STATIC(KConfigStaticData, globalData)

void KConfig::setMainConfigName(const QString &str)
{
    globalData()->globalMainConfigName = str;
}

QString KConfig::mainConfigName()
{
    KConfigStaticData *data = globalData();
    if (data->appArgs.isEmpty()) {
        data->appArgs = QCoreApplication::arguments();
    }

    // --config on the command line overrides everything else
    const QStringList args = data->appArgs;
    for (int i = 1; i < args.count(); ++i) {
        if (args.at(i) == QLatin1String("--config") && i < args.count() - 1) {
            return args.at(i + 1);
        }
    }
    const QString globalName = data->globalMainConfigName;
    if (!globalName.isEmpty()) {
        return globalName;
    }

    QString appName = QCoreApplication::applicationName();
    return appName + QLatin1String("rc");
}

void KConfigPrivate::changeFileName(const QString &name)
{
    fileName = name;

    QString file;
    if (name.isEmpty()) {
        if (wantDefaults()) { // accessing default app-specific config "appnamerc"
            fileName = KConfig::mainConfigName();
            file = QStandardPaths::writableLocation(resourceType) + QLatin1Char('/') + fileName;
        } else if (wantGlobals()) { // accessing "kdeglobals" by specifying no filename and NoCascade - XXX used anywhere?
            resourceType = QStandardPaths::GenericConfigLocation;
            fileName = QStringLiteral("kdeglobals");
            file = *sGlobalFileName;
        } else {
            // anonymous config
            openFlags = KConfig::SimpleConfig;
            return;
        }
    } else if (QDir::isAbsolutePath(fileName)) {
        fileName = QFileInfo(fileName).canonicalFilePath();
        if (fileName.isEmpty()) { // file doesn't exist (yet)
            fileName = name;
        }
        file = fileName;
    } else {
        file = QStandardPaths::writableLocation(resourceType) + QLatin1Char('/') + fileName;
    }

    Q_ASSERT(!file.isEmpty());

    bSuppressGlobal = (file.compare(*sGlobalFileName, sPathCaseSensitivity) == 0);

    if (bDynamicBackend || !mBackend) { // allow dynamic changing of backend
        mBackend = KConfigBackend::create(file);
    } else {
        mBackend->setFilePath(file);
    }

    configState = mBackend->accessMode();
}

void KConfig::reparseConfiguration()
{
    Q_D(KConfig);
    if (d->fileName.isEmpty()) {
        return;
    }

    // Don't lose pending changes
    if (!d->isReadOnly() && d->bDirty) {
        sync();
    }

    d->entryMap.clear();

    d->bFileImmutable = false;

    {
        QMutexLocker locker(&s_globalFilesMutex);
        s_globalFiles()->clear();
    }

    // Parse all desired files from the least to the most specific.
    if (d->wantGlobals()) {
        d->parseGlobalFiles();
    }

    d->parseConfigFiles();
}

QStringList KConfigPrivate::getGlobalFiles() const
{
    QMutexLocker locker(&s_globalFilesMutex);
    if (s_globalFiles()->isEmpty()) {
        const QStringList paths1 = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("kdeglobals"));
        const QStringList paths2 = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation, QStringLiteral("system.kdeglobals"));

        const bool useEtcKderc = !etc_kderc.isEmpty();
        s_globalFiles()->reserve(paths1.size() + paths2.size() + (useEtcKderc ? 1 : 0));

        for (const QString &dir1 : paths1) {
            s_globalFiles()->push_front(dir1);
        }
        for (const QString &dir2 : paths2) {
            s_globalFiles()->push_front(dir2);
        }

        if (useEtcKderc) {
            s_globalFiles()->push_front(etc_kderc);
        }
    }

    return *s_globalFiles();
}

void KConfigPrivate::parseGlobalFiles()
{
    const QStringList globalFiles = getGlobalFiles();
    //    qDebug() << "parsing global files" << globalFiles;

    Q_ASSERT(entryMap.isEmpty());
    const ParseCacheKey key = {globalFiles, locale};
    auto data = sGlobalParse->localData().object(key);
    QDateTime newest;
    for (const auto &file : globalFiles) {
        const auto fileDate = QFileInfo(file).lastModified();
        if (fileDate > newest) {
            newest = fileDate;
        }
    }
    if (data) {
        if (data->parseTime < newest) {
            data = nullptr;
        } else {
            entryMap = data->entries;
            return;
        }
    }

    const QByteArray utf8Locale = locale.toUtf8();
    for (const QString &file : globalFiles) {
        KConfigBackend::ParseOptions parseOpts = KConfigBackend::ParseGlobal | KConfigBackend::ParseExpansions;

        if (file.compare(*sGlobalFileName, sPathCaseSensitivity) != 0) {
            parseOpts |= KConfigBackend::ParseDefaults;
        }

        QExplicitlySharedDataPointer<KConfigBackend> backend = KConfigBackend::create(file);
        if (backend->parseConfig(utf8Locale, entryMap, parseOpts) == KConfigBackend::ParseImmutable) {
            break;
        }
    }
    sGlobalParse->localData().insert(key, new ParseCacheValue({entryMap, newest}));
}

void KConfigPrivate::parseConfigFiles()
{
    // can only read the file if there is a backend and a file name
    if (mBackend && !fileName.isEmpty()) {
        bFileImmutable = false;

        QList<QString> files;
        if (wantDefaults()) {
            if (bSuppressGlobal) {
                files = getGlobalFiles();
            } else {
                if (QDir::isAbsolutePath(fileName)) {
                    const QString canonicalFile = QFileInfo(fileName).canonicalFilePath();
                    if (!canonicalFile.isEmpty()) { // empty if it doesn't exist
                        files << canonicalFile;
                    }
                } else {
                    const QStringList localFilesPath = QStandardPaths::locateAll(resourceType, fileName);
                    for (const QString &f : localFilesPath) {
                        files.prepend(QFileInfo(f).canonicalFilePath());
                    }

                    // allow fallback to config files bundled in resources
                    const QString resourceFile(QStringLiteral(":/kconfig/") + fileName);
                    if (QFile::exists(resourceFile)) {
                        files.prepend(resourceFile);
                    }
                }
            }
        } else {
            files << mBackend->filePath();
        }
        if (!isSimple()) {
            files = QList<QString>(extraFiles.cbegin(), extraFiles.cend()) + files;
        }

        //        qDebug() << "parsing local files" << files;

        const QByteArray utf8Locale = locale.toUtf8();
        for (const QString &file : std::as_const(files)) {
            if (file.compare(mBackend->filePath(), sPathCaseSensitivity) == 0) {
                switch (mBackend->parseConfig(utf8Locale, entryMap, KConfigBackend::ParseExpansions)) {
                case KConfigBackend::ParseOk:
                    break;
                case KConfigBackend::ParseImmutable:
                    bFileImmutable = true;
                    break;
                case KConfigBackend::ParseOpenError:
                    configState = KConfigBase::NoAccess;
                    break;
                }
            } else {
                QExplicitlySharedDataPointer<KConfigBackend> backend = KConfigBackend::create(file);
                constexpr auto parseOpts = KConfigBackend::ParseDefaults | KConfigBackend::ParseExpansions;
                bFileImmutable = backend->parseConfig(utf8Locale, entryMap, parseOpts) == KConfigBackend::ParseImmutable;
            }

            if (bFileImmutable) {
                break;
            }
        }
    }
}

KConfig::AccessMode KConfig::accessMode() const
{
    Q_D(const KConfig);
    return d->configState;
}

void KConfig::addConfigSources(const QStringList &files)
{
    Q_D(KConfig);
    for (const QString &file : files) {
        d->extraFiles.push(file);
    }

    if (!files.isEmpty()) {
        reparseConfiguration();
    }
}

QStringList KConfig::additionalConfigSources() const
{
    Q_D(const KConfig);
    return d->extraFiles.toList();
}

QString KConfig::locale() const
{
    Q_D(const KConfig);
    return d->locale;
}

bool KConfigPrivate::setLocale(const QString &aLocale)
{
    if (aLocale != locale) {
        locale = aLocale;
        return true;
    }
    return false;
}

bool KConfig::setLocale(const QString &locale)
{
    Q_D(KConfig);
    if (d->setLocale(locale)) {
        reparseConfiguration();
        return true;
    }
    return false;
}

void KConfig::setReadDefaults(bool b)
{
    Q_D(KConfig);
    d->bReadDefaults = b;
}

bool KConfig::readDefaults() const
{
    Q_D(const KConfig);
    return d->bReadDefaults;
}

bool KConfig::isImmutable() const
{
    Q_D(const KConfig);
    return d->bFileImmutable;
}

bool KConfig::isGroupImmutableImpl(const QByteArray &aGroup) const
{
    Q_D(const KConfig);
    return isImmutable() || d->entryMap.getEntryOption(aGroup, {}, {}, KEntryMap::EntryImmutable);
}

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(4, 0)
void KConfig::setForceGlobal(bool b)
{
    Q_D(KConfig);
    d->bForceGlobal = b;
}
#endif

#if KCONFIGCORE_BUILD_DEPRECATED_SINCE(4, 0)
bool KConfig::forceGlobal() const
{
    Q_D(const KConfig);
    return d->bForceGlobal;
}
#endif

KConfigGroup KConfig::groupImpl(const QByteArray &group)
{
    return KConfigGroup(this, group.constData());
}

const KConfigGroup KConfig::groupImpl(const QByteArray &group) const
{
    return KConfigGroup(this, group.constData());
}

KEntryMap::EntryOptions convertToOptions(KConfig::WriteConfigFlags flags)
{
    KEntryMap::EntryOptions options = {};

    if (flags & KConfig::Persistent) {
        options |= KEntryMap::EntryDirty;
    }
    if (flags & KConfig::Global) {
        options |= KEntryMap::EntryGlobal;
    }
    if (flags & KConfig::Localized) {
        options |= KEntryMap::EntryLocalized;
    }
    if (flags.testFlag(KConfig::Notify)) {
        options |= KEntryMap::EntryNotify;
    }
    return options;
}

void KConfig::deleteGroupImpl(const QByteArray &aGroup, WriteConfigFlags flags)
{
    Q_D(KConfig);
    KEntryMap::EntryOptions options = convertToOptions(flags) | KEntryMap::EntryDeleted;

    const QSet<QByteArray> groups = d->allSubGroups(aGroup);
    for (const QByteArray &group : groups) {
        const QStringList keys = d->keyListImpl(group);
        for (const QString &_key : keys) {
            const QByteArray &key = _key.toUtf8();
            if (d->canWriteEntry(group, key.constData())) {
                d->entryMap.setEntry(group, key, QByteArray(), options);
                d->bDirty = true;
            }
        }
    }
}

bool KConfig::isConfigWritable(bool warnUser)
{
    Q_D(KConfig);
    bool allWritable = (d->mBackend ? d->mBackend->isWritable() : false);

    if (warnUser && !allWritable) {
        QString errorMsg;
        if (d->mBackend) { // TODO how can be it be null? Set errorMsg appropriately
            errorMsg = d->mBackend->nonWritableErrorMessage();
        }

        // Note: We don't ask the user if we should not ask this question again because we can't save the answer.
        errorMsg += QCoreApplication::translate("KConfig", "Please contact your system administrator.");
        QString cmdToExec = QStandardPaths::findExecutable(QStringLiteral("kdialog"));
        if (!cmdToExec.isEmpty()) {
            QProcess::execute(cmdToExec, QStringList{QStringLiteral("--title"), QCoreApplication::applicationName(), QStringLiteral("--msgbox"), errorMsg});
        }
    }

    d->configState = allWritable ? ReadWrite : ReadOnly; // update the read/write status

    return allWritable;
}

bool KConfig::hasGroupImpl(const QByteArray &aGroup) const
{
    Q_D(const KConfig);

    // No need to look for the actual group entry anymore, or for subgroups:
    // a group exists if it contains any non-deleted entry.

    return d->hasNonDeletedEntries(aGroup);
}

bool KConfigPrivate::canWriteEntry(const QByteArray &group, const char *key, bool isDefault) const
{
    if (bFileImmutable || entryMap.getEntryOption(group, key, KEntryMap::SearchLocalized, KEntryMap::EntryImmutable)) {
        return isDefault;
    }
    return true;
}

void KConfigPrivate::putData(const QByteArray &group, const char *key, const QByteArray &value, KConfigBase::WriteConfigFlags flags, bool expand)
{
    KEntryMap::EntryOptions options = convertToOptions(flags);

    if (bForceGlobal) {
        options |= KEntryMap::EntryGlobal;
    }
    if (expand) {
        options |= KEntryMap::EntryExpansion;
    }

    if (value.isNull()) { // deleting entry
        options |= KEntryMap::EntryDeleted;
    }

    bool dirtied = entryMap.setEntry(group, key, value, options);
    if (dirtied && (flags & KConfigBase::Persistent)) {
        bDirty = true;
    }
}

void KConfigPrivate::revertEntry(const QByteArray &group, const char *key, KConfigBase::WriteConfigFlags flags)
{
    KEntryMap::EntryOptions options = convertToOptions(flags);

    bool dirtied = entryMap.revertEntry(group, key, options);
    if (dirtied) {
        bDirty = true;
    }
}

QByteArray KConfigPrivate::lookupData(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags) const
{
    return lookupInternalEntry(group, key, flags).mValue;
}

KEntry KConfigPrivate::lookupInternalEntry(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags) const
{
    if (bReadDefaults) {
        flags |= KEntryMap::SearchDefaults;
    }
    const auto it = entryMap.constFindEntry(group, key, flags);
    if (it == entryMap.constEnd()) {
        return {};
    }
    return it.value();
}

QString KConfigPrivate::lookupData(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags, bool *expand) const
{
    if (bReadDefaults) {
        flags |= KEntryMap::SearchDefaults;
    }
    return entryMap.getEntry(group, key, QString(), flags, expand);
}

QStandardPaths::StandardLocation KConfig::locationType() const
{
    Q_D(const KConfig);
    return d->resourceType;
}

void KConfig::virtual_hook(int /*id*/, void * /*data*/)
{
    /* nothing */
}
