/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "elementwrapper.h"
#include "constants.h"

#include <QUrl>

#include <QDomDocument>
#include <QDomElement>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>

namespace Syndication
{
class SYNDICATION_NO_EXPORT ElementWrapper::ElementWrapperPrivate
{
public:
    QDomElement element;
    QDomDocument ownerDoc;
    mutable QString xmlBase;
    mutable bool xmlBaseParsed;
    mutable QString xmlLang;
    mutable bool xmlLangParsed;
};

ElementWrapper::ElementWrapper()
    : d(new ElementWrapperPrivate)
{
    d->xmlBaseParsed = true;
    d->xmlLangParsed = true;
}

ElementWrapper::ElementWrapper(const ElementWrapper &other)
{
    *this = other;
}

ElementWrapper::ElementWrapper(const QDomElement &element)
    : d(new ElementWrapperPrivate)
{
    d->element = element;
    d->ownerDoc = element.ownerDocument(); // keep a copy of the (shared, thus cheap) document around to ensure the element isn't deleted too early (Bug 190068)
    d->xmlBaseParsed = false;
    d->xmlLangParsed = false;
}

ElementWrapper::~ElementWrapper()
{
}

ElementWrapper &ElementWrapper::operator=(const ElementWrapper &other)
{
    d = other.d;
    return *this;
}

bool ElementWrapper::operator==(const ElementWrapper &other) const
{
    return d->element == other.d->element;
}

bool ElementWrapper::isNull() const
{
    return d->element.isNull();
}

const QDomElement &ElementWrapper::element() const
{
    return d->element;
}

QString ElementWrapper::xmlBase() const
{
    if (!d->xmlBaseParsed) { // xmlBase not computed yet
        QDomElement current = d->element;

        /*
        An atom feed can contain nested xml:base elements, like this:

        <feed xml:base="http://example.com/foo.atom">
          <entry xml:base="subdir/">
            <link href="foo.html"/>
          </entry>
        </feed>

        To compute xml:base we explore the tree all the way up to the top.
        `bases` stores all the xml:base values from the deepest element up to
        the root element.
        */
        QStringList bases;
        while (!current.isNull()) {
            if (current.hasAttributeNS(xmlNamespace(), QStringLiteral("base"))) {
                bases << current.attributeNS(xmlNamespace(), QStringLiteral("base"));
            }

            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement()) {
                current = parent.toElement();
            } else {
                current = QDomElement();
            }
        }
        while (!bases.isEmpty()) {
            QUrl u = QUrl(d->xmlBase).resolved(QUrl(bases.takeLast()));
            d->xmlBase = u.url();
        }

        d->xmlBaseParsed = true;
    }

    return d->xmlBase;
}

QString ElementWrapper::completeURI(const QString &uri) const
{
    QUrl u = QUrl(xmlBase()).resolved(QUrl(uri));

    if (u.isValid()) {
        return u.url();
    }

    return uri;
}

QString ElementWrapper::xmlLang() const
{
    if (!d->xmlLangParsed) { // xmlLang not computed yet
        QDomElement current = d->element;

        while (!current.isNull()) {
            if (current.hasAttributeNS(xmlNamespace(), QStringLiteral("lang"))) {
                d->xmlLang = current.attributeNS(xmlNamespace(), QStringLiteral("lang"));
                return d->xmlLang;
            }

            QDomNode parent = current.parentNode();

            if (!parent.isNull() && parent.isElement()) {
                current = parent.toElement();
            } else {
                current = QDomElement();
            }
        }
        d->xmlLangParsed = true;
    }
    return d->xmlLang;
}

QString ElementWrapper::extractElementText(const QString &tagName) const
{
    const QDomElement el = d->element.namedItem(tagName).toElement();
    return el.isNull() ? QString() : el.text().trimmed();
}

QString ElementWrapper::extractElementTextNS(const QString &namespaceURI, const QString &localName) const
{
    const QDomElement el = firstElementByTagNameNS(namespaceURI, localName);
    return el.isNull() ? QString() : el.text().trimmed();
}

QString ElementWrapper::childNodesAsXML(const QDomElement &parent)
{
    ElementWrapper wrapper(parent);

    if (parent.isNull()) {
        return QString();
    }

    QDomNodeList list = parent.childNodes();

    QString str;
    QTextStream ts(&str, QIODevice::WriteOnly);

    // if there is a xml:base in our scope, first set it for
    // each child element so the xml:base shows up in the
    // serialization
    QString base = wrapper.xmlBase();

    for (int i = 0; i < list.count(); ++i) {
        QDomNode it = list.item(i);
        if (!base.isEmpty() //
            && it.isElement() //
            && !it.toElement().hasAttributeNS(xmlNamespace(), QStringLiteral("base"))) {
            it.toElement().setAttributeNS(xmlNamespace(), QStringLiteral("base"), base);
        }

        ts << it;
    }
    return str.trimmed();
}

QString ElementWrapper::childNodesAsXML() const
{
    return childNodesAsXML(d->element);
}

QList<QDomElement> ElementWrapper::elementsByTagName(const QString &tagName) const
{
    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (e.tagName() == tagName) {
                elements.append(e);
            }
        }
    }
    return elements;
}

QDomElement ElementWrapper::firstElementByTagNameNS(const QString &nsURI, const QString &localName) const
{
    if (isNull()) {
        return QDomElement();
    }

    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI) {
                return e;
            }
        }
    }

    return QDomElement();
}

QList<QDomElement> ElementWrapper::elementsByTagNameNS(const QString &nsURI, const QString &localName) const
{
    if (isNull()) {
        return QList<QDomElement>();
    }

    QList<QDomElement> elements;
    for (QDomNode n = d->element.firstChild(); !n.isNull(); n = n.nextSibling()) {
        if (n.isElement()) {
            QDomElement e = n.toElement();
            if (e.localName() == localName && e.namespaceURI() == nsURI) {
                elements.append(e);
            }
        }
    }
    return elements;
}

QString ElementWrapper::text() const
{
    return d->element.text();
}

QString ElementWrapper::attribute(const QString &name, const QString &defValue) const
{
    return d->element.attribute(name, defValue);
}

QString ElementWrapper::attributeNS(const QString &nsURI, const QString &localName, const QString &defValue) const
{
    return d->element.attributeNS(nsURI, localName, defValue);
}

bool ElementWrapper::hasAttribute(const QString &name) const
{
    return d->element.hasAttribute(name);
}

bool ElementWrapper::hasAttributeNS(const QString &nsURI, const QString &localName) const
{
    return d->element.hasAttributeNS(nsURI, localName);
}

} // namespace Syndication
