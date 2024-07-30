/*
    SPDX-FileCopyrightText: 2010 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../src/kcodecsbase64.h"

#include <KCodecs>

#include <QByteArray>
#include <QObject>
#include <QTest>

class Base64Benchmark : public QObject
{
    Q_OBJECT
private:
    static QByteArray fillByteArray(int size)
    {
        char c = 0;
        QByteArray result;
        result.reserve(size);
        while (result.size() < size) {
            result.append(c++);
        }
        return result;
    }

    void createTestSet()
    {
        QTest::addColumn<QByteArray>("output");
        QTest::addColumn<QByteArray>("input");
        QTest::newRow("empty") << QByteArray() << QByteArray();
        QTest::newRow("128") << fillByteArray(128) << KCodecs::base64Encode(fillByteArray(128));
        QTest::newRow("1k") << fillByteArray(1 << 10) << KCodecs::base64Encode(fillByteArray(1 << 10));
        QTest::newRow("1M") << fillByteArray(1 << 20) << KCodecs::base64Encode(fillByteArray(1 << 20));
    }
private Q_SLOTS:
    void benchmarkKCodecDecode_data()
    {
        createTestSet();
    }

    void benchmarkKCodecDecode()
    {
        QFETCH(QByteArray, input);
        QFETCH(QByteArray, output);
        QByteArray result;
        QBENCHMARK {
            result = KCodecs::base64Decode(input);
        }
        QCOMPARE(result, output);
    }

    void benchmarkQByteArrayDecode_data()
    {
        createTestSet();
    }

    void benchmarkQByteArrayDecode()
    {
        QFETCH(QByteArray, input);
        QFETCH(QByteArray, output);
        QByteArray result;
        QBENCHMARK {
            result = QByteArray::fromBase64(input);
        }
        QCOMPARE(result, output);
    }

    void benchmarkKMimeBase64Decoder_data()
    {
        createTestSet();
    }

    void benchmarkKMimeBase64Decoder()
    {
        QFETCH(QByteArray, input);
        QFETCH(QByteArray, output);
        QByteArray result;
        QBENCHMARK {
            KCodecs::Codec *codec = KCodecs::Codec::codecForName("base64");
            QVERIFY(codec);
            result.resize(codec->maxDecodedSizeFor(input.size()));
            KCodecs::Decoder *decoder = codec->makeDecoder();
            QByteArray::const_iterator inputIt = input.constBegin();
            QByteArray::iterator resultIt = result.begin();
            decoder->decode(inputIt, input.constEnd(), resultIt, result.constEnd());
            result.truncate(resultIt - result.begin());
            delete decoder;
        }
        QCOMPARE(result, output);
    }
};

QTEST_MAIN(Base64Benchmark)

#include "base64benchmark.moc"
