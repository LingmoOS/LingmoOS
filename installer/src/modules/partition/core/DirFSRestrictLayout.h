/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2018-2019 Collabora Ltd <arnaud.ferraris@collabora.com>
 *   SPDX-FileCopyrightText: 2019 Adriaan de Groot <groot@kde.org>
 *   SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#ifndef DIRFSRESTRICTLAYOUT_H
#define DIRFSRESTRICTLAYOUT_H

#include "Config.h"

// KPMcore
#include <kpmcore/fs/filesystem.h>

// Qt
#include <QList>
#include <QObject>
#include <QVariantMap>

class DirFSRestrictLayout
{
public:
    struct DirFSRestrictEntry
    {
        QString dirPath;
        QList< FileSystem::Type > dirAllowedFSTypes;
        bool useOnlyWhenMountpoint = false;

        /// @brief All-zeroes DirFSRestrictEntry
        DirFSRestrictEntry() = default;
        /** @brief Parse @p path, @p allowedFSTypes, and @p onlyWhenMountpoint to their respective member variables
         *
         * Sets a specific set of allowed filesystems for a mountpoint.
         */
        DirFSRestrictEntry( const QString& path,
                            QList< FileSystem::Type > allowedFSTypes,
                            bool onlyWhenMountpoint );
        /// @brief Copy DirFSRestrictEntry
        DirFSRestrictEntry( const DirFSRestrictEntry& e ) = default;
    };

    DirFSRestrictLayout();
    DirFSRestrictLayout( const DirFSRestrictLayout& layout );
    ~DirFSRestrictLayout();

    /** @brief create the configuration from @p config
     *
     * @p config is a list of partition entries (in QVariant form,
     * read from YAML). If no entries are given, the only restriction is that
     * the EFI system partition must use fat32.
     *
     * Any unknown values in the config will be ignored.
     */
    void init( const QVariantList& config );

    /** @brief get a list of allowable filesystems for a path
     *
     * @p path is the path one wants to get the allowed FS types for.
     * @p existingMountpoints is the list of all mountpoints that are
     * currently configured to be placed on their own partition.
     */
    QList< FileSystem::Type > allowedFSTypes( const QString& path, const QStringList& existingMountpoints, bool overlayDirs );

    /** @brief determine which directory restriction rule makes a particular mountpoint + filesystem combination invalid
     *
     * @p path is the path with an improper filesystem chosen.
     * @p fsType is the improper filesystem used on that path.
     * @p existingMountpoints is the list of all mountpoints that are
     * currently configured to be placed on their own partition.
     */
    QString diagnoseFSConflict( const QString& path, const FileSystem::Type& fsType, const QStringList& existingMountpoints );

    /// @brief get a global filesystem whitelist
    QList< FileSystem::Type > anyAllowedFSTypes();

private:
    QList< DirFSRestrictEntry > m_dirFSRestrictLayout;

    QList< FileSystem::Type > fullFSList();
};

#endif /* DIRFSRESTRICTLAYOUT_H */
