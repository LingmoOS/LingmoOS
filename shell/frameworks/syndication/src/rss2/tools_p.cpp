/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "tools.h"
#include <constants.h>
#include <elementwrapper.h>

#include <QDomElement>
#include <QList>
#include <QString>

namespace Syndication
{
namespace RSS2
{
//@cond PRIVATE
QString extractContent(const ElementWrapper &wrapper)
{
    if (wrapper.isNull()) {
        return QString();
    }

    QList<QDomElement> list = wrapper.elementsByTagNameNS(contentNameSpace(), QStringLiteral("encoded"));

    if (!list.isEmpty()) {
        return list.first().text().trimmed();
    }

    list = wrapper.elementsByTagNameNS(xhtmlNamespace(), QStringLiteral("body"));

    if (!list.isEmpty()) {
        return ElementWrapper::childNodesAsXML(list.first()).trimmed();
    }

    list = wrapper.elementsByTagNameNS(xhtmlNamespace(), QStringLiteral("div"));

    if (!list.isEmpty()) {
        return ElementWrapper::childNodesAsXML(list.first()).trimmed();
    }

    return QString();
}
//@endcond

} // namespace RSS2
} // namespace Syndication
