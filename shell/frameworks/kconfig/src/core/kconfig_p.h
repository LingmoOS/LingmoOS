/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
    SPDX-FileCopyrightText: 2001 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIG_P_H
#define KCONFIG_P_H

#include "kconfig.h"
#include "kconfigdata_p.h"
#include "kconfiggroup.h"
#include "kconfigini_p.h"

#include <QDir>
#include <QFile>
#include <QStack>
#include <QStringList>

class KConfigPrivate
{
    friend class KConfig;

public:
    KConfig::OpenFlags openFlags;
    QStandardPaths::StandardLocation resourceType;

    void changeFileName(const QString &fileName);

    // functions for KConfigGroup
    bool canWriteEntry(const QString &group, QAnyStringView key, bool isDefault = false) const;
    QString lookupData(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags, bool *expand) const;
    QByteArray lookupData(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags) const;
    KEntry lookupInternalEntry(const QString &group, QAnyStringView key, KEntryMap::SearchFlags flags) const;

    void putData(const QString &groupName, const char *key, const QByteArray &value, KConfigBase::WriteConfigFlags flags, bool expand = false);
    void setEntryData(const QString &groupName, const char *key, const QByteArray &value, KEntryMap::EntryOptions flags)
    {
        if (entryMap.setEntry(groupName, key, value, flags)) {
            bDirty = true;
        }
    }
    void revertEntry(const QString &group, QAnyStringView key, KConfigBase::WriteConfigFlags flags);
    /**
     * Returns a list of keys used with entries in a particular group.
     *
     * This does not include keys of deleted entries or those with default values.
     * I.e. all the keys for which there will be also entries in KConfig::entryMap().
     *
     * @return A sorted list of keys in the group specified.
     *         The returned list may be empty if the group is empty, or not found.
     */
    QStringList usedKeyList(const QString &theGroup) const;
    QStringList groupList(const QString &groupName) const;
    // copies the entries from @p source to @p otherGroup changing all occurrences
    // of @p source with @p destination
    void copyGroup(const QString &source, const QString &destination, KConfigGroup *otherGroup, KConfigBase::WriteConfigFlags flags) const;
    QList<QByteArray> keyListImpl(const QString &groupName) const;
    QSet<QString> allSubGroups(const QString &parentGroupName) const;
    bool hasNonDeletedEntries(const QString &groupName) const;

    void notifyClients(const QHash<QString, QByteArrayList> &changes, const QString &path);

    static QString expandString(const QString &value);

protected:
    QExplicitlySharedDataPointer<KConfigIniBackend> mBackend;

    KConfigPrivate(KConfig::OpenFlags flags, QStandardPaths::StandardLocation type);

    virtual ~KConfigPrivate()
    {
    }

private:
    bool bDirty : 1;
    bool bReadDefaults : 1;
    bool bFileImmutable : 1;
    bool bForceGlobal : 1;
    bool bSuppressGlobal : 1;

    static bool mappingsRegistered;

    KEntryMap entryMap;
    QString backendType;
    QStack<QString> extraFiles;

    QString locale;
    QString fileName;
    QString etc_kderc;
    KConfigBase::AccessMode configState;

    bool wantGlobals() const
    {
        return openFlags & KConfig::IncludeGlobals && !bSuppressGlobal;
    }
    bool wantDefaults() const
    {
        return openFlags & KConfig::CascadeConfig;
    }
    bool isSimple() const
    {
        return openFlags == KConfig::SimpleConfig;
    }
    bool isReadOnly() const
    {
        return configState == KConfig::ReadOnly;
    }

    bool setLocale(const QString &aLocale);
    QStringList getGlobalFiles() const;
    void parseGlobalFiles();
    void parseConfigFiles();
    void initCustomized(KConfig *);
    bool lockLocal();
};

#endif // KCONFIG_P_H
