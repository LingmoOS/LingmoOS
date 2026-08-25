/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2014-2015 Teo Mrnjavac <teo@kde.org>
 *   SPDX-FileCopyrightText: 2017-2018 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 *
 */

#include "GlobalStorage.h"

#include "compat/Mutex.h"

#include "utils/Logger.h"
#include "utils/Units.h"
#include "utils/Yaml.h"

#include <QFile>
#include <QJsonDocument>

using namespace Calamares::Units;

namespace Calamares
{

class GlobalStorage::ReadLock : public MutexLocker
{
public:
    ReadLock( const GlobalStorage* gs )
        : MutexLocker( &gs->m_mutex )
    {
    }
};

class GlobalStorage::WriteLock : public MutexLocker
{
public:
    WriteLock( GlobalStorage* gs )
        : MutexLocker( &gs->m_mutex )
        , m_gs( gs )
    {
    }
    ~WriteLock() { m_gs->changed(); }

    GlobalStorage* m_gs;
};

GlobalStorage::GlobalStorage( QObject* parent )
    : QObject( parent )
{
}

bool
GlobalStorage::contains( const QString& key ) const
{
    ReadLock l( this );
    return m.contains( key );
}

int
GlobalStorage::count() const
{
    ReadLock l( this );
    return m.count();
}

void
GlobalStorage::insert( const QString& key, const QVariant& value )
{
    WriteLock l( this );
    m.insert( key, value );
}

QStringList
GlobalStorage::keys() const
{
    ReadLock l( this );
    return m.keys();
}

int
GlobalStorage::remove( const QString& key )
{
    WriteLock l( this );
    int nItems = m.remove( key );
    return nItems;
}

void
GlobalStorage::clear()
{
    WriteLock l( this );
    m.clear();
}

QVariant
GlobalStorage::value( const QString& key ) const
{
    ReadLock l( this );
    return m.value( key );
}

void
GlobalStorage::debugDump() const
{
    ReadLock l( this );
    cDebug() << "GlobalStorage" << Logger::Pointer( this ) << m.count() << "items";
    for ( auto it = m.cbegin(); it != m.cend(); ++it )
    {
        cDebug() << Logger::SubEntry << it.key() << '\t' << it.value();
    }
}

bool
GlobalStorage::saveJson( const QString& filename ) const
{
    ReadLock l( this );
    QFile f( filename );
    if ( !f.open( QFile::WriteOnly ) )
    {
        return false;
    }

    f.write( QJsonDocument::fromVariant( m ).toJson() );
    f.close();
    return true;
}

bool
GlobalStorage::loadJson( const QString& filename )
{
    QFile f( filename );
    if ( !f.open( QFile::ReadOnly ) )
    {
        return false;
    }

    QJsonParseError e;
    QJsonDocument d = QJsonDocument::fromJson( f.read( 1_MiB ), &e );
    if ( d.isNull() )
    {
        cWarning() << filename << e.errorString();
    }
    else if ( !d.isObject() )
    {
        cWarning() << filename << "Not suitable JSON.";
    }
    else
    {
        WriteLock l( this );
        // Do **not** use method insert() here, because it would
        //   recursively lock the mutex, leading to deadlock. Also,
        //   that would emit changed() for each key.
        auto map = d.toVariant().toMap();
        for ( auto i = map.constBegin(); i != map.constEnd(); ++i )
        {
            m.insert( i.key(), *i );
        }
        return true;
    }
    return false;
}

bool
GlobalStorage::saveYaml( const QString& filename ) const
{
    ReadLock l( this );
    return Calamares::YAML::save( filename, m );
}

bool
GlobalStorage::loadYaml( const QString& filename )
{
    bool ok = false;
    auto map = Calamares::YAML::load( filename, &ok );
    if ( ok )
    {
        WriteLock l( this );
        // Do **not** use method insert() here, because it would
        //   recursively lock the mutex, leading to deadlock. Also,
        //   that would emit changed() for each key.
        for ( auto i = map.constBegin(); i != map.constEnd(); ++i )
        {
            m.insert( i.key(), *i );
        }
        return true;
    }
    return false;
}

///@brief Implementation for recursively looking up dotted selector parts.
static QVariant
lookup( const QStringList& nestedKey, int index, const QVariant& v, bool& ok )
{
    if ( !v.canConvert< QVariantMap >() )
    {
        // Mismatch: we're still looking for keys, but v is not a submap
        ok = false;
        return {};
    }
    if ( index >= nestedKey.length() )
    {
        cError() << "Recursion error looking at index" << index << "of" << nestedKey;
        ok = false;
        return {};
    }

    const QVariantMap map = v.toMap();
    const QString& key = nestedKey.at( index );
    if ( index == nestedKey.length() - 1 )
    {
        ok = map.contains( key );
        return ok ? map.value( key ) : QVariant();
    }
    else
    {
        return lookup( nestedKey, index + 1, map.value( key ), ok );
    }
}

QVariant
lookup( const GlobalStorage* storage, const QString& nestedKey, bool& ok )
{
    ok = false;
    if ( !storage )
    {
        return {};
    }

    if ( nestedKey.contains( '.' ) )
    {
        QStringList steps = nestedKey.split( '.' );
        return lookup( steps, 1, storage->value( steps.first() ), ok );
    }
    else
    {
        ok = storage->contains( nestedKey );
        return ok ? storage->value( nestedKey ) : QVariant();
    }
}

}  // namespace Calamares
