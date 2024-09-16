/********************************************************************
Copyright 2020  wugang <wugang@uniontech.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H
#define WAYLAND_SERVER_CLIENT_MANAGEMENT_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QVector>
#include <QImage>

#include "surface_interface.h"
#include "deepin-kwin_export.h"

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
class KWIN_EXPORT ClientManagementInterface : public QObject
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

    void sendWindowFromPoint(uint32_t wid);
    void sendAllWindowId(const QList<uint32_t> &id_list);
    void sendSpecificWindowState(const WindowState &state);

Q_SIGNALS:
    void windowStatesRequest();
    void windowStatesChanged();

    void windowFromPointRequest();
    void showSplitMenuRequest(const QRect &botton_rect, uint32_t wid);
    void hideSplitMenuRequest(bool delay);
    void allWindowIdRequest();
    void specificWindowStateRequest(uint32_t wid);

private:
    QScopedPointer<ClientManagementInterfacePrivate> d;
};

}

#endif
