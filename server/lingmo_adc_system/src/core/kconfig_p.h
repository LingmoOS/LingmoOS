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

#include "kconfigbackend_p.h"
#include "kconfigdata_p.h"
#include "kconfiggroup.h"

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
    bool canWriteEntry(const QByteArray &group, const char *key, bool isDefault = false) const;
    QString lookupData(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags, bool *expand) const;
    QByteArray lookupData(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags) const;
    KEntry lookupInternalEntry(const QByteArray &group, const char *key, KEntryMap::SearchFlags flags) const;

    void putData(const QByteArray &group, const char *key, const QByteArray &value, KConfigBase::WriteConfigFlags flags, bool expand = false);
    void setEntryData(const QByteArray &group, const char *key, const QByteArray &value, KEntryMap::EntryOptions flags)
    {
        if (entryMap.setEntry(group, key, value, flags)) {
            bDirty = true;
        }
    }
    void revertEntry(const QByteArray &group, const char *key, KConfigBase::WriteConfigFlags flags);
    QStringList groupList(const QByteArray &group) const;
    // copies the entries from @p source to @p otherGroup changing all occurrences
    // of @p source with @p destination
    void copyGroup(const QByteArray &source, const QByteArray &destination, KConfigGroup *otherGroup, KConfigBase::WriteConfigFlags flags) const;
    QStringList keyListImpl(const QByteArray &theGroup) const;
    QSet<QByteArray> allSubGroups(const QByteArray &parentGroup) const;
    bool hasNonDeletedEntries(const QByteArray &group) const;

    void notifyClients(const QHash<QString, QByteArrayList> &changes, const QString &path);

    static QString expandString(const QString &value);

protected:
    QExplicitlySharedDataPointer<KConfigBackend> mBackend;

    KConfigPrivate(KConfig::OpenFlags flags, QStandardPaths::StandardLocation type);

    virtual ~KConfigPrivate()
    {
    }

    bool bDynamicBackend : 1; // do we own the backend?
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
