/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "database.h"
#include "transaction.h"
#include "document.h"
#include "termgenerator.h"
#include "enginequery.h"
#include "idutils.h"
#include "query.h"

#include <memory>
#include <QTest>
#include <QTemporaryDir>

using namespace Baloo;

class SortedIdVector : public QVector<quint64> {
    public:
        SortedIdVector(const QVector<quint64>& list)
        : QVector<quint64>(list) {
            std::sort(begin(), end());
        }
        SortedIdVector(std::initializer_list<quint64> args)
        : SortedIdVector(QVector<quint64>(args)) {}
};

char *toString(const QVector<quint64> &idlist)
{
    QByteArray text("IDs[");
    text += QByteArray::number(idlist.size()) + "]:";
    for (auto id : idlist) {
        text += " " + QByteArray::number(id, 16);
    }
    return qstrdup(text.data());
}

namespace {
QVector<quint64> execQuery(const Transaction& tr, const EngineQuery& query)
{
    PostingIterator* it = tr.postingIterator(query);
    if (!it) {
        return {};
    }

    QVector<quint64> results;
    while (it->next()) {
        results << it->docId();
    }
    return results;
}
} // namespace

class QueryTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
        dir = std::make_unique<QTemporaryDir>();
    }

    void init() {
        dbDir = std::make_unique<QTemporaryDir>();
        db = std::make_unique<Database>(dbDir->path());
        db->open(Database::CreateDatabase);
        setenv("BALOO_DB_PATH", dbDir->path().toStdString().c_str(), 1);

        m_parentId = filePathToId(QFile::encodeName(dir->path()));
        m_id1 = m_parentId + 1;
        m_id2 = m_parentId + 2;
        m_id3 = m_parentId + 3;
        m_id4 = m_parentId + 4;
        m_id5 = m_parentId + 5;
        m_id6 = m_parentId + 6;
        m_id7 = m_parentId + 7;
        m_id8 = m_parentId + 8;
        m_id9 = m_parentId + 9;

        insertDocuments();
        insertTagDocuments();
    }

    void cleanup() {
        db.reset();
        dbDir.reset();
    }

    void testTermEqual();
    void testTermStartsWith();
    void testTermPhrase_data();
    void testTermPhrase();

    void testTagTerm_data();
    void testTagTerm();
    void testTagTermPhrase_data();
    void testTagTermPhrase();

    void testSearchstringParser();
    void testSearchstringParser_data();

private:
    std::unique_ptr<QTemporaryDir> dir;
    std::unique_ptr<QTemporaryDir> dbDir;
    std::unique_ptr<Database> db;
    quint64 m_parentId = 0;

    void insertDocuments();
    void addDocument(Transaction* tr,const QString& text, quint64 id, const QString& fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        tg.indexText(text);
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setParentId(m_parentId);
        doc.setMTime(1);
        doc.setCTime(2);

        tr->addDocument(doc);
    }

    void renameDocument(Transaction* tr, quint64 id, const QString& newName)
    {
        Document doc;

        TermGenerator tg(doc);
        tg.indexFileNameText(newName);
        doc.setId(id);
        doc.setParentId(m_parentId);
        doc.setUrl(QFile::encodeName(newName));

        tr->replaceDocument(doc, FileNameTerms | DocumentUrl);
    }

    void insertTagDocuments();
    void addTagDocument(Transaction* tr,const QStringList& tags, quint64 id, const QString& fileName)
    {
        Document doc;
        QString url = dir->path() + QLatin1Char('/') + fileName;
        doc.setUrl(QFile::encodeName(url));

        TermGenerator tg(doc);
        tg.indexText(QStringLiteral("text/plain"), QByteArray("M"));
        for (const QString& tag : tags) {
            tg.indexXattrText(tag, QByteArray("TA"));
        }
        tg.indexFileNameText(fileName);
        doc.setId(id);
        doc.setParentId(m_parentId);
        doc.setMTime(3);
        doc.setCTime(4);

        tr->addDocument(doc);
    }

    quint64 m_id1;
    quint64 m_id2;
    quint64 m_id3;
    quint64 m_id4;
    quint64 m_id5;
    quint64 m_id6;
    quint64 m_id7;
    quint64 m_id8;
    quint64 m_id9;
};

void QueryTest::insertDocuments()
{
    Transaction tr(db.get(), Transaction::ReadWrite);
    addDocument(&tr, QStringLiteral("The quick brown fox jumped over the crazy dog"), m_id1, QStringLiteral("file1.txt"));
    addDocument(&tr, QStringLiteral("The quick brown fox jumped over the lazy dog"), m_id7, QStringLiteral("file7_lazy"));
    addDocument(&tr, QStringLiteral("A quick brown fox ran around a easy dog"), m_id8, QStringLiteral("file8_dog"));
    addDocument(&tr, QStringLiteral("The night is dark and full of terror"), m_id2, QStringLiteral("file2"));
    addDocument(&tr, QStringLiteral("Don't feel sorry for yourself. Only assholes do that"), m_id3, QStringLiteral("file3"));
    addDocument(&tr, QStringLiteral("Only the dead stay 17 forever. crazy"), m_id4, QStringLiteral("file4"));
    addDocument(&tr, QStringLiteral("Some content with isolated dot . Test it"), m_id9, QStringLiteral("file - with hyphen.txt"));

    renameDocument(&tr, m_id8, QStringLiteral("file8_easy"));
    tr.commit();
}

void QueryTest::insertTagDocuments()
{
    Transaction tr(db.get(), Transaction::ReadWrite);
    addTagDocument(&tr, {
	QStringLiteral("One"),
	QStringLiteral("Two"),
	QStringLiteral("Three"),
	QStringLiteral("Four"),
	QStringLiteral("F1")
    }, m_id5, QStringLiteral("tagFile1"));
    addTagDocument(&tr, {
	QStringLiteral("One"),
	QStringLiteral("Two-Three"),
	QStringLiteral("Four"),
	QStringLiteral("F2")
    }, m_id6, QStringLiteral("tagFile2"));
    tr.commit();
}

void QueryTest::testTermEqual()
{
    EngineQuery q("the");

    QVector<quint64> result = SortedIdVector{m_id1, m_id2, m_id4, m_id7};
    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), result);
}

void QueryTest::testTermStartsWith()
{
    EngineQuery q("for", EngineQuery::StartsWith);

    QVector<quint64> result = SortedIdVector{m_id3, m_id4};
    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), result);
}

void QueryTest::testTermPhrase_data()
{
    QTest::addColumn<QByteArrayList>("phrase");
    QTest::addColumn<QVector<quint64>>("contentMatches");
    QTest::addColumn<QVector<quint64>>("filenameMatches");

    auto addRow = [](const char* name, const QByteArrayList& phrase,
                     const QVector<quint64> contentMatches,
                     const QVector<quint64> filenameMatches)
        { QTest::addRow("%s", name) << phrase << contentMatches << filenameMatches;};

    // Content matches
    addRow("Crazy dog",        {QByteArrayLiteral("crazy"), QByteArrayLiteral("dog")},  SortedIdVector{ m_id1 }, {});
    addRow("Lazy dog",         {QByteArrayLiteral("lazy"),  QByteArrayLiteral("dog")},  SortedIdVector{ m_id7 }, {});
    addRow("Brown fox",        {QByteArrayLiteral("brown"), QByteArrayLiteral("fox")},  SortedIdVector{ m_id1, m_id7, m_id8 }, {});
    addRow("Dog",              {QByteArrayLiteral("dog")},                              SortedIdVector{ m_id1, m_id7, m_id8 }, {});
    // Filename matches
    addRow("Crazy dog file 1", {QByteArrayLiteral("file1")},                            {}, SortedIdVector{ m_id1 });
    addRow("Crazy dog file 2", {QByteArrayLiteral("file1"), QByteArrayLiteral("txt")},  {}, SortedIdVector{ m_id1 });
    addRow("Lazy dog file 1",  {QByteArrayLiteral("file7")},                            {}, SortedIdVector{ m_id7 });
    addRow("Lazy dog file 2",  {QByteArrayLiteral("file7"), QByteArrayLiteral("lazy")}, {}, SortedIdVector{ m_id7 });
    // Matches content and filename
    addRow("Lazy both",        {QByteArrayLiteral("lazy")},                             { m_id7 }, { m_id7 });
    addRow("Easy both",        {QByteArrayLiteral("easy")},                             { m_id8 }, { m_id8 });
}

void QueryTest::testTermPhrase()
{
    QFETCH(QByteArrayList, phrase);
    QFETCH(QVector<quint64>, contentMatches);
    QFETCH(QVector<quint64>, filenameMatches);

    QVector<EngineQuery> queries;
    for (const QByteArray& term : phrase) {
        queries << EngineQuery(term);
    }
    EngineQuery q(queries);

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), contentMatches);

    queries.clear();
    const QByteArray fPrefix = QByteArrayLiteral("F");
    for (QByteArray term : phrase) {
        term = fPrefix + term;
        queries << EngineQuery(term);
    }
    EngineQuery qf(queries);
    QCOMPARE(execQuery(tr, qf), filenameMatches);
}

void QueryTest::testTagTerm_data()
{
    QTest::addColumn<QByteArray>("term");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArray("one")
        << QVector<quint64> { m_id5, m_id6 };
    QTest::addRow("Only one") << QByteArray("f1")
        << QVector<quint64> { m_id5 };
    QTest::addRow("Also from phrase") << QByteArray("three")
        << QVector<quint64> { m_id5, m_id6 };
}

void QueryTest::testTagTerm()
{
    QFETCH(QByteArray, term);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    EngineQuery q(prefix + term);

    Transaction tr(db.get(), Transaction::ReadOnly);
    QCOMPARE(execQuery(tr, q), matchIds);
}

void QueryTest::testTagTermPhrase_data()
{
    QTest::addColumn<QByteArrayList>("terms");
    QTest::addColumn<QVector<quint64>>("matchIds");

    QTest::addRow("Simple match") << QByteArrayList({"one"})
        << QVector<quint64> { m_id5, m_id6 };
    QTest::addRow("Apart") << QByteArrayList({"two", "four"})
        << QVector<quint64> { };
    QTest::addRow("Adjacent") << QByteArrayList({"three", "four"})
        << QVector<quint64> { };
    QTest::addRow("Only phrase") << QByteArrayList({"two", "three"})
        << QVector<quint64> { m_id6 };
}

void QueryTest::testTagTermPhrase()
{
    QFETCH(QByteArrayList, terms);
    QFETCH(QVector<quint64>, matchIds);

    QByteArray prefix{"TA"};
    QVector<EngineQuery> queries;
    for (const QByteArray& term : terms) {
        queries << EngineQuery(prefix + term);
    }

    EngineQuery q(queries);

    Transaction tr(db.get(), Transaction::ReadOnly);
    auto res = execQuery(tr, q);
    QCOMPARE(res, matchIds);
}

void QueryTest::testSearchstringParser()
{
    QFETCH(QString, searchString);
    QFETCH(QStringList, expectedFiles);

    Query q;
    q.setSearchString(searchString);

    auto res = q.exec();
    QStringList matches;
    while (res.next()) {
        auto path = res.filePath();
        auto name = path.section(QLatin1Char('/'), -1, -1);
        matches.append(name);
    }
    QEXPECT_FAIL("Match 'dot . Test'", "Bug 407664: Tries to match isolated dot", Continue);
    QEXPECT_FAIL("Match 'file - with hyphen.txt'", "Bug 407664: Tries to match hyphen", Continue);
    QCOMPARE(matches, expectedFiles);
}

void QueryTest::testSearchstringParser_data()
{
    QTest::addColumn<QString>("searchString");
    QTest::addColumn<QStringList>("expectedFiles");

    auto addRow = [](const QString& searchString,
                     const QStringList& filenameMatches)
    {
        QTest::addRow("Match '%s'", qPrintable(searchString)) << searchString << filenameMatches;
    };

    addRow(QStringLiteral("crazy"), { QStringLiteral("file1.txt"), QStringLiteral("file4") });
    addRow(QStringLiteral("content:crazy"), { QStringLiteral("file1.txt"), QStringLiteral("file4") });
    addRow(QStringLiteral("content:dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("filename:dog"), {});
    addRow(QStringLiteral("filename:easy"), { QStringLiteral("file8_easy") });
    addRow(QStringLiteral("content:for"), { QStringLiteral("file3"), QStringLiteral("file4") });
    addRow(QStringLiteral("content=for"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=don't"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=yourself"), { QStringLiteral("file3") });
    addRow(QStringLiteral("content=\"over the\""), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy") });
    addRow(QStringLiteral("content=\"over the crazy dog\""), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("content=\"over the dog\""), {});
    addRow(QStringLiteral("quick AND crazy AND dog"), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("quick crazy dog"), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("\"quick brown\" dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("\"quick brown\" the dog"), { QStringLiteral("file1.txt"), QStringLiteral("file7_lazy") });
    addRow(QStringLiteral("\"quick brown\" content=\"the dog\""), {});
    addRow(QStringLiteral("\"quick brown\" content=\"'the dog'\""), {});
    addRow(QStringLiteral("\"quick brown\" content:\"the dog\""), {});
    addRow(QStringLiteral("\"quick brown\" content:\"'the dog'\""), {});
    addRow(QStringLiteral("\"quick brown\" \"the crazy dog\""), { QStringLiteral("file1.txt") });
    addRow(QStringLiteral("content=for OR filename:eas"), { QStringLiteral("file3"), QStringLiteral("file8_easy") });
    addRow(QStringLiteral("for sorry"), { QStringLiteral("file3") });
    addRow(QStringLiteral("over OR terror"), {  QStringLiteral("file1.txt"), QStringLiteral("file2"), QStringLiteral("file7_lazy") });

    addRow(QStringLiteral("tag:f1"), {  QStringLiteral("tagFile1") });
    addRow(QStringLiteral("tag:f2"), {  QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:one"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two AND tag:three"), {  QStringLiteral("tagFile1"), QStringLiteral("tagFile2") });
    addRow(QStringLiteral("tag:two-three"), { QStringLiteral("tagFile2") });

    addRow(QStringLiteral("filename:hyphen"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("file with hyphen.txt"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("file - with hyphen.txt"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("\"file - with hyphen.txt\""), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("content:dot"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("dot . Test"), { QStringLiteral("file - with hyphen.txt") });
    addRow(QStringLiteral("\"dot . Test\""), { QStringLiteral("file - with hyphen.txt") });
}

QTEST_MAIN(QueryTest)

#include "querytest.moc"
