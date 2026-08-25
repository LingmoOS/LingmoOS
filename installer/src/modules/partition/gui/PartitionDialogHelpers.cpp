/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014 Aurélien Gâteau <agateau@kde.org>
 *   SPDX-FileCopyrightText: 2016 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2018-2021 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "PartitionDialogHelpers.h"

#include "core/PartUtils.h"
#include "core/PartitionCoreModule.h"
#include "gui/CreatePartitionDialog.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "utils/Logger.h"

#include <kpmcore/fs/filesystem.h>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

QStringList
standardMountPoints()
{
    QStringList mountPoints { "/", "/boot", "/home", "/opt", "/srv", "/usr", "/var" };
    if ( PartUtils::isEfiSystem() )
    {
        mountPoints << Calamares::JobQueue::instance()->globalStorage()->value( "efiSystemPartition" ).toString();
    }
    mountPoints.removeDuplicates();
    mountPoints.sort();
    return mountPoints;
}

void
standardMountPoints( QComboBox& combo )
{
    combo.clear();
    combo.lineEdit()->setPlaceholderText( QObject::tr( "(no mount point)" ) );
    combo.addItems( standardMountPoints() );
}

void
standardMountPoints( QComboBox& combo, const QString& selected )
{
    standardMountPoints( combo );
    setSelectedMountPoint( combo, selected );
}

QString
selectedMountPoint( QComboBox& combo )
{
    return combo.currentText();
}

void
setSelectedMountPoint( QComboBox& combo, const QString& selected )
{
    if ( selected.isEmpty() )
    {
        combo.setCurrentIndex( -1 );  // (no mount point)
    }
    else
    {
        for ( int i = 0; i < combo.count(); ++i )
        {
            if ( selected == combo.itemText( i ) )
            {
                combo.setCurrentIndex( i );
                return;
            }
        }
        combo.addItem( selected );
        combo.setCurrentIndex( combo.count() - 1 );
    }
}

bool
validateMountPoint( PartitionCoreModule* core, const QString& mountPoint, const QStringList& inUse, const QString& fileSystem, QLabel* label, QPushButton* button )
{
    QString msg;
    bool ok = true;

    if ( inUse.contains( mountPoint ) )
    {
        msg = CreatePartitionDialog::tr( "Mountpoint already in use. Please select another one.", "@info" );
        ok = false;
    }
    else if ( !mountPoint.isEmpty() && !mountPoint.startsWith( '/' ) )
    {
        msg = CreatePartitionDialog::tr( "Mountpoint must start with a <tt>/</tt>.", "@info" );
        ok = false;
    } else {
        // Validate the chosen filesystem + mountpoint combination.
        FileSystem::Type selectedFsType;
        PartUtils::canonicalFilesystemName( fileSystem, &selectedFsType );
        bool fsTypeIsAllowed = false;
        if ( selectedFsType == FileSystem::Type::Unknown )
        {
            fsTypeIsAllowed = true;
        }
        else
        {
            QList< FileSystem::Type > anyAllowedFsTypes = core->dirFSRestrictLayout().anyAllowedFSTypes();
            for ( auto& anyAllowedFsType : anyAllowedFsTypes )
            {
                if ( selectedFsType == anyAllowedFsType )
                {
                    fsTypeIsAllowed = true;
                    break;
                }
            }
        }

        bool fsTypeIsAllowedForMountPoint = false;
        // We allow arbitrary unmountable filesystems here since an
        // unmountable filesystem has no mount point associated with it, thus
        // any filesystem restriction we'd find at this point would be
        // irrelevant.
        if ( selectedFsType == FileSystem::Type::Unknown || s_unmountableFS.contains( selectedFsType ) )
        {
            fsTypeIsAllowedForMountPoint = true;
        }
        else
        {
            QList< FileSystem::Type > allowedFsTypes = core->dirFSRestrictLayout().allowedFSTypes( mountPoint, inUse, false );
            for ( auto& allowedFsType : allowedFsTypes )
            {
                if ( selectedFsType == allowedFsType )
                {
                    fsTypeIsAllowedForMountPoint = true;
                    break;
                }
            }
        }

        if ( !fsTypeIsAllowed ) {
            msg = CreatePartitionDialog::tr( "Filesystem is prohibited by this distro. Consider selecting another one.", "@info" );
            ok = true;
        }
        else if ( !fsTypeIsAllowedForMountPoint ) {
            msg = CreatePartitionDialog::tr( "Filesystem is prohibited for use on this mountpoint. Consider selecting a different filesystem or mountpoint.", "@info" );
            ok = true;
        }
    }

    if ( label )
    {
        label->setText( msg );
    }
    if ( button )
    {
        button->setEnabled( ok );
    }
    return ok;
}


PartitionTable::Flags
flagsFromList( const QListWidget& list )
{
    PartitionTable::Flags flags;

    for ( int i = 0; i < list.count(); i++ )
    {
        if ( list.item( i )->checkState() == Qt::Checked )
        {
            flags |= static_cast< PartitionTable::Flag >( list.item( i )->data( Qt::UserRole ).toInt() );
        }
    }

    return flags;
}

void
setFlagList( QListWidget& list, PartitionTable::Flags available, PartitionTable::Flags checked )
{
    int f = 1;
    QString s;
    while ( !( s = PartitionTable::flagName( static_cast< PartitionTable::Flag >( f ) ) ).isEmpty() )
    {
        if ( available & f )
        {
            QListWidgetItem* item = new QListWidgetItem( s );
            list.addItem( item );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            item->setData( Qt::UserRole, f );
            item->setCheckState( ( checked & f ) ? Qt::Checked : Qt::Unchecked );
        }

        f <<= 1;
    }
}
