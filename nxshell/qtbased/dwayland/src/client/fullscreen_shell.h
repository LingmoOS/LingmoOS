/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_FULLSCREEN_SHELL_H
#define WAYLAND_FULLSCREEN_SHELL_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct _wl_fullscreen_shell;
struct wl_output;
struct wl_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class Output;

/**
 * @short Wrapper for the _wl_fullscreen_shell interface.
 *
 * This class provides a convenient wrapper for the _wl_fullscreen_shell interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the FullscreenShell interface:
 * @code
 * FullscreenShell *f = registry->createFullscreenShell(name, version);
 * @endcode
 *
 * This creates the FullscreenShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * FullscreenShell *f = new FullscreenShell;
 * f->setup(registry->bindFullscreenShell(name, version));
 * @endcode
 *
 * The FullscreenShell can be used as a drop-in replacement for any _wl_fullscreen_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT FullscreenShell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool capabilityArbitraryModes READ hasCapabilityArbitraryModes NOTIFY capabilityArbitraryModesChanged)
    Q_PROPERTY(bool capabilityCursorPlane READ hasCapabilityCursorPlane NOTIFY capabilityCursorPlaneChanged)
public:
    explicit FullscreenShell(QObject *parent = nullptr);
    ~FullscreenShell() override;

    bool isValid() const;
    void release();
    void destroy();
    bool hasCapabilityArbitraryModes() const;
    bool hasCapabilityCursorPlane() const;
    void setup(_wl_fullscreen_shell *shell);
    void present(wl_surface *surface, wl_output *output);
    void present(Surface *surface, Output *output);

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

Q_SIGNALS:
    void capabilityArbitraryModesChanged(bool);
    void capabilityCursorPlaneChanged(bool);

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createFullscreenShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
