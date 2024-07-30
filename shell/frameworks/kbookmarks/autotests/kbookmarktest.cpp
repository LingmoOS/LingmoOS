/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <kbookmark.h>
#include <kbookmarkmanager.h>

#include <QDir>
#include <QMimeData>
#include <QObject>
#include <QStandardPaths>
#include <QTest>

class KBookmarkTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testMimeDataOneBookmark();
    void testMimeDataBookmarkList();
    void testFileCreatedExternally();
    void testBookmarkManager();
};

static const QString placesFile()
{
    const QString datadir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    QDir().mkpath(datadir);
    return datadir + "/user-places.xbel";
}

void KBookmarkTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    QFile::remove(placesFile());
}

void KBookmarkTest::cleanupTestCase()
{
    QFile::remove(placesFile());
}

static void compareBookmarks(const KBookmark &initialBookmark, const KBookmark &decodedBookmark)
{
    QCOMPARE(decodedBookmark.url(), initialBookmark.url());
    QCOMPARE(decodedBookmark.icon(), initialBookmark.icon());
    QCOMPARE(decodedBookmark.text(), initialBookmark.text());
    QCOMPARE(decodedBookmark.description(), initialBookmark.description());
    QDomNamedNodeMap decodedAttribs = decodedBookmark.internalElement().attributes();
    QDomNamedNodeMap initialAttribs = initialBookmark.internalElement().attributes();
    QCOMPARE(decodedAttribs.count(), initialAttribs.count());
    for (int i = 0; i < decodedAttribs.length(); ++i) {
        QDomAttr decodedAttr = decodedAttribs.item(i).toAttr();
        QDomAttr initialAttr = initialAttribs.item(i).toAttr();
        QCOMPARE(decodedAttr.name(), initialAttr.name());
        QCOMPARE(decodedAttr.value(), initialAttr.value());
    }
}

void KBookmarkTest::testMimeDataOneBookmark()
{
    QMimeData *mimeData = new QMimeData;

    KBookmark bookmark = KBookmark::standaloneBookmark(QStringLiteral("KDE"), QUrl(QStringLiteral("http://www.kde.org")), QStringLiteral("icon"));
    bookmark.setDescription(QStringLiteral("Comment"));
    QVERIFY(!bookmark.isNull());
    bookmark.populateMimeData(mimeData);

    QDomDocument doc;
    QVERIFY(mimeData->hasUrls());
    QVERIFY(KBookmark::List::canDecode(mimeData));
    const KBookmark::List decodedBookmarks = KBookmark::List::fromMimeData(mimeData, doc);
    QVERIFY(!decodedBookmarks.isEmpty());
    QCOMPARE(decodedBookmarks.count(), 1);
    QVERIFY(!decodedBookmarks[0].isNull());
    compareBookmarks(bookmark, decodedBookmarks[0]);

    // Do like keditbookmarks's paste code: (CreateCommand::execute)
    // Crashed before passing "doc" to fromMimeData (#160679)
    QDomElement clonedElem = decodedBookmarks[0].internalElement().cloneNode().toElement();

    delete mimeData;
}

void KBookmarkTest::testMimeDataBookmarkList()
{
    QMimeData *mimeData = new QMimeData;

    KBookmark bookmark1 = KBookmark::standaloneBookmark(QStringLiteral("KDE"), QUrl(QStringLiteral("http://www.kde.org")), QStringLiteral("icon"));
    bookmark1.setDescription(QStringLiteral("KDE comment"));
    QVERIFY(!bookmark1.isNull());
    KBookmark bookmark2 = KBookmark::standaloneBookmark(QStringLiteral("KOffice"), QUrl(QStringLiteral("http://www.koffice.org")), QStringLiteral("koicon"));
    bookmark2.setDescription(QStringLiteral("KOffice comment"));
    QVERIFY(!bookmark2.isNull());
    bookmark2.setMetaDataItem(QStringLiteral("key"), QStringLiteral("value"));

    KBookmark::List initialBookmarks;
    initialBookmarks.append(bookmark1);
    initialBookmarks.append(bookmark2);
    initialBookmarks.populateMimeData(mimeData);

    QDomDocument doc;
    QVERIFY(mimeData->hasUrls());
    QVERIFY(KBookmark::List::canDecode(mimeData));
    const KBookmark::List decodedBookmarks = KBookmark::List::fromMimeData(mimeData, doc);
    QCOMPARE(decodedBookmarks.count(), 2);
    QVERIFY(!decodedBookmarks[0].isNull());
    QVERIFY(!decodedBookmarks[1].isNull());
    compareBookmarks(bookmark1, decodedBookmarks[0]);
    compareBookmarks(bookmark2, decodedBookmarks[1]);

    delete mimeData;
}

void KBookmarkTest::testFileCreatedExternally()
{
    KBookmarkManager sharedBookmarkManager(placesFile());
    QVERIFY(sharedBookmarkManager.root().first().isNull());

    QFile file(placesFile());
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?><xbel version=\"1.0\"><bookmark href=\"file:///external\"><title>external</title></bookmark></xbel>");
    file.close();

    QTRY_VERIFY(!sharedBookmarkManager.root().first().isNull());
    KBookmark bk = sharedBookmarkManager.root().first();
    QCOMPARE(bk.url().toString(), QString("file:///external"));
    QCOMPARE(bk.fullText(), QString("external"));
}

void KBookmarkTest::testBookmarkManager()
{
    // like kfileplacesmodel.cpp used to do
    const QString placesModelFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kfileplaces/bookmarks.xml";
    KBookmarkManager bookmarkManager(placesModelFile);

    // like kfileplacessharedbookmarks.cpp does
    KBookmarkManager sharedBookmarkManager(placesFile());

    // like kfilebookmarkhandler.cpp does
    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kfile/bookmarks.xml"));
    if (file.isEmpty()) {
        file = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/kfile/bookmarks.xml";
    }
    KBookmarkManager manager(file);

    // This is just to check an old crash in the global-static destructor, not doing anything with
    // these managers yet.
    Q_UNUSED(bookmarkManager);
    Q_UNUSED(sharedBookmarkManager);
    Q_UNUSED(manager);
}

QTEST_MAIN(KBookmarkTest)

#include "kbookmarktest.moc"
