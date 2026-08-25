/* === This file is part of Calamares - <https://calamares.io> ===
 *
 *   SPDX-FileCopyrightText: 2024 Adriaan de Groot <groot@kde.org>
 *   SPDX-License-Identifier: GPL-3.0-or-later
 *
 *   Calamares is Free Software: see the License-Identifier above.
 *
 *
 */
#ifndef CALAMARES_COMPAT_XML_H
#define CALAMARES_COMPAT_XML_H

#include <QDomDocument>

namespace Calamares
{
#if QT_VERSION < QT_VERSION_CHECK( 6, 6, 0 )
struct ParseResult
{
    QString errorMessage;
    int errorLine = -1;
    int errorColumn = -1;
};

[[nodiscard]] inline ParseResult
setXmlContent( QDomDocument& doc, const QByteArray& ba )
{
    ParseResult p;
    const bool r = doc.setContent( ba, &p.errorMessage, &p.errorLine, &p.errorColumn );
    return r ? ParseResult {} : p;
}
#else
[[nodiscard]] inline QDomDocument::ParseResult
setXmlContent( QDomDocument& doc, const QByteArray& ba )
{
    return doc.setContent( ba );
}
#endif

}  // namespace Calamares

#endif
