/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "simpleextractionresult.h"
#include "indexerextractortestsconfig.h"
#include "extractors/popplerextractor.h"
#include "mimeutils.h"

#include <QObject>
#include <QTest>
#include <QMimeDatabase>
#include <QTimeZone>

using namespace KFileMetaData;

namespace {
QString testFilePath(const QString& fileName)
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}
} // namespace

class PopplerExtractorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMimeType();
    void testMimeType_data();
    void testExtraction();
    void testExtraction_data();
};

void PopplerExtractorTest::testMimeType()
{
    QFETCH(QString, fileName);

    QMimeDatabase mimeDb;
    QString filePath = testFilePath(fileName);
    QString mimeType = MimeUtils::strictMimeType(filePath, mimeDb).name();
    QCOMPARE(mimeType, QStringLiteral("application/pdf"));

    PopplerExtractor plugin{this};
    QVERIFY(plugin.mimetypes().contains(QStringLiteral("application/pdf")));
}

void PopplerExtractorTest::testMimeType_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::addRow("Simple PDF") << QStringLiteral("test.pdf");
}

void PopplerExtractorTest::testExtraction()
{
    QFETCH(QString, fileName);
    QFETCH(ExtractionResult::Flags, flags);
    QFETCH(QString, content);
    QFETCH(KFileMetaData::PropertyMultiMap, expectedProperties);


    PopplerExtractor plugin{this};

    QString filePath = testFilePath(fileName);
    SimpleExtractionResult result(filePath, QStringLiteral("application/pdf"), flags);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Document);

    for (auto [property, value] : expectedProperties.asKeyValueRange()) {
        QCOMPARE(result.properties().value(property), value);
    }
    QCOMPARE(result.properties().size(), expectedProperties.size());

    QEXPECT_FAIL("Multicol - metadata and text", "Content extracted uses layout", Continue);
    QCOMPARE(result.text(), content);
}

void PopplerExtractorTest::testExtraction_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<ExtractionResult::Flags>("flags");
    QTest::addColumn<QString>("content");
    QTest::addColumn<KFileMetaData::PropertyMultiMap>("expectedProperties");

    using ER = ExtractionResult;

    KFileMetaData::PropertyMultiMap simplePdfProperties{
        {Property::Author, QStringLiteral("Happy Man")},
        {Property::Title, QStringLiteral("The Big Brown Bear")},
        {Property::Subject, QStringLiteral("PDF Metadata")},
        {Property::Generator, QStringLiteral("LibreOffice 4.2")},
        {Property::PageCount, 1},
        {Property::CreationDate, QDateTime::fromString(QStringLiteral("2014-07-01T13:38:50+00"), Qt::ISODate)},
    };
    auto simplePdfContent = QStringLiteral("This is a sample PDF file for KFileMetaData. ");

    QTest::addRow("No extraction")     << QStringLiteral("test.pdf") << ER::Flags{ER::ExtractNothing}  << QString() << KFileMetaData::PropertyMultiMap{};
    QTest::addRow("Metadata only")     << QStringLiteral("test.pdf") << ER::Flags{ER::ExtractMetaData} << QString() << simplePdfProperties;
    QTest::addRow("Metadata and text") << QStringLiteral("test.pdf") << ER::Flags{ER::ExtractMetaData | ER::ExtractPlainText} << simplePdfContent << simplePdfProperties;

    KFileMetaData::PropertyMultiMap multicolPdfProperties{
        {Property::Author, QStringLiteral("Author KFM")},
        {Property::Title, QStringLiteral("KFileMetaData Title")},
        {Property::Subject, QStringLiteral("KFileMetaData Subject")},
        {Property::Generator, QStringLiteral("LibreOffice 7.6")},
        {Property::PageCount, 1},
        {Property::CreationDate, QDateTime::fromString(QStringLiteral("2024-03-18T01:37:42+00"), Qt::ISODate)},
    };
    auto multicolPdfContent = QStringLiteral("1: First headline for KfileMetaData\n");

    QTest::addRow("Multicol - no extraction")     << QStringLiteral("test_multicolumn.pdf") << ER::Flags{ER::ExtractNothing}  << QString() << KFileMetaData::PropertyMultiMap{};
    QTest::addRow("Multicol - metadata only")     << QStringLiteral("test_multicolumn.pdf") << ER::Flags{ER::ExtractMetaData} << QString() << multicolPdfProperties;
    QTest::addRow("Multicol - metadata and text") << QStringLiteral("test_multicolumn.pdf") << ER::Flags{ER::ExtractMetaData | ER::ExtractPlainText} << multicolPdfContent << multicolPdfProperties;
}

QTEST_GUILESS_MAIN(PopplerExtractorTest)

#include "popplerextractortest.moc"
