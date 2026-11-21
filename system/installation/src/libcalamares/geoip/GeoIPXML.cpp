/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2018 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 */

#include "GeoIPXML.h"

#include "compat/Xml.h"
#include "utils/Logger.h"

#include <QtXml/QDomDocument>

namespace Calamares
{
namespace GeoIP
{

GeoIPXML::GeoIPXML( const QString& element )
    : Interface( element.isEmpty() ? QStringLiteral( "TimeZone" ) : element )
{
}

static QStringList
getElementTexts( const QByteArray& data, const QString& tag )
{
    QStringList elements;

    QDomDocument doc;
    const auto p = Calamares::setXmlContent( doc, data );
    if ( p.errorMessage.isEmpty() )
    {
        const auto tzElements = doc.elementsByTagName( tag );
        cDebug() << "GeoIP found" << tzElements.length() << "elements";
        for ( int it = 0; it < tzElements.length(); ++it )
        {
            auto e = tzElements.at( it ).toElement();
            auto e_text = e.text();
            if ( !e_text.isEmpty() )
            {
                elements.append( e_text );
            }
        }
    }
    else
    {
        cWarning() << "GeoIP XML data error:" << p.errorMessage << "(line" << p.errorLine << ':' << p.errorColumn
                   << ')';
    }

    if ( elements.count() < 1 )
    {
        cWarning() << "GeopIP XML had no non-empty elements" << tag;
    }

    return elements;
}

QString
GeoIPXML::rawReply( const QByteArray& data )
{
    for ( const auto& e : getElementTexts( data, m_element ) )
    {
        if ( !e.isEmpty() )
        {
            return e;
        }
    }

    return QString();
}

GeoIP::RegionZonePair
GeoIPXML::processReply( const QByteArray& data )
{
    for ( const auto& e : getElementTexts( data, m_element ) )
    {
        auto tz = splitTZString( e );
        if ( tz.isValid() )
        {
            return RegionZonePair( tz );
        }
    }

    return RegionZonePair();
}

}  // namespace GeoIP
}  // namespace Calamares
