/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "windowsystem.h"
#include "logging.h"
#include "surfacehelper.h"
#include "waylandxdgactivationv1_p.h"
#include "waylandxdgforeignv2_p.h"

#include <KWaylandExtras>
#include <KWindowSystem>

#include "qwayland-plasma-window-management.h"
#include <QEvent>
#include <QGuiApplication>
#include <QPixmap>
#include <QPoint>
#include <QString>
#include <QTimer>
#include <QWaylandClientExtensionTemplate>
#include <QWindow>
#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow_p.h>

constexpr const char *c_kdeXdgForeignExportedProperty("_kde_xdg_foreign_exported_v2");
constexpr const char *c_kdeXdgForeignImportedProperty("_kde_xdg_foreign_imported_v2");
constexpr const char *c_kdeXdgForeignPendingHandleProperty("_kde_xdg_foreign_pending_handle");

class WindowManagement : public QWaylandClientExtensionTemplate<WindowManagement>, public QtWayland::org_kde_plasma_window_management
{
public:
    WindowManagement()
        : QWaylandClientExtensionTemplate<WindowManagement>(17)
    {
    }

    void org_kde_plasma_window_management_show_desktop_changed(uint32_t state) override
    {
        showingDesktop = state == show_desktop_enabled;
        KWindowSystem::self()->showingDesktopChanged(showingDesktop);
    }

    bool showingDesktop = false;
};

WindowSystem::WindowSystem()
    : QObject()
    , KWindowSystemPrivateV2()
    , m_lastToken(qEnvironmentVariable("XDG_ACTIVATION_TOKEN"))
{
    m_windowManagement = new WindowManagement;
}

WindowSystem::~WindowSystem()
{
    delete m_windowManagement;
}

void WindowSystem::activateWindow(QWindow *win, long int time)
{
    Q_UNUSED(time);
    auto s = surfaceForWindow(win);
    if (!s) {
        return;
    }
    WaylandXdgActivationV1 *activation = WaylandXdgActivationV1::self();
    if (!activation->isActive()) {
        return;
    }
    activation->activate(m_lastToken, s);
}

void WindowSystem::requestToken(QWindow *window, uint32_t serial, const QString &app_id)
{
    if (window) {
        window->create();
    }
    wl_surface *wlSurface = surfaceForWindow(window);

    WaylandXdgActivationV1 *activation = WaylandXdgActivationV1::self();
    if (!activation->isActive()) {
        // Ensure that xdgActivationTokenArrived is always emitted asynchronously
        QTimer::singleShot(0, [serial] {
            Q_EMIT KWaylandExtras::self()->xdgActivationTokenArrived(serial, {});
        });
        return;
    }

    auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    auto seat = waylandApp ? waylandApp->lastInputSeat() : nullptr;
    auto tokenReq = activation->requestXdgActivationToken(seat, wlSurface, serial, app_id);
    connect(tokenReq, &WaylandXdgActivationTokenV1::failed, KWindowSystem::self(), [serial, app_id]() {
        Q_EMIT KWaylandExtras::self()->xdgActivationTokenArrived(serial, {});
    });
    connect(tokenReq, &WaylandXdgActivationTokenV1::done, KWindowSystem::self(), [serial](const QString &token) {
        Q_EMIT KWaylandExtras::self()->xdgActivationTokenArrived(serial, token);
    });
}

void WindowSystem::setCurrentToken(const QString &token)
{
    m_lastToken = token;
}

quint32 WindowSystem::lastInputSerial(QWindow *window)
{
    Q_UNUSED(window)
    if (auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>()) {
        return waylandApp->lastInputSerial();
    }
    return 0;
}

void WindowSystem::setShowingDesktop(bool showing)
{
    if (!m_windowManagement->isActive()) {
        return;
    }
    m_windowManagement->show_desktop(showing ? WindowManagement::show_desktop_enabled : WindowManagement::show_desktop_disabled);
}

bool WindowSystem::showingDesktop()
{
    if (!m_windowManagement->isActive()) {
        return false;
    }
    return m_windowManagement->showingDesktop;
}

void WindowSystem::exportWindow(QWindow *window)
{
    auto emitHandle = [window](const QString &handle) {
        // Ensure that windowExported is always emitted asynchronously.
        QMetaObject::invokeMethod(
            window,
            [window, handle] {
                Q_EMIT KWaylandExtras::self()->windowExported(window, handle);
            },
            Qt::QueuedConnection);
    };

    if (!window) {
        return;
    }

    window->create();

    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow) {
        emitHandle({});
        return;
    }

    auto &exporter = WaylandXdgForeignExporterV2::self();
    if (!exporter.isActive()) {
        emitHandle({});
        return;
    }

    // We want to use QObject::property(char*) and use dynamic properties on the object rather than
    // call QWaylandWindow::property(QString) and send it around.
    WaylandXdgForeignExportedV2 *exported = waylandWindow->property(c_kdeXdgForeignExportedProperty).value<WaylandXdgForeignExportedV2 *>();
    if (!exported) {
        exported = exporter.exportToplevel(surfaceForWindow(window));
        exported->setParent(waylandWindow);

        waylandWindow->setProperty(c_kdeXdgForeignExportedProperty, QVariant::fromValue(exported));
        connect(exported, &QObject::destroyed, waylandWindow, [waylandWindow] {
            waylandWindow->setProperty(c_kdeXdgForeignExportedProperty, QVariant());
        });

        connect(exported, &WaylandXdgForeignExportedV2::handleReceived, window, [window](const QString &handle) {
            Q_EMIT KWaylandExtras::self()->windowExported(window, handle);
        });
    }

    if (!exported->handle().isEmpty()) {
        emitHandle(exported->handle());
    }
}

void WindowSystem::unexportWindow(QWindow *window)
{
    auto waylandWindow = window ? window->nativeInterface<QNativeInterface::Private::QWaylandWindow>() : nullptr;
    if (!waylandWindow) {
        return;
    }

    WaylandXdgForeignExportedV2 *exported = waylandWindow->property(c_kdeXdgForeignExportedProperty).value<WaylandXdgForeignExportedV2 *>();
    delete exported;
    Q_ASSERT(!waylandWindow->property(c_kdeXdgForeignExportedProperty).isValid());
}

void WindowSystem::setMainWindow(QWindow *window, const QString &handle)
{
    if (!window) {
        return;
    }

    window->create();
    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow) {
        return;
    }

    // We want to use QObject::property(char*) and use dynamic properties on the object rather than
    // call QWaylandWindow::property(QString) and send it around.
    auto *imported = waylandWindow->property(c_kdeXdgForeignImportedProperty).value<WaylandXdgForeignImportedV2 *>();
    // Window already parented with a different handle? Delete imported so we import the new one later.
    if (imported && imported->handle() != handle) {
        delete imported;
        imported = nullptr;
        Q_ASSERT(!waylandWindow->property(c_kdeXdgForeignImportedProperty).isValid());
    }

    // Don't bother.
    if (handle.isEmpty()) {
        return;
    }

    if (window->isExposed()) {
        doSetMainWindow(window, handle);
    } else {
        // We can only import an XDG toplevel. QtWayland currently has no proper signal
        // for shell surface creation. wlSurfaceCreated() is too early.
        // Instead, we wait for it being exposed and then set its parent.
        window->setProperty(c_kdeXdgForeignPendingHandleProperty, handle);
        window->installEventFilter(this);
    }
}

bool WindowSystem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        auto *window = static_cast<QWindow *>(watched);
        if (window->isExposed()) {
            const QString handle = window->property(c_kdeXdgForeignPendingHandleProperty).toString();
            if (!handle.isEmpty()) {
                doSetMainWindow(window, handle);
                window->setProperty(c_kdeXdgForeignPendingHandleProperty, QVariant());
            }

            window->removeEventFilter(this);
        }
    }

    return QObject::eventFilter(watched, event);
}

void WindowSystem::doSetMainWindow(QWindow *window, const QString &handle)
{
    Q_ASSERT(window);
    Q_ASSERT(!handle.isEmpty());

    auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
    if (!waylandWindow) {
        return;
    }

    auto &importer = WaylandXdgForeignImporterV2::self();
    if (!importer.isActive()) {
        return;
    }

    Q_ASSERT(!waylandWindow->property(c_kdeXdgForeignImportedProperty).isValid());

    WaylandXdgForeignImportedV2 *imported = importer.importToplevel(handle);
    imported->set_parent_of(surfaceForWindow(window)); // foreign parent.
    imported->setParent(waylandWindow); // memory owner.

    waylandWindow->setProperty(c_kdeXdgForeignImportedProperty, QVariant::fromValue(imported));
    connect(imported, &QObject::destroyed, waylandWindow, [waylandWindow] {
        waylandWindow->setProperty(c_kdeXdgForeignImportedProperty, QVariant());
    });
}

#include "moc_windowsystem.cpp"
