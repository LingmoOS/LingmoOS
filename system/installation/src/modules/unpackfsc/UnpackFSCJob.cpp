/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2021 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "UnpackFSCJob.h"

#include "FSArchiverRunner.h"
#include "TarballRunner.h"
#include "UnsquashRunner.h"

#include "GlobalStorage.h"
#include "JobQueue.h"
#include "compat/Variant.h"
#include "utils/Logger.h"
#include "utils/NamedEnum.h"
#include "utils/RAII.h"
#include "utils/Variant.h"

#include <memory>

static const NamedEnumTable< UnpackFSCJob::Type >
typeNames()
{
    using T = UnpackFSCJob::Type;
    // clang-format off
    static const NamedEnumTable< T > names
    {
        { "none", T::None },
        { "fsarchiver", T::FSArchive },
        { "fsarchive", T::FSArchive },
        { "fsa", T::FSArchive },
        { "fsa-dir", T::FSArchive },
        { "fsa-block", T::FSArchiveFS },
        { "fsa-fs", T::FSArchiveFS },
        { "squashfs", T::Squashfs },
        { "squash", T::Squashfs },
        { "unsquash", T::Squashfs },
        { "tar", T::Tarball },
        { "tarball", T::Tarball },
        { "tgz", T::Tarball },
    };
    // clang-format on
    return names;
}

UnpackFSCJob::UnpackFSCJob( QObject* parent )
    : Calamares::CppJob( parent )
{
}

UnpackFSCJob::~UnpackFSCJob() {}

QString
UnpackFSCJob::prettyName() const
{
    return tr( "Unpack filesystems" );
}

QString
UnpackFSCJob::prettyStatusMessage() const
{
    return m_progressMessage;
}

static bool
checkCondition( const QString& condition )
{
    if ( condition.isEmpty() )
    {
        return true;
    }

    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();

    bool ok = false;
    const auto v = Calamares::lookup( gs, condition, ok );
    if ( !ok )
    {
        cWarning() << "Item has condition '" << condition << "' which is not set at all (assuming 'true').";
        return true;
    }

    if ( !v.canConvert< bool >() )
    {
        cWarning() << "Item has condition '" << condition << "' with value" << v << "(assuming 'true').";
        return true;
    }

    return v.toBool();
}

Calamares::JobResult
UnpackFSCJob::exec()
{
    if ( !checkCondition( m_condition ) )
    {
        cDebug() << "Skipping item with condition '" << m_condition << "' which is set to false.";
        return Calamares::JobResult::ok();
    }

    cScopedAssignment messageClearer( &m_progressMessage, QString() );
    std::unique_ptr< Runner > r;
    switch ( m_type )
    {
    case Type::FSArchive:
        r = std::make_unique< FSArchiverDirRunner >( m_source, m_destination );
        break;
    case Type::FSArchiveFS:
        r = std::make_unique< FSArchiverFSRunner >( m_source, m_destination );
        break;
    case Type::Squashfs:
        r = std::make_unique< UnsquashRunner >( m_source, m_destination );
        break;
    case Type::Tarball:
        r = std::make_unique< TarballRunner >( m_source, m_destination );
        break;
    case Type::None:
    default:
        cDebug() << "Nothing to do.";
        return Calamares::JobResult::ok();
    }

    connect( r.get(),
             &Runner::progress,
             [ = ]( qreal percent, const QString& message )
             {
                 m_progressMessage = message;
                 Q_EMIT progress( percent );
             } );
    return r->run();
}

void
UnpackFSCJob::setConfigurationMap( const QVariantMap& map )
{
    m_type = Type::None;

    const QString source = Calamares::getString( map, "source" );
    const QString sourceTypeName = Calamares::getString( map, "sourcefs" );
    if ( source.isEmpty() || sourceTypeName.isEmpty() )
    {
        cWarning() << "Skipping item with bad source data:" << map;
        return;
    }
    bool bogus = false;
    Type sourceType = typeNames().find( sourceTypeName, bogus );
    if ( sourceType == Type::None )
    {
        cWarning() << "Skipping item with source type None";
        return;
    }
    const QString destination = Calamares::getString( map, "destination" );
    if ( destination.isEmpty() )
    {
        cWarning() << "Skipping item with empty destination";
        return;
    }
    const auto conditionKey = QStringLiteral( "condition" );
    if ( map.contains( conditionKey ) )
    {
        const auto value = map[ conditionKey ];
        if ( Calamares::typeOf( value ) == Calamares::BoolVariantType )
        {
            if ( !value.toBool() )
            {
                cDebug() << "Skipping item with condition set to false.";
                // Leave type set to None, which will be skipped later
                return;
            }
            // Else the condition is true, and we're fine leaving the string empty because that defaults to true
        }
        else
        {
            const auto variable = value.toString();
            if ( variable.isEmpty() )
            {
                cDebug() << "Skipping item with condition '" << value << "' that is empty (use 'true' instead).";
                return;
            }
            m_condition = variable;
        }
    }

    m_source = source;
    m_destination = destination;
    m_type = sourceType;
}

CALAMARES_PLUGIN_FACTORY_DEFINITION( UnpackFSCFactory, registerPlugin< UnpackFSCJob >(); )
