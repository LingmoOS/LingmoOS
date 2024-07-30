/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "externalwritertest.h"
#include "writedata.h"
#include "indexerextractortestsconfig.h"
#include "externalwriter.h"
#include "config-kfilemetadata.h"

#include <QTest>
#include <QTemporaryFile>

using namespace KFileMetaData;

QString ExternalWriterTest::testFilePath(const QString& fileName) const
{
    return QLatin1String(INDEXER_TESTS_SAMPLE_CONFIGURED_FILES_PATH) + QLatin1Char('/') + fileName;
}

void ExternalWriterTest::test()
{
    QTemporaryFile file;
    ExternalWriter plugin{testFilePath("testexternalwriter")};
    QVERIFY(file.open());
    WriteData data(file.fileName(), "application/text");
    plugin.write(data);

    QCOMPARE(QString(file.readAll()), QStringLiteral("{}"));
}

void ExternalWriterTest::testProperties()
{
    QTemporaryFile file;
    ExternalWriter plugin{testFilePath("testexternalwriter")};
    QVERIFY(file.open());
    WriteData data(file.fileName(), "application/text");
    data.add(Property::Author, QStringLiteral("Name"));
    plugin.write(data);

    auto expected = QStringLiteral("{\"author\": \"Name\"}");
    QCOMPARE(QString(file.readAll()), expected);
}

QTEST_GUILESS_MAIN(ExternalWriterTest)

#include "moc_externalwritertest.cpp"
