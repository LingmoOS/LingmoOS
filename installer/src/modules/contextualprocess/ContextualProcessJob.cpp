/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2017-2020 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "ContextualProcessJob.h"

#include "Binding.h"

#include "CalamaresVersion.h"
#include "GlobalStorage.h"
#include "JobQueue.h"

#include "compat/Variant.h"
#include "utils/CommandList.h"
#include "utils/Logger.h"
#include "utils/Variant.h"

ContextualProcessBinding::~ContextualProcessBinding()
{
    m_wildcard = nullptr;
    for ( const auto& c : m_checks )
    {
        delete c.commands();
    }
}

void
ContextualProcessBinding::append( const QString& value, Calamares::CommandList* commands )
{
    m_checks.append( ValueCheck( value, commands ) );
    if ( value == QString( "*" ) )
    {
        m_wildcard = commands;
    }
}

Calamares::JobResult
ContextualProcessBinding::run( const QString& value ) const
{
    for ( const auto& c : m_checks )
    {
        if ( value == c.value() )
        {
            return c.commands()->run();
        }
    }

    if ( m_wildcard )
    {
        return m_wildcard->run();
    }

    return Calamares::JobResult::ok();
}

bool
ContextualProcessBinding::fetch( Calamares::GlobalStorage* storage, QString& value ) const
{
    value.clear();
    bool ok = false;
    const auto v = Calamares::lookup( storage, m_variable, ok );
    if ( !ok )
    {
        return false;
    }
    value = v.toString();
    return true;
}

ContextualProcessJob::ContextualProcessJob( QObject* parent )
    : Calamares::CppJob( parent )
{
}

ContextualProcessJob::~ContextualProcessJob()
{
    qDeleteAll( m_commands );
}

QString
ContextualProcessJob::prettyName() const
{
    return tr( "Performing contextual processes' job…", "@status" );
}

Calamares::JobResult
ContextualProcessJob::exec()
{
    Calamares::GlobalStorage* gs = Calamares::JobQueue::instance()->globalStorage();

    for ( const ContextualProcessBinding* binding : m_commands )
    {
        QString value;
        if ( binding->fetch( gs, value ) )
        {
            Calamares::JobResult r = binding->run( value );
            if ( !r )
            {
                return r;
            }
        }
        else
        {
            cWarning() << "ContextualProcess checks for unknown variable" << binding->variable();
        }
    }
    return Calamares::JobResult::ok();
}

void
ContextualProcessJob::setConfigurationMap( const QVariantMap& configurationMap )
{
    bool dontChroot = Calamares::getBool( configurationMap, "dontChroot", false );
    qint64 timeout = Calamares::getInteger( configurationMap, "timeout", 10 );
    if ( timeout < 1 )
    {
        timeout = 10;
    }

    for ( QVariantMap::const_iterator iter = configurationMap.cbegin(); iter != configurationMap.cend(); ++iter )
    {
        QString variableName = iter.key();
        if ( variableName.isEmpty() || ( variableName == "dontChroot" ) || ( variableName == "timeout" ) )
        {
            continue;
        }

        if ( Calamares::typeOf( iter.value() ) != Calamares::MapVariantType )
        {
            cWarning() << moduleInstanceKey() << "bad configuration values for" << variableName;
            continue;
        }

        auto binding = new ContextualProcessBinding( variableName );
        m_commands.append( binding );
        QVariantMap values = iter.value().toMap();
        for ( QVariantMap::const_iterator valueiter = values.cbegin(); valueiter != values.cend(); ++valueiter )
        {
            QString valueString = valueiter.key();
            if ( variableName.isEmpty() )
            {
                cWarning() << moduleInstanceKey() << "variable" << variableName << "unrecognized value"
                           << valueiter.key();
                continue;
            }

            Calamares::CommandList* commands
                = new Calamares::CommandList( valueiter.value(), !dontChroot, std::chrono::seconds( timeout ) );

            binding->append( valueString, commands );
        }
    }
}

int
ContextualProcessJob::count()
{
    return m_commands.count();
}

int
ContextualProcessJob::count( const QString& variableName )
{
    for ( const ContextualProcessBinding* binding : m_commands )
    {
        if ( binding->variable() == variableName )
        {
            return binding->count();
        }
    }
    return -1;
}

CALAMARES_PLUGIN_FACTORY_DEFINITION( ContextualProcessJobFactory, registerPlugin< ContextualProcessJob >(); )
