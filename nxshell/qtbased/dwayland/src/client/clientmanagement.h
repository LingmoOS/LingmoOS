// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_CLIENT_MANAGEMENT_H
#define WAYLAND_CLIENT_MANAGEMENT_H

#include <QObject>
#include <QPointer>
#include <QSize>
#include <QVector>

#include <wayland-client-protocol.h>

#include <DWayland/Client/kwaylandclient_export.h>

struct com_deepin_client_management;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;

/**
 * @short Wrapper for the com_deepin_client_management interface.
 *
 * This class provides a convenient wrapper for the com_deepin_client_management interface.
 * Its main purpose is to hold the information about one ClientManagement.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an ClientManagement interface:
 * @code
 * ClientManagement *c = registry->createClientManagement(name, version);
 * @endcode
 *
 * This creates the ClientManagement and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ClientManagement *c = new ClientManagement;
 * c->setup(registry->bindClientManagement(name, version));
 * @endcode
 *
 * The ClientManagement can be used as a drop-in replacement for any com_deepin_client_management
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of ClientManagement are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the ClientManagement instance. By emitting changed
 * the ClientManagement indicates that all relevant information is available.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT ClientManagement : public QObject
{
    Q_OBJECT
public:

    explicit ClientManagement(QObject *parent = nullptr);
    virtual ~ClientManagement();

    struct WindowState {
        int32_t pid;
        int32_t windowId;
        char resourceName[256];
        struct Geometry {
            int32_t x;
            int32_t y;
            int32_t width;
            int32_t height;
        } geometry;
        bool isMinimized;
        bool isFullScreen;
        bool isActive;
        int32_t splitable;
        char uuid[256];
    };

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

    /**
     * Setup this Compositor to manage the @p clientManagement.
     * When using Registry::createClientManagement there is no need to call this
     * method.
     **/
    void setup(com_deepin_client_management *clientManagement);

    /**
     * @returns @c true if managing a com_deepin_client_management.
     **/
    bool isValid() const;
    operator com_deepin_client_management*();
    operator com_deepin_client_management*() const;
    com_deepin_client_management *clientManagement();

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    /**
    * Destroys the data hold by this ClientManagement.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * clientManagement gets destroyed.
    **/
    void destroy();

    const QVector <ClientManagement::WindowState> &getWindowStates() const;

    void getWindowCaption(int windowId, wl_buffer* buffer);

    void requestSplitWindow(const char *uuid, SplitType splitType);

Q_SIGNALS:
    /**
     * Emitted whenever window State changed.
     **/
    void windowStatesChanged();
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ClientManagement got created by
     * Registry::createClientManagement
     *
     * @since 5.5
     **/
    void removed();

    void captionWindowDone(int windowId, bool succeed);

    void splitStateChange(const char* uuid, int splitable);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
