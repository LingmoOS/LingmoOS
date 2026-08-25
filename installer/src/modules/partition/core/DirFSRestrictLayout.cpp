/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014-2017 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2017-2018 Adriaan de Groot <groot@kde.org>
 *   SPDX-FileCopyrightText: 2018-2019 Collabora Ltd <arnaud.ferraris@collabora.com>
 *   SPDX-FileCopyrightText: 2024 Aaron Rainbolt <arraybolt3@gmail.com>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "utils/Logger.h"

#include "core/DirFSRestrictLayout.h"

#include "core/KPMHelpers.h"
#include "core/PartUtils.h"

#include "utils/Variant.h"

#include "GlobalStorage.h"
#include "JobQueue.h"

#include <kpmcore/fs/filesystem.h>
#include <kpmcore/fs/filesystemfactory.h>

#include <QSet>

DirFSRestrictLayout::DirFSRestrictLayout() {}

DirFSRestrictLayout::DirFSRestrictLayout( const DirFSRestrictLayout& layout )
    : m_dirFSRestrictLayout( layout.m_dirFSRestrictLayout )
{
}

DirFSRestrictLayout::~DirFSRestrictLayout() {}

DirFSRestrictLayout::DirFSRestrictEntry::DirFSRestrictEntry( const QString& path,
                                                             QList< FileSystem::Type > allowedFSTypes,
                                                             bool onlyWhenMountpoint )
    : dirPath( path )
    , dirAllowedFSTypes( allowedFSTypes )
    , useOnlyWhenMountpoint( onlyWhenMountpoint )
{
}

void
DirFSRestrictLayout::init( const QVariantList& config )
{
    m_dirFSRestrictLayout.clear();
    bool efiNeedsSet = true;

    for ( const auto& r : config )
    {
        QVariantMap pentry = r.toMap();
        if ( !pentry.contains( "directory" ) || !pentry.contains( "allowedFilesystemTypes" ) )
        {
            cError() << "Directory filesystem restriction layout entry #" << config.indexOf( r )
                     << "lacks mandatory attributes, switching to default layout.";
            m_dirFSRestrictLayout.clear();
            break;
        }

        QString directory = Calamares::getString( pentry, "directory" );
        QStringList allowedFSTypeStrings = Calamares::getStringList( pentry, "allowedFilesystemTypes" );
        QList< FileSystem::Type > allowedFSTypes;
        if ( allowedFSTypeStrings.length() == 1 && allowedFSTypeStrings[0] == "all" )
        {
            allowedFSTypes = fullFSList();
        }
        else
        {
            for ( const auto& fsStr : allowedFSTypeStrings )
            {
                FileSystem::Type allowedFSType;
                PartUtils::canonicalFilesystemName( fsStr, &allowedFSType );
                if ( allowedFSType == FileSystem::Type::Unknown )
                {
                    continue;
                }
               allowedFSTypes.append( allowedFSType );
            }
        }
        bool onlyWhenMountpoint = Calamares::getBool( pentry, "onlyWhenMountpoint", false );
        if ( directory == "efi" )
        {
            efiNeedsSet = false;
        }
        DirFSRestrictEntry restrictEntry( directory, allowedFSTypes, onlyWhenMountpoint );
        m_dirFSRestrictLayout.append( restrictEntry );
    }

    if ( efiNeedsSet )
    {
        QList< FileSystem::Type > efiAllowedFSTypes = { FileSystem::Fat32 };
        DirFSRestrictEntry efiRestrictEntry( "efi", efiAllowedFSTypes, true );
        m_dirFSRestrictLayout.append( efiRestrictEntry );
    }
}

QList< FileSystem::Type >
DirFSRestrictLayout::allowedFSTypes( const QString& path, const QStringList& existingMountpoints, bool overlayDirs )
{
    QSet< FileSystem::Type > typeSet;
    bool foundTypeList = false;

    for ( const auto& entry : m_dirFSRestrictLayout )
    {
        QString dirPath = entry.dirPath;
        if ( dirPath == "efi" )
        {
            dirPath = Calamares::JobQueue::instance()->globalStorage()->value( "efiSystemPartition" ).toString();
        }
        if ( dirPath == path || ( !entry.useOnlyWhenMountpoint && overlayDirs && path.startsWith( QStringLiteral( "/" ) ) && dirPath.startsWith( path ) && !existingMountpoints.contains( dirPath ) ) )
        {
            QSet< FileSystem::Type > newTypeSet = QSet< FileSystem::Type >( entry.dirAllowedFSTypes.cbegin(), entry.dirAllowedFSTypes.cend() );
            foundTypeList = true;
            if ( typeSet.isEmpty() )
            {
                typeSet = newTypeSet;
                if ( !overlayDirs )
                {
                    break;
                }
            }
            else
            {
                typeSet.intersect( newTypeSet );
            }
        }
    }

    if ( overlayDirs )
    {
        QList< FileSystem::Type > anyTypeList = anyAllowedFSTypes();
        QSet< FileSystem::Type > anyTypeSet = QSet< FileSystem::Type >( anyTypeList.cbegin(), anyTypeList.cend() );
        if ( !foundTypeList )
        {
            typeSet = anyTypeSet;
            foundTypeList = true;
        }
        else
        {
            typeSet.intersect( anyTypeSet );
        }
    }

    if ( foundTypeList )
    {
        return QList< FileSystem::Type >( typeSet.cbegin(), typeSet.cend() );
    }
    else
    {
        // This directory doesn't have any allowed filesystems explicitly
        // configured, so all filesystems are valid.
        return fullFSList();
    }
}

QString
DirFSRestrictLayout::diagnoseFSConflict( const QString& path, const FileSystem::Type& fsType, const QStringList& existingMountpoints )
{
    QSet< FileSystem::Type > typeSet;
    bool foundTypeList = false;

    for ( const auto& entry : m_dirFSRestrictLayout )
    {
        QString dirPath = entry.dirPath;
        if ( dirPath == "efi" )
        {
            dirPath = Calamares::JobQueue::instance()->globalStorage()->value( "efiSystemPartition" ).toString();
        }
        if ( dirPath == path || ( !entry.useOnlyWhenMountpoint && path.startsWith( QStringLiteral( "/" ) ) && ( dirPath.startsWith( path ) || dirPath == "any" ) && !existingMountpoints.contains( dirPath ) ) )
        {
            QSet< FileSystem::Type > newTypeSet = QSet< FileSystem::Type >( entry.dirAllowedFSTypes.cbegin(), entry.dirAllowedFSTypes.cend() );
            foundTypeList = true;
            if ( typeSet.isEmpty() )
            {
                typeSet = newTypeSet;
            }
            else
            {
                typeSet.intersect( newTypeSet );
            }
        }

        if ( foundTypeList && !typeSet.contains( fsType ) )
        {
            if ( typeSet.isEmpty() )
            {
                cWarning() << "no filesystems are valid for path '" << path << "', check directoryFilesystemRestrictions for issues";
            }
            // At this point, we've found the first mountpoint that, when
            // taken into account, results in the currently chosen filesystem
            // being invalid. Return that mountpoint.
            return dirPath;
        }
    }

    return QString();
}

QList< FileSystem::Type >
DirFSRestrictLayout::anyAllowedFSTypes()
{
    for ( const auto& entry : m_dirFSRestrictLayout )
    {
        if ( entry.dirPath == "any" )
        {
            return entry.dirAllowedFSTypes;
        }
    }

    // No global filesystem whitelist defined, so all filesystems are
    // considered valid unless a mountpoint-specific whitelist is used to
    // restrict the allowed filesystems.
    return fullFSList();
}

QList< FileSystem::Type >
DirFSRestrictLayout::fullFSList()
{
    QList< FileSystem::Type > typeList;
    FileSystemFactory::init();
    for ( auto fs : FileSystemFactory::map() )
    {
        typeList.append( fs->type() );
    }
    return typeList;
}
