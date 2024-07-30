/*
 * SPDX-FileCopyrightText: 2014 Daniel Vratil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 */

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "../src/backendmanager_p.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/configoperation.h"
#include "../src/getconfigoperation.h"
#include "../src/output.h"
#include "../src/setconfigoperation.h"
#include <QSignalSpy>

#include "fakebackendinterface.h"

class TestConfigMonitor : public QObject
{
    Q_OBJECT
public:
    TestConfigMonitor()
    {
    }

    KScreen::ConfigPtr getConfig()
    {
        auto op = new KScreen::GetConfigOperation();
        if (!op->exec()) {
            qWarning("Failed to retrieve backend: %s", qPrintable(op->errorString()));
            return KScreen::ConfigPtr();
        }

        return op->config();
    }

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("KSCREEN_LOGGING", "false");
        qputenv("KSCREEN_BACKEND", "Fake");
        // This particular test is only useful for out of process operation, so enforce that
        qputenv("KSCREEN_BACKEND_INPROCESS", "0");
        KScreen::BackendManager::instance()->shutdownBackend();
    }

    void cleanupTestCase()
    {
        KScreen::BackendManager::instance()->shutdownBackend();
    }

    void testChangeNotifyInProcess()
    {
        qputenv("KSCREEN_BACKEND_INPROCESS", "1");
        KScreen::BackendManager::instance()->shutdownBackend();
        KScreen::BackendManager::instance()->setMethod(KScreen::BackendManager::InProcess);
        // json file for the fake backend
        KScreen::BackendManager::instance()->setBackendArgs({{QStringLiteral("TEST_DATA"), TEST_DATA "singleoutput.json"}});

        // Prepare monitor
        KScreen::ConfigMonitor *monitor = KScreen::ConfigMonitor::instance();
        QSignalSpy spy(monitor, SIGNAL(configurationChanged()));

        // Get config and monitor it for changes
        KScreen::ConfigPtr config = getConfig();
        monitor->addConfig(config);
        QSignalSpy enabledSpy(config->outputs().first().data(), SIGNAL(isEnabledChanged()));

        auto output = config->outputs().first();

        output->setEnabled(false);
        auto setop = new KScreen::SetConfigOperation(config);
        QVERIFY(!setop->hasError());
        setop->exec();
        QTRY_VERIFY(!spy.isEmpty());

        QCOMPARE(spy.size(), 1);
        QCOMPARE(enabledSpy.size(), 1);
        QCOMPARE(config->output(1)->isEnabled(), false);

        output->setEnabled(false);
        auto setop2 = new KScreen::SetConfigOperation(config);
        QVERIFY(!setop2->hasError());
        setop2->exec();
        QTRY_VERIFY(!spy.isEmpty());
        QCOMPARE(spy.size(), 2);
    }
};

QTEST_MAIN(TestConfigMonitor)

#include "testconfigmonitor.moc"
