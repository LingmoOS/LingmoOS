// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>
    SPDX-FileCopyrightText: 2008 Norbert Frese <nf2@scheinwelt.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmark.h"

#include <KStringHandler>
#include <kurlmimedata.h>

#include <QCoreApplication>
#include <QMimeDatabase>
#include <QStack>
#include <QDateTime>
#include <QMimeData>

namespace
{
namespace Strings
{
QString metaDataKDEOwner()
{
    return QStringLiteral("http://www.kde.org");
}
QString metaDataFreedesktopOwner()
{
    return QStringLiteral("http://freedesktop.org");
}
QString metaDataMimeOwner()
{
    return QStringLiteral("http://www.freedesktop.org/standards/shared-mime-info");
}

QString xbelMimeType()
{
    return QStringLiteral("application/x-xbel");
}
}
}

////// utility functions

static QDomNode cd(QDomNode node, const QString &name, bool create)
{
    QDomNode subnode = node.namedItem(name);
    if (create && subnode.isNull()) {
        subnode = node.ownerDocument().createElement(name);
        node.appendChild(subnode);
    }
    return subnode;
}

static QDomNode cd_or_create(const QDomNode &node, const QString &name)
{
    return cd(node, name, true);
}

static QDomText get_or_create_text(QDomNode node)
{
    QDomNode subnode = node.firstChild();
    if (subnode.isNull()) {
        subnode = node.ownerDocument().createTextNode(QLatin1String(""));
        node.appendChild(subnode);
    }
    return subnode.toText();
}

static QDomNode findMetadata(const QString &forOwner, QDomNode &parent, bool create)
{
    const bool forOwnerIsKDE = (forOwner == Strings::metaDataKDEOwner());

    QDomElement metadataElement;
    for (QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling()) {
        QDomElement elem = _node.toElement();
        if (!elem.isNull() && elem.tagName() == QLatin1String("metadata")) {
            const QString owner = elem.attribute(QStringLiteral("owner"));
            if (owner == forOwner) {
                return elem;
            }
            if (owner.isEmpty() && forOwnerIsKDE) {
                metadataElement = elem;
            }
        }
    }
    if (create && metadataElement.isNull()) {
        metadataElement = parent.ownerDocument().createElement(QStringLiteral("metadata"));
        parent.appendChild(metadataElement);
        metadataElement.setAttribute(QStringLiteral("owner"), forOwner);

    } else if (!metadataElement.isNull() && forOwnerIsKDE) {
        // i'm not sure if this is good, we shouldn't take over foreign metadata
        metadataElement.setAttribute(QStringLiteral("owner"), Strings::metaDataKDEOwner());
    }
    return metadataElement;
}

//////

KBookmarkGroup::KBookmarkGroup()
    : KBookmark(QDomElement())
{
}

KBookmarkGroup::KBookmarkGroup(const QDomElement &elem)
    : KBookmark(elem)
{
}

bool KBookmarkGroup::isOpen() const
{
    return element.attribute(QStringLiteral("folded")) == QLatin1String("no"); // default is: folded
}

KBookmark KBookmarkGroup::first() const
{
    return KBookmark(nextKnownTag(element.firstChildElement(), true));
}

KBookmark KBookmarkGroup::previous(const KBookmark &current) const
{
    return KBookmark(nextKnownTag(current.element.previousSiblingElement(), false));
}

KBookmark KBookmarkGroup::next(const KBookmark &current) const
{
    return KBookmark(nextKnownTag(current.element.nextSiblingElement(), true));
}

int KBookmarkGroup::indexOf(const KBookmark &child) const
{
    int counter = 0;
    for (KBookmark bk = first(); !bk.isNull(); bk = next(bk), ++counter) {
        if (bk.element == child.element) {
            return counter;
        }
    }
    return -1;
}

QDomElement KBookmarkGroup::nextKnownTag(const QDomElement &start, bool goNext) const
{
    for (QDomElement elem = start; !elem.isNull();) {
        QString tag = elem.tagName();
        if (tag == QLatin1String("folder") || tag == QLatin1String("bookmark") || tag == QLatin1String("separator")) {
            return elem;
        }
        if (goNext) {
            elem = elem.nextSiblingElement();
        } else {
            elem = elem.previousSiblingElement();
        }
    }
    return QDomElement();
}

KBookmarkGroup KBookmarkGroup::createNewFolder(const QString &text)
{
    if (isNull()) {
        return KBookmarkGroup();
    }
    QDomDocument doc = element.ownerDocument();
    QDomElement groupElem = doc.createElement(QStringLiteral("folder"));
    element.appendChild(groupElem);
    QDomElement textElem = doc.createElement(QStringLiteral("title"));
    groupElem.appendChild(textElem);
    textElem.appendChild(doc.createTextNode(text));
    return KBookmarkGroup(groupElem);
}

KBookmark KBookmarkGroup::createNewSeparator()
{
    if (isNull()) {
        return KBookmark();
    }
    QDomDocument doc = element.ownerDocument();
    Q_ASSERT(!doc.isNull());
    QDomElement sepElem = doc.createElement(QStringLiteral("separator"));
    element.appendChild(sepElem);
    return KBookmark(sepElem);
}

bool KBookmarkGroup::moveBookmark(const KBookmark &item, const KBookmark &after)
{
    QDomNode n;
    if (!after.isNull()) {
        n = element.insertAfter(item.element, after.element);
    } else { // first child
        if (element.firstChild().isNull()) { // Empty element -> set as real first child
            n = element.insertBefore(item.element, QDomElement());
        }

        // we have to skip everything up to the first valid child
        QDomElement firstChild = nextKnownTag(element.firstChild().toElement(), true);
        if (!firstChild.isNull()) {
            if (firstChild == item.element) { // item is already the first child, done
                return true;
            }
            n = element.insertBefore(item.element, firstChild);
        } else {
            // No real first child -> append after the <title> etc.
            n = element.appendChild(item.element);
        }
    }
    return (!n.isNull());
}

KBookmark KBookmarkGroup::addBookmark(const KBookmark &bm)
{
    element.appendChild(bm.internalElement());
    return bm;
}

KBookmark KBookmarkGroup::addBookmark(const QString &text, const QUrl &url, const QString &icon)
{
    if (isNull()) {
        return KBookmark();
    }
    QDomDocument doc = element.ownerDocument();
    QDomElement elem = doc.createElement(QStringLiteral("bookmark"));
    elem.setAttribute(QStringLiteral("href"), url.toString(QUrl::FullyEncoded));

    QDomElement textElem = doc.createElement(QStringLiteral("title"));
    elem.appendChild(textElem);
    textElem.appendChild(doc.createTextNode(text));

    KBookmark newBookmark = addBookmark(KBookmark(elem));

    // as icons are moved to metadata, we have to use the KBookmark API for this
    newBookmark.setIcon(icon);
    return newBookmark;
}

void KBookmarkGroup::deleteBookmark(const KBookmark &bk)
{
    element.removeChild(bk.element);
}

bool KBookmarkGroup::isToolbarGroup() const
{
    return (element.attribute(QStringLiteral("toolbar")) == QLatin1String("yes"));
}

QDomElement KBookmarkGroup::findToolbar() const
{
    if (element.attribute(QStringLiteral("toolbar")) == QLatin1String("yes")) {
        return element;
    }
    for (QDomElement e = element.firstChildElement(QStringLiteral("folder")); !e.isNull(); e = e.nextSiblingElement(QStringLiteral("folder"))) {
        QDomElement result = KBookmarkGroup(e).findToolbar();
        if (!result.isNull()) {
            return result;
        }
    }
    return QDomElement();
}

QList<QUrl> KBookmarkGroup::groupUrlList() const
{
    QList<QUrl> urlList;
    for (KBookmark bm = first(); !bm.isNull(); bm = next(bm)) {
        if (bm.isSeparator() || bm.isGroup()) {
            continue;
        }
        urlList << bm.url();
    }
    return urlList;
}

//////

KBookmark::KBookmark()
{
}

KBookmark::KBookmark(const QDomElement &elem)
    : element(elem)
{
}

bool KBookmark::isGroup() const
{
    QString tag = element.tagName();
    return tag == QLatin1String("folder") //
        || tag == QLatin1String("xbel"); // don't forget the toplevel group
}

bool KBookmark::isSeparator() const
{
    return (element.tagName() == QLatin1String("separator"));
}

bool KBookmark::isNull() const
{
    return element.isNull();
}

bool KBookmark::hasParent() const
{
    QDomElement parent = element.parentNode().toElement();
    return !parent.isNull();
}

QString KBookmark::text() const
{
    return KStringHandler::csqueeze(fullText());
}

QString KBookmark::fullText() const
{
    if (isSeparator()) {
        return QCoreApplication::translate("KBookmark", "--- separator ---", "Bookmark separator");
    }

    QString text = element.namedItem(QStringLiteral("title")).toElement().text();
    text.replace(QLatin1Char('\n'), QLatin1Char(' ')); // #140673
    return text;
}

void KBookmark::setFullText(const QString &fullText)
{
    QDomNode titleNode = element.namedItem(QStringLiteral("title"));
    if (titleNode.isNull()) {
        titleNode = element.ownerDocument().createElement(QStringLiteral("title"));
        element.appendChild(titleNode);
    }

    if (titleNode.firstChild().isNull()) {
        QDomText domtext = titleNode.ownerDocument().createTextNode(QLatin1String(""));
        titleNode.appendChild(domtext);
    }

    QDomText domtext = titleNode.firstChild().toText();
    domtext.setData(fullText);
}

QUrl KBookmark::url() const
{
    return QUrl(element.attribute(QStringLiteral("href")));
}

void KBookmark::setUrl(const QUrl &url)
{
    element.setAttribute(QStringLiteral("href"), url.toString());
}

QString KBookmark::icon() const
{
    QDomNode metaDataNode = metaData(Strings::metaDataFreedesktopOwner(), false);
    QDomElement iconElement = cd(metaDataNode, QStringLiteral("bookmark:icon"), false).toElement();

    QString icon = iconElement.attribute(QStringLiteral("name"));

    // migration code
    if (icon.isEmpty()) {
        icon = element.attribute(QStringLiteral("icon"));
    }
    if (icon == QLatin1String("www")) { // common icon for kde3 bookmarks
        return QStringLiteral("internet-web-browser");
    }
    // end migration code

    if (icon == QLatin1String("bookmark_folder")) {
        return QStringLiteral("folder-bookmarks");
    }
    if (icon.isEmpty()) {
        // Default icon depends on URL for bookmarks, and is default directory
        // icon for groups.
        if (isGroup()) {
            icon = QStringLiteral("folder-bookmarks");
        } else {
            if (isSeparator()) {
                icon = QStringLiteral("edit-clear"); // whatever
            } else {
                // get icon from mimeType
                QMimeDatabase db;
                QMimeType mime;
                QString _mimeType = mimeType();
                if (!_mimeType.isEmpty()) {
                    mime = db.mimeTypeForName(_mimeType);
                } else {
                    mime = db.mimeTypeForUrl(url());
                }
                if (mime.isValid()) {
                    icon = mime.iconName();
                }
            }
        }
    }
    return icon;
}

void KBookmark::setIcon(const QString &icon)
{
    QDomNode metaDataNode = metaData(Strings::metaDataFreedesktopOwner(), true);
    QDomElement iconElement = cd_or_create(metaDataNode, QStringLiteral("bookmark:icon")).toElement();
    iconElement.setAttribute(QStringLiteral("name"), icon);

    // migration code
    if (!element.attribute(QStringLiteral("icon")).isEmpty()) {
        element.removeAttribute(QStringLiteral("icon"));
    }
}

QString KBookmark::description() const
{
    if (isSeparator()) {
        return QString();
    }

    QString description = element.namedItem(QStringLiteral("desc")).toElement().text();
    description.replace(QLatin1Char('\n'), QLatin1Char(' ')); // #140673
    return description;
}

void KBookmark::setDescription(const QString &description)
{
    QDomNode descNode = element.namedItem(QStringLiteral("desc"));
    if (descNode.isNull()) {
        descNode = element.ownerDocument().createElement(QStringLiteral("desc"));
        element.appendChild(descNode);
    }

    if (descNode.firstChild().isNull()) {
        QDomText domtext = descNode.ownerDocument().createTextNode(QString());
        descNode.appendChild(domtext);
    }

    QDomText domtext = descNode.firstChild().toText();
    domtext.setData(description);
}

QString KBookmark::mimeType() const
{
    QDomNode metaDataNode = metaData(Strings::metaDataMimeOwner(), false);
    QDomElement mimeTypeElement = cd(metaDataNode, QStringLiteral("mime:mime-type"), false).toElement();
    return mimeTypeElement.attribute(QStringLiteral("type"));
}

void KBookmark::setMimeType(const QString &mimeType)
{
    QDomNode metaDataNode = metaData(Strings::metaDataMimeOwner(), true);
    QDomElement iconElement = cd_or_create(metaDataNode, QStringLiteral("mime:mime-type")).toElement();
    iconElement.setAttribute(QStringLiteral("type"), mimeType);
}

bool KBookmark::showInToolbar() const
{
    if (element.hasAttribute(QStringLiteral("showintoolbar"))) {
        bool show = element.attribute(QStringLiteral("showintoolbar")) == QLatin1String("yes");
        const_cast<QDomElement *>(&element)->removeAttribute(QStringLiteral("showintoolbar"));
        const_cast<KBookmark *>(this)->setShowInToolbar(show);
    }
    return metaDataItem(QStringLiteral("showintoolbar")) == QLatin1String("yes");
}

void KBookmark::setShowInToolbar(bool show)
{
    setMetaDataItem(QStringLiteral("showintoolbar"), show ? QStringLiteral("yes") : QStringLiteral("no"));
}

KBookmarkGroup KBookmark::parentGroup() const
{
    return KBookmarkGroup(element.parentNode().toElement());
}

KBookmarkGroup KBookmark::toGroup() const
{
    Q_ASSERT(isGroup());
    return KBookmarkGroup(element);
}

QString KBookmark::address() const
{
    if (element.tagName() == QLatin1String("xbel")) {
        return QLatin1String(""); // not QString() !
    } else {
        // Use keditbookmarks's DEBUG_ADDRESSES flag to debug this code :)
        if (element.parentNode().isNull()) {
            Q_ASSERT(false);
            return QStringLiteral("ERROR"); // Avoid an infinite loop
        }
        KBookmarkGroup group = parentGroup();
        QString parentAddress = group.address();
        int pos = group.indexOf(*this);
        Q_ASSERT(pos != -1);
        return parentAddress + QLatin1Char('/') + QString::number(pos);
    }
}

int KBookmark::positionInParent() const
{
    return parentGroup().indexOf(*this);
}

QDomElement KBookmark::internalElement() const
{
    return element;
}

KBookmark KBookmark::standaloneBookmark(const QString &text, const QUrl &url, const QString &icon)
{
    QDomDocument doc(QStringLiteral("xbel"));
    QDomElement elem = doc.createElement(QStringLiteral("xbel"));
    doc.appendChild(elem);
    KBookmarkGroup grp(elem);
    grp.addBookmark(text, url, icon);
    return grp.first();
}

QString KBookmark::commonParent(const QString &first, const QString &second)
{
    QString A = first;
    QString B = second;
    QString error(QStringLiteral("ERROR"));
    if (A == error || B == error) {
        return error;
    }

    A += QLatin1Char('/');
    B += QLatin1Char('/');

    int lastCommonSlash = 0;
    int lastPos = A.length() < B.length() ? A.length() : B.length();
    for (int i = 0; i < lastPos; ++i) {
        if (A[i] != B[i]) {
            return A.left(lastCommonSlash);
        }
        if (A[i] == QLatin1Char('/')) {
            lastCommonSlash = i;
        }
    }
    return A.left(lastCommonSlash);
}

void KBookmark::updateAccessMetadata()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmark::updateAccessMetadata " << address() << " " << url();

    const uint timet = QDateTime::currentDateTimeUtc().toSecsSinceEpoch();
    setMetaDataItem(QStringLiteral("time_added"), QString::number(timet), DontOverwriteMetaData);
    setMetaDataItem(QStringLiteral("time_visited"), QString::number(timet));

    QString countStr = metaDataItem(QStringLiteral("visit_count")); // TODO use spec'ed name
    bool ok;
    int currentCount = countStr.toInt(&ok);
    if (!ok) {
        currentCount = 0;
    }
    currentCount++;
    setMetaDataItem(QStringLiteral("visit_count"), QString::number(currentCount));

    // TODO - time_modified
}

QString KBookmark::parentAddress(const QString &address)
{
    return address.left(address.lastIndexOf(QLatin1Char('/')));
}

uint KBookmark::positionInParent(const QString &address)
{
    return QStringView(address).mid(address.lastIndexOf(QLatin1Char('/')) + 1).toInt();
}

QString KBookmark::previousAddress(const QString &address)
{
    uint pp = positionInParent(address);
    return pp > 0 ? parentAddress(address) + QLatin1Char('/') + QString::number(pp - 1) : QString();
}

QString KBookmark::nextAddress(const QString &address)
{
    return parentAddress(address) + QLatin1Char('/') + QString::number(positionInParent(address) + 1);
}

QDomNode KBookmark::metaData(const QString &owner, bool create) const
{
    QDomNode infoNode = cd(internalElement(), QStringLiteral("info"), create);
    if (infoNode.isNull()) {
        return QDomNode();
    }
    return findMetadata(owner, infoNode, create);
}

QString KBookmark::metaDataItem(const QString &key) const
{
    QDomNode metaDataNode = metaData(Strings::metaDataKDEOwner(), false);
    for (QDomElement e = metaDataNode.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
        if (e.tagName() == key) {
            return e.text();
        }
    }
    return QString();
}

void KBookmark::setMetaDataItem(const QString &key, const QString &value, MetaDataOverwriteMode mode)
{
    QDomNode metaDataNode = metaData(Strings::metaDataKDEOwner(), true);
    QDomNode item = cd_or_create(metaDataNode, key);
    QDomText text = get_or_create_text(item);
    if (mode == DontOverwriteMetaData && !text.data().isEmpty()) {
        return;
    }

    text.setData(value);
}

bool KBookmark::operator==(const KBookmark &rhs) const
{
    return element == rhs.element;
}

////

KBookmarkGroupTraverser::~KBookmarkGroupTraverser()
{
}

void KBookmarkGroupTraverser::traverse(const KBookmarkGroup &root)
{
    QStack<KBookmarkGroup> stack;
    stack.push(root);
    KBookmark bk = root.first();
    for (;;) {
        if (bk.isNull()) {
            if (stack.count() == 1) { // only root is on the stack
                return;
            }
            if (!stack.isEmpty()) {
                visitLeave(stack.top());
                bk = stack.pop();
            }
            bk = stack.top().next(bk);
        } else if (bk.isGroup()) {
            KBookmarkGroup gp = bk.toGroup();
            visitEnter(gp);
            bk = gp.first();
            stack.push(gp);
        } else {
            visit(bk);
            bk = stack.top().next(bk);
        }
    }
}

void KBookmarkGroupTraverser::visit(const KBookmark &)
{
}

void KBookmarkGroupTraverser::visitEnter(const KBookmarkGroup &)
{
}

void KBookmarkGroupTraverser::visitLeave(const KBookmarkGroup &)
{
}

void KBookmark::populateMimeData(QMimeData *mimeData) const
{
    KBookmark::List bookmarkList;
    bookmarkList.append(*this);
    bookmarkList.populateMimeData(mimeData);
}

KBookmark::List::List()
    : QList<KBookmark>()
{
}

void KBookmark::List::populateMimeData(QMimeData *mimeData) const
{
    QList<QUrl> urls;

    QDomDocument doc(QStringLiteral("xbel"));
    QDomElement elem = doc.createElement(QStringLiteral("xbel"));
    doc.appendChild(elem);

    for (const_iterator it = begin(), end = this->end(); it != end; ++it) {
        urls.append((*it).url());
        elem.appendChild((*it).internalElement().cloneNode(true /* deep */));
    }

    // This sets text/uri-list and text/plain into the mimedata
    mimeData->setUrls(urls);

    mimeData->setData(Strings::xbelMimeType(), doc.toByteArray());
}

bool KBookmark::List::canDecode(const QMimeData *mimeData)
{
    return mimeData->hasFormat(Strings::xbelMimeType()) || mimeData->hasUrls();
}

QStringList KBookmark::List::mimeDataTypes()
{
    return QStringList() << Strings::xbelMimeType() << KUrlMimeData::mimeDataTypes();
}

KBookmark::List KBookmark::List::fromMimeData(const QMimeData *mimeData, QDomDocument &doc)
{
    KBookmark::List bookmarks;
    const QByteArray payload = mimeData->data(Strings::xbelMimeType());
    if (!payload.isEmpty()) {
        doc.setContent(payload);
        QDomElement elem = doc.documentElement();
        const QDomNodeList children = elem.childNodes();
        bookmarks.reserve(children.count());
        for (int childno = 0; childno < children.count(); childno++) {
            bookmarks.append(KBookmark(children.item(childno).toElement()));
        }
        return bookmarks;
    }
    const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
    bookmarks.reserve(urls.size());
    for (int i = 0; i < urls.size(); ++i) {
        const QUrl url = urls.at(i);
        bookmarks.append(KBookmark::standaloneBookmark(url.toDisplayString(), url, QString() /*TODO icon*/));
    }
    return bookmarks;
}
