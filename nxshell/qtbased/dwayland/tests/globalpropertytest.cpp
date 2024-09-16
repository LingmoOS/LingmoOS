/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "../src/client/compositor.h"
#include "../src/client/connection_thread.h"
#include "../src/client/event_queue.h"
#include "../src/client/pointer.h"
#include "../src/client/registry.h"
#include "../src/client/seat.h"
#include "../src/client/shadow.h"
#include "../src/client/shell.h"
#include "../src/client/shm_pool.h"
#include "../src/client/surface.h"
#include "../src/client/server_decoration.h"
#include "../src/client/xdgshell.h"
#include "../src/client/ddeshell.h"
#include "../src/client/globalproperty.h"
// Qt
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QThread>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

using namespace KWayland::Client;

class GlobalPropertyTest : public QObject
{
    Q_OBJECT
public:
    explicit GlobalPropertyTest(QObject *parent = nullptr);
    virtual ~GlobalPropertyTest();

    void init();

private:
    void setupRegistry(Registry *registry);
    void render();
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    ShmPool *m_shm = nullptr;
    Surface *m_surface = nullptr;
    XdgShell *m_xdgShell = nullptr;
    XdgShellSurface *m_xdgShellSurface = nullptr;
    Surface *m_popupSurface = nullptr;
    XdgShellPopup *m_xdgShellPopup = nullptr;
    GlobalProperty *m_ddeGlobalProperty = nullptr;

    DDEShell *m_ddeShell = nullptr;
    DDEShellSurface *m_ddeShellSurface = nullptr;
    KWayland::Client::ServerSideDecorationManager *m_decoration = nullptr;
};

GlobalPropertyTest::GlobalPropertyTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

GlobalPropertyTest::~GlobalPropertyTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void GlobalPropertyTest::init()
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

void GlobalPropertyTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_compositor = registry->createCompositor(name, version, this);
    });
    connect(registry, &Registry::shmAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_shm = registry->createShmPool(name, version, this);
    });
    connect(registry, &Registry::xdgShellStableAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_xdgShell = registry->createXdgShell(name, version, this);
        m_xdgShell->setEventQueue(m_eventQueue);
    });
    connect(registry, &Registry::ddeShellAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_ddeShell = registry->createDDEShell(name, version, this);
        m_ddeShell->setEventQueue(m_eventQueue);
    });
    connect(registry, &Registry::ddeGlobalPropertyAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_ddeGlobalProperty = registry->createGlobalProperty(name, version, this);
        m_ddeGlobalProperty->setEventQueue(m_eventQueue);
    });
    connect(registry, &Registry::serverSideDecorationManagerAnnounced, this, [this, registry](quint32 name, quint32 version) {
        m_decoration = registry->createServerSideDecorationManager(name, version, this);
        m_decoration->setEventQueue(m_eventQueue);
    });
    connect(registry, &Registry::interfacesAnnounced, this, [this] {
        Q_ASSERT(m_compositor);
        Q_ASSERT(m_xdgShell);
        Q_ASSERT(m_shm);
        m_surface = m_compositor->createSurface(this);
        Q_ASSERT(m_surface);
        m_xdgShellSurface = m_xdgShell->createSurface(m_surface, this);
        m_ddeShellSurface = m_ddeShell->createShellSurface(m_surface, this);
        auto parentDeco = m_decoration->create(m_surface, this);
        Q_ASSERT(m_xdgShellSurface);
        connect(m_xdgShellSurface,
                &XdgShellSurface::configureRequested,
                this,
                [this](const QSize &size, KWayland::Client::XdgShellSurface::States states, int serial) {
                    Q_UNUSED(size);
                    Q_UNUSED(states);
                    m_xdgShellSurface->ackConfigure(serial);
                    render();
                });

        m_xdgShellSurface->setTitle(QStringLiteral("Test Window"));

        m_surface->commit();

        QJsonDocument doc;
        QJsonObject obj;
        QJsonArray arr;
        unsigned char scissor_point[] = {0x0, 0x0, 0x0, 0x14, 0x0, 0x0, 0x0, 0x0, 0x40, 0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x66, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x40, 0x67, 0xfe, 0x1d, 0xb3, 0x37, 0x8f, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x40, 0x69, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x20, 0x1e, 0x24, 0xcc, 0x87, 0xf, 0x7b, 0x0, 0x0, 0x0, 0x3, 0x40, 0x69, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x69, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x5c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x40, 0x69, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x5e, 0x7c, 0x3b, 0x66, 0x6f, 0x1e, 0x11, 0x0, 0x0, 0x0, 0x3, 0x40, 0x67, 0xfe, 0x1d, 0xb3, 0x37, 0x8f, 0x8, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x40, 0x66, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x5b, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x59, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x61, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x56, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x40, 0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x40, 0x20, 0x1e, 0x24, 0xcc, 0x87, 0xf, 0x7b, 0x40, 0x60, 0x40, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x5e, 0x7c, 0x3b, 0x66, 0x6f, 0x1e, 0x11, 0x0, 0x0, 0x0, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x5c, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x40, 0x20, 0x1e, 0x24, 0xcc, 0x87, 0xf, 0x7b, 0x0, 0x0, 0x0, 0x3, 0x40, 0x20, 0x1e, 0x24, 0xcc, 0x87, 0xf, 0x7b, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x40, 0x32, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        QByteArray scissor_array;
        scissor_array.resize(sizeof(scissor_point) / sizeof(unsigned char));
        for (unsigned int i = 0; i < scissor_array.size(); i++) {
            scissor_array[i] = scissor_point[i];
            arr.append(QJsonValue((char)scissor_point[i]));
        }
        obj.insert("clippath", arr);
        obj.insert("direction", 1);
        obj.insert("blurRegion", 1);
        obj.insert("unstandardwindow", 1);
        obj.insert("shadow", 1);
        doc.setObject(obj);
        QByteArray jsonByteArr = doc.toJson(QJsonDocument::JsonFormat::Compact);
        // QString jsonStr = QString::fromUtf8(jsonByteArr);
        m_ddeGlobalProperty->setProperty("window", "decorate", m_surface, 1, jsonByteArr.constData());
    });

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void GlobalPropertyTest::render()
{
    const QSize &size = m_xdgShellSurface->size().isValid() ? m_xdgShellSurface->size() : QSize(500, 500);
    auto buffer = m_shm->getBuffer(size, size.width() * 4).toStrongRef();
    buffer->setUsed(true);
    QImage image(buffer->address(), size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(255, 255, 255, 255));
    // draw a red rectangle indicating the anchor of the top level
    QPainter painter(&image);
    painter.setBrush(Qt::red);
    painter.setPen(Qt::black);
    painter.drawRect(10, 10, 180, 100);

    m_surface->attachBuffer(*buffer);
    m_surface->damage(QRect(QPoint(0, 0), size));
    m_surface->commit(Surface::CommitFlag::None);
    buffer->setUsed(false);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    GlobalPropertyTest client;
    client.init();

    return app.exec();
}

#include "globalpropertytest.moc"
