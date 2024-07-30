/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "kwindowsystem.h"
#include "kwindowsystem_debug.h"
#include "kwindowsystem_dummy_p.h"
#include "pluginwrapper_p.h"

#include <config-kwindowsystem.h>

#if KWINDOWSYSTEM_HAVE_X11
#include "kstartupinfo.h"
#endif

#include <QGuiApplication>
#include <QMetaMethod>
#include <QPixmap>
#include <QPluginLoader>
#include <QTimer>
#include <QWindow>
#if KWINDOWSYSTEM_HAVE_X11
#include <private/qtx11extras_p.h>
#endif

// QPoint and QSize all have handy / operators which are useful for scaling, positions and sizes for high DPI support
// QRect does not, so we create one for internal purposes within this class
inline QRect operator/(const QRect &rectangle, qreal factor)
{
    return QRect(rectangle.topLeft() / factor, rectangle.size() / factor);
}

class KWindowSystemStaticContainer
{
public:
    KWindowSystemStaticContainer()
    {
        d.reset(KWindowSystemPluginWrapper::self().createWindowSystem());

        if (QCoreApplication::instance()) {
            kwm.moveToThread(QCoreApplication::instance()->thread());
        }
    }
    KWindowSystem kwm;
    std::unique_ptr<KWindowSystemPrivate> d;
};

Q_GLOBAL_STATIC(KWindowSystemStaticContainer, g_kwmInstanceContainer)

KWindowSystemPrivate::~KWindowSystemPrivate()
{
}

void KWindowSystemPrivateDummy::activateWindow(QWindow *win, long time)
{
    Q_UNUSED(win)
    Q_UNUSED(time)
}

bool KWindowSystemPrivateDummy::showingDesktop()
{
    return false;
}

void KWindowSystemPrivateDummy::setShowingDesktop(bool showing)
{
    Q_UNUSED(showing);
}

KWindowSystem *KWindowSystem::self()
{
    return &(g_kwmInstanceContainer()->kwm);
}

KWindowSystemPrivate *KWindowSystem::d_func()
{
    return g_kwmInstanceContainer()->d.get();
}

void KWindowSystem::activateWindow(QWindow *win, long time)
{
    Q_D(KWindowSystem);
    d->activateWindow(win, time);
}

void KWindowSystem::setMainWindow(QWindow *subWindow, WId mainWindowId)
{
    QWindow *mainWindow = QWindow::fromWinId(mainWindowId);
    if (mainWindow) { // foreign windows not supported on all platforms
        subWindow->setTransientParent(mainWindow);

        // mainWindow is not the child of any object, so make sure it gets deleted at some point
        connect(subWindow, &QObject::destroyed, mainWindow, &QObject::deleteLater);
    }
}

void KWindowSystem::setMainWindow(QWindow *subWindow, const QString &mainWindowId)
{
    Q_D(KWindowSystem);
    if (isPlatformWayland()) {
        if (auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(d)) {
            dv2->setMainWindow(subWindow, mainWindowId);
        }
    } else {
        bool ok = false;
        // base 0 means "C style" parsing with 0x for base 16, 0b for base 2, etc.
        WId wid = mainWindowId.toULongLong(&ok, 0);
        if (ok) {
            setMainWindow(subWindow, wid);
        } else {
            qCWarning(LOG_KWINDOWSYSTEM) << "Failed to convert" << mainWindowId << "to WId";
        }
    }
}

bool KWindowSystem::showingDesktop()
{
    Q_D(KWindowSystem);
    return d->showingDesktop();
}

void KWindowSystem::setShowingDesktop(bool showing)
{
    Q_D(KWindowSystem);
    return d->setShowingDesktop(showing);
}

static inline KWindowSystem::Platform initPlatform()
{
    auto platformName = QGuiApplication::platformName();
    if (platformName == QLatin1String("flatpak")) {
        // here we cannot know what is the actual windowing system, let's try it's env variable
        const auto flatpakPlatform = QString::fromLocal8Bit(qgetenv("QT_QPA_FLATPAK_PLATFORM"));
        if (!flatpakPlatform.isEmpty()) {
            platformName = flatpakPlatform;
        }
    }
#if KWINDOWSYSTEM_HAVE_X11
    if (platformName == QLatin1String("xcb")) {
        return KWindowSystem::Platform::X11;
    }
#endif
    if (platformName.startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        return KWindowSystem::Platform::Wayland;
    }
    return KWindowSystem::Platform::Unknown;
}

KWindowSystem::Platform KWindowSystem::platform()
{
    static Platform s_platform = initPlatform();
    return s_platform;
}

bool KWindowSystem::isPlatformX11()
{
    return platform() == Platform::X11;
}

bool KWindowSystem::isPlatformWayland()
{
    return platform() == Platform::Wayland;
}

void KWindowSystem::updateStartupId(QWindow *window)
{
    // clang-format off
    // TODO: move to a new KWindowSystemPrivate interface
#if KWINDOWSYSTEM_HAVE_X11
    if (isPlatformX11()) {
        const QByteArray startupId = QX11Info::nextStartupId();
        if (!startupId.isEmpty()) {
            KStartupInfo::setNewStartupId(window, startupId);
        }
    } else
#else
    Q_UNUSED(window);
#endif
    if (isPlatformWayland()) {
        const QString token = qEnvironmentVariable("XDG_ACTIVATION_TOKEN");
        if (!token.isEmpty()) {
            setCurrentXdgActivationToken(token);
            qunsetenv("XDG_ACTIVATION_TOKEN");
        }
    }
    // clang-format on
}

void KWindowSystem::setCurrentXdgActivationToken(const QString &token)
{
    Q_D(KWindowSystem);
    auto dv2 = dynamic_cast<KWindowSystemPrivateV2 *>(d);
    if (!dv2) {
        return;
    }
    dv2->setCurrentToken(token);
}

#include "moc_kwindowsystem.cpp"
