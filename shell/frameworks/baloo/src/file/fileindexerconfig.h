/*
    SPDX-FileCopyrightText: 2008-2009 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_FILEINDEXER_CONFIG_H_
#define BALOO_FILEINDEXER_CONFIG_H_

#include <QObject>
#include <QList>
#include <QSet>
#include <QDebug>

#include "regexpcache.h"

class BalooSettings;

namespace Baloo
{

class StorageDevices;

/**
 * Active config class which emits signals if the config
 * was changed, for example if the KCM saved the config file.
 */
class FileIndexerConfig : public QObject
{
    Q_OBJECT

public:

    explicit FileIndexerConfig(QObject* parent = nullptr);
    ~FileIndexerConfig() override;

    /**
    * Folders to search for files to index and analyze.
    * \return list of paths.
    */
    QStringList includeFolders() const;

    /**
     * Folders that are excluded from indexing.
     * (Descendant folders of an excluded folder can be added
     * and they will be indexed.)
     * \return list of paths.
     */
    QStringList excludeFolders() const;

    QStringList excludeFilters() const;

    QStringList excludeMimetypes() const;

    bool indexHiddenFilesAndFolders() const;

    bool onlyBasicIndexing() const;

    /**
    * Check if \p folder can be searched.
    * \p folder can be searched if itself or one of its descendants is indexed.
    *
    * Example:
    * if ~/foo is not indexed and ~/foo/bar is indexed
    * then ~/foo can be searched.
    *
    * \return \c true if the \p folder can be searched.
    */
    bool canBeSearched(const QString& folder) const;

    /**
     * Check if file or folder \p path should be indexed taking into account
     * the includeFolders(), the excludeFolders(), and the excludeFilters().
     * Inclusion takes precedence.
     *
     * Be aware that this method does not check if parent dirs
     * match any of the exclude filters. Only the path of the
     * dir itself it checked.
     *
     * \return \c true if the file or folder at \p path should
     * be indexed according to the configuration.
     */
    bool shouldBeIndexed(const QString& path) const;

    /**
     * Check if the folder at \p path should be indexed.
     *
     * Be aware that this method does not check if parent dirs
     * match any of the exclude filters. Only the name of the
     * dir itself it checked.
     *
     * \return \c true if the folder at \p path should
     * be indexed according to the configuration.
     */
    bool shouldFolderBeIndexed(const QString& path) const;

    /**
     * Check \p fileName for all exclude filters. This does
     * not take file paths into account.
     *
     * \return \c true if a file with name \p filename should
     * be indexed according to the configuration.
     */
    bool shouldFileBeIndexed(const QString& fileName) const;

    /**
     * Checks if \p mimeType should be indexed
     *
     * \return \c true if the mimetype should be indexed according
     * to the configuration
     */
    bool shouldMimeTypeBeIndexed(const QString& mimeType) const;

    /**
     * Check if \p path is in the list of folders to be indexed taking
     * include and exclude folders into account.
     * \p folder is set to the folder which was the reason for the decision.
     */
    bool folderInFolderList(const QString& path, QString& folder) const;

    /**
     * Returns the internal version number of the Baloo database
     */
    int databaseVersion() const;
    void setDatabaseVersion(int version);

    bool indexingEnabled() const;

    /**
      * Returns batch size
      */
    uint maxUncomittedFiles() const;

public Q_SLOTS:
    /**
     * Reread the config from disk and update the configuration cache.
     * This is only required for testing as normally the config updates
     * itself whenever the config file on disk changes.
     *
     * \return \c true if the config has actually changed
     */
    void forceConfigUpdate();

private:

    void buildFolderCache();
    void buildExcludeFilterRegExpCache();
    void buildMimeTypeCache();

    BalooSettings *m_settings;

    struct FolderConfig
    {
        QString path;
        bool isIncluded;

        /// Sort by path length, and on ties lexicographically.
        /// Longest path first
        bool operator<(const FolderConfig& other) const;
    };

    class FolderCache : public std::vector<FolderConfig>
    {
    public:
        void cleanup();

        bool addFolderConfig(const FolderConfig&);
    };
    friend QDebug operator<<(QDebug dbg, const FolderConfig& config);

    /// Caching cleaned up list (no duplicates, no non-default entries, etc.)
    FolderCache m_folderCache;
    /// Whether the folder cache needs to be rebuilt the next time it is used
    bool m_folderCacheDirty;

    /// cache of regexp objects for all exclude filters
    /// to prevent regexp parsing over and over
    RegExpCache m_excludeFilterRegExpCache;

    /// A set of mimetypes which should never be indexed
    QSet<QString> m_excludeMimetypes;

    bool m_indexHidden;
    bool m_onlyBasicIndexing;

    StorageDevices* m_devices;

    const uint m_maxUncomittedFiles;
};

QDebug operator<<(QDebug dbg, const FileIndexerConfig::FolderCache::value_type&);

}

#endif
