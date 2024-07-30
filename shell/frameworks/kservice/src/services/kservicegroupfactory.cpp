/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kservice.h"
#include "kservicegroupfactory_p.h"
#include "ksycoca.h"
#include "ksycocadict_p.h"
#include "ksycocatype.h"

#include "servicesdebug.h"

#include <QIODevice>

KServiceGroupFactory::KServiceGroupFactory(KSycoca *db)
    : KSycocaFactory(KST_KServiceGroupFactory, db)
    , m_baseGroupDict(nullptr)
    , m_baseGroupDictOffset(0)
{
    if (!sycoca()->isBuilding()) {
        QDataStream *str = stream();
        if (!str) {
            return;
        }
        // Read Header
        qint32 i;
        (*str) >> i;
        m_baseGroupDictOffset = i;

        const qint64 saveOffset = str->device()->pos();
        // Init index tables
        m_baseGroupDict = new KSycocaDict(str, m_baseGroupDictOffset);
        str->device()->seek(saveOffset);
    }
}

KServiceGroupFactory::~KServiceGroupFactory()
{
    delete m_baseGroupDict;
}

KServiceGroup::Ptr KServiceGroupFactory::findGroupByDesktopPath(const QString &_name, bool deep)
{
    if (!sycocaDict()) {
        return KServiceGroup::Ptr(); // Error!
    }
    int offset = sycocaDict()->find_string(_name);
    if (!offset) {
        return KServiceGroup::Ptr(); // Not found
    }

    KServiceGroup::Ptr newGroup(createGroup(offset, deep));

    // Check whether the dictionary was right.
    if (newGroup && (newGroup->relPath() != _name)) {
        // No it wasn't...
        newGroup = nullptr; // Not found
    }
    return newGroup;
}

KServiceGroup::Ptr KServiceGroupFactory::findBaseGroup(const QString &_baseGroupName, bool deep)
{
    if (!m_baseGroupDict) {
        return KServiceGroup::Ptr(); // Error!
    }

    // Warning : this assumes we're NOT building a database
    // But since findBaseGroup isn't called in that case...
    // [ see KServiceTypeFactory for how to do it if needed ]

    int offset = m_baseGroupDict->find_string(_baseGroupName);
    if (!offset) {
        return KServiceGroup::Ptr(); // Not found
    }

    KServiceGroup::Ptr newGroup(createGroup(offset, deep));

    // Check whether the dictionary was right.
    if (newGroup && (newGroup->baseGroupName() != _baseGroupName)) {
        // No it wasn't...
        newGroup = nullptr; // Not found
    }
    return newGroup;
}

KServiceGroup *KServiceGroupFactory::createGroup(int offset, bool deep) const
{
    KSycocaType type;
    QDataStream *str = sycoca()->findEntry(offset, type);
    if (type != KST_KServiceGroup) {
        qCWarning(SERVICES) << "KServiceGroupFactory: unexpected object entry in KSycoca database (type = " << int(type) << ")";
        return nullptr;
    }

    KServiceGroup *newEntry = new KServiceGroup(*str, offset, deep);
    if (!newEntry->isValid()) {
        qCWarning(SERVICES) << "KServiceGroupFactory: corrupt object in KSycoca database!";
        delete newEntry;
        newEntry = nullptr;
    }
    return newEntry;
}

KServiceGroup *KServiceGroupFactory::createEntry(int offset) const
{
    return createGroup(offset, true);
}

void KServiceGroupFactory::virtual_hook(int id, void *data)
{
    KSycocaFactory::virtual_hook(id, data);
}
