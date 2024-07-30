/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H
#define XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H

#include "waylandintegration.h"

#include <QDateTime>
#include <QList>
#include <QMap>
#include <QWaylandClientExtension>

#include "qwayland-fake-input.h"

class Screencasting;
class ScreencastingStream;

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class EventQueue;
class Registry;
class PlasmaWindow;
class PlasmaWindowManagement;
class RemoteBuffer;
class XdgImporter;
}
}

class QWindow;

namespace WaylandIntegration
{

class FakeInput : public QWaylandClientExtensionTemplate<FakeInput>, public QtWayland::org_kde_kwin_fake_input
{
public:
    FakeInput();
    ~FakeInput() override;
};

class WaylandIntegrationPrivate : public WaylandIntegration::WaylandIntegration
{
    Q_OBJECT
public:
    WaylandIntegrationPrivate();
    ~WaylandIntegrationPrivate() override;

    void initWayland();

    KWayland::Client::PlasmaWindowManagement *plasmaWindowManagement();

private:
    bool m_registryInitialized = false;

    KWayland::Client::Registry *m_registry = nullptr;
    KWayland::Client::PlasmaWindowManagement *m_windowManagement = nullptr;
    KWayland::Client::XdgImporter *m_xdgImporter = nullptr;

public:
    void authenticate();

    bool isStreamingEnabled() const;
    bool isStreamingAvailable() const;

    void acquireStreamingInput(bool acquire);

    Stream startStreamingOutput(QScreen *screen, Screencasting::CursorMode mode);
    Stream startStreamingWindow(const QMap<int, QVariant> &win, Screencasting::CursorMode mode);
    Stream startStreamingWorkspace(Screencasting::CursorMode mode);
    Stream startStreamingRegion(const QRect region, Screencasting::CursorMode mode);
    Stream startStreamingVirtualOutput(const QString &name, const QSize &size, Screencasting::CursorMode mode);
    void stopStreaming(uint32_t nodeid);

    void requestPointerButtonPress(quint32 linuxButton);
    void requestPointerButtonRelease(quint32 linuxButton);
    void requestPointerMotion(const QSizeF &delta);
    void requestPointerMotionAbsolute(uint stream, const QPointF &pos);
    void requestPointerAxis(qreal x, qreal y);
    void requestPointerAxisDiscrete(Qt::Orientation axis, qreal delta);
    void requestKeyboardKeycode(int keycode, bool state);
    void requestKeyboardKeysym(int keysym, bool state);
    void requestTouchDown(quint32 touchPoint, const QPointF &pos);
    void requestTouchMotion(quint32 touchPoint, const QPointF &pos);
    void requestTouchUp(quint32 touchPoint);

    void setParentWindow(QWindow *window, const QString &parentHandle);

private:
    Stream startStreaming(ScreencastingStream *stream, const QVariantMap &streamOptions);
    bool eventFilter(QObject *watched, QEvent *event) override;

    uint m_streamInput = 0;
    bool m_waylandAuthenticationRequested = false;

    QDateTime m_lastFrameTime;
    QList<Stream> m_streams;

    std::unique_ptr<FakeInput> m_fakeInput;
    Screencasting *m_screencasting = nullptr;
};

}

#endif // XDG_DESKTOP_PORTAL_KDE_WAYLAND_INTEGRATION_P_H
