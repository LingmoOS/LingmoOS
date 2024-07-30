/*
    SPDX-FileCopyrightText: 2014 Pinak Ahuja <pinak.ahuja@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "basicindexingjob.h"

#include <vector>
#include <QByteArray>
#include <QDateTime>
#include <QFile>
#include <QString>
#include <QTest>
#include <QTemporaryDir>
#include <QVector>
#include <KFileMetaData/Types>

namespace Baloo {

class BasicIndexingJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();

    void testBasicIndexing_data();
    void testBasicIndexing();
    void testBasicIndexingTypes_data();
    void testBasicIndexingTypes();

private:
    struct TestFile {
	QString filename;
	QString mimetype;
	QList<KFileMetaData::Type::Type> types;
    };
    std::vector<TestFile> m_testFiles;

    qint64 m_startTime;
    QTemporaryDir m_workDir;
};

}
using namespace Baloo;

void BasicIndexingJobTest::initTestCase()
{
    m_startTime = QDateTime::currentSecsSinceEpoch();

    using Type = KFileMetaData::Type::Type;
    m_testFiles = {
	{QStringLiteral("test.epub"),    QStringLiteral("application/epub+zip"),                            {Type::Document}},
	{QStringLiteral("test.jpg"),     QStringLiteral("image/jpeg"),                                      {Type::Image}},
	{QStringLiteral("test.mp3"),     QStringLiteral("audio/mpeg"),                                      {Type::Audio}},
	{QStringLiteral("test.ogv"),     QStringLiteral("video/x-theora+ogg"),                              {Type::Video}},
	{QStringLiteral("test.odp"),     QStringLiteral("application/vnd.oasis.opendocument.presentation"), {Type::Document, Type::Presentation}},
	{QStringLiteral("test.odt"),     QStringLiteral("application/vnd.oasis.opendocument.text"),         {Type::Document}},
	{QStringLiteral("test.tar.bz2"), QStringLiteral("application/x-bzip-compressed-tar"),               {Type::Archive}},
	{QStringLiteral("empty.jpg"),    QStringLiteral("application/x-zerosize"),                          {}},
    };

    for (const auto& entry : m_testFiles) {
	QFile file(m_workDir.filePath(entry.filename));
	file.open(QIODevice::WriteOnly);
	if (entry.filename.startsWith(QStringLiteral("test"))) {
	    file.write("\0", 1);
	}
	file.close();
    }
}

void BasicIndexingJobTest::testBasicIndexing_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("mimetype");

    for (const auto& entry : m_testFiles) {
	QTest::addRow("%s", qPrintable(entry.mimetype))
	   << entry.filename << entry.mimetype;
    }
}

void BasicIndexingJobTest::testBasicIndexing()
{
    QFETCH(QString, filename);
    QFETCH(QString, mimetype);

    QString filepath(m_workDir.filePath(filename));
    BasicIndexingJob job(filepath, mimetype, BasicIndexingJob::IndexingLevel::NoLevel);

    QVERIFY(job.index());

    auto doc = job.document();

    QVERIFY(doc.id());
    QVERIFY(doc.parentId());

    QCOMPARE(doc.url(), QFile::encodeName(filepath));
    QVERIFY(doc.m_mTime >= m_startTime);
    QVERIFY(doc.m_cTime >= m_startTime);

    auto fileNameTerms = doc.m_fileNameTerms.keys();
    std::sort(fileNameTerms.begin(), fileNameTerms.end());

    QCOMPARE(fileNameTerms.size(), filename.count(QLatin1Char('.')) + 1);
}

void BasicIndexingJobTest::testBasicIndexingTypes_data()
{
    QTest::addColumn<QString>("filename");
    QTest::addColumn<QString>("mimetype");
    QTest::addColumn<QList<QByteArray>>("types");

    for (const auto& entry : m_testFiles) {
	QByteArrayList list;
	for(const auto type : entry.types) {
	    list.append("T" + QByteArray::number(static_cast<int>(type)));
	}
	QTest::addRow("%s", qPrintable(entry.mimetype)) << entry.filename << entry.mimetype << list;
    }
}

void BasicIndexingJobTest::testBasicIndexingTypes()
{
    QFETCH(QString, filename);
    QFETCH(QString, mimetype);
    QFETCH(QList<QByteArray>, types);

    QString filepath(m_workDir.filePath(filename));
    BasicIndexingJob job(filepath, mimetype, BasicIndexingJob::IndexingLevel::NoLevel);

    QVERIFY(job.index());

    auto doc = job.document();

    auto terms = doc.m_terms.keys();
    auto split = std::partition(terms.begin(), terms.end(),
      [](const QByteArray& t) { return t[0] == 'T'; });

    // Types
    QByteArrayList docTypes{terms.begin(), split};
    std::sort(docTypes.begin(), docTypes.end());
    QCOMPARE(types, docTypes);

    // Mimetype terms
    QByteArrayList docMimeTerms{split, terms.end()};
    QVERIFY(docMimeTerms.size() >= 2);
    for (const auto& term : docMimeTerms) {
	QByteArray mimeBA = mimetype.toLatin1();
	// Strip 'M' prefix from term
	auto stripped = term.mid(1);
	QVERIFY2(mimeBA.contains(stripped), stripped.constData());
    }
}

QTEST_GUILESS_MAIN(BasicIndexingJobTest)

#include "basicindexingjobtest.moc"
