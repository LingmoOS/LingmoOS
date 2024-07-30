/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "metadatamover.h"

#include "database.h"
#include "transaction.h"
#include "document.h"
#include "termgenerator.h"

#include <memory>
#include <QDir>
#include <QTemporaryDir>
#include <QTest>

using namespace Baloo;

class MetadataMoverTest : public QObject
{
    Q_OBJECT
public:
    MetadataMoverTest(QObject* parent = nullptr);

private Q_SLOTS:

    void init();
    void cleanup();

    void testRemoveFile();
    void testRenameFile();
    void testMoveFile();
    void testMoveRenameFile();
    void testMoveFolder();
    void testMoveTwiceSequential();
    void testMoveDeleteFile();
    void testMoveFileRenameParent();
    void testRenameMoveFileRenameParent();
    void testMoveFileMoveParent();

private:
    quint64 insertDoc(const QString& url, quint64 parentId);

    std::unique_ptr<QTemporaryDir> m_tempDir;
    std::unique_ptr<Database> m_db;

    struct DocumentInfo {
        QString url;
        DocumentTimeDB::TimeInfo timeInfo;
        QVector<QByteArray> docTerms;
        QVector<QByteArray> filenameTerms;
    };
    DocumentInfo documentInfo(quint64 docId);
};

MetadataMoverTest::MetadataMoverTest(QObject* parent)
    : QObject(parent)
{
}

void MetadataMoverTest::cleanup()
{
    m_db.reset();
    m_tempDir.reset();
}

void MetadataMoverTest::init()
{
    m_tempDir = std::make_unique<QTemporaryDir>();
    m_db = std::make_unique<Database>(m_tempDir->path());
    m_db->open(Database::CreateDatabase);
    QVERIFY(m_db->isOpen());
}

quint64 MetadataMoverTest::insertDoc(const QString& url, quint64 parentId)
{
    static quint64 fileId = 1;
    fileId++;

    Document doc;
    const QByteArray path = QFile::encodeName(url);
    doc.setUrl(path);
    doc.setId(fileId);
    doc.setParentId(parentId);

    // Docterms are mandatory. Use "Type:Empty" as placeholder
    doc.addTerm("T0");

    auto lastSlash = path.lastIndexOf('/');
    const QByteArray fileName = path.mid(lastSlash + 1);
    TermGenerator tg(doc);
    tg.indexFileNameText(QFile::decodeName(fileName));

    Transaction tr(m_db.get(), Transaction::ReadWrite);
    tr.addDocument(doc);
    tr.commit();
    return doc.id();
}

MetadataMoverTest::DocumentInfo MetadataMoverTest::documentInfo(quint64 id)
{
    Transaction tr(m_db.get(), Transaction::ReadOnly);
    if (!tr.hasDocument(id)) {
        return DocumentInfo();
    }

    DocumentInfo docInfo;
    docInfo.url = QFile::decodeName(tr.documentUrl(id));
    docInfo.timeInfo = tr.documentTimeInfo(id);
    docInfo.docTerms = tr.documentTerms(id);
    docInfo.filenameTerms = tr.documentFileNameTerms(id);

    return docInfo;
}

void MetadataMoverTest::testRemoveFile()
{
    QString url = QStringLiteral("/somefile");
    quint64 fid = insertDoc(url, 99);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db.get(), this);
    mover.removeFileMetadata(url);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(!tr.hasDocument(fid));
    }
}

// Rename "file" to "file2"
// mv <tmpdir>/file <tmpdir>/file2
void MetadataMoverTest::testRenameFile()
{
    QTemporaryDir dir;
    auto parentId = filePathToId(QFile::encodeName(dir.path()));

    QString url = dir.path() + QStringLiteral("/file");
    quint64 fid = insertDoc(url, parentId);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file2");
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QVERIFY(newInfo.filenameTerms != oldInfo.filenameTerms);
    QVERIFY(!newInfo.filenameTerms.empty());
}

// Change parent directory of "file"
// mv <tmpdir>/a/b/c/file <tmpdir>/file
void MetadataMoverTest::testMoveFile()
{
    QTemporaryDir dir;
    auto parentId = filePathToId(QFile::encodeName(dir.path()));

    auto idA = insertDoc(dir.path() + QStringLiteral("/a"), parentId);
    auto idB = insertDoc(dir.path() + QStringLiteral("/a/b"), idA);
    auto idC = insertDoc(dir.path() + QStringLiteral("/a/b/c"), idB);

    QString url = dir.path() + QStringLiteral("/a/b/c/file");
    quint64 fid = insertDoc(url, idC);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file");
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QCOMPARE(newInfo.filenameTerms, oldInfo.filenameTerms);
}

// Change parent directory of "file", and rename to "file2"
// mv <tmpdir>/a/b/c/file <tmpdir>/file2
void MetadataMoverTest::testMoveRenameFile()
{
    QTemporaryDir dir;
    auto parentId = filePathToId(QFile::encodeName(dir.path()));

    auto idA = insertDoc(dir.path() + QStringLiteral("/a"), parentId);
    auto idB = insertDoc(dir.path() + QStringLiteral("/a/b"), idA);
    auto idC = insertDoc(dir.path() + QStringLiteral("/a/b/c"), idB);

    QString url = dir.path() + QStringLiteral("/a/b/c/file");
    quint64 fid = insertDoc(url, idC);

    DocumentInfo oldInfo = documentInfo(fid);
    QVERIFY(!oldInfo.url.isEmpty());
    QCOMPARE(oldInfo.url, url);

    MetadataMover mover(m_db.get(), this);
    QString url2 = dir.path() + QStringLiteral("/file2");
    mover.moveFileMetadata(url, url2);

    DocumentInfo newInfo = documentInfo(fid);
    QCOMPARE(newInfo.url, url2);
    QCOMPARE(newInfo.docTerms, oldInfo.docTerms);
    QVERIFY(newInfo.filenameTerms != oldInfo.filenameTerms);
    QVERIFY(!newInfo.filenameTerms.empty());
}

// Rename parent directory of "file"
// mv <tmpdir>/folder <tmpdir>/dir
// New path of <tmpdir>/folder/file becomes <tmpdir>/dir/file
void MetadataMoverTest::testMoveFolder()
{
    QTemporaryDir dir;
    auto parentId = filePathToId(QFile::encodeName(dir.path()));

    QString folder = dir.path() + QStringLiteral("/folder");
    quint64 did = insertDoc(folder, parentId);

    QString fileUrl = folder + QStringLiteral("/file");
    quint64 fid = insertDoc(fileUrl, did);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    QString newFolderUrl = dir.path() + QStringLiteral("/dir");
    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(folder, newFolderUrl);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(newFolderUrl));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(newFolderUrl) + "/file");
    }
}

// Rename a file twice in a row
// Mimic Inotify/Filewatch behavior as if there was sufficient
// time to process the first rename before the second happens
void MetadataMoverTest::testMoveTwiceSequential()
{
    QTemporaryDir dir;
    auto did = filePathToId(QFile::encodeName(dir.path()));

    const QString fileUrl = dir.path() + QStringLiteral("/file");
    const quint64 fid = insertDoc(fileUrl, did);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    MetadataMover mover(m_db.get(), this);

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/file1");
    mover.moveFileMetadata(fileUrl, fileUrl1);

    // Second rename
    const QString fileUrl2 = dir.path() + QStringLiteral("/file2");
    mover.moveFileMetadata(fileUrl1, fileUrl2);

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(fileUrl2));
    }
}

// Rename a file and then immediately delete it
void MetadataMoverTest::testMoveDeleteFile()
{
    QTemporaryDir dir;
    auto did = filePathToId(QFile::encodeName(dir.path()));

    const QString fileUrl = dir.path() + QStringLiteral("/file");
    const quint64 fid = insertDoc(fileUrl, did);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(fid));
    }

    // Rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/file1");

    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);

    // Delete
    mover.removeFileMetadata(fileUrl1);

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QVERIFY(!tr.hasDocument(fid));
    }
}

// Move a file to a different parent directory and rename the new parent
void MetadataMoverTest::testMoveFileRenameParent()
{
    QTemporaryDir dir;
    auto did = filePathToId(QFile::encodeName(dir.path()));

    quint64 did_a = insertDoc(dir.path() + QStringLiteral("/a"), did);
    quint64 did_b = insertDoc(dir.path() + QStringLiteral("/b"), did);

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    const quint64 fid = insertDoc(fileUrl, did_a);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + QStringLiteral("/b/file1");

    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl_b1);

    // Rename parent (delete old dir first)
    mover.removeFileMetadata(dir.path() + QStringLiteral("/a"));
    mover.moveFileMetadata(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path()) + "/a");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/a/file1");
    }
}

// Rename a file, move to a different parent directory and rename the new parent
void MetadataMoverTest::testRenameMoveFileRenameParent()
{
    QTemporaryDir dir;
    auto did = filePathToId(QFile::encodeName(dir.path()));

    quint64 did_a = insertDoc(dir.path() + QStringLiteral("/a"), did);
    quint64 did_b = insertDoc(dir.path() + QStringLiteral("/b"), did);

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    const quint64 fid = insertDoc(fileUrl, did_a);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(did_b));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl_a1 = dir.path() + QStringLiteral("/a/file1");

    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl_a1);

    // Move to new parent
    const QString fileUrl_b1 = dir.path() + QStringLiteral("/b/file1");
    mover.moveFileMetadata(fileUrl_a1, fileUrl_b1);

    // Rename parent (delete old dir first)
    mover.removeFileMetadata(dir.path() + QStringLiteral("/a"));
    mover.moveFileMetadata(dir.path() + QStringLiteral("/b"), dir.path() + QStringLiteral("/a"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_b), QFile::encodeName(dir.path()) + "/a");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/a/file1");
    }
}

// Rename a file and then immediately rename a parent directory
void MetadataMoverTest::testMoveFileMoveParent()
{
    QTemporaryDir dir;
    auto did = filePathToId(QFile::encodeName(dir.path()));

    quint64 did_a = insertDoc(dir.path() + QStringLiteral("/a"), did);

    const QString fileUrl = dir.path() + QStringLiteral("/a/file");
    const quint64 fid = insertDoc(fileUrl, did_a);

    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QVERIFY(tr.hasDocument(fid));
    }

    // First rename
    const QString fileUrl1 = dir.path() + QStringLiteral("/a/file1");

    MetadataMover mover(m_db.get(), this);
    mover.moveFileMetadata(fileUrl, fileUrl1);

    // Rename parent
    mover.moveFileMetadata(dir.path() + QStringLiteral("/a"), dir.path() + QStringLiteral("/b"));

    // Check result
    {
        Transaction tr(m_db.get(), Transaction::ReadOnly);
        QVERIFY(tr.hasDocument(did));
        QVERIFY(tr.hasDocument(did_a));
        QCOMPARE(tr.documentUrl(did), QFile::encodeName(dir.path()));
        QCOMPARE(tr.documentUrl(did_a), QFile::encodeName(dir.path()) + "/b");
        QVERIFY(tr.hasDocument(fid));
        QCOMPARE(tr.documentUrl(fid), QFile::encodeName(dir.path()) + "/b/file1");
    }
}

QTEST_GUILESS_MAIN(MetadataMoverTest)

#include "metadatamovertest.moc"
