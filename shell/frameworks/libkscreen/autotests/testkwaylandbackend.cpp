/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QObject>
#include <QtTest>

#include "../src/backendmanager_p.h"
#include "../src/config.h"
#include "../src/configmonitor.h"
#include "../src/edid.h"
#include "../src/getconfigoperation.h"
#include "../src/mode.h"
#include "../src/output.h"
#include "../src/screen.h"
#include "../src/setconfigoperation.h"

// KWayland
#include <KWayland/Server/display.h>
#include <KWayland/Server/outputdevice_interface.h>

#include "waylandtestserver.h"

Q_LOGGING_CATEGORY(KSCREEN_WAYLAND, "kscreen.kwayland")

using namespace KScreen;

class testWaylandBackend : public QObject
{
    Q_OBJECT

public:
    explicit testWaylandBackend(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void loadConfig();

    void verifyConfig();
    void verifyOutputs();
    void verifyModes();
    void verifyScreen();
    void verifyIds();
    void verifyFeatures();
    void simpleWrite();
    void addOutput();
    void removeOutput();
    void testEdid();

private:
    ConfigPtr m_config;
    WaylandTestServer *m_server;
    KWayland::Server::OutputDeviceInterface *m_serverOutputDevice;
};

testWaylandBackend::testWaylandBackend(QObject *parent)
    : QObject(parent)
    , m_config(nullptr)
{
    qputenv("KSCREEN_LOGGING", "false");
    m_server = new WaylandTestServer(this);
    m_server->setConfig(QLatin1String(TEST_DATA) + QLatin1String("multipleoutput.json"));
}

void testWaylandBackend::initTestCase()
{
    qputenv("KSCREEN_BACKEND", "kwayland");
    KScreen::BackendManager::instance()->shutdownBackend();
    // This is how KWayland will pick up the right socket,
    // and thus connect to our internal test server.
    setenv("WAYLAND_DISPLAY", s_socketName.toLocal8Bit().constData(), 1);
    m_server->start();

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
}

void testWaylandBackend::loadConfig()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    QVERIFY(m_config->isValid());
    qCDebug(KSCREEN_WAYLAND) << "ops" << m_config->outputs();
}

void testWaylandBackend::verifyConfig()
{
    QVERIFY(m_config != nullptr);
    if (!m_config) {
        QSKIP("Wayland backend invalid", SkipAll);
    }
}

void testWaylandBackend::verifyScreen()
{
    ScreenPtr screen = m_config->screen();

    QVERIFY(screen->minSize().width() <= screen->maxSize().width());
    QVERIFY(screen->minSize().height() <= screen->maxSize().height());

    QVERIFY(screen->minSize().width() <= screen->currentSize().width());
    QVERIFY(screen->minSize().height() <= screen->currentSize().height());

    QVERIFY(screen->maxSize().width() >= screen->currentSize().width());
    QVERIFY(screen->maxSize().height() >= screen->currentSize().height());
    QVERIFY(m_config->screen()->maxActiveOutputsCount() > 0);
}

void testWaylandBackend::verifyOutputs()
{
    bool primaryFound = false;
    for (const KScreen::OutputPtr &op : m_config->outputs()) {
        if (op->isPrimary()) {
            primaryFound = true;
        }
    }
    // qCDebug(KSCREEN_WAYLAND) << "Primary found? " << primaryFound << m_config->outputs();
    QVERIFY(primaryFound);
    QVERIFY(m_config->outputs().count());
    QCOMPARE(m_server->outputCount(), m_config->outputs().count());

    KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary->isEnabled());
    QVERIFY(primary->isConnected());

    QList<int> ids;
    for (const auto &output : m_config->outputs()) {
        QVERIFY(!output->name().isEmpty());
        QVERIFY(output->id() > -1);
        QVERIFY(output->isConnected());
        QVERIFY(output->geometry() != QRectF(1, 1, 1, 1));
        QVERIFY(output->geometry() != QRectF());
        QVERIFY(output->sizeMm() != QSize());
        QVERIFY(output->edid() != nullptr);
        QVERIFY(output->preferredModes().size() == 1);
        QCOMPARE(output->rotation(), Output::None);
        QVERIFY(!ids.contains(output->id()));
        ids << output->id();
    }
}

void testWaylandBackend::verifyModes()
{
    KScreen::OutputPtr primary = m_config->primaryOutput();
    QVERIFY(primary);
    QVERIFY(primary->modes().count() > 0);

    for (const auto &output : m_config->outputs()) {
        for (const auto &mode : output->modes()) {
            QVERIFY(!mode->name().isEmpty());
            QVERIFY(mode->refreshRate() > 0);
            QVERIFY(mode->size().isValid());
        }
    }
}

void testWaylandBackend::verifyIds()
{
    QList<quint32> ids;
    for (const auto &output : m_config->outputs()) {
        QVERIFY(ids.contains(output->id()) == false);
        QVERIFY(output->id() > 0);
        ids << output->id();
    }
}

void testWaylandBackend::simpleWrite()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    m_config = op->config();
    auto output = m_config->output(3);
    QVERIFY(output);
    auto n_mode = QStringLiteral("800x600@60");
    auto o_mode = output->currentModeId();
    output->setCurrentModeId(n_mode);

    auto setop = new SetConfigOperation(m_config);
    QVERIFY(setop->exec());
}

void testWaylandBackend::cleanupTestCase()
{
    m_config->deleteLater();
    KScreen::BackendManager::instance()->shutdownBackend();
}

void testWaylandBackend::addOutput()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    QCOMPARE(config->outputs().count(), 2);
    KScreen::ConfigMonitor *monitor = KScreen::ConfigMonitor::instance();
    monitor->addConfig(config);
    QSignalSpy configSpy(monitor, &KScreen::ConfigMonitor::configurationChanged);

    // Now add an outputdevice on the server side
    m_serverOutputDevice = m_server->display()->createOutputDevice(this);
    m_serverOutputDevice->setUuid("1337");

    OutputDeviceInterface::Mode m0;
    m0.id = 0;
    m0.size = QSize(800, 600);
    m0.flags = OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
    m_serverOutputDevice->addMode(m0);

    OutputDeviceInterface::Mode m1;
    m1.id = 1;
    m1.size = QSize(1024, 768);
    m_serverOutputDevice->addMode(m1);

    OutputDeviceInterface::Mode m2;
    m2.id = 2;
    m2.size = QSize(1280, 1024);
    m2.refreshRate = 90000;
    m_serverOutputDevice->addMode(m2);

    m_serverOutputDevice->setCurrentMode(1);

    m_serverOutputDevice->create();

    QVERIFY(configSpy.wait());
    // QTRY_VERIFY(configSpy.count());

    GetConfigOperation *op2 = new GetConfigOperation();
    op2->exec();
    auto newconfig = op2->config();
    QCOMPARE(newconfig->outputs().count(), 3);
}

void testWaylandBackend::removeOutput()
{
    KScreen::BackendManager::instance()->shutdownBackend();
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    QCOMPARE(config->outputs().count(), 3);
    KScreen::ConfigMonitor *monitor = KScreen::ConfigMonitor::instance();
    monitor->addConfig(config);
    QSignalSpy configSpy(monitor, &KScreen::ConfigMonitor::configurationChanged);

    delete m_serverOutputDevice;
    QVERIFY(configSpy.wait());
    GetConfigOperation *op2 = new GetConfigOperation();
    op2->exec();
    auto newconfig = op2->config();
    QCOMPARE(newconfig->outputs().count(), 2);
}

void testWaylandBackend::testEdid()
{
    m_server->showOutputs();

    QByteArray data = QByteArray::fromBase64(
        "AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/"
        "QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHow"
        "K0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==");

    QScopedPointer<Edid> edid(new Edid(data));
    QVERIFY(edid->isValid());

    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    QVERIFY(config->outputs().count() > 0);

    auto o = config->outputs().last();
    qCDebug(KSCREEN_WAYLAND) << "Edid: " << o->edid()->isValid();
    QVERIFY(o->edid()->isValid());
    QCOMPARE(o->edid()->deviceId(), edid->deviceId());
    QCOMPARE(o->edid()->name(), edid->name());
    QCOMPARE(o->edid()->vendor(), edid->vendor());
    QCOMPARE(o->edid()->eisaId(), edid->eisaId());
    QCOMPARE(o->edid()->serial(), edid->serial());
    QCOMPARE(o->edid()->hash(), edid->hash());
    QCOMPARE(o->edid()->width(), edid->width());
    QCOMPARE(o->edid()->height(), edid->height());
    QCOMPARE(o->edid()->gamma(), edid->gamma());
    QCOMPARE(o->edid()->red(), edid->red());
    QCOMPARE(o->edid()->green(), edid->green());
    QCOMPARE(o->edid()->blue(), edid->blue());
    QCOMPARE(o->edid()->white(), edid->white());
}

void testWaylandBackend::verifyFeatures()
{
    GetConfigOperation *op = new GetConfigOperation();
    op->exec();
    auto config = op->config();
    QVERIFY(!config->supportedFeatures().testFlag(Config::Feature::None));
    QVERIFY(config->supportedFeatures().testFlag(Config::Feature::Writable));
    QVERIFY(!config->supportedFeatures().testFlag(Config::Feature::PrimaryDisplay));
}

QTEST_GUILESS_MAIN(testWaylandBackend)

#include "testkwaylandbackend.moc"
