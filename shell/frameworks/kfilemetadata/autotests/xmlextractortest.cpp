/*
    SPDX-FileCopyrightText: 2018 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>
#include <QTemporaryFile>
#include <QMimeDatabase>

#include "simpleextractionresult.h"
#include "indexerextractortestsconfig.h"
#include "extractors/xmlextractor.h"
#include "mimeutils.h"

using namespace KFileMetaData;

class XmlExtractorTests : public QObject
{
    Q_OBJECT
public:
    explicit XmlExtractorTests(QObject* parent = nullptr);

private:
    QString testFilePath(const QString& fileName) const;

private Q_SLOTS:
    void testNoExtraction();
    void benchMarkXmlExtractor();
    void testXmlExtractor();
    void testXmlExtractorSvgNoContent();
    void testXmlExtractorSvgNoContentDcterms();
    void testXmlExtractorSvgContainer();
    void testXmlExtractorSvgCompressed();
    void testXmlExtractorMathML();
};

XmlExtractorTests::XmlExtractorTests(QObject* parent) :
    QObject(parent)
{
}

QString XmlExtractorTests::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_FILES_PATH) + QLatin1Char('/') + fileName;
}

void XmlExtractorTests::testNoExtraction()
{
    XmlExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_with_metadata.svg"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType,
            ExtractionResult::ExtractNothing);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);
    QCOMPARE(result.properties().size(), 0);
}

void XmlExtractorTests::benchMarkXmlExtractor()
{
    XmlExtractor plugin(this);

    // generate a test file with varying number of words per line
    QTemporaryFile file(QStringLiteral("XXXXXX.xml"));
    QVERIFY(file.open());

    int count = 0;
    file.write("<xml>\n");
    QByteArray chunk("<text>foo bar </text>");
    for (int line = 0; line < 10000; ++line) {
        // staircase pattern, 0, 1, 2, ... 98, 0, 0, 1 ... chunks per line
        for (int i = 0; i < line % 100; ++i) {
            count++;
            file.write(chunk);
        }
        file.write("\n");
    }
    file.write("</xml>\n");
    file.close();

    SimpleExtractionResult result(file.fileName(), QStringLiteral("application/xml"));

    plugin.extract(&result);

    QString content = QStringLiteral("foo bar\n");
    content.replace(QLatin1Char('\n'), QLatin1Char(' '));
    QCOMPARE(result.text().left(8), content.left(8));
    QCOMPARE(result.text().size(), 8 * count);

    QBENCHMARK {
        plugin.extract(&result);
    }
}

void XmlExtractorTests::testXmlExtractor()
{
    XmlExtractor plugin{this};

    QString fileName = testFilePath(QStringLiteral("test_with_metadata.svg"));
    QMimeDatabase mimeDb;
    QString mimeType = MimeUtils::strictMimeType(fileName, mimeDb).name();
    QVERIFY(plugin.mimetypes().contains(mimeType));

    SimpleExtractionResult result(fileName, mimeType,
            ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText);
    plugin.extract(&result);

    QString content = QStringLiteral("Some text\n");

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);

    QCOMPARE(result.properties().size(), 1);
    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("Document Title"));

    content.replace(QLatin1Char('\n'), QLatin1Char(' '));
    QCOMPARE(result.text(), content);
}

void XmlExtractorTests::testXmlExtractorSvgNoContent()
{
    XmlExtractor plugin{this};

    SimpleExtractionResult result(testFilePath(QStringLiteral("test_with_metadata.svg")),
            QStringLiteral("image/svg"),
            ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);

    QCOMPARE(result.properties().size(), 1);
    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("Document Title"));

    QVERIFY(result.text().isEmpty());
}

void XmlExtractorTests::testXmlExtractorSvgNoContentDcterms()
{
    XmlExtractor plugin{this};

    SimpleExtractionResult result(testFilePath(QStringLiteral("test_dcterms.svg")),
            QStringLiteral("image/svg"),
            ExtractionResult::ExtractMetaData);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);

    const auto properties = result.properties();
    QCOMPARE(properties.size(), 6);
    QCOMPARE(properties.value(Property::Description).toString(),
             QStringLiteral("A test document for Dublin Core Terms namespace"));
    QCOMPARE(properties.value(Property::Title).toString(), QStringLiteral("Document Title"));
    QCOMPARE(properties.value(Property::Author).toString(), QStringLiteral("Stefan Brüns"));
    QCOMPARE(properties.value(Property::Language).toString(), QStringLiteral("en"));

    const auto subjects = QVariantList(properties.lowerBound(Property::Subject),
                                       properties.upperBound(Property::Subject));
    QCOMPARE(subjects.size(), 2);
    QCOMPARE(subjects, QVariantList({QStringLiteral("Testing"), QStringLiteral("Dublin Core")}));

    QVERIFY(result.text().isEmpty());
}

void XmlExtractorTests::testXmlExtractorSvgContainer()
{
    XmlExtractor plugin{this};

    SimpleExtractionResult result(testFilePath(QStringLiteral("test_with_container.svg")),
            QStringLiteral("image/svg"),
            ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText);
    plugin.extract(&result);

    QString content = QStringLiteral("Some text below <a>\n");

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);

    QCOMPARE(result.properties().size(), 0);

    content.replace(QLatin1Char('\n'), QLatin1Char(' '));
    QCOMPARE(result.text(), content);
}

void XmlExtractorTests::testXmlExtractorSvgCompressed()
{
    XmlExtractor plugin{this};

    SimpleExtractionResult result(testFilePath(QStringLiteral("test_with_metadata.svgz")),
            QStringLiteral("image/svg+xml-compressed"),
            ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText);
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Image);

#ifdef SVG_XML_COMPRESSED_SUPPORT
    QCOMPARE(result.properties().size(), 1);
    QCOMPARE(result.properties().value(Property::Title).toString(), QStringLiteral("Document Title"));
    QCOMPARE(result.text(), "Some text ");
#else
    QVERIFY(result.properties().isEmpty());
    QVERIFY(result.text().isEmpty());
#endif
}

void XmlExtractorTests::testXmlExtractorMathML()
{
    XmlExtractor plugin{this};

    SimpleExtractionResult result(testFilePath(QStringLiteral("test.mml")),
            QStringLiteral("application/mathml+xml"),
            ExtractionResult::ExtractMetaData | ExtractionResult::ExtractPlainText);
    plugin.extract(&result);

    QString content = QStringLiteral("1 + 1 = 2\n");

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().at(0), Type::Text);

    QCOMPARE(result.properties().size(), 0);

    content.replace(QLatin1Char('\n'), QLatin1Char(' '));
    QCOMPARE(result.text(), content);
}


QTEST_GUILESS_MAIN(XmlExtractorTests)

#include "xmlextractortest.moc"
