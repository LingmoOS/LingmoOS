/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "constants.h"
#include "tools.h"

#include <syndication/elementwrapper.h>
#include <syndication/tools.h>

#include <QDomElement>
#include <QString>

namespace Syndication
{
namespace Atom
{
QString extractAtomText(const Syndication::ElementWrapper &parent, const QString &tagname)
{
    QString str;

    QDomElement el = parent.firstElementByTagNameNS(atom1Namespace(), tagname);

    bool isCDATA = el.firstChild().isCDATASection();

    QString type = el.attribute(QStringLiteral("type"), QStringLiteral("text"));

    if (type == QLatin1String("text")) {
        str = parent.extractElementTextNS(atom1Namespace(), tagname).trimmed();
        if (isCDATA) {
            str = resolveEntities(str);
        }

        str = escapeSpecialCharacters(str);
    } else if (type == QLatin1String("html")) {
        str = parent.extractElementTextNS(atom1Namespace(), tagname).trimmed();
    } else if (type == QLatin1String("xhtml")) {
        str = ElementWrapper::childNodesAsXML(el).trimmed();
    }

    return str;
}

} // namespace Atom
} // namespace Syndication
