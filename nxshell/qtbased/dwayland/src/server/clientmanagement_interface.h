// Copyright 2020  wugang <wugang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H
#define WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QVector>
#include <QImage>

#include "surface_interface.h"
#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class ClientManagementInterfacePrivate;

/** @class ClientManagementInterface
 *
 *
 * @see ClientManagementInterface
 * @since 5.5
 */
class KWAYLANDSERVER_EXPORT ClientManagementInterface : public QObject
{
    Q_OBJECT

public:
    explicit ClientManagementInterface(Display *display, QObject *parent = nullptr);
    ~ClientManagementInterface() override;

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

    static ClientManagementInterface *get(wl_resource *native);
    void setWindowStates(QList<WindowState*> &windowStates);

    void sendWindowCaptionImage(int windowId, wl_resource *buffer, QImage image);
    void sendWindowCaption(int windowId, wl_resource *buffer, SurfaceInterface* surface);
    void sendSplitChange(const QString& uuid, int splitable);

Q_SIGNALS:
    void windowStatesRequest();
    void windowStatesChanged();

    void captureWindowImageRequest(int windowId, wl_resource *buffer);
    void splitWindowRequest(QString uuid, int splitType);

private:
    QScopedPointer<ClientManagementInterfacePrivate> d;
};

}

#endif
