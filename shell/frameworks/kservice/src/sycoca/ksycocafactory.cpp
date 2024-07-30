/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocaentry.h"
#include "ksycocaentry_p.h"
#include "ksycocafactory_p.h"
#include "ksycocatype.h"
#include "sycocadebug.h"

#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <QThread>

class KSycocaFactoryPrivate
{
public:
    KSycocaFactoryPrivate()
    {
    }
    ~KSycocaFactoryPrivate()
    {
        delete m_sycocaDict;
    }

    int mOffset = 0;
    int m_sycocaDictOffset = 0;
    int m_beginEntryOffset = 0;
    int m_endEntryOffset = 0;
    KSycocaDict *m_sycocaDict = nullptr;
};

KSycocaFactory::KSycocaFactory(KSycocaFactoryId factory_id, KSycoca *sycoca)
    : m_sycoca(sycoca)
    , d(new KSycocaFactoryPrivate)
{
    if (!m_sycoca->isBuilding() && (m_str = m_sycoca->findFactory(factory_id))) {
        // Read position of index tables....
        qint32 i;
        (*m_str) >> i;
        d->m_sycocaDictOffset = i;
        (*m_str) >> i;
        d->m_beginEntryOffset = i;
        (*m_str) >> i;
        d->m_endEntryOffset = i;

        QDataStream *str = stream();
        qint64 saveOffset = str->device()->pos();
        // Init index tables
        d->m_sycocaDict = new KSycocaDict(str, d->m_sycocaDictOffset);
        saveOffset = str->device()->seek(saveOffset);
    } else {
        // We are in kbuildsycoca -- build new database!
        m_entryDict = new KSycocaEntryDict;
        d->m_sycocaDict = new KSycocaDict;
        d->m_beginEntryOffset = 0;
        d->m_endEntryOffset = 0;

        // m_resourceList will be filled in by inherited constructors
    }
    m_sycoca->addFactory(this);
}

KSycocaFactory::~KSycocaFactory()
{
    delete m_entryDict;
}

void KSycocaFactory::saveHeader(QDataStream &str)
{
    // Write header
    str.device()->seek(d->mOffset);
    str << qint32(d->m_sycocaDictOffset);
    str << qint32(d->m_beginEntryOffset);
    str << qint32(d->m_endEntryOffset);
}

void KSycocaFactory::save(QDataStream &str)
{
    if (!m_entryDict) {
        return; // Error! Function should only be called when building database
    }
    if (!d->m_sycocaDict) {
        return; // Error!
    }

    d->mOffset = str.device()->pos(); // store position in member variable
    d->m_sycocaDictOffset = 0;

    // Write header (pass #1)
    saveHeader(str);

    d->m_beginEntryOffset = str.device()->pos();

    // Write all entries.
    int entryCount = 0;
    for (KSycocaEntry::Ptr entry : std::as_const(*m_entryDict)) {
        entry->d_ptr->save(str);
        entryCount++;
    }

    d->m_endEntryOffset = str.device()->pos();

    // Write indices...
    // Linear index
    str << qint32(entryCount);
    for (const KSycocaEntry::Ptr &entry : std::as_const(*m_entryDict)) {
        str << qint32(entry.data()->offset());
    }

    // Dictionary index
    d->m_sycocaDictOffset = str.device()->pos();
    d->m_sycocaDict->save(str);

    qint64 endOfFactoryData = str.device()->pos();

    // Update header (pass #2)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

void KSycocaFactory::addEntry(const KSycocaEntry::Ptr &newEntry)
{
    if (!m_entryDict) {
        return; // Error! Function should only be called when
    }
    // building database

    if (!d->m_sycocaDict) {
        return; // Error!
    }

    KSycocaEntry::Ptr oldEntry = m_entryDict->value(newEntry->storageId());
    if (oldEntry) {
        // Already exists -> replace
        // We found a more-local override, e.g. ~/.local/share/applications/kde5/foo.desktop
        // So forget about the more global file.
        //
        // This can also happen with two .protocol files using the same protocol= entry.
        // If we didn't remove one here, we would end up asserting because save()
        // wasn't called on one of the entries.
        // qDebug() << "removing" << oldEntry.data() << oldEntry->entryPath() << "because of" << newEntry->entryPath() << "they have the same storageId" <<
        // newEntry->storageId();
        removeEntry(newEntry->storageId());
    }

    const QString name = newEntry->storageId();
    m_entryDict->insert(name, newEntry);
    d->m_sycocaDict->add(name, newEntry);
}

void KSycocaFactory::removeEntry(const QString &entryName)
{
    if (!m_entryDict) {
        return; // Error! Function should only be called when
    }
    // building database

    if (!d->m_sycocaDict) {
        return; // Error!
    }

    m_entryDict->remove(entryName);
    d->m_sycocaDict->remove(entryName); // O(N)
}

KSycocaEntry::List KSycocaFactory::allEntries() const
{
    KSycocaEntry::List list;

    // Assume we're NOT building a database

    QDataStream *str = stream();
    if (!str) {
        return list;
    }
    str->device()->seek(d->m_endEntryOffset);
    qint32 entryCount;
    (*str) >> entryCount;

    if (entryCount > 8192) {
        qCWarning(SYCOCA) << QThread::currentThread() << "error detected in factory" << this;
        KSycoca::flagError();
        return list;
    }

    // offsetList is needed because createEntry() modifies the stream position
    qint32 *offsetList = new qint32[entryCount];
    for (int i = 0; i < entryCount; i++) {
        (*str) >> offsetList[i];
    }

    for (int i = 0; i < entryCount; i++) {
        KSycocaEntry *newEntry = createEntry(offsetList[i]);
        if (newEntry) {
            list.append(KSycocaEntry::Ptr(newEntry));
        }
    }
    delete[] offsetList;
    return list;
}

int KSycocaFactory::offset() const
{
    return d->mOffset;
}

const KSycocaResourceList &KSycocaFactory::resourceList() const
{
    return m_resourceList;
}

const KSycocaDict *KSycocaFactory::sycocaDict() const
{
    return d->m_sycocaDict;
}

bool KSycocaFactory::isEmpty() const
{
    return d->m_beginEntryOffset == d->m_endEntryOffset;
}

QDataStream *KSycocaFactory::stream() const
{
    return m_str;
}

QStringList KSycocaFactory::allDirectories(const QString &subdir)
{
    // We don't use QStandardPaths::locateAll() because we want all paths, even those that don't exist yet
    QStringList topDirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (auto &dir : topDirs) {
        dir += QLatin1Char('/') + subdir;
    }
    return topDirs;
}

void KSycocaFactory::virtual_hook(int /*id*/, void * /*data*/)
{
    /*BASE::virtual_hook( id, data );*/
}
