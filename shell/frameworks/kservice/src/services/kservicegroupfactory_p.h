/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSERVICEGROUPFACTORY_P_H
#define KSERVICEGROUPFACTORY_P_H

#include "kservicegroup.h"
#include "ksycocafactory_p.h"
#include <assert.h>

class KSycoca;
class KSycocaDict;

/**
 * @internal
 * A sycoca factory for service groups (e.g. list of applications)
 * It loads the services from parsing directories (e.g. share/applications/)
 *
 * Exported for kbuildsycoca, but not installed.
 */
class KServiceGroupFactory : public KSycocaFactory
{
    K_SYCOCAFACTORY(KST_KServiceGroupFactory)
public:
    /**
     * Create factory
     */
    explicit KServiceGroupFactory(KSycoca *db);
    ~KServiceGroupFactory() override;

    /**
     * Construct a KServiceGroup from a config file.
     */
    KSycocaEntry *createEntry(const QString &) const override
    {
        assert(0);
        return nullptr;
    }

    /**
     * Find a group ( by desktop path, e.g. "Applications/Editors")
     */
    virtual KServiceGroup::Ptr findGroupByDesktopPath(const QString &_name, bool deep = true);

    /**
     * Find a base group by name, e.g. "settings"
     */
    KServiceGroup::Ptr findBaseGroup(const QString &_baseGroupName, bool deep = true);

    /**
     * @return the unique service group factory, creating it if necessary
     */
    static KServiceGroupFactory *self();

protected:
    KServiceGroup *createGroup(int offset, bool deep) const;
    KServiceGroup *createEntry(int offset) const override;
    KSycocaDict *m_baseGroupDict;
    int m_baseGroupDictOffset;

protected:
    void virtual_hook(int id, void *data) override;

private:
    class KServiceGroupFactoryPrivate *d;
};

#endif
