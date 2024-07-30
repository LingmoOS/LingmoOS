/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
// Qt
#include <QSignalSpy>
#include <QTest>
// KWin
#include "wayland/compositor.h"
#include "wayland/display.h"
#include "wayland/lingmovirtualdesktop.h"
#include "wayland/lingmowindowmanagement.h"

#include "KWayland/Client/compositor.h"
#include "KWayland/Client/connection_thread.h"
#include "KWayland/Client/event_queue.h"
#include "KWayland/Client/lingmovirtualdesktop.h"
#include "KWayland/Client/lingmowindowmanagement.h"
#include "KWayland/Client/region.h"
#include "KWayland/Client/registry.h"
#include "KWayland/Client/surface.h"

class TestVirtualDesktop : public QObject
{
    Q_OBJECT
public:
    explicit TestVirtualDesktop(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreate();
    void testSetRows();
    void testConnectNewClient();
    void testDestroy();
    void testActivate();

    void testEnterLeaveDesktop();
    void testAllDesktops();
    void testCreateRequested();
    void testRemoveRequested();

private:
    KWin::Display *m_display;
    KWin::CompositorInterface *m_compositorInterface;
    KWin::LingmoVirtualDesktopManagementInterface *m_lingmoVirtualDesktopManagementInterface;
    KWin::LingmoWindowManagementInterface *m_windowManagementInterface;
    KWin::LingmoWindowInterface *m_windowInterface;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::LingmoVirtualDesktopManagement *m_lingmoVirtualDesktopManagement;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::LingmoWindowManagement *m_windowManagement;
    KWayland::Client::LingmoWindow *m_window;

    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-virtual-desktop-0");

TestVirtualDesktop::TestVirtualDesktop(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
{
}

void TestVirtualDesktop::init()
{
    using namespace KWin;
    delete m_display;
    m_display = new KWin::Display(this);
    m_display->addSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, &KWayland::Client::ConnectionThread::connected);
    m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    m_queue = new KWayland::Client::EventQueue(this);
    QVERIFY(!m_queue->isValid());
    m_queue->setup(m_connection);
    QVERIFY(m_queue->isValid());

    KWayland::Client::Registry registry;
    QSignalSpy compositorSpy(&registry, &KWayland::Client::Registry::compositorAnnounced);

    QSignalSpy lingmoVirtualDesktopManagementSpy(&registry, &KWayland::Client::Registry::lingmoVirtualDesktopManagementAnnounced);

    QSignalSpy windowManagementSpy(&registry, &KWayland::Client::Registry::lingmoWindowManagementAnnounced);

    QVERIFY(!registry.eventQueue());
    registry.setEventQueue(m_queue);
    QCOMPARE(registry.eventQueue(), m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    m_compositorInterface = new CompositorInterface(m_display, m_display);
    QVERIFY(compositorSpy.wait());
    m_compositor = registry.createCompositor(compositorSpy.first().first().value<quint32>(), compositorSpy.first().last().value<quint32>(), this);

    m_lingmoVirtualDesktopManagementInterface = new LingmoVirtualDesktopManagementInterface(m_display, m_display);

    QVERIFY(lingmoVirtualDesktopManagementSpy.wait());
    m_lingmoVirtualDesktopManagement = registry.createLingmoVirtualDesktopManagement(lingmoVirtualDesktopManagementSpy.first().first().value<quint32>(),
                                                                                     lingmoVirtualDesktopManagementSpy.first().last().value<quint32>(),
                                                                                     this);

    m_windowManagementInterface = new LingmoWindowManagementInterface(m_display, m_display);
    m_windowManagementInterface->setLingmoVirtualDesktopManagementInterface(m_lingmoVirtualDesktopManagementInterface);

    QVERIFY(windowManagementSpy.wait());
    m_windowManagement =
        registry.createLingmoWindowManagement(windowManagementSpy.first().first().value<quint32>(), windowManagementSpy.first().last().value<quint32>(), this);

    QSignalSpy windowSpy(m_windowManagement, &KWayland::Client::LingmoWindowManagement::windowCreated);
    m_windowInterface = m_windowManagementInterface->createWindow(this, QUuid::createUuid());
    m_windowInterface->setPid(1337);

    QVERIFY(windowSpy.wait());
    m_window = windowSpy.first().first().value<KWayland::Client::LingmoWindow *>();
}

void TestVirtualDesktop::cleanup()
{
#define CLEANUP(variable)   \
    if (variable) {         \
        delete variable;    \
        variable = nullptr; \
    }
    CLEANUP(m_compositor)
    CLEANUP(m_lingmoVirtualDesktopManagement)
    CLEANUP(m_windowInterface)
    CLEANUP(m_windowManagement)
    CLEANUP(m_queue)
    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    CLEANUP(m_compositorInterface)
    CLEANUP(m_lingmoVirtualDesktopManagementInterface)
    CLEANUP(m_windowManagementInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void TestVirtualDesktop::testCreate()
{
    QSignalSpy desktopCreatedSpy(m_lingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::desktopCreated);
    QSignalSpy managementDoneSpy(m_lingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::done);

    // on this createDesktop bind() isn't called already, the desktopadded signals will be sent after bind happened
    KWin::LingmoVirtualDesktopInterface *desktop1Int = m_lingmoVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-1"));
    desktop1Int->setName("Desktop 1");

    QVERIFY(desktopCreatedSpy.wait());
    QList<QVariant> arguments = desktopCreatedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(arguments.at(1).toUInt(), (quint32)0);
    m_lingmoVirtualDesktopManagementInterface->sendDone();
    QVERIFY(managementDoneSpy.wait());

    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 1);

    KWayland::Client::LingmoVirtualDesktop *desktop1 = m_lingmoVirtualDesktopManagement->desktops().first();
    QSignalSpy desktop1DoneSpy(desktop1, &KWayland::Client::LingmoVirtualDesktop::done);
    desktop1Int->sendDone();
    QVERIFY(desktop1DoneSpy.wait());

    QCOMPARE(desktop1->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop1->name(), QStringLiteral("Desktop 1"));

    // on those createDesktop the bind will already be done
    KWin::LingmoVirtualDesktopInterface *desktop2Int = m_lingmoVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-2"));
    desktop2Int->setName("Desktop 2");
    QVERIFY(desktopCreatedSpy.wait());
    arguments = desktopCreatedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-2"));
    QCOMPARE(arguments.at(1).toUInt(), (quint32)1);
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 2);

    KWin::LingmoVirtualDesktopInterface *desktop3Int = m_lingmoVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-3"));
    desktop3Int->setName("Desktop 3");
    QVERIFY(desktopCreatedSpy.wait());
    arguments = desktopCreatedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-3"));
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 3);

    m_lingmoVirtualDesktopManagementInterface->sendDone();
    QVERIFY(managementDoneSpy.wait());

    // get the clients
    KWayland::Client::LingmoVirtualDesktop *desktop2 = m_lingmoVirtualDesktopManagement->desktops()[1];
    QSignalSpy desktop2DoneSpy(desktop2, &KWayland::Client::LingmoVirtualDesktop::done);
    desktop2Int->sendDone();
    QVERIFY(desktop2DoneSpy.wait());

    KWayland::Client::LingmoVirtualDesktop *desktop3 = m_lingmoVirtualDesktopManagement->desktops()[2];
    QSignalSpy desktop3DoneSpy(desktop3, &KWayland::Client::LingmoVirtualDesktop::done);
    desktop3Int->sendDone();
    QVERIFY(desktop3DoneSpy.wait());

    QCOMPARE(desktop1->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop1->name(), QStringLiteral("Desktop 1"));

    QCOMPARE(desktop2->id(), QStringLiteral("0-2"));
    QCOMPARE(desktop2->name(), QStringLiteral("Desktop 2"));

    QCOMPARE(desktop3->id(), QStringLiteral("0-3"));
    QCOMPARE(desktop3->name(), QStringLiteral("Desktop 3"));

    // coherence of order between client and server
    QCOMPARE(m_lingmoVirtualDesktopManagementInterface->desktops().length(), 3);
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 3);

    for (int i = 0; i < m_lingmoVirtualDesktopManagement->desktops().length(); ++i) {
        QCOMPARE(m_lingmoVirtualDesktopManagementInterface->desktops().at(i)->id(), m_lingmoVirtualDesktopManagement->desktops().at(i)->id());
    }
}

void TestVirtualDesktop::testSetRows()
{
    // rebuild some desktops
    testCreate();

    QSignalSpy rowsChangedSpy(m_lingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::rowsChanged);

    m_lingmoVirtualDesktopManagementInterface->setRows(3);
    QVERIFY(rowsChangedSpy.wait());
    QCOMPARE(m_lingmoVirtualDesktopManagement->rows(), 3);
}

void TestVirtualDesktop::testConnectNewClient()
{
    // rebuild some desktops
    testCreate();

    KWayland::Client::Registry registry;
    QVERIFY(!registry.eventQueue());
    registry.setEventQueue(m_queue);
    QCOMPARE(registry.eventQueue(), m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    QSignalSpy lingmoVirtualDesktopManagementSpy(&registry, &KWayland::Client::Registry::lingmoVirtualDesktopManagementAnnounced);

    QVERIFY(lingmoVirtualDesktopManagementSpy.wait());

    KWayland::Client::LingmoVirtualDesktopManagement *otherLingmoVirtualDesktopManagement =
        registry.createLingmoVirtualDesktopManagement(lingmoVirtualDesktopManagementSpy.first().first().value<quint32>(),
                                                      lingmoVirtualDesktopManagementSpy.first().last().value<quint32>(),
                                                      this);

    QSignalSpy managementDoneSpy(otherLingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::done);

    QVERIFY(managementDoneSpy.wait());
    QCOMPARE(otherLingmoVirtualDesktopManagement->desktops().length(), 3);

    delete otherLingmoVirtualDesktopManagement;
}

void TestVirtualDesktop::testDestroy()
{
    // rebuild some desktops
    testCreate();

    KWin::LingmoVirtualDesktopInterface *desktop1Int = m_lingmoVirtualDesktopManagementInterface->desktops().first();
    KWayland::Client::LingmoVirtualDesktop *desktop1 = m_lingmoVirtualDesktopManagement->desktops().first();

    QSignalSpy desktop1IntDestroyedSpy(desktop1Int, &QObject::destroyed);
    QSignalSpy desktop1DestroyedSpy(desktop1, &QObject::destroyed);
    QSignalSpy desktop1RemovedSpy(desktop1, &KWayland::Client::LingmoVirtualDesktop::removed);
    m_lingmoVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-1"));

    // test that both server and client desktoip interfaces go away
    QVERIFY(!desktop1IntDestroyedSpy.isEmpty());
    QVERIFY(desktop1RemovedSpy.wait());
    QVERIFY(desktop1DestroyedSpy.wait());

    // coherence of order between client and server
    QCOMPARE(m_lingmoVirtualDesktopManagementInterface->desktops().length(), 2);
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 2);

    for (int i = 0; i < m_lingmoVirtualDesktopManagement->desktops().length(); ++i) {
        QCOMPARE(m_lingmoVirtualDesktopManagementInterface->desktops().at(i)->id(), m_lingmoVirtualDesktopManagement->desktops().at(i)->id());
    }

    // Test the desktopRemoved signal of the manager, remove another desktop as the signals can't be tested at the same time
    QSignalSpy desktopManagerRemovedSpy(m_lingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::desktopRemoved);
    m_lingmoVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-2"));
    QVERIFY(desktopManagerRemovedSpy.wait());
    QCOMPARE(desktopManagerRemovedSpy.takeFirst().at(0).toString(), QStringLiteral("0-2"));

    QCOMPARE(m_lingmoVirtualDesktopManagementInterface->desktops().length(), 1);
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().length(), 1);
}

void TestVirtualDesktop::testActivate()
{
    // rebuild some desktops
    testCreate();

    KWin::LingmoVirtualDesktopInterface *desktop1Int = m_lingmoVirtualDesktopManagementInterface->desktops().first();
    KWayland::Client::LingmoVirtualDesktop *desktop1 = m_lingmoVirtualDesktopManagement->desktops().first();
    QVERIFY(desktop1->isActive());
    QVERIFY(desktop1Int->isActive());

    KWin::LingmoVirtualDesktopInterface *desktop2Int = m_lingmoVirtualDesktopManagementInterface->desktops()[1];
    KWayland::Client::LingmoVirtualDesktop *desktop2 = m_lingmoVirtualDesktopManagement->desktops()[1];
    QVERIFY(!desktop2Int->isActive());

    QSignalSpy requestActivateSpy(desktop2Int, &KWin::LingmoVirtualDesktopInterface::activateRequested);
    QSignalSpy activatedSpy(desktop2, &KWayland::Client::LingmoVirtualDesktop::activated);

    desktop2->requestActivate();
    QVERIFY(requestActivateSpy.wait());

    // This simulates a compositor which supports only one active desktop at a time
    for (auto *deskInt : m_lingmoVirtualDesktopManagementInterface->desktops()) {
        if (deskInt->id() == desktop2->id()) {
            deskInt->setActive(true);
        } else {
            deskInt->setActive(false);
        }
    }
    QVERIFY(activatedSpy.wait());

    // correct state in the server
    QVERIFY(desktop2Int->isActive());
    QVERIFY(!desktop1Int->isActive());
    // correct state in the client
    QVERIFY(desktop2Int->isActive());
    QVERIFY(!desktop1Int->isActive());

    // Test the deactivated signal
    QSignalSpy deactivatedSpy(desktop2, &KWayland::Client::LingmoVirtualDesktop::deactivated);

    for (auto *deskInt : m_lingmoVirtualDesktopManagementInterface->desktops()) {
        if (deskInt->id() == desktop1->id()) {
            deskInt->setActive(true);
        } else {
            deskInt->setActive(false);
        }
    }
    QVERIFY(deactivatedSpy.wait());
}

void TestVirtualDesktop::testEnterLeaveDesktop()
{
    testCreate();

    QSignalSpy enterRequestedSpy(m_windowInterface, &KWin::LingmoWindowInterface::enterLingmoVirtualDesktopRequested);
    m_window->requestEnterVirtualDesktop(QStringLiteral("0-1"));
    QVERIFY(enterRequestedSpy.wait());

    QCOMPARE(enterRequestedSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));

    QSignalSpy virtualDesktopEnteredSpy(m_window, &KWayland::Client::LingmoWindow::lingmoVirtualDesktopEntered);

    // agree to the request
    m_windowInterface->addLingmoVirtualDesktop(QStringLiteral("0-1"));
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().length(), 1);
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().first(), QStringLiteral("0-1"));

    // check if the client received the enter
    QVERIFY(virtualDesktopEnteredSpy.wait());
    QCOMPARE(virtualDesktopEnteredSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 1);
    QCOMPARE(m_window->lingmoVirtualDesktops().first(), QStringLiteral("0-1"));

    // add another desktop, server side
    m_windowInterface->addLingmoVirtualDesktop(QStringLiteral("0-3"));
    QVERIFY(virtualDesktopEnteredSpy.wait());
    QCOMPARE(virtualDesktopEnteredSpy.takeFirst().at(0).toString(), QStringLiteral("0-3"));
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().length(), 2);
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 2);
    QCOMPARE(m_window->lingmoVirtualDesktops()[1], QStringLiteral("0-3"));

    // try to add an invalid desktop
    m_windowInterface->addLingmoVirtualDesktop(QStringLiteral("invalid"));
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 2);

    // remove a desktop
    QSignalSpy leaveRequestedSpy(m_windowInterface, &KWin::LingmoWindowInterface::leaveLingmoVirtualDesktopRequested);
    m_window->requestLeaveVirtualDesktop(QStringLiteral("0-1"));
    QVERIFY(leaveRequestedSpy.wait());

    QCOMPARE(leaveRequestedSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));

    QSignalSpy virtualDesktopLeftSpy(m_window, &KWayland::Client::LingmoWindow::lingmoVirtualDesktopLeft);

    // agree to the request
    m_windowInterface->removeLingmoVirtualDesktop(QStringLiteral("0-1"));
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().length(), 1);
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().first(), QStringLiteral("0-3"));

    // check if the client received the leave
    QVERIFY(virtualDesktopLeftSpy.wait());
    QCOMPARE(virtualDesktopLeftSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 1);
    QCOMPARE(m_window->lingmoVirtualDesktops().first(), QStringLiteral("0-3"));

    // Destroy desktop 2
    m_lingmoVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-3"));
    // the window should receive a left signal from the destroyed desktop
    QVERIFY(virtualDesktopLeftSpy.wait());

    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 0);
}

void TestVirtualDesktop::testAllDesktops()
{
    testCreate();
    QSignalSpy virtualDesktopEnteredSpy(m_window, &KWayland::Client::LingmoWindow::lingmoVirtualDesktopEntered);
    QSignalSpy virtualDesktopLeftSpy(m_window, &KWayland::Client::LingmoWindow::lingmoVirtualDesktopLeft);

    // in the beginning the window is on desktop 1 and desktop 3
    m_windowInterface->addLingmoVirtualDesktop(QStringLiteral("0-1"));
    m_windowInterface->addLingmoVirtualDesktop(QStringLiteral("0-3"));
    QVERIFY(virtualDesktopEnteredSpy.wait());

    // setting on all desktops
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 2);
    m_windowInterface->setOnAllDesktops(true);
    // setting on all desktops, the window will leave every desktop

    QVERIFY(virtualDesktopLeftSpy.wait());
    QCOMPARE(virtualDesktopLeftSpy.count(), 2);
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 0);
    QVERIFY(m_window->isOnAllDesktops());

    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 0);
    QVERIFY(m_window->isOnAllDesktops());

    // return to the active desktop (0-1)
    m_windowInterface->setOnAllDesktops(false);
    QVERIFY(virtualDesktopEnteredSpy.wait());
    QCOMPARE(m_window->lingmoVirtualDesktops().length(), 1);
    QCOMPARE(m_windowInterface->lingmoVirtualDesktops().first(), QStringLiteral("0-1"));
    QVERIFY(!m_window->isOnAllDesktops());
}

void TestVirtualDesktop::testCreateRequested()
{
    // rebuild some desktops
    testCreate();

    QSignalSpy desktopCreateRequestedSpy(m_lingmoVirtualDesktopManagementInterface,
                                         &KWin::LingmoVirtualDesktopManagementInterface::desktopCreateRequested);
    QSignalSpy desktopCreatedSpy(m_lingmoVirtualDesktopManagement, &KWayland::Client::LingmoVirtualDesktopManagement::desktopCreated);

    // listen for createdRequested
    m_lingmoVirtualDesktopManagement->requestCreateVirtualDesktop(QStringLiteral("Desktop"), 1);
    QVERIFY(desktopCreateRequestedSpy.wait());
    QCOMPARE(desktopCreateRequestedSpy.first().first().toString(), QStringLiteral("Desktop"));
    QCOMPARE(desktopCreateRequestedSpy.first().at(1).toUInt(), (quint32)1);

    // actually create
    m_lingmoVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-4"), 1);
    KWin::LingmoVirtualDesktopInterface *desktopInt = m_lingmoVirtualDesktopManagementInterface->desktops().at(1);

    QCOMPARE(desktopInt->id(), QStringLiteral("0-4"));
    desktopInt->setName(QStringLiteral("Desktop"));

    QVERIFY(desktopCreatedSpy.wait());

    QCOMPARE(desktopCreatedSpy.first().first().toString(), QStringLiteral("0-4"));
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().count(), 4);

    KWayland::Client::LingmoVirtualDesktop *desktop = m_lingmoVirtualDesktopManagement->desktops().at(1);
    QSignalSpy desktopDoneSpy(desktop, &KWayland::Client::LingmoVirtualDesktop::done);
    desktopInt->sendDone();
    // desktopDoneSpy.wait();
    // check the order is correct
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().at(0)->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop->id(), QStringLiteral("0-4"));
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().at(2)->id(), QStringLiteral("0-2"));
    QCOMPARE(m_lingmoVirtualDesktopManagement->desktops().at(3)->id(), QStringLiteral("0-3"));
}

void TestVirtualDesktop::testRemoveRequested()
{
    // rebuild some desktops
    testCreate();

    QSignalSpy desktopRemoveRequestedSpy(m_lingmoVirtualDesktopManagementInterface,
                                         &KWin::LingmoVirtualDesktopManagementInterface::desktopRemoveRequested);

    // request a remove, just check the request arrived, ignore the request.
    m_lingmoVirtualDesktopManagement->requestRemoveVirtualDesktop(QStringLiteral("0-1"));
    QVERIFY(desktopRemoveRequestedSpy.wait());
    QCOMPARE(desktopRemoveRequestedSpy.first().first().toString(), QStringLiteral("0-1"));
}

QTEST_GUILESS_MAIN(TestVirtualDesktop)
#include "test_lingmo_virtual_desktop.moc"
