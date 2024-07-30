/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "positioncodec.h"
#include "positioninfo.h"

#include <QTest>

using namespace Baloo;

class PositionCodecBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    // data 1 - small number of documents, each with many positions
    void benchEncodeData1();
    void benchDecodeData1();
    // data 2 - large number of documents, few positions (10) each
    void benchEncodeData2();
    void benchDecodeData2();
    // data 3 - small number of documents, many positions with large increment
    void benchEncodeData3();
    void benchDecodeData3();
private:
    QVector<PositionInfo> m_benchmarkData1;
    QVector<PositionInfo> m_benchmarkData2;
    QVector<PositionInfo> m_benchmarkData3;
};

void PositionCodecBenchmark::initTestCase()
{
    /*
     * Same dataset as in autotests/unit/codecs/positioncodectest.cpp
     * Correctness of encoding/decoding is checked there.
     */
    m_benchmarkData1.clear();
    m_benchmarkData1.reserve(100);
    for(int i = 0; i < 100; ++i)
    {
        PositionInfo info;
        info.docId = (i + 1) * 4711;
        info.positions.reserve(3000);
        for (int j = 0; j < 3000; j++) {
            info.positions.append(((j + 1) * (i + 2)));
        }
        m_benchmarkData1.append(info);
    }

    m_benchmarkData2.clear();
    m_benchmarkData2.reserve(5000);
    for (int i = 0; i < 5000; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        m_benchmarkData2.append(info);
    }

    m_benchmarkData3.clear();
    m_benchmarkData3.reserve(200);
    for (int i = 0; i < 200; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions.reserve(30000); // > 2^14 -> 3 byte VarInt32
        for (int j = 0; j < 30000; j++) {
            info.positions.append((j + 1) * 200); // increment 200 -> 2 byte DiffVarInt32
        }

        m_benchmarkData3.append(info);
    }
}

void PositionCodecBenchmark::benchEncodeData1()
{
    QBENCHMARK { PositionCodec::encode(m_benchmarkData1); }
}

void PositionCodecBenchmark::benchDecodeData1()
{
    const QByteArray ba = PositionCodec::encode(m_benchmarkData1);
    QBENCHMARK { PositionCodec::decode(ba); }
}

void PositionCodecBenchmark::benchEncodeData2()
{
    QBENCHMARK { PositionCodec::encode(m_benchmarkData2); }
}

void PositionCodecBenchmark::benchDecodeData2()
{
    const QByteArray ba = PositionCodec::encode(m_benchmarkData2);
    QBENCHMARK { PositionCodec::decode(ba); }
}

void PositionCodecBenchmark::benchEncodeData3()
{
    QBENCHMARK { PositionCodec::encode(m_benchmarkData3); }
}

void PositionCodecBenchmark::benchDecodeData3()
{
    const QByteArray ba = PositionCodec::encode(m_benchmarkData3);
    QBENCHMARK { PositionCodec::decode(ba); }
}

QTEST_MAIN(PositionCodecBenchmark)

#include "positioncodecbenchmark.moc"
