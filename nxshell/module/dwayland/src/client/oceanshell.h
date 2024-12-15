// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_OCEAN_SHELL_H
#define WAYLAND_OCEAN_SHELL_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QRect>
#include <QVector>

#include <DWayland/Client/kwaylandclient_export.h>

struct ocean_shell;
struct ocean_shell_surface;
struct wl_surface;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class OCEANShellSurface;

/**
 * @short Wrapper for the ocean_shell interface.
 *
 * This class provides a convenient wrapper for the ocean_shell interface.
 * Its main purpose is to hold the information about one OCEANShell.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an OCEANShell interface:
 * @code
 * OCEANShell *c = registry->createOCEANShell(name, version);
 * @endcode
 *
 * This creates the OCEANShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * OCEANShell *c = new OCEANShell;
 * c->setup(registry->bindOCEANShell(name, version));
 * @endcode
 *
 * The OCEANShell can be used as a drop-in replacement for any ocean_shell
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of OCEANShell are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the OCEANShell instance. By emitting changed
 * the OCEANShell indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT OCEANShell : public QObject
{
    Q_OBJECT
public:

    explicit OCEANShell(QObject *parent = nullptr);
    virtual ~OCEANShell();

    /**
     * Setup this Compositor to manage the @p OCEANShell.
     * When using Registry::createOCEANShell there is no need to call this
     * method.
     **/
    void setup(ocean_shell *OCEANShell);

    /**
     * @returns @c true if managing a ocean_shell.
     **/
    bool isValid() const;
    operator ocean_shell*();
    operator ocean_shell*() const;
    ocean_shell *oceanShell();

    /**
     * Creates a OCEANShellSurface for the given @p surface and sets it up.
     *
     * If a OCEANShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The native surface to create the OCEANShellSurface for
     * @param parent The parent to use for the OCEANShellSurface
     * @returns created OCEANShellSurface
     **/
    OCEANShellSurface *createShellSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a OCEANShellSurface for the given @p surface and sets it up.
     *
     * If a OCEANShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The Surface to create the OCEANShellSurface for
     * @param parent The parent to use for the OCEANShellSurface
     * @returns created OCEANShellSurface
     **/
    OCEANShellSurface *createShellSurface(Surface *surface, QObject *parent = nullptr);

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    void release();

    /**
    * Destroys the data hold by this OCEANShell.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * OCEANShell gets destroyed.
    **/
    void destroy();

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OCEANShell got created by
     * Registry::createOCEANShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};


class KWAYLANDCLIENT_EXPORT OCEANShellSurface : public QObject
{
    Q_OBJECT
public:

    explicit OCEANShellSurface(QObject *parent = nullptr);
    virtual ~OCEANShellSurface();

    void setup(ocean_shell_surface *oceanShellSuface);

    bool isValid() const;
    operator ocean_shell_surface*();
    operator ocean_shell_surface*() const;

    void release();
    void destroy();

    static OCEANShellSurface *get(wl_surface *surf);
    static OCEANShellSurface *get(Surface *surf);

    bool isActive() const;
    bool isFullscreen() const;
    bool isKeepAbove() const;
    bool isKeepBelow() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isCloseable() const;
    bool isMaximizeable() const;
    bool isMinimizeable() const;
    bool isFullscreenable() const;
    bool isMovable() const;
    bool isResizable() const;
    bool isAcceptFocus() const;
    bool isModal() const;
    bool isOnAllDesktops() const;
    bool isSplitable() const;
    int getSplitable() const;

    void requestActivate();
    void requestKeepAbove(bool set);
    void requestKeepBelow(bool set);
    void requestMinimized(bool set);
    void requestMaximized(bool set);
    void requestAcceptFocus(bool set);
    void requestModal(bool set);
    void requestMinizeable(bool set);
    void requestMaximizeable(bool set);
    void requestResizable(bool set);
    void requestOnAllDesktops(bool set);

    void requestGeometry() const;
    void requestActive() const;
    QRect getGeometry() const;

    void requestNoTitleBarProperty(qint32 value);
    void requestWindowRadiusProperty(QPointF windowRadius);

    enum class SplitType {
        None        = 0,
        Left        = 1 << 0,
        Right       = 1 << 1,
        Top         = 1 << 2,
        Bottom      = 1 << 3,
        LeftTop     = Left | Top,
        RightTop    = Right | Top,
        LeftBottom  = Left | Bottom,
        RightBottom = Right | Bottom,
    };
    enum class SplitMode {
        Two         = 1 << 0,
        Three       = 1 << 1,
        Four        = 1 << 2,
    };

    void requestSplitWindow(SplitType splitType, SplitMode mode = SplitMode::Two);
Q_SIGNALS:
    void geometryChanged(const QRect &geom);
    void activeChanged();
    void fullscreenChanged();
    void keepAboveChanged();
    void keepBelowChanged();
    void minimizedChanged();
    void maximizedChanged();
    void closeableChanged();
    void minimizeableChanged();
    void maximizeableChanged();
    void fullscreenableChanged();
    void movableChanged();
    void resizableChanged();
    void acceptFocusChanged();
    void modalityChanged();
    void onAllDesktopsChanged();

private:
    friend class OCEANShell;
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
