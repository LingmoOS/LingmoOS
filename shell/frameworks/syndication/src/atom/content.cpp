/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "content.h"

#include <tools.h>

#include <QByteArray>
#include <QDomElement>
#include <QStringList>

namespace Syndication
{
namespace Atom
{
class SYNDICATION_NO_EXPORT Content::ContentPrivate
{
public:
    ContentPrivate()
        : format(Format::PlainText)
        , formatIdentified(false)
    {
    }
    mutable Format format;
    mutable bool formatIdentified;
};

Content::Content()
    : ElementWrapper()
    , d(new ContentPrivate)
{
}

Content::Content(const QDomElement &element)
    : ElementWrapper(element)
    , d(new ContentPrivate)
{
}

Content::Content(const Content &other)
    : ElementWrapper(other)
    , d(other.d)
{
}

Content::~Content()
{
}

Content &Content::operator=(const Content &other)
{
    ElementWrapper::operator=(other);
    d = other.d;
    return *this;
}

QString Content::type() const
{
    return attribute(QStringLiteral("type"));
}

QString Content::src() const
{
    return completeURI(attribute(QStringLiteral("src")));
}

QByteArray Content::asByteArray() const
{
    if (!isBinary()) {
        return QByteArray();
    }
    return QByteArray::fromBase64(text().trimmed().toLatin1());
}

Content::Format Content::mapTypeToFormat(const QString &typep, const QString &src)
{
    QString type = typep;
    //"If neither the type attribute nor the src attribute is provided,
    // Atom Processors MUST behave as though the type attribute were
    // present with a value of "text""
    if (type.isNull() && src.isEmpty()) {
        type = QStringLiteral("text");
    }

    if (type == QLatin1String("html") || type == QLatin1String("text/html")) {
        return EscapedHTML;
    }

    /* clang-format off */
    if (type == QLatin1String("text")
        || (type.startsWith(QLatin1String("text/"), Qt::CaseInsensitive)
            && !type.startsWith(QLatin1String("text/xml"), Qt::CaseInsensitive))) { /* clang-format on */
        return PlainText;
    }

    static QStringList xmltypes;
    if (xmltypes.isEmpty()) {
        xmltypes.reserve(8);
        xmltypes.append(QStringLiteral("xhtml"));
        xmltypes.append(QStringLiteral("application/xhtml+xml"));
        // XML media types as defined in RFC3023:
        xmltypes.append(QStringLiteral("text/xml"));
        xmltypes.append(QStringLiteral("application/xml"));
        xmltypes.append(QStringLiteral("text/xml-external-parsed-entity"));
        xmltypes.append(QStringLiteral("application/xml-external-parsed-entity"));
        xmltypes.append(QStringLiteral("application/xml-dtd"));
        xmltypes.append(QStringLiteral("text/x-dtd")); // from shared-mime-info
    }

    /* clang-format off */
    if (xmltypes.contains(type)
        || type.endsWith(QLatin1String("+xml"), Qt::CaseInsensitive)
        || type.endsWith(QLatin1String("/xml"), Qt::CaseInsensitive)) { /* clang-format on */
        return XML;
    }

    return Binary;
}

Content::Format Content::format() const
{
    if (d->formatIdentified == false) {
        d->format = mapTypeToFormat(type(), src());
        d->formatIdentified = true;
    }
    return d->format;
}

bool Content::isBinary() const
{
    return format() == Binary;
}

bool Content::isContained() const
{
    return src().isEmpty();
}

bool Content::isPlainText() const
{
    return format() == PlainText;
}

bool Content::isEscapedHTML() const
{
    return format() == EscapedHTML;
}

bool Content::isXML() const
{
    return format() == XML;
}

QString Content::asString() const
{
    Format f = format();

    if (f == PlainText) {
        return plainTextToHtml(text()).trimmed();
    } else if (f == EscapedHTML) {
        return text().trimmed();
    } else if (f == XML) {
        return childNodesAsXML().trimmed();
    }

    return QString();
}

QString Content::debugInfo() const
{
    QString info = QLatin1String("### Content: ###################\n");
    info += QLatin1String("type: #") + type() + QLatin1String("#\n");
    if (!src().isNull()) {
        info += QLatin1String("src: #") + src() + QLatin1String("#\n");
    }
    if (!isBinary()) {
        info += QLatin1String("content: #") + asString() + QLatin1String("#\n");
    } else {
        info += QLatin1String("binary length: #") + QString::number(asByteArray().size()) + QLatin1String("#\n");
    }
    info += QLatin1String("### Content end ################\n");

    return info;
}

} // namespace Atom
} // namespace Syndication
