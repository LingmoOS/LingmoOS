// -*- c-basic-offset:4; indent-tabs-mode:nil -*-
/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2003 Alexander Kellett <lypanov@kde.org>
    SPDX-FileCopyrightText: 2008 Norbert Frese <nf2@scheinwelt.at>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kbookmarkmanager.h"
#include "kbookmarks_debug.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>

#include <KBackup>
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>

namespace
{
namespace Strings
{
QString piData()
{
    return QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"");
}
}
}

class KBookmarkMap : private KBookmarkGroupTraverser
{
public:
    KBookmarkMap()
        : m_mapNeedsUpdate(true)
    {
    }
    void setNeedsUpdate()
    {
        m_mapNeedsUpdate = true;
    }
    void update(KBookmarkManager *);
    QList<KBookmark> find(const QString &url) const
    {
        return m_bk_map.value(url);
    }

private:
    void visit(const KBookmark &) override;
    void visitEnter(const KBookmarkGroup &) override
    {
        ;
    }
    void visitLeave(const KBookmarkGroup &) override
    {
        ;
    }

private:
    typedef QList<KBookmark> KBookmarkList;
    QMap<QString, KBookmarkList> m_bk_map;
    bool m_mapNeedsUpdate;
};

void KBookmarkMap::update(KBookmarkManager *manager)
{
    if (m_mapNeedsUpdate) {
        m_mapNeedsUpdate = false;

        m_bk_map.clear();
        KBookmarkGroup root = manager->root();
        traverse(root);
    }
}

void KBookmarkMap::visit(const KBookmark &bk)
{
    if (!bk.isSeparator()) {
        // add bookmark to url map
        m_bk_map[bk.internalElement().attribute(QStringLiteral("href"))].append(bk);
    }
}

// #########################
// KBookmarkManagerPrivate
class KBookmarkManagerPrivate
{
public:
    KBookmarkManagerPrivate(bool bDocIsloaded)
        : m_doc(QStringLiteral("xbel"))
        , m_docIsLoaded(bDocIsloaded)
        , m_dirWatch(nullptr)
    {
    }

    mutable QDomDocument m_doc;
    mutable QDomDocument m_toolbarDoc;
    QString m_bookmarksFile;
    mutable bool m_docIsLoaded;

    KDirWatch *m_dirWatch; // for monitoring changes on bookmark files

    KBookmarkMap m_map;
};

// ################
// KBookmarkManager

static QDomElement createXbelTopLevelElement(QDomDocument &doc)
{
    QDomElement topLevel = doc.createElement(QStringLiteral("xbel"));
    topLevel.setAttribute(QStringLiteral("xmlns:mime"), QStringLiteral("http://www.freedesktop.org/standards/shared-mime-info"));
    topLevel.setAttribute(QStringLiteral("xmlns:bookmark"), QStringLiteral("http://www.freedesktop.org/standards/desktop-bookmarks"));
    topLevel.setAttribute(QStringLiteral("xmlns:kdepriv"), QStringLiteral("http://www.kde.org/kdepriv"));
    doc.appendChild(topLevel);
    doc.insertBefore(doc.createProcessingInstruction(QStringLiteral("xml"), Strings::piData()), topLevel);
    return topLevel;
}

KBookmarkManager::KBookmarkManager(const QString &bookmarksFile, QObject *parent)
    : QObject(parent)
    , d(new KBookmarkManagerPrivate(false))
{
    Q_ASSERT(!bookmarksFile.isEmpty());
    d->m_bookmarksFile = bookmarksFile;

    if (!QFile::exists(d->m_bookmarksFile)) {
        createXbelTopLevelElement(d->m_doc);
    } else {
        parse();
    }
    d->m_docIsLoaded = true;

    // start KDirWatch
    KDirWatch::self()->addFile(d->m_bookmarksFile);
    QObject::connect(KDirWatch::self(), &KDirWatch::dirty, this, &KBookmarkManager::slotFileChanged);
    QObject::connect(KDirWatch::self(), &KDirWatch::created, this, &KBookmarkManager::slotFileChanged);
    QObject::connect(KDirWatch::self(), &KDirWatch::deleted, this, &KBookmarkManager::slotFileChanged);

    // qCDebug(KBOOKMARKS_LOG) << "starting KDirWatch for" << d->m_bookmarksFile;
}

void KBookmarkManager::slotFileChanged(const QString &path)
{
    if (path == d->m_bookmarksFile) {
        // qCDebug(KBOOKMARKS_LOG) << "file changed (KDirWatch) " << path ;
        // Reparse
        parse();
        // Tell our GUI
        // (emit where group is "" to directly mark the root menu as dirty)
        Q_EMIT changed(QLatin1String(""));
    }
}

KBookmarkManager::~KBookmarkManager()
{
}

QDomDocument KBookmarkManager::internalDocument() const
{
    if (!d->m_docIsLoaded) {
        parse();
        d->m_toolbarDoc.clear();
    }
    return d->m_doc;
}

void KBookmarkManager::parse() const
{
    d->m_docIsLoaded = true;
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::parse " << d->m_bookmarksFile;
    QFile file(d->m_bookmarksFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(KBOOKMARKS_LOG) << "Can't open" << d->m_bookmarksFile;
        d->m_doc = QDomDocument(QStringLiteral("xbel"));
        createXbelTopLevelElement(d->m_doc);
        return;
    }
    d->m_doc = QDomDocument(QStringLiteral("xbel"));
    d->m_doc.setContent(&file);

    if (d->m_doc.documentElement().isNull()) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::parse : main tag is missing, creating default " << d->m_bookmarksFile;
        QDomElement element = d->m_doc.createElement(QStringLiteral("xbel"));
        d->m_doc.appendChild(element);
    }

    QDomElement docElem = d->m_doc.documentElement();

    QString mainTag = docElem.tagName();
    if (mainTag != QLatin1String("xbel")) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::parse : unknown main tag " << mainTag;
    }

    QDomNode n = d->m_doc.documentElement().previousSibling();
    if (n.isProcessingInstruction()) {
        QDomProcessingInstruction pi = n.toProcessingInstruction();
        pi.parentNode().removeChild(pi);
    }

    QDomProcessingInstruction pi;
    pi = d->m_doc.createProcessingInstruction(QStringLiteral("xml"), Strings::piData());
    d->m_doc.insertBefore(pi, docElem);

    file.close();

    d->m_map.setNeedsUpdate();
}

bool KBookmarkManager::save(bool toolbarCache) const
{
    return saveAs(d->m_bookmarksFile, toolbarCache);
}

bool KBookmarkManager::saveAs(const QString &filename, bool toolbarCache) const
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::save " << filename;

    // Save the bookmark toolbar folder for quick loading
    // but only when it will actually make things quicker
    const QString cacheFilename = filename + QLatin1String(".tbcache");
    if (toolbarCache && !root().isToolbarGroup()) {
        QSaveFile cacheFile(cacheFilename);
        if (cacheFile.open(QIODevice::WriteOnly)) {
            QString str;
            QTextStream stream(&str, QIODevice::WriteOnly);
            stream << root().findToolbar();
            const QByteArray cstr = str.toUtf8();
            cacheFile.write(cstr.data(), cstr.length());
            cacheFile.commit();
        }
    } else { // remove any (now) stale cache
        QFile::remove(cacheFilename);
    }

    // Create parent dirs
    QFileInfo info(filename);
    QDir().mkpath(info.absolutePath());

    if (filename == d->m_bookmarksFile) {
        KDirWatch::self()->removeFile(d->m_bookmarksFile);
    }

    QSaveFile file(filename);
    bool success = false;
    if (file.open(QIODevice::WriteOnly)) {
        KBackup::simpleBackupFile(file.fileName(), QString(), QStringLiteral(".bak"));
        QTextStream stream(&file);
        // In Qt6 it's UTF-8 by default
        stream << internalDocument().toString();
        stream.flush();
        success = file.commit();
    }

    if (filename == d->m_bookmarksFile) {
        KDirWatch::self()->addFile(d->m_bookmarksFile);
    }

    if (!success) {
        QString err = tr("Unable to save bookmarks in %1. Reported error was: %2. "
                         "This error message will only be shown once. The cause "
                         "of the error needs to be fixed as quickly as possible, "
                         "which is most likely a full hard drive.")
                          .arg(filename, file.errorString());
        qCCritical(KBOOKMARKS_LOG)
            << QStringLiteral("Unable to save bookmarks in %1. File reported the following error-code: %2.").arg(filename).arg(file.error());
        Q_EMIT const_cast<KBookmarkManager *>(this)->error(err);
    }

    return success;
}

QString KBookmarkManager::path() const
{
    return d->m_bookmarksFile;
}

KBookmarkGroup KBookmarkManager::root() const
{
    return KBookmarkGroup(internalDocument().documentElement());
}

KBookmarkGroup KBookmarkManager::toolbar()
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar begin";
    // Only try to read from a toolbar cache if the full document isn't loaded
    if (!d->m_docIsLoaded) {
        // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar trying cache";
        const QString cacheFilename = d->m_bookmarksFile + QLatin1String(".tbcache");
        QFileInfo bmInfo(d->m_bookmarksFile);
        QFileInfo cacheInfo(cacheFilename);
        if (d->m_toolbarDoc.isNull() && QFile::exists(cacheFilename) && bmInfo.lastModified() < cacheInfo.lastModified()) {
            // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar reading file";
            QFile file(cacheFilename);

            if (file.open(QIODevice::ReadOnly)) {
                d->m_toolbarDoc = QDomDocument(QStringLiteral("cache"));
                d->m_toolbarDoc.setContent(&file);
                // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar opened";
            }
        }
        if (!d->m_toolbarDoc.isNull()) {
            // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::toolbar returning element";
            QDomElement elem = d->m_toolbarDoc.firstChild().toElement();
            return KBookmarkGroup(elem);
        }
    }

    // Fallback to the normal way if there is no cache or if the bookmark file
    // is already loaded
    QDomElement elem = root().findToolbar();
    if (elem.isNull()) {
        // Root is the bookmark toolbar if none has been set.
        // Make it explicit to speed up invocations of findToolbar()
        root().internalElement().setAttribute(QStringLiteral("toolbar"), QStringLiteral("yes"));
        return root();
    } else {
        return KBookmarkGroup(elem);
    }
}

KBookmark KBookmarkManager::findByAddress(const QString &address)
{
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::findByAddress " << address;
    KBookmark result = root();
    // The address is something like /5/10/2+
    static const QRegularExpression separator(QStringLiteral("[/+]"));
    const QStringList addresses = address.split(separator, Qt::SkipEmptyParts);
    // qCWarning(KBOOKMARKS_LOG) << addresses.join(",");
    for (QStringList::const_iterator it = addresses.begin(); it != addresses.end();) {
        bool append = ((*it) == QLatin1String("+"));
        uint number = (*it).toUInt();
        Q_ASSERT(result.isGroup());
        KBookmarkGroup group = result.toGroup();
        KBookmark bk = group.first();
        KBookmark lbk = bk; // last non-null bookmark
        for (uint i = 0; ((i < number) || append) && !bk.isNull(); ++i) {
            lbk = bk;
            bk = group.next(bk);
            // qCWarning(KBOOKMARKS_LOG) << i;
        }
        it++;
        // qCWarning(KBOOKMARKS_LOG) << "found section";
        result = bk;
    }
    if (result.isNull()) {
        qCWarning(KBOOKMARKS_LOG) << "KBookmarkManager::findByAddress: couldn't find item " << address;
    }
    // qCWarning(KBOOKMARKS_LOG) << "found " << result.address();
    return result;
}

void KBookmarkManager::emitChanged()
{
    emitChanged(root());
}

void KBookmarkManager::emitChanged(const KBookmarkGroup &group)
{
    (void)save(); // KDE5 TODO: emitChanged should return a bool? Maybe rename it to saveAndEmitChanged?

    // Tell the other processes too
    // qCDebug(KBOOKMARKS_LOG) << "KBookmarkManager::emitChanged : broadcasting change " << group.address();

    Q_EMIT changed(group.address());
}

///////
bool KBookmarkManager::updateAccessMetadata(const QString &url)
{
    d->m_map.update(this);
    QList<KBookmark> list = d->m_map.find(url);
    if (list.isEmpty()) {
        return false;
    }

    for (QList<KBookmark>::iterator it = list.begin(); it != list.end(); ++it) {
        (*it).updateAccessMetadata();
    }

    return true;
}

#include "moc_kbookmarkmanager.cpp"
