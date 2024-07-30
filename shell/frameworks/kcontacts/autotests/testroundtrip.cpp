/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2012 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressee.h"

#include "converter/vcardconverter.h"

#include <QDebug>
#include <QDir>
#include <QObject>
#include <QTest>

using namespace KContacts;

class RoundtripTest : public QObject
{
    Q_OBJECT

private:
    QDir mInputDir;
    QDir mOutput2_1Dir;
    QDir mOutput3_0Dir;
    QDir mOutput4_0Dir;

    QStringList mInputFiles;

private Q_SLOTS:
    void initTestCase();
    void testVCardRoundtrip_data();
    void testVCardRoundtrip();
};

// check the validity of our test data set
void RoundtripTest::initTestCase()
{
    // check that all resource prefixes exist

    mInputDir = QDir(QStringLiteral(":/input"));
    QVERIFY(mInputDir.exists());
    QVERIFY(mInputDir.cd(QStringLiteral("data")));

    mOutput2_1Dir = QDir(QStringLiteral(":/output2.1"));
    QVERIFY(mOutput2_1Dir.exists());
    QVERIFY(mOutput2_1Dir.cd(QStringLiteral("data")));

    mOutput3_0Dir = QDir(QStringLiteral(":/output3.0"));
    QVERIFY(mOutput3_0Dir.exists());
    QVERIFY(mOutput3_0Dir.cd(QStringLiteral("data")));

    mOutput4_0Dir = QDir(QStringLiteral(":/output4.0"));
    QVERIFY(mOutput4_0Dir.exists());
    QVERIFY(mOutput4_0Dir.cd(QStringLiteral("data")));

    // check that there are input files

    mInputFiles = mInputDir.entryList();

    QVERIFY(!mInputFiles.isEmpty());
}

void RoundtripTest::testVCardRoundtrip_data()
{
    QTest::addColumn<QString>("inputFile");
    QTest::addColumn<QString>("output2_1File");
    QTest::addColumn<QString>("output3_0File");
    QTest::addColumn<QString>("output4_0File");

    QString outFile21Pattern = QLatin1String("%1.2_1ref");
    QString outFile4Pattern = QLatin1String("v4_0.%1.ref");
    QString outFilePattern = QLatin1String("%1.ref");
    for (const QString &inputFile : std::as_const(mInputFiles)) {
        const QString outFile = outFilePattern.arg(inputFile);
        const QString outFileV2_1 = outFile21Pattern.arg(inputFile);
        const QString outFileV4 = outFile4Pattern.arg(inputFile);
        QTest::newRow(QFile::encodeName(inputFile).constData())
            << inputFile << (mOutput2_1Dir.exists(outFileV2_1) ? outFileV2_1 : QString()) << (mOutput3_0Dir.exists(outFile) ? outFile : QString())
            << (mOutput4_0Dir.exists(outFileV4) ? outFileV4 : QString());
    }
}

static void compareBuffers(const char *version, const QByteArray &outputData, const QByteArray &outputRefData)
{
    if (outputData != outputRefData) {
        qDebug() << " outputData " << outputData;
        qDebug() << " outputRefData " << outputRefData;
    }
    const QList<QByteArray> outputLines = outputData.split('\n');
    const QList<QByteArray> outputRefLines = outputRefData.split('\n');
    for (int i = 0; i < qMin(outputLines.count(), outputRefLines.count()); ++i) {
        const QByteArray &actual = outputLines.at(i);
        const QByteArray &expect = outputRefLines.at(i);
        if (actual != expect) {
            qCritical() << "Mismatch in" << version << "output line" << (i + 1);
            QCOMPARE(actual, expect);
            QCOMPARE(actual.size(), expect.size());
        }
    }
    QCOMPARE(outputLines.count(), outputRefLines.count());
    QCOMPARE(outputData.size(), outputRefData.size());
}

void RoundtripTest::testVCardRoundtrip()
{
    QFETCH(QString, inputFile);
    QFETCH(QString, output2_1File);
    QFETCH(QString, output3_0File);
    QFETCH(QString, output4_0File);

    QVERIFY2(!output2_1File.isEmpty() || !output3_0File.isEmpty() || !output4_0File.isEmpty(), "No reference output file for either format version");

    QFile input(QFileInfo(mInputDir, inputFile).absoluteFilePath());
    QVERIFY(input.open(QIODevice::ReadOnly));

    const QByteArray inputData = input.readAll();
    QVERIFY(!inputData.isEmpty());

    VCardConverter converter;
    const Addressee::List list = converter.parseVCards(inputData);
    QVERIFY(!list.isEmpty());

    if (!output2_1File.isEmpty()) {
        const QByteArray outputData = converter.createVCards(list, VCardConverter::v2_1);

        QFile outputFile(QFileInfo(mOutput2_1Dir, output2_1File).absoluteFilePath());
        QVERIFY(outputFile.open(QIODevice::ReadOnly));

        const QByteArray outputRefData = outputFile.readAll();
        compareBuffers("v2.1", outputData, outputRefData);
    }

    if (!output3_0File.isEmpty()) {
        const QByteArray outputData = converter.createVCards(list, VCardConverter::v3_0);

        QFile outputFile(QFileInfo(mOutput3_0Dir, output3_0File).absoluteFilePath());
        QVERIFY(outputFile.open(QIODevice::ReadOnly));

        const QByteArray outputRefData = outputFile.readAll();
        compareBuffers("v3.0", outputData, outputRefData);
    }
#if 0
    if (!output4_0File.isEmpty()) {
        const QByteArray outputData = converter.createVCards(list, VCardConverter::v4_0);

        QFile outputFile(QFileInfo(mOutput4_0Dir, output4_0File).absoluteFilePath());
        QVERIFY(outputFile.open(QIODevice::ReadOnly));

        const QByteArray outputRefData = outputFile.readAll();
        compareBuffers("v4.0", outputData, outputRefData);
    }
#endif
}

QTEST_MAIN(RoundtripTest)

#include "testroundtrip.moc"
