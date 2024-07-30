/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "parser.h"
#include "constants.h"
#include "content.h"
#include "document.h"

#include <documentsource.h>

#include <QDomAttr>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>

#include <QHash>
#include <QString>

namespace Syndication
{
namespace Atom
{
class SYNDICATION_NO_EXPORT Parser::ParserPrivate
{
public:
    static QDomDocument convertAtom0_3(const QDomDocument &document);
    static QDomNode convertNode(QDomDocument &doc, const QDomNode &node, const QHash<QString, QString> &nameMapper);
};

bool Parser::accept(const Syndication::DocumentSource &source) const
{
    QDomElement root = source.asDomDocument().documentElement();
    return !root.isNull() && (root.namespaceURI() == atom1Namespace() || root.namespaceURI() == atom0_3Namespace());
}

Syndication::SpecificDocumentPtr Parser::parse(const Syndication::DocumentSource &source) const
{
    QDomDocument doc = source.asDomDocument();

    if (doc.isNull()) {
        // if this is not atom, return an invalid feed document
        return FeedDocumentPtr(new FeedDocument());
    }

    QDomElement feed = doc.namedItem(QStringLiteral("feed")).toElement();

    bool feedValid = !feed.isNull();

    if (feedValid //
        && feed.attribute(QStringLiteral("version")) == QLatin1String("0.3")) {
        doc = ParserPrivate::convertAtom0_3(doc);
        feed = doc.namedItem(QStringLiteral("feed")).toElement();
    }

    feedValid = !feed.isNull() && feed.namespaceURI() == atom1Namespace();

    if (feedValid) {
        return FeedDocumentPtr(new FeedDocument(feed));
    }

    QDomElement entry = doc.namedItem(QStringLiteral("entry")).toElement();
    bool entryValid = !entry.isNull() && entry.namespaceURI() == atom1Namespace();

    if (entryValid) {
        return EntryDocumentPtr(new EntryDocument(feed));
    }

    // if this is not atom, return an invalid feed document
    return FeedDocumentPtr(new FeedDocument());
}

QString Parser::format() const
{
    return QStringLiteral("atom");
}

QDomNode Parser::ParserPrivate::convertNode(QDomDocument &doc, const QDomNode &node, const QHash<QString, QString> &nameMapper)
{
    if (!node.isElement()) {
        return node.cloneNode(true);
    }

    bool isAtom03Element = node.namespaceURI() == atom0_3Namespace();
    QDomElement oldEl = node.toElement();

    // use new namespace
    QString newNS = isAtom03Element ? atom1Namespace() : node.namespaceURI();

    QString newName = node.localName();

    // rename tags that are listed in the nameMapper
    if (isAtom03Element && nameMapper.contains(node.localName())) {
        newName = nameMapper[node.localName()];
    }

    QDomElement newEl = doc.createElementNS(newNS, newName);

    QDomNamedNodeMap attributes = oldEl.attributes();

    // copy over attributes
    const int numberOfAttributes(attributes.count());
    for (int i = 0; i < numberOfAttributes; ++i) {
        const QDomAttr attr = attributes.item(i).toAttr();
        if (attr.namespaceURI().isEmpty()) {
            newEl.setAttribute(attr.name(), attr.value());
        } else {
            newEl.setAttributeNS(attr.namespaceURI(), attr.name(), attr.value());
        }
    }

    /* clang-format off */
    bool isTextConstruct = newNS == atom1Namespace()
                           && (newName == QLatin1String("title")
                               || newName == QLatin1String("rights")
                               || newName == QLatin1String("subtitle")
                               || newName == QLatin1String("summary"));
    /* clang-format on */

    // for atom text constructs, map to new type schema (which only allows text, type, xhtml)

    if (isTextConstruct) {
        QString oldType = newEl.attribute(QStringLiteral("type"), QStringLiteral("text/plain"));
        QString newType;

        Content::Format format = Content::mapTypeToFormat(oldType);
        switch (format) {
        case Content::XML:
            newType = QStringLiteral("xhtml");
            break;
        case Content::EscapedHTML:
            newType = QStringLiteral("html");
            break;
        case Content::PlainText:
        case Content::Binary:
        default:
            newType = QStringLiteral("text");
        }

        newEl.setAttribute(QStringLiteral("type"), newType);
    } else {
        // for generator, rename the "url" attribute to "uri"

        bool isGenerator = newNS == atom1Namespace() && newName == QLatin1String("generator");
        if (isGenerator && newEl.hasAttribute(QStringLiteral("url"))) {
            newEl.setAttribute(QStringLiteral("uri"), newEl.attribute(QStringLiteral("url")));
        }
    }

    // process child nodes recursively and append them
    QDomNodeList children = node.childNodes();
    for (int i = 0; i < children.count(); ++i) {
        newEl.appendChild(convertNode(doc, children.item(i), nameMapper));
    }

    return newEl;
}

QDomDocument Parser::ParserPrivate::convertAtom0_3(const QDomDocument &doc03)
{
    QDomDocument doc = doc03.cloneNode(false).toDocument();

    // these are the tags that were renamed in 1.0
    QHash<QString, QString> nameMapper;
    nameMapper.insert(QStringLiteral("issued"), QStringLiteral("published"));
    nameMapper.insert(QStringLiteral("modified"), QStringLiteral("updated"));
    nameMapper.insert(QStringLiteral("url"), QStringLiteral("uri"));
    nameMapper.insert(QStringLiteral("copyright"), QStringLiteral("rights"));
    nameMapper.insert(QStringLiteral("tagline"), QStringLiteral("subtitle"));

    const QDomNodeList children = doc03.childNodes();

    for (int i = 0; i < children.count(); ++i) {
        doc.appendChild(convertNode(doc, children.item(i), nameMapper));
    }

    return doc;
}

Parser::Parser()
    : d(nullptr)
{
    Q_UNUSED(d) // silence -Wunused-private-field
}

Parser::~Parser() = default;

Parser::Parser(const Parser &other)
    : AbstractParser(other)
    , d(nullptr)
{
}
Parser &Parser::operator=(const Parser & /*other*/)
{
    return *this;
}

} // namespace Atom
} // namespace Syndication
