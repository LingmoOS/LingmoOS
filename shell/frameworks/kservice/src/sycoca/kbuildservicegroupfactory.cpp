/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbuildservicegroupfactory_p.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "sycocadebug.h"
#include <kservicegroup_p.h>

#include <QDebug>
#include <QHash>
#include <QIODevice>
#include <assert.h>

KBuildServiceGroupFactory::KBuildServiceGroupFactory(KSycoca *db)
    : KServiceGroupFactory(db)
{
    m_baseGroupDict = new KSycocaDict();
}

KBuildServiceGroupFactory::~KBuildServiceGroupFactory()
{
}

KServiceGroup *KBuildServiceGroupFactory::createEntry(const QString &) const
{
    // Unused
    qCWarning(SYCOCA) << "called!";
    return nullptr;
}

void KBuildServiceGroupFactory::addNewEntryTo(const QString &menuName, const KService::Ptr &newEntry)
{
    KSycocaEntry::Ptr ptr = m_entryDict->value(menuName);
    KServiceGroup::Ptr entry;
    if (ptr && ptr->isType(KST_KServiceGroup)) {
        entry = KServiceGroup::Ptr(static_cast<KServiceGroup *>(ptr.data()));
    }

    if (!entry) {
        qCWarning(SYCOCA) << "( " << menuName << ", " << newEntry->name() << " ): menu does not exists!";
        return;
    }
    entry->addEntry(KSycocaEntry::Ptr(newEntry));
}

KServiceGroup::Ptr KBuildServiceGroupFactory::addNew(const QString &menuName, const QString &file, KServiceGroup::Ptr entry, bool isDeleted)
{
    KSycocaEntry::Ptr ptr = m_entryDict->value(menuName);
    if (ptr) {
        qCWarning(SYCOCA) << "( " << menuName << ", " << file << " ): menu already exists!";
        return KServiceGroup::Ptr(static_cast<KServiceGroup *>(ptr.data()));
    }

    // Create new group entry
    if (!entry) {
        entry = new KServiceGroup(file, menuName);
    }

    entry->d_func()->m_childCount = -1; // Recalculate

    addEntry(KSycocaEntry::Ptr(entry));

    if (menuName != QLatin1String("/")) {
        // Make sure parent dir exists.
        QString parent = menuName.left(menuName.length() - 1);
        int i = parent.lastIndexOf(QLatin1Char('/'));
        if (i > 0) {
            parent = parent.left(i + 1);
        } else {
            parent = QLatin1Char('/');
        }

        KServiceGroup::Ptr parentEntry;
        ptr = m_entryDict->value(parent);
        if (ptr && ptr->isType(KST_KServiceGroup)) {
            parentEntry = KServiceGroup::Ptr(static_cast<KServiceGroup *>(ptr.data()));
        }
        if (!parentEntry) {
            qCWarning(SYCOCA) << "( " << menuName << ", " << file << " ): parent menu does not exist!";
        } else {
            if (!isDeleted && !entry->isDeleted()) {
                parentEntry->addEntry(KSycocaEntry::Ptr(entry));
            }
        }
    }
    return entry;
}

void KBuildServiceGroupFactory::addNewChild(const QString &parent, const KSycocaEntry::Ptr &newEntry)
{
    QString name = QLatin1String("#parent#") + parent;

    KServiceGroup::Ptr entry;
    KSycocaEntry::Ptr ptr = m_entryDict->value(name);
    if (ptr && ptr->isType(KST_KServiceGroup)) {
        entry = KServiceGroup::Ptr(static_cast<KServiceGroup *>(ptr.data()));
    }

    if (!entry) {
        entry = new KServiceGroup(name);
        addEntry(KSycocaEntry::Ptr(entry));
    }
    if (newEntry) {
        entry->addEntry(newEntry);
    }
}

void KBuildServiceGroupFactory::addEntry(const KSycocaEntry::Ptr &newEntry)
{
    KSycocaFactory::addEntry(newEntry);

    KServiceGroup::Ptr serviceGroup(static_cast<KServiceGroup *>(newEntry.data()));
    serviceGroup->d_func()->m_serviceList.clear();

    if (!serviceGroup->baseGroupName().isEmpty()) {
        m_baseGroupDict->add(serviceGroup->baseGroupName(), newEntry);
    }
}

void KBuildServiceGroupFactory::saveHeader(QDataStream &str)
{
    KSycocaFactory::saveHeader(str);

    str << qint32(m_baseGroupDictOffset);
}

void KBuildServiceGroupFactory::save(QDataStream &str)
{
    KSycocaFactory::save(str);

    m_baseGroupDictOffset = str.device()->pos();
    m_baseGroupDict->save(str);

    qint64 endOfFactoryData = str.device()->pos();

    // Update header (pass #3)
    saveHeader(str);

    // Seek to end.
    str.device()->seek(endOfFactoryData);
}

KServiceGroup::Ptr KBuildServiceGroupFactory::findGroupByDesktopPath(const QString &_name, bool deep)
{
    assert(sycoca()->isBuilding());
    Q_UNUSED(deep); // ?
    // We're building a database - the service type must be in memory
    KSycocaEntry::Ptr group = m_entryDict->value(_name);
    return KServiceGroup::Ptr(static_cast<KServiceGroup *>(group.data()));
}
