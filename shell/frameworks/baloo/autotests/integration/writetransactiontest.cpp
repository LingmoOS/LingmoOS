/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "writetransaction.h"
#include "dbstate.h"
#include "database.h"
#include "idutils.h"

#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class WriteTransactionTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init() {
        dir = std::make_unique<QTemporaryDir>();
        db = std::make_unique<Database>(dir->path());
        db->open(Database::CreateDatabase);
        m_dirId = filePathToId(QFile::encodeName(dir->path()));
        QVERIFY(m_dirId);
    }

    void cleanup() {
        db.reset();
        dir.reset();
    }

    void testAddDocument();
    void testAddDocumentTwoDocuments();
    void testAddAndRemoveOneDocument();
    void testAddAndReplaceOneDocument();
    void testIdempotentDocumentChange();

    void testRemoveRecursively();
    void testDocumentId();
    void testTermPositions();
    void testTransactionReset();
private:
    std::unique_ptr<QTemporaryDir> dir;
    std::unique_ptr<Database> db;
    quint64 m_dirId = 0;
};

void WriteTransactionTest::testAddDocument()
{
    Transaction tr(db.get(), Transaction::ReadWrite);

    const QString filePath(dir->path() + QStringLiteral("/file"));
    QByteArray url = QFile::encodeName(filePath);
    quint64 id = 99;

    QCOMPARE(tr.hasDocument(id), false);

    Document doc;
    doc.setId(id);
    doc.setParentId(m_dirId);
    doc.setUrl(url);
    doc.addTerm("a");
    doc.addTerm("ab");
    doc.addTerm("abc");
    doc.addTerm("power");
    doc.addFileNameTerm("link");
    doc.addXattrTerm("system");
    doc.setMTime(1);
    doc.setCTime(2);

    tr.addDocument(doc);
    tr.commit();

    Transaction tr2(db.get(), Transaction::ReadOnly);

    DBState state;
    state.postingDb = {{"a", {id}}, {"ab", {id}}, {"abc", {id}}, {"power", {id}}, {"system", {id}}, {"link", {id}}};
    state.positionDb = {};
    state.docTermsDb = {{id, {"a", "ab", "abc", "power"} }};
    state.docFileNameTermsDb = {{id, {"link"} }};
    state.docXAttrTermsDb = {{id, {"system"} }};
    state.docTimeDb = {{id, DocumentTimeDB::TimeInfo(1, 2)}};
    state.mtimeDb = {{1, id}};

    DBState actualState = DBState::fromTransaction(&tr2);
    QCOMPARE(actualState, state);
}

static Document createDocument(const QString& filePath, quint32 mtime, quint32 ctime, const QVector<QByteArray>& terms,
                               const QVector<QByteArray>& fileNameTerms, const QVector<QByteArray>& xattrTerms, quint64 parentId)
{
    Document doc;

    // Generate a unique file id. A monotonic counter is sufficient here.
    static quint64 fileid = parentId;
    fileid++;

    const QByteArray url = QFile::encodeName(filePath);
    doc.setId(fileid);
    doc.setParentId(parentId);
    doc.setUrl(url);

    for (const QByteArray& term: terms) {
        doc.addTerm(term);
    }
    for (const QByteArray& term: fileNameTerms) {
        doc.addFileNameTerm(term);
    }
    for (const QByteArray& term: xattrTerms) {
        doc.addXattrTerm(term);
    }
    doc.setMTime(mtime);
    doc.setCTime(ctime);

    return doc;
}

void WriteTransactionTest::testAddDocumentTwoDocuments()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));
    const QString url2(dir->path() + QStringLiteral("/file2"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);
    Document doc2 = createDocument(url2, 6, 2, {"a", "abcd", "dab"}, {"file2"}, {}, m_dirId);

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.addDocument(doc2);
        tr.commit();
    }

    Transaction tr(db.get(), Transaction::ReadOnly);

    quint64 id1 = doc1.id();
    quint64 id2 = doc2.id();

    DBState state;
    state.postingDb = {{"a", {id1, id2}}, {"abc", {id1}}, {"abcd", {id2}}, {"dab", {id1, id2}}, {"file1", {id1}}, {"file2", {id2}}};
    state.positionDb = {};
    state.docTermsDb = {{id1, {"a", "abc", "dab"}}, {id2, {"a", "abcd", "dab"}}};
    state.docFileNameTermsDb = {{id1, {"file1"}}, {id2, {"file2"}}};
    state.docXAttrTermsDb = {};
    state.docTimeDb = {{id1, DocumentTimeDB::TimeInfo(5, 1)}, {id2, DocumentTimeDB::TimeInfo(6, 2)}};
    state.mtimeDb = {{5, id1}, {6, id2}};

    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, state));
}

void WriteTransactionTest::testAddAndRemoveOneDocument()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }
    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.removeDocument(doc1.id());
        tr.commit();
    }

    Transaction tr(db.get(), Transaction::ReadOnly);
    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, DBState()));
}

void WriteTransactionTest::testAddAndReplaceOneDocument()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);
    Document doc2 = createDocument(url1, 6, 2, {"a", "abc", "xxx"}, {"file1", "yyy"}, {}, m_dirId);
    quint64 id = doc1.id();
    doc2.setId(id);

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }

    DBState state;
    state.postingDb = {{"a", {id}}, {"abc", {id}}, {"dab", {id}}, {"file1", {id}} };
    state.positionDb = {};
    state.docTermsDb = {{id, {"a", "abc", "dab"} }};
    state.docFileNameTermsDb = {{id, {"file1"} }};
    state.docXAttrTermsDb = {};
    state.docTimeDb = {{id, DocumentTimeDB::TimeInfo(5, 1)}};
    state.mtimeDb = {{5, id}};

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.replaceDocument(doc2, DocumentOperation::Everything);
        tr.commit();
    }

    state.postingDb = {{"a", {id}}, {"abc", {id}}, {"xxx", {id}}, {"file1", {id}}, {"yyy", {id}} };
    state.docTermsDb = {{id, {"a", "abc", "xxx"} }};
    state.docFileNameTermsDb = {{id, {"file1", "yyy"} }};
    state.docTimeDb = {{id, DocumentTimeDB::TimeInfo(6, 2)}};
    state.mtimeDb = {{6, id}};

    Transaction tr(db.get(), Transaction::ReadOnly);
    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, state));
}

void WriteTransactionTest::testRemoveRecursively()
{
    const QString path = dir->path();
    const QString url1(path + QStringLiteral("/file1"));
    const QString dirPath(path + QStringLiteral("/dir"));
    const QString url2(dirPath + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);
    Document doc2 = createDocument(dirPath, 5, 1, {"a"}, {"dir"}, {}, m_dirId);
    Document doc3 = createDocument(url2, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, doc2.id());

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.addDocument(doc2);
        tr.addDocument(doc3);
        tr.commit();
    }
    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.removeRecursively(m_dirId);
        tr.commit();
    }

    Transaction tr(db.get(), Transaction::ReadOnly);
    DBState actualState = DBState::fromTransaction(&tr);
    QVERIFY(DBState::debugCompare(actualState, DBState()));
}

void WriteTransactionTest::testDocumentId()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(tr.documentId(doc1.url()), doc1.id());
}

void WriteTransactionTest::testTermPositions()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));
    const QString url2(dir->path() + QStringLiteral("/file2"));
    const QString url3(dir->path() + QStringLiteral("/file3"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);
    quint64 id1 = doc1.id();
    Document doc2 = createDocument(url2, 5, 2, {"a", "abcd"}, {"file2"}, {}, m_dirId);
    quint64 id2 = doc2.id();
    Document doc3 = createDocument(url3, 6, 3, {"dab"}, {"file3"}, {}, m_dirId);
    quint64 id3 = doc3.id();

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.addDocument(doc2);
        tr.addDocument(doc3);
        tr.commit();
    }

    DBState state;
    state.postingDb = {{"a", {id1, id2}}, {"abc", {id1}}, {"abcd", {id2}}, {"dab", {id1, id3}}, {"file1", {id1}}, {"file2", {id2}}, {"file3", {id3}} };
    state.positionDb = {};
    state.docTermsDb = {{id1, {"a", "abc", "dab"}}, {id2, {"a", "abcd"}}, {id3, {"dab"}} };
    state.docFileNameTermsDb = {{id1, {"file1"}}, {id2, {"file2"}}, {id3, {"file3"}} };
    state.docXAttrTermsDb = {};
    state.docTimeDb = {{id1, DocumentTimeDB::TimeInfo(5, 1)}, {id2, DocumentTimeDB::TimeInfo(5, 2)}, {id3, DocumentTimeDB::TimeInfo(6, 3)} };
    state.mtimeDb = {{5, id1}, {5, id2}, {6, id3} };

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }

    Document doc1_clone = doc1; // save state for later reset

    for (auto pos : {1, 3, 6}) {
        doc1.addPositionTerm("a", pos);
    }
    for (auto pos : {2, 4, 5}) {
        doc1.addPositionTerm("abc", pos);
    }
    for (auto pos : {12, 14, 15}) {
        doc1.addPositionTerm("dab", pos);
    }
    for (auto pos : {11, 12}) {
        doc3.addPositionTerm("dab", pos);
    }
    state.positionDb["a"] = {PositionInfo(id1, {1, 3, 6})};
    state.positionDb["abc"] = {PositionInfo(id1, {2, 4, 5})};
    if (id1 < id3) {
        state.positionDb["dab"] = {PositionInfo(id1, {12, 14, 15}), PositionInfo(id3, {11, 12})};
    } else {
        state.positionDb["dab"] = {PositionInfo(id3, {11, 12}), PositionInfo(id1, {12, 14, 15})};
    }

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.replaceDocument(doc1, DocumentOperation::Everything);
        tr.replaceDocument(doc3, DocumentOperation::Everything);
        tr.commit();
    }
    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }

    for (auto pos : {11, 12}) { // extend
        doc1.addPositionTerm("abc", pos);
    }
    for (auto pos : {16, 17}) { // extend, make sure positions for doc3 are kept
        doc1.addPositionTerm("dab", pos);
    }
    for (auto pos : {7, 8, 9}) { // add positions
        doc2.addPositionTerm("a", pos);
    }
    for (auto pos : {7, 8, 9}) { // add new term with positions
        doc2.addPositionTerm("abc", pos);
    }

    Document doc2_clone = doc2; // save state for later reset
    doc2.addPositionTerm("abcd", 500); // add position for existing term

    state.postingDb = {{"a", {id1, id2}}, {"abc", {id1, id2}}, {"abcd", {id2}}, {"dab", {id1, id3}}, {"file1", {id1}}, {"file2", {id2}}, {"file3", {id3}} };
    state.docTermsDb = {{id1, {"a", "abc", "dab"}}, {id2, {"a", "abc", "abcd"}}, {id3, {"dab"}} };
    if (id1 < id2) {
        state.positionDb["a"] = {PositionInfo(id1, {1, 3, 6}), PositionInfo(id2, {7, 8, 9})};
        state.positionDb["abc"] = {PositionInfo(id1, {2, 4, 5, 11, 12}), PositionInfo(id2, {7, 8, 9})};
    } else {
        state.positionDb["a"] = {PositionInfo(id2, {7, 8, 9}), PositionInfo(id1, {1, 3, 6})};
        state.positionDb["abc"] = {PositionInfo(id2, {7, 8, 9}), PositionInfo(id1, {2, 4, 5, 11, 12})};
    }
    if (id1 < id3) {
        state.positionDb["dab"] = {PositionInfo(id1, {12, 14, 15, 16, 17}), PositionInfo(id3, {11, 12})};
    } else {
        state.positionDb["dab"] = {PositionInfo(id3, {11, 12}), PositionInfo(id1, {12, 14, 15, 16, 17})};
    }
    state.positionDb["abcd"] = {PositionInfo(id2, {500})};

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.replaceDocument(doc1, DocumentOperation::Everything);
        tr.replaceDocument(doc2, DocumentOperation::Everything);
        tr.commit();
    }
    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }

    // Reset some positions of doc1
    for (auto pos : {2, 4, 5, 11, 12}) { // keep "abc"
        doc1_clone.addPositionTerm("abc", pos);
    }
    for (auto pos : {12, 14, 17}) { // remove 15, 16 from dab
        doc1_clone.addPositionTerm("dab", pos);
    }
    state.positionDb["a"] = {PositionInfo(id2, {7, 8, 9})}; // doc1 removed
    state.positionDb.remove("abcd"); // positions for abcd removed
    if (id1 < id3) {
        state.positionDb["dab"] = {PositionInfo(id1, {12, 14, 17}), PositionInfo(id3, {11, 12})};
    } else {
        state.positionDb["dab"] = {PositionInfo(id3, {11, 12}), PositionInfo(id1, {12, 14, 17})};
    }

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.replaceDocument(doc1_clone, DocumentOperation::Everything);
        tr.replaceDocument(doc2_clone, DocumentOperation::Everything);
        tr.commit();
    }
    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }
}

void WriteTransactionTest::testIdempotentDocumentChange()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 5, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);
    Document doc2 = createDocument(url1, 5, 1, {"a", "abcd", "dab"}, {"file1"}, {}, m_dirId);
    quint64 id = doc1.id();
    doc2.setId(doc1.id());

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }

    DBState state;
    state.postingDb = {{"a", {id}}, {"abc", {id}}, {"dab", {id}}, {"file1", {id}} };
    state.positionDb = {};
    state.docTermsDb = {{id, {"a", "abc", "dab"} }};
    state.docFileNameTermsDb = {{id, {"file1"} }};
    state.docXAttrTermsDb = {};
    state.docTimeDb = {{id, DocumentTimeDB::TimeInfo(5, 1)}};
    state.mtimeDb = {{5, id}};

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.replaceDocument(doc2, DocumentOperation::Everything);
        tr.replaceDocument(doc2, DocumentOperation::Everything);
        tr.commit();
    }

    state.postingDb = {{"a", {id}}, {"abcd", {id}}, {"dab", {id}}, {"file1", {id}} };
    state.docTermsDb = {{id, {"a", "abcd", "dab"} }};

    {
        Transaction tr(db.get(), Transaction::ReadOnly);
        DBState actualState = DBState::fromTransaction(&tr);
        QVERIFY(DBState::debugCompare(actualState, state));
    }
}

void WriteTransactionTest::testTransactionReset()
{
    const QString url1(dir->path() + QStringLiteral("/file1"));

    Document doc1 = createDocument(url1, 0, 1, {"a", "abc", "dab"}, {"file1"}, {}, m_dirId);

    {
        Transaction tr(db.get(), Transaction::ReadWrite);
        tr.addDocument(doc1);
        tr.commit();
    }

    Transaction tr(db.get(), Transaction::ReadWrite);
    for (int i = 0; i < 3; i++) {
        doc1.setMTime(i);
        tr.replaceDocument(doc1, DocumentOperation::DocumentTime);
        if (i % 2) {
            tr.commit();
            tr.reset(Transaction::ReadWrite);
        }
    }
    tr.commit();
}

QTEST_MAIN(WriteTransactionTest)

#include "writetransactiontest.moc"
