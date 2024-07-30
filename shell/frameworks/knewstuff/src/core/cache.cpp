/*
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>
    SPDX-FileCopyrightText: 2010 Matthias Fuchs <mat69@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "cache.h"

#include <QDir>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QPointer>
#include <QTimer>
#include <QXmlStreamReader>
#include <knewstuffcore_debug.h>
#include <qstandardpaths.h>

class KNSCore::CachePrivate
{
public:
    CachePrivate(Cache *qq)
        : q(qq)
    {
    }
    ~CachePrivate()
    {
    }

    Cache *q;
    QHash<QString, Entry::List> requestCache;

    QPointer<QTimer> throttleTimer;

    // The file that is used to keep track of downloaded entries
    QString registryFile;

    QSet<Entry> cache;

    bool dirty = false;
    bool writingRegistry = false;
    bool reloadingRegistry = false;

    void throttleWrite()
    {
        if (!throttleTimer) {
            throttleTimer = new QTimer(q);
            QObject::connect(throttleTimer, &QTimer::timeout, q, [this]() {
                q->writeRegistry();
            });
            throttleTimer->setSingleShot(true);
            throttleTimer->setInterval(1000);
        }
        throttleTimer->start();
    }
};

using namespace KNSCore;

typedef QHash<QString, QWeakPointer<Cache>> CacheHash;
Q_GLOBAL_STATIC(CacheHash, s_caches)
Q_GLOBAL_STATIC(QFileSystemWatcher, s_watcher)

Cache::Cache(const QString &appName)
    : QObject(nullptr)
    , d(new CachePrivate(this))
{
    const QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/");
    QDir().mkpath(path);
    d->registryFile = path + appName + QStringLiteral(".knsregistry");
    qCDebug(KNEWSTUFFCORE) << "Using registry file: " << d->registryFile;

    s_watcher->addPath(d->registryFile);

    std::function<void()> changeChecker = [this, &changeChecker]() {
        if (d->writingRegistry) {
            QTimer::singleShot(0, this, changeChecker);
        } else {
            d->reloadingRegistry = true;
            const QSet<KNSCore::Entry> oldCache = d->cache;
            d->cache.clear();
            readRegistry();
            // First run through the old cache and see if any have disappeared (at
            // which point we need to set them as available and emit that change)
            for (const Entry &entry : oldCache) {
                if (!d->cache.contains(entry) && entry.status() != KNSCore::Entry::Deleted) {
                    Entry removedEntry(entry);
                    removedEntry.setEntryDeleted();
                    Q_EMIT entryChanged(removedEntry);
                }
            }
            // Then run through the new cache and see if there's any that were not
            // in the old cache (at which point just emit those as having changed,
            // they're already the correct status)
            for (const Entry &entry : std::as_const(d->cache)) {
                auto iterator = oldCache.constFind(entry);
                if (iterator == oldCache.constEnd()) {
                    Q_EMIT entryChanged(entry);
                } else if ((*iterator).status() != entry.status()) {
                    // If there are entries which are in both, but which have changed their
                    // status, we should adopt the status from the newly loaded cache in place
                    // of the one in the old cache. In reality, what this means is we just
                    // need to emit the changed signal for anything in the new cache which
                    // doesn't match the old one
                    Q_EMIT entryChanged(entry);
                }
            }
            d->reloadingRegistry = false;
        }
    };
    connect(&*s_watcher, &QFileSystemWatcher::fileChanged, this, [this, changeChecker](const QString &file) {
        if (file == d->registryFile) {
            changeChecker();
        }
    });
}

QSharedPointer<Cache> Cache::getCache(const QString &appName)
{
    CacheHash::const_iterator it = s_caches()->constFind(appName);
    if ((it != s_caches()->constEnd()) && !(*it).isNull()) {
        return QSharedPointer<Cache>(*it);
    }

    QSharedPointer<Cache> p(new Cache(appName));
    s_caches()->insert(appName, QWeakPointer<Cache>(p));
    QObject::connect(p.data(), &QObject::destroyed, [appName] {
        if (auto cache = s_caches()) {
            cache->remove(appName);
        }
    });

    return p;
}

Cache::~Cache()
{
    s_watcher->removePath(d->registryFile);
}

void Cache::readRegistry()
{
    QFile f(d->registryFile);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (QFileInfo::exists(d->registryFile)) {
            qWarning() << "The file " << d->registryFile << " could not be opened.";
        }
        return;
    }

    QXmlStreamReader reader(&f);
    if (reader.hasError() || !reader.readNextStartElement()) {
        qCWarning(KNEWSTUFFCORE) << "The file could not be parsed.";
        return;
    }

    if (reader.name() != QLatin1String("hotnewstuffregistry")) {
        qCWarning(KNEWSTUFFCORE) << "The file doesn't seem to be of interest.";
        return;
    }

    for (auto token = reader.readNext(); !reader.atEnd(); token = reader.readNext()) {
        if (token != QXmlStreamReader::StartElement) {
            continue;
        }
        Entry e;
        e.setEntryXML(reader);
        e.setSource(Entry::Cache);
        d->cache.insert(e);
        Q_ASSERT(reader.tokenType() == QXmlStreamReader::EndElement);
    }

    qCDebug(KNEWSTUFFCORE) << "Cache read... entries: " << d->cache.size();
}

Entry::List Cache::registryForProvider(const QString &providerId)
{
    Entry::List entries;
    for (const Entry &e : std::as_const(d->cache)) {
        if (e.providerId() == providerId) {
            entries.append(e);
        }
    }
    return entries;
}

Entry::List Cache::registry() const
{
    Entry::List entries;
    for (const Entry &e : std::as_const(d->cache)) {
        entries.append(e);
    }
    return entries;
}

void Cache::writeRegistry()
{
    if (!d->dirty) {
        return;
    }

    qCDebug(KNEWSTUFFCORE) << "Write registry";

    d->writingRegistry = true;
    QFile f(d->registryFile);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot write meta information to" << d->registryFile;
        return;
    }

    QDomDocument doc(QStringLiteral("khotnewstuff3"));
    doc.appendChild(doc.createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")));
    QDomElement root = doc.createElement(QStringLiteral("hotnewstuffregistry"));
    doc.appendChild(root);

    for (const Entry &entry : std::as_const(d->cache)) {
        // Write the entry, unless the policy is CacheNever and the entry is not installed.
        if (entry.status() == KNSCore::Entry::Installed || entry.status() == KNSCore::Entry::Updateable) {
            QDomElement exml = entry.entryXML();
            root.appendChild(exml);
        }
    }

    QTextStream metastream(&f);
    metastream << doc.toByteArray();

    d->dirty = false;
    d->writingRegistry = false;
}

void Cache::registerChangedEntry(const KNSCore::Entry &entry)
{
    // If we have intermediate states, like updating or installing we do not want to write them
    if (entry.status() == KNSCore::Entry::Updating || entry.status() == KNSCore::Entry::Installing) {
        return;
    }
    if (!d->reloadingRegistry) {
        d->dirty = true;
        d->cache.remove(entry); // If value already exists in the set, the set is left unchanged
        d->cache.insert(entry);
        d->throttleWrite();
    }
}

void Cache::insertRequest(const KNSCore::Provider::SearchRequest &request, const KNSCore::Entry::List &entries)
{
    // append new entries
    auto &cacheList = d->requestCache[request.hashForRequest()];
    for (const auto &entry : entries) {
        if (!cacheList.contains(entry)) {
            cacheList.append(entry);
        }
    }
    qCDebug(KNEWSTUFFCORE) << request.hashForRequest() << " add to cache: " << entries.size() << " keys: " << d->requestCache.keys();
}

Entry::List Cache::requestFromCache(const KNSCore::Provider::SearchRequest &request)
{
    qCDebug(KNEWSTUFFCORE) << "from cache" << request.hashForRequest();
    return d->requestCache.value(request.hashForRequest());
}

void KNSCore::Cache::removeDeletedEntries()
{
    QMutableSetIterator<KNSCore::Entry> i(d->cache);
    while (i.hasNext()) {
        const KNSCore::Entry &entry = i.next();
        bool installedFileExists{false};
        const QStringList installedFiles = entry.installedFiles();
        for (const auto &installedFile : installedFiles) {
            // Handle the /* notation, BUG: 425704
            if (installedFile.endsWith(QLatin1String("/*"))) {
                if (QDir(installedFile.left(installedFile.size() - 2)).exists()) {
                    installedFileExists = true;
                    break;
                }
            } else if (QFile::exists(installedFile)) {
                installedFileExists = true;
                break;
            }
        }
        if (!installedFileExists) {
            i.remove();
            d->dirty = true;
        }
    }
    writeRegistry();
}

KNSCore::Entry KNSCore::Cache::entryFromInstalledFile(const QString &installedFile) const
{
    for (const Entry &entry : std::as_const(d->cache)) {
        if (entry.installedFiles().contains(installedFile)) {
            return entry;
        }
    }
    return Entry{};
}

#include "moc_cache.cpp"
