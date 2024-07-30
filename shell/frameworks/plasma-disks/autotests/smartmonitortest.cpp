// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QObject>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <functional>

#include "devicenotifier.h"
#include <device.h>
#include <smartctl.h>
#include <smartmonitor.h>

class SMARTMonitorTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRun()
    {
        struct Ctl : public AbstractSMARTCtl {
            static QString readCLIData()
            {
                QFile file(QFINDTESTDATA("fixtures/cli.txt"));
                const bool open = file.open(QFile::ReadOnly);
                Q_ASSERT(open);
                return file.readAll();
            }

            void run(const QString &devicePath) override
            {
                static QMap<QString, QString> data{{"/dev/testfoobarpass", "fixtures/pass.json"},
                                                   {"/dev/invalid-vbox.json", "fixtures/invalid-vbox.json"},
                                                   {"/dev/testfoobarfail", "fixtures/fail.json"},
                                                   {"/dev/invalid-cmdline-bad-usb-bridge", "fixtures/invalid-cmdline-bad-usb-bridge.json"}};

                const QString fixture = data.value(devicePath);
                Q_ASSERT(!fixture.isEmpty());
                QFile file(QFINDTESTDATA(fixture));
                const bool open = file.open(QFile::ReadOnly);
                Q_ASSERT(open);
                QJsonParseError err{};
                const auto document = QJsonDocument::fromJson(file.readAll(), &err);
                Q_ASSERT(err.error == QJsonParseError::NoError);

                static QString cliData = readCLIData();
                Q_EMIT finished(devicePath, document, cliData);
            }
        };

        struct Notifier : public DeviceNotifier {
            using DeviceNotifier::DeviceNotifier;
            void start() override
            {
                loadData();
            }
            void loadData() override
            {
                Q_EMIT addDevice(new Device{"udi-pass", "product", "/dev/testfoobarpass"});
                Q_EMIT addDevice(new Device{"udi-invalid", "product", "/dev/invalid-vbox.json"});
                Q_EMIT addDevice(new Device{"udi-invalid-cmdline-bad-usb-bridge", "product", "/dev/invalid-cmdline-bad-usb-bridge"});
                // discover this twice to ensure notifications aren't duplicated!
                Q_EMIT addDevice(new Device{"udi-fail", "product", "/dev/testfoobarfail"});
                Q_EMIT addDevice(new Device{"udi-fail", "product", "/dev/testfoobarfail"});
            }
        };

        SMARTMonitor monitor(std::make_unique<Ctl>(), std::make_unique<Notifier>());
        QSignalSpy spy(&monitor, &SMARTMonitor::deviceAdded);
        QVERIFY(spy.isValid());
        monitor.start();
        // The signals are all emitted in one go and as such should arrive
        // within a single wait.
        QVERIFY(spy.wait());
        QCOMPARE(spy.count(), 2); // There are 3 devices but one is a dupe.
        QCOMPARE(monitor.devices().count(), 2); // There are 3 devices but one is a dupe.

        bool sawPass = false;
        bool sawInvalid = false;
        bool sawFail = false;
        bool sawInvalidCmdLine = false;
        for (const auto *device : monitor.devices()) {
            if (device->path() == "/dev/testfoobarpass") {
                QVERIFY(!device->failed());
                sawPass = true;
            }
            if (device->path() == "/dev/invalid") {
                sawInvalid = true;
            }
            if (device->path() == "/dev/invalid-cmdline-bad-usb-bridge") {
                sawInvalidCmdLine = true;
            }
            if (device->path() == "/dev/testfoobarfail") {
                QVERIFY(device->failed());
                sawFail = true;
            }
        }
        QVERIFY(sawPass);
        QVERIFY(!sawInvalid); // mustn't be seen, it's an invalid device ;)
        QVERIFY(!sawInvalidCmdLine); // ditto
        QVERIFY(sawFail);

        // Ensure removing works as well.
        // https://bugs.kde.org/show_bug.cgi?id=428746

        monitor.removeUDI("udi-pass");
        QCOMPARE(monitor.devices().size(), 1);
    }
};

QTEST_MAIN(SMARTMonitorTest)

#include "smartmonitortest.moc"
