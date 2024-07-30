/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "../src/client/compositor.h"
#include "../src/client/connection_thread.h"
#include "../src/client/event_queue.h"
#include "../src/client/lingmoshell.h"
#include "../src/client/registry.h"
#include "../src/client/shell.h"
#include "../src/client/shm_pool.h"
#include "../src/client/surface.h"
// Qt
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QImage>
#include <QThread>

using namespace KWayland::Client;

class LingmoSurfaceTest : public QObject
{
    Q_OBJECT
public:
    explicit LingmoSurfaceTest(QObject *parent = nullptr);
    ~LingmoSurfaceTest() override;

    void init();

    void setRole(LingmoShellSurface::Role role)
    {
        m_role = role;
    }
    void setSkipTaskbar(bool set)
    {
        m_skipTaskbar = set;
    }

    void setSkipSwitcher(bool set)
    {
        m_skipSwitcher = set;
    }

private:
    void setupRegistry(Registry *registry);
    void render();
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    Shell *m_shell = nullptr;
    ShellSurface *m_shellSurface = nullptr;
    ShmPool *m_shm = nullptr;
    Surface *m_surface = nullptr;
    LingmoShell *m_lingmoShell = nullptr;
    LingmoShellSurface *m_lingmoShellSurface = nullptr;
    LingmoShellSurface::Role m_role = LingmoShellSurface::Role::Normal;
    bool m_skipTaskbar = false;
    bool m_skipSwitcher = false;
};

LingmoSurfaceTest::LingmoSurfaceTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

LingmoSurfaceTest::~LingmoSurfaceTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void LingmoSurfaceTest::init()
{
    connect(
        m_connectionThreadObject,
        &ConnectionThread::connected,
        this,
        [this] {
            m_eventQueue = new EventQueue(this);
            m_eventQueue->setup(m_connectionThreadObject);

            Registry *registry = new Registry(this);
            setupRegistry(registry);
        },
        Qt::QueuedConnection);
    m_connectionThreadObject->moveToThread(m_connectionThread);
    m_connectionThread->start();

    m_connectionThreadObject->initConnection();
}

void LingmoSurfaceTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_compositor = registry->createCompositor(name, version, this);
    });
    connect(registry, &Registry::shellAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_shell = registry->createShell(name, version, this);
    });
    connect(registry, &Registry::shmAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_shm = registry->createShmPool(name, version, this);
    });
    connect(registry, &Registry::lingmoShellAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_lingmoShell = registry->createLingmoShell(name, version, this);
        m_lingmoShell->setEventQueue(m_eventQueue);
    });
    connect(registry, &Registry::interfacesAnnounced, this, [this] {
        Q_ASSERT(m_compositor);
        Q_ASSERT(m_shell);
        Q_ASSERT(m_shm);
        Q_ASSERT(m_lingmoShell);
        m_surface = m_compositor->createSurface(this);
        Q_ASSERT(m_surface);
        m_shellSurface = m_shell->createSurface(m_surface, this);
        Q_ASSERT(m_shellSurface);
        m_shellSurface->setToplevel();
        connect(m_shellSurface, &ShellSurface::sizeChanged, this, &LingmoSurfaceTest::render);
        m_lingmoShellSurface = m_lingmoShell->createSurface(m_surface, this);
        Q_ASSERT(m_lingmoShellSurface);
        m_lingmoShellSurface->setSkipTaskbar(m_skipTaskbar);
        m_lingmoShellSurface->setSkipSwitcher(m_skipSwitcher);
        m_lingmoShellSurface->setRole(m_role);
        render();
    });
    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void LingmoSurfaceTest::render()
{
    const QSize &size = m_shellSurface->size().isValid() ? m_shellSurface->size() : QSize(300, 200);
    auto buffer = m_shm->getBuffer(size, size.width() * 4).toStrongRef();
    buffer->setUsed(true);
    QImage image(buffer->address(), size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(255, 255, 255, 128));

    m_surface->attachBuffer(*buffer);
    m_surface->damage(QRect(QPoint(0, 0), size));
    m_surface->commit(Surface::CommitFlag::None);
    buffer->setUsed(false);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption notificationOption(QStringLiteral("notification"));
    parser.addOption(notificationOption);
    QCommandLineOption criticalNotificationOption(QStringLiteral("criticalNotification"));
    parser.addOption(criticalNotificationOption);
    QCommandLineOption appletPopupOption(QStringLiteral("appletPopup"));
    parser.addOption(appletPopupOption);
    QCommandLineOption panelOption(QStringLiteral("panel"));
    parser.addOption(panelOption);
    QCommandLineOption desktopOption(QStringLiteral("desktop"));
    parser.addOption(desktopOption);
    QCommandLineOption osdOption(QStringLiteral("osd"));
    parser.addOption(osdOption);
    QCommandLineOption tooltipOption(QStringLiteral("tooltip"));
    parser.addOption(tooltipOption);
    QCommandLineOption skipTaskbarOption(QStringLiteral("skipTaskbar"));
    parser.addOption(skipTaskbarOption);
    parser.process(app);
    QCommandLineOption skipSwitcherOption(QStringLiteral("skipSwitcher"));
    parser.addOption(skipSwitcherOption);
    parser.process(app);

    LingmoSurfaceTest client;

    if (parser.isSet(notificationOption)) {
        client.setRole(LingmoShellSurface::Role::Notification);
    } else if (parser.isSet(criticalNotificationOption)) {
        client.setRole(LingmoShellSurface::Role::CriticalNotification);
    } else if (parser.isSet(panelOption)) {
        client.setRole(LingmoShellSurface::Role::Panel);
    } else if (parser.isSet(desktopOption)) {
        client.setRole(LingmoShellSurface::Role::Desktop);
    } else if (parser.isSet(osdOption)) {
        client.setRole(LingmoShellSurface::Role::OnScreenDisplay);
    } else if (parser.isSet(tooltipOption)) {
        client.setRole(LingmoShellSurface::Role::ToolTip);
    } else if (parser.isSet(appletPopupOption)) {
        client.setRole(LingmoShellSurface::Role::AppletPopup);
    }
    client.setSkipTaskbar(parser.isSet(skipTaskbarOption));
    client.setSkipSwitcher(parser.isSet(skipSwitcherOption));

    client.init();

    return app.exec();
}

#include "lingmosurfacetest.moc"
