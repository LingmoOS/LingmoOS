// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QStandardPaths>
#include <QTest>

#include <KSycoca>
#include <smartdata.h>

class SMARTDataTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPass()
    {
        QFile file(QFINDTESTDATA("fixtures/pass.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/testfoobarpass");
        QCOMPARE(data.m_status.m_passed, true);
        QVERIFY(!data.m_smartctl.failure());
        QVERIFY(data.m_valid);
    }

    void testFail()
    {
        // NB: fixture isn't actually of a failure, so fields need tweaking as necessary
        QFile file(QFINDTESTDATA("fixtures/fail.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/testfoobarfail");
        QCOMPARE(data.m_status.m_passed, false);
        QVERIFY(!data.m_smartctl.failure());
        QVERIFY(data.m_valid);
    }

    void testBroken()
    {
        // Actual broken device provided by ahiemstra@heimr.nl
        QFile file(QFINDTESTDATA("fixtures/broken.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/sdc");
        QCOMPARE(data.m_status.m_passed, false);
        QCOMPARE(data.m_smartctl.failure(),
                 SMART::Failures({SMART::Failure::Disk, SMART::Failure::Prefail, SMART::Failure::ErrorsRecorded, SMART::Failure::SelfTestErrors}));
        QVERIFY(data.m_smartctl.failure());
        QVERIFY(!!data.m_smartctl.failure());
        QVERIFY(data.m_valid);
    }

    void testTimeout()
    {
        // Query timed out but overall status is available and success.
        // From https://bugs.kde.org/show_bug.cgi?id=428844
        QFile file(QFINDTESTDATA("fixtures/error-info-log-failed.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/nvme0n1");
        QCOMPARE(data.m_status.m_passed, true);
        QCOMPARE(data.m_smartctl.failure(), SMART::Failures({SMART::Failure::InternalCommand}));
        QVERIFY(data.m_smartctl.failure());
        QVERIFY(!!data.m_smartctl.failure());
        QVERIFY(data.m_valid);
    }

    void testFailingSectorsButPassingStatus()
    {
        // SMART status is a pass but there are problems.
        QFile file(QFINDTESTDATA("fixtures/failing-sectors-passing-status.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/sdb");
        QCOMPARE(data.m_status.m_passed, true);
        QCOMPARE(data.m_smartctl.failure(), SMART::Failures({SMART::Failure::ErrorsRecorded | SMART::Failure::SelfTestErrors}));
        QVERIFY(data.m_valid);
    }

    void testVBoxDrive()
    {
        // VBox virtual drives fail with bit2 and have no status
        QFile file(QFINDTESTDATA("fixtures/invalid-vbox.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/sda");
        QCOMPARE(data.m_status.m_passed, false);
        QCOMPARE(data.m_smartctl.failure(), SMART::Failures({SMART::Failure::InternalCommand}));
        QVERIFY(!data.m_valid);
    }

    void testNoSmartStatusNoError()
    {
        // When SMART is disabled we get no smart_status but also no error. Ought to be invalid all the same and
        // ignored.
        // https://bugs.kde.org/show_bug.cgi?id=435699
        QFile file(QFINDTESTDATA("fixtures/pass-without-status.json"));
        QVERIFY(file.open(QFile::ReadOnly));
        auto doc = QJsonDocument::fromJson(file.readAll());
        SMARTData data(doc);
        QCOMPARE(data.m_device, "/dev/sdb");
        QCOMPARE(data.m_status.m_passed, false);
        QCOMPARE(data.m_smartctl.failure(), SMART::Failures());
        QVERIFY(!data.m_valid);
    }
};

QTEST_MAIN(SMARTDataTest)

#include "smartdatatest.moc"
