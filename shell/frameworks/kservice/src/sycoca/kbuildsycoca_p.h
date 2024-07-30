/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KBUILDSYCOCA_H
#define KBUILDSYCOCA_H

#include "kbuildsycocainterface_p.h"

#include <kservice.h>
#include <ksycoca.h>

#include "vfolder_menu_p.h"

class KBuildServiceGroupFactory;
class QDataStream;
class KCTimeFactory;
class KCTimeDict;

/**
 * @internal
 * Exported for kbuildsycoca, but not installed.
 */
class KSERVICE_EXPORT KBuildSycoca : public KSycoca, public KBuildSycocaInterface
{
    Q_OBJECT
public:
    explicit KBuildSycoca();
    ~KBuildSycoca() override;

    /**
     * Recreate the database file.
     * @return true if it was indeed recreated (by us or possibly by someone else), false on error
     */
    bool recreate(bool incremental = true);

    void setTrackId(const QString &id)
    {
        m_trackId = id;
    }

    void setMenuTest(bool b)
    {
        m_menuTest = b;
    }

    static QStringList factoryResourceDirs();
    static QStringList factoryExtraFiles();
    static QStringList existingResourceDirs();

    /**
     * Returns a number that identifies the current version of the file @p filename,
     * which is located under GenericDataLocation (including local overrides).
     *
     * When a change is made to the file this number will change.
     */
    static quint32 calcResourceHash(const QString &subdir, const QString &filename);

    /**
     * Compare our current settings (language, prefixes...) with the ones from the existing ksycoca global header.
     * @return true if they match (= we can reuse this ksycoca), false otherwise (full build)
     */
    bool checkGlobalHeader();

    /**
     * @brief path to the sycoca file, for the crash handler in kbuildsycoca
     */
    static const char *sycocaPath();

private:
    /**
     * Add single entry to the sycoca database.
     * Either from a previous database or regenerated from file.
     */
    KSERVICE_NO_EXPORT KSycocaEntry::Ptr createEntry(KSycocaFactory *currentFactory, const QString &file);

    /**
     * Implementation of KBuildSycocaInterface
     * Create service and return it. The caller must add it to the servicefactory.
     */
    KService::Ptr createService(const QString &path) override;

    /**
     * Convert a VFolderMenu::SubMenu to KServiceGroups.
     */
    KSERVICE_NO_EXPORT void createMenu(const QString &caption, const QString &name, VFolderMenu::SubMenu *menu);

    /**
     * Build the whole system cache, from .desktop files
     */
    KSERVICE_NO_EXPORT bool build();

    /**
     * Save the ksycoca file
     */
    KSERVICE_NO_EXPORT void save(QDataStream *str);

    /**
     * Clear the factories
     */
    KSERVICE_NO_EXPORT void clear();

    /**
     * @internal
     * @return true if building (i.e. if a KBuildSycoca);
     */
    bool isBuilding() override
    {
        return true;
    }

    QMap<QString, qint64> m_allResourceDirs; // dir, mtime in ms since epoch
    QMap<QString, qint64> m_extraFiles; // file, mtime in ms since epoch
    QString m_trackId;

    QByteArray m_resource; // e.g. "services" (old resource name, now only used for the signal, see kctimefactory.cpp)
    QString m_resourceSubdir; // e.g. "mime" (xdgdata subdir)

    KSycocaEntry::List m_tempStorage;
    typedef QList<KSycocaEntry::List> KSycocaEntryListList;
    KSycocaEntryListList *m_allEntries; // entries from existing ksycoca
    KBuildServiceGroupFactory *m_buildServiceGroupFactory = nullptr;
    KCTimeFactory *m_ctimeFactory = nullptr;
    KCTimeDict *m_ctimeDict; // old timestamps
    typedef QHash<QString, KSycocaEntry::Ptr> KBSEntryDict;
    KBSEntryDict *m_currentEntryDict = nullptr;
    KBSEntryDict *m_serviceGroupEntryDict = nullptr;
    VFolderMenu *m_vfolder = nullptr;
    qint64 m_newTimestamp;

    bool m_menuTest;
    bool m_changed;
};

#endif
