/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "externalextractortest.h"
#include "simpleextractionresult.h"
#include "indexerextractortestsconfig.h"
#include "externalextractor.h"
#include "config-kfilemetadata.h"

#include <QTest>
#include <QTemporaryFile>

using namespace KFileMetaData;

QString ExternalExtractorTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_CONFIGURED_FILES_PATH) + QLatin1Char('/') + fileName;
}

void ExternalExtractorTest::test()
{
    QTemporaryFile file;
    file.open();
    file.write("Hello\nWorld");
    file.close();
    ExternalExtractor plugin{testFilePath("testexternalextractor")};
    QVERIFY(plugin.mimetypes().contains("application/text"));

    SimpleExtractionResult result(file.fileName(), "application/text");
    plugin.extract(&result);

    QCOMPARE(result.types().size(), 1);
    QCOMPARE(result.types().constFirst(), Type::Text);

    QCOMPARE(result.text(), QStringLiteral("Hello\nWorld "));

    QCOMPARE(result.properties().value(Property::LineCount), 2);
}

QTEST_GUILESS_MAIN(ExternalExtractorTest)

#include "moc_externalextractortest.cpp"
