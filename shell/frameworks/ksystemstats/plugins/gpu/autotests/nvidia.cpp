/*
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QSignalSpy>
#include <QTest>

#include "../NvidiaSmiProcess.h"

class NvidiaParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDmonParsing_data();
    void testDmonParsing();

private:
};

void NvidiaParserTest::testDmonParsing_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<NvidiaSmiProcess::GpuData>("expected");
    QTest::newRow("520")
        << QFINDTESTDATA("fixtures/520.txt")
        << NvidiaSmiProcess::GpuData{.index = 0, .power = 11, .temperature = 46, .usage = 3, .memoryUsed = 434, .coreFrequency = 139, .memoryFrequency = 405};
    QTest::newRow("525")
        << QFINDTESTDATA("fixtures/525.txt")
        << NvidiaSmiProcess::GpuData{.index = 0, .power = 29, .temperature = 49, .usage = 1, .memoryUsed = 403, .coreFrequency = 974, .memoryFrequency = 4006};
    QTest::newRow("530")
        << QFINDTESTDATA("fixtures/530.txt")
        << NvidiaSmiProcess::GpuData{.index = 0, .power = 28, .temperature = 44, .usage = 4, .memoryUsed = 902, .coreFrequency = 210, .memoryFrequency = 405};
}

void NvidiaParserTest::testDmonParsing()
{
    QFETCH(QString, data);
    QFETCH(NvidiaSmiProcess::GpuData, expected);

    NvidiaSmiProcess smi;
    QSignalSpy dataSpy(&smi, &NvidiaSmiProcess::dataReceived);

    QFile dataFile(data);
    QVERIFY(dataFile.open(QIODevice::ReadOnly));
    int lines = -2; // subtract two headers
    while (!dataFile.atEnd()) {
        ++lines;
        smi.readStatisticsData(dataFile.readLine());
    }

    QCOMPARE(dataSpy.count(), lines);
    for (const auto &emission : dataSpy) {
        const auto gpuData = emission[0].value<NvidiaSmiProcess::GpuData>();
        QCOMPARE(gpuData.index, expected.index);
        QCOMPARE(gpuData.power, expected.power);
        QCOMPARE(gpuData.temperature, expected.temperature);
        QCOMPARE(gpuData.usage, expected.usage);
        QCOMPARE(gpuData.memoryUsed, expected.memoryUsed);
        QCOMPARE(gpuData.coreFrequency, expected.coreFrequency);
        QCOMPARE(gpuData.memoryFrequency, expected.memoryFrequency);
    }
}

QTEST_MAIN(NvidiaParserTest)

#include "nvidia.moc"
