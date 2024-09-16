/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "connection_thread.h"
#include "logging.h"
// Qt
#include <QAbstractEventDispatcher>
#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QGuiApplication>
#include <QMutex>
#include <QMutexLocker>
#include <QSocketNotifier>
#include <qpa/qplatformnativeinterface.h>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN ConnectionThread::Private
{
public:
    Private(ConnectionThread *q);
    ~Private();
    void doInitConnection();
    void setupSocketNotifier();
    void setupSocketFileWatcher();

    wl_display *display = nullptr;
    int fd = -1;
    QString socketName;
    QDir runtimeDir;
    QScopedPointer<QSocketNotifier> socketNotifier;
    QScopedPointer<QFileSystemWatcher> socketWatcher;
    bool serverDied = false;
    bool foreign = false;
    QMetaObject::Connection eventDispatcherConnection;
    int error = 0;
    static QVector<ConnectionThread *> connections;
    static QRecursiveMutex mutex;

private:
    ConnectionThread *q;
};

QVector<ConnectionThread *> ConnectionThread::Private::connections = QVector<ConnectionThread *>{};
QRecursiveMutex ConnectionThread::Private::mutex;

ConnectionThread::Private::Private(ConnectionThread *q)
    : socketName(QString::fromUtf8(qgetenv("WAYLAND_DISPLAY")))
    , runtimeDir(QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR")))
    , q(q)
{
    if (socketName.isEmpty()) {
        socketName = QStringLiteral("wayland-0");
    }
    {
        QMutexLocker lock(&mutex);
        connections << q;
    }
}

ConnectionThread::Private::~Private()
{
    {
        QMutexLocker lock(&mutex);
        connections.removeOne(q);
    }
    if (display && !foreign) {
        wl_display_flush(display);
        wl_display_disconnect(display);
    }
}

void ConnectionThread::Private::doInitConnection()
{
    if (fd != -1) {
        display = wl_display_connect_to_fd(fd);
    } else {
        display = wl_display_connect(socketName.toUtf8().constData());
    }
    if (!display) {
        qCWarning(KWAYLAND_CLIENT) << "Failed connecting to Wayland display";
        Q_EMIT q->failed();
        return;
    }
    if (fd != -1) {
        qCDebug(KWAYLAND_CLIENT) << "Connected to Wayland server over file descriptor:" << fd;
    } else {
        qCDebug(KWAYLAND_CLIENT) << "Connected to Wayland server at:" << socketName;
    }

    // setup socket notifier
    setupSocketNotifier();
    setupSocketFileWatcher();
    Q_EMIT q->connected();
}

void ConnectionThread::Private::setupSocketNotifier()
{
    const int fd = wl_display_get_fd(display);
    socketNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
    QObject::connect(socketNotifier.data(), &QSocketNotifier::activated, q, [this]() {
        if (!display) {
            return;
        }
        if (wl_display_dispatch(display) == -1) {
            error = wl_display_get_error(display);
            if (error != 0) {
                if (display) {
                    free(display);
                    display = nullptr;
                }
                Q_EMIT q->errorOccurred();
                return;
            }
        }
        Q_EMIT q->eventsRead();
    });
}

void ConnectionThread::Private::setupSocketFileWatcher()
{
    if (!runtimeDir.exists() || fd != -1) {
        return;
    }
    socketWatcher.reset(new QFileSystemWatcher);
    socketWatcher->addPath(runtimeDir.absoluteFilePath(socketName));
    QObject::connect(socketWatcher.data(), &QFileSystemWatcher::fileChanged, q, [this](const QString &file) {
        if (QFile::exists(file) || serverDied) {
            return;
        }
        qCWarning(KWAYLAND_CLIENT) << "Connection to server went away";
        serverDied = true;
        if (display) {
            free(display);
            display = nullptr;
        }
        socketNotifier.reset();

        // need a new filesystem watcher
        socketWatcher.reset(new QFileSystemWatcher);
        socketWatcher->addPath(runtimeDir.absolutePath());
        QObject::connect(socketWatcher.data(), &QFileSystemWatcher::directoryChanged, q, [this]() {
            if (!serverDied) {
                return;
            }
            if (runtimeDir.exists(socketName)) {
                qCDebug(KWAYLAND_CLIENT) << "Socket reappeared";
                socketWatcher.reset();
                serverDied = false;
                error = 0;
                q->initConnection();
            }
        });
        Q_EMIT q->connectionDied();
    });
}

ConnectionThread::ConnectionThread(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->eventDispatcherConnection = connect(
        QCoreApplication::eventDispatcher(),
        &QAbstractEventDispatcher::aboutToBlock,
        this,
        [this] {
            if (d->display) {
                wl_display_flush(d->display);
            }
        },
        Qt::DirectConnection);
}

ConnectionThread::ConnectionThread(wl_display *display, QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    d->display = display;
    d->foreign = true;
}

ConnectionThread::~ConnectionThread()
{
    disconnect(d->eventDispatcherConnection);
}

ConnectionThread *ConnectionThread::fromApplication(QObject *parent)
{
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    wl_display *display = reinterpret_cast<wl_display *>(native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
    if (!display) {
        return nullptr;
    }
    ConnectionThread *ct = new ConnectionThread(display, parent);
    connect(native, &QObject::destroyed, ct, &ConnectionThread::connectionDied);
    return ct;
}

void ConnectionThread::initConnection()
{
    QMetaObject::invokeMethod(this, "doInitConnection", Qt::QueuedConnection);
}

void ConnectionThread::doInitConnection()
{
    d->doInitConnection();
}

void ConnectionThread::setSocketName(const QString &socketName)
{
    if (d->display) {
        // already initialized
        return;
    }
    d->socketName = socketName;
}

void ConnectionThread::setSocketFd(int fd)
{
    if (d->display) {
        // already initialized
        return;
    }
    d->fd = fd;
}

wl_display *ConnectionThread::display()
{
    return d->display;
}

QString ConnectionThread::socketName() const
{
    return d->socketName;
}

void ConnectionThread::flush()
{
    if (!d->display) {
        return;
    }
    wl_display_flush(d->display);
}

void ConnectionThread::roundtrip()
{
    if (!d->display) {
        return;
    }
    if (d->foreign) {
        // try to perform roundtrip through the QPA plugin if it's supported
        if (QPlatformNativeInterface *native = qApp->platformNativeInterface()) {
            // in case the platform provides a dedicated roundtrip function use that install of wl_display_roundtrip
            QFunctionPointer roundtripFunction = native->platformFunction(QByteArrayLiteral("roundtrip"));
            if (roundtripFunction) {
                roundtripFunction();
                return;
            }
        }
    }
    wl_display_roundtrip(d->display);
}

bool ConnectionThread::hasError() const
{
    return d->error != 0;
}

int ConnectionThread::errorCode() const
{
    return d->error;
}

QVector<ConnectionThread *> ConnectionThread::connections()
{
    return Private::connections;
}

}
}
