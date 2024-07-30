/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2016 Christian Ehrlicher <ch.ehrlicher@gmx.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCryptographicHash>
#include <QTest>
#include "positioncodec.h"
#include "positioninfo.h"

using namespace Baloo;

class PositionCodecTest : public QObject
{
Q_OBJECT
public:
    PositionCodecTest() = default;
    ~PositionCodecTest() = default;
private Q_SLOTS:
    void initTestCase();
    void checkEncodeOutput();
    void checkEncodeOutput2();
    void checkEncodeOutput3();
private:
    QVector<PositionInfo> m_data;
    QVector<PositionInfo> m_data2;
    QVector<PositionInfo> m_data3;
};

QTEST_MAIN ( PositionCodecTest )

void PositionCodecTest::initTestCase()
{
    m_data.clear();
    m_data.reserve(100);
    for(int i = 0; i < 100; ++i)
    {
        PositionInfo info;
        info.docId = (i + 1) * 4711;
        info.positions.reserve(3000);
        for (int j = 0; j < 3000; j++) {
            info.positions.append((j + 1) * (i * 2));
        }
        m_data.append(info);
    }

    m_data2.clear();
    m_data2.reserve(5000);
    for (int i = 0; i < 5000; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

        m_data2.append(info);
    }

    m_data3.clear();
    m_data3.reserve(200);
    for (int i = 0; i < 200; i++) {
        PositionInfo info;
        info.docId = i;
        info.positions.reserve(30000); // > 2^14 -> 3 byte VarInt32
        for (int j = 0; j < 30000; j++) {
            info.positions.append((j + 1) * 200); // increment 200 -> 2 byte DiffVarInt32
        }

        m_data3.append(info);
    }
}

void PositionCodecTest::checkEncodeOutput()
{
    const QByteArray ba = PositionCodec::encode(m_data);
    QCOMPARE(ba.size(), 409000);
    const QByteArray md5 = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
    QCOMPARE(md5, QByteArray("ae49eb3279bdda36ef91d29ce3c94c2c"));
    // and now decode the whole stuff
    QVector<PositionInfo> decodedData = PositionCodec::decode(ba);
    QCOMPARE(m_data, decodedData);
}

void PositionCodecTest::checkEncodeOutput2()
{
    const QByteArray ba = PositionCodec::encode(m_data2);
    QCOMPARE(ba.size(), (8 + 1 + 10) * 5000); // DocId, VarInt32 len, DiffVarInt position
    const QByteArray md5 = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
    QCOMPARE(md5, QByteArray("2f3710246331002e2332dce560ccd783"));
    // and now decode the whole stuff
    QVector<PositionInfo> decodedData = PositionCodec::decode(ba);
    QCOMPARE(m_data2, decodedData);
}

void PositionCodecTest::checkEncodeOutput3()
{
    const QByteArray ba = PositionCodec::encode(m_data3);
    QCOMPARE(ba.size(), (8 + 3 + (2 * 30000)) * 200); // DocId, VarInt32 len, DiffVarInt position
    const QByteArray md5 = QCryptographicHash::hash(ba, QCryptographicHash::Md5).toHex();
    QCOMPARE(md5, QByteArray("79e942003c082073b4cee8e376fffdaa"));
    // and now decode the whole stuff
    QVector<PositionInfo> decodedData = PositionCodec::decode(ba);
    QCOMPARE(m_data3, decodedData);
}

#include "positioncodectest.moc"
