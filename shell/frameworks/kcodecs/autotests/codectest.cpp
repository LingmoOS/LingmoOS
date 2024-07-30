/*
    SPDX-FileCopyrightText: 2010 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "codectest.h"

#include <QTest>

#include <QDir>

#include "../src/kcodecs.h"

using namespace KCodecs;

QTEST_MAIN(CodecTest)

enum Mode {
    Decode,
    Encode,
};
Q_DECLARE_METATYPE(Mode)

void CodecTest::testCodecs_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("expResult");
    QTest::addColumn<QByteArray>("codecName");
    QTest::addColumn<QString>("tag");
    QTest::addColumn<Mode>("mode");

    QDir codecBaseDir(QFINDTESTDATA("data"));
    const QStringList lst = codecBaseDir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot, QDir::NoSort);
    for (const QString &dir : lst) {
        if (dir.toLower().startsWith(QLatin1String("codec_"))) {
            const QString codecName = dir.right(dir.size() - 6);
            QDir codecDir(codecBaseDir.path() + QLatin1String("/") + dir);
            const QStringList lst2 = codecDir.entryList(QStringList(), QDir::Files, QDir::NoSort);
            for (const QString &file : lst2) {
                if (file.toLower().endsWith(QLatin1String(".expected"))) {
                    const QString dataFileNameBase = file.left(file.size() - 9);
                    QFile dataFile(codecDir.path() + QLatin1Char('/') + dataFileNameBase);
                    QFile expectedFile(codecDir.path() + QLatin1Char('/') + file);
                    QVERIFY(dataFile.open(QIODevice::ReadOnly));
                    QVERIFY(expectedFile.open(QIODevice::ReadOnly));

                    Mode mode = Decode;
                    if (file.contains(QLatin1String("-decode"))) {
                        mode = Decode;
                    } else if (file.contains(QLatin1String("-encode"))) {
                        mode = Encode;
                    }

                    const QByteArray data = dataFile.readAll();
                    const QByteArray expected = expectedFile.readAll();

                    const QString tag = codecName + QLatin1Char('/') + dataFileNameBase;
                    QTest::newRow(tag.toLatin1().constData()) << data << expected << codecName.toLatin1() << tag << mode;

                    dataFile.close();
                    expectedFile.close();
                }
            }
        }
    }
}

void CodecTest::testCodecs()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, expResult);
    QFETCH(QByteArray, codecName);
    QFETCH(QString, tag);
    QFETCH(Mode, mode);

    Codec *codec = Codec::codecForName(codecName);
    QVERIFY(codec);

    QStringList blacklistedTags;
    if (blacklistedTags.contains(tag)) {
        QEXPECT_FAIL(tag.toLatin1().constData(), "Codec broken", Continue);
    }

    QByteArray result;
    if (mode == Decode) {
        result = codec->decode(input, Codec::NewlineLF);
    } else {
        result = codec->encode(input, Codec::NewlineLF);
    }

    // More usable version of QCOMPARE(result, expResult), in case the difference is at the end...
    if (result != expResult) {
        const QList<QByteArray> lines = result.split('\n');
        const QList<QByteArray> expLines = expResult.split('\n');
        if (lines.count() == expLines.count()) {
            QCOMPARE(result.split('\n'), expResult.split('\n'));
        }
    }
    QCOMPARE(result, expResult);
}

#include "moc_codectest.cpp"
