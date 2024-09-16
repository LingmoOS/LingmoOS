// Copyright 2015  Marco Martin <mart@kde.org>
// Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef KWAYLAND_SERVER_STRUT_INTERFACE_H
#define KWAYLAND_SERVER_STRUT_INTERFACE_H

#include <QObject>
#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class wl_surface;
class SurfaceInterface;
class StrutInterfacePrivate;

struct deepinKwinStrut
{
    int left;
    int right;
    int top;
    int bottom;
    int left_start_y;
    int left_end_y;
    int right_start_y;
    int right_end_y;
    int top_start_x;
    int top_end_x;
    int bottom_start_x;
    int bottom_end_x;

    deepinKwinStrut():left(0),
                      right(0),
                      top(0),
                      bottom(0),
                      left_start_y(0),
                      left_end_y(0),
                      right_start_y(0),
                      right_end_y(0),
                      top_start_x(0),
                      top_end_x(0),
                      bottom_start_x(0),
                      bottom_end_x(0)
    {};

    deepinKwinStrut(int32_t left,
                    int32_t right,
                    int32_t top,
                    int32_t bottom,
                    int32_t left_start_y,
                    int32_t left_end_y,
                    int32_t right_start_y,
                    int32_t right_end_y,
                    int32_t top_start_x,
                    int32_t top_end_x,
                    int32_t bottom_start_x,
                    int32_t bottom_end_x):
                    left(left),
                    right(right),
                    top(top),
                    bottom(bottom),
                    left_start_y(left_start_y),
                    left_end_y(left_end_y),
                    right_start_y(right_start_y),
                    right_end_y(right_end_y),
                    top_start_x(top_start_x),
                    top_end_x(top_end_x),
                    bottom_start_x(bottom_start_x),
                    bottom_end_x(bottom_end_x)
    {};

    deepinKwinStrut &operator=(const deepinKwinStrut &rhs)
    {
        if(this == &rhs) {
            return *this;
        }
        left = rhs.left;
        right = rhs.right;
        top = rhs.top;
        bottom = rhs.bottom;
        left_start_y = rhs.left_start_y;
        left_end_y = rhs.left_end_y;
        right_start_y = rhs.right_start_y;
        right_end_y = rhs.right_end_y;
        top_start_x = rhs.top_start_x;
        top_end_x = rhs.top_end_x;
        bottom_start_x = rhs.bottom_start_x;
        bottom_end_x = rhs.bottom_end_x;

        return *this;
    };
};

/**
 * @brief Represents the Global for com_deepin_kwin_strut interface.
 *
 * This class set strut area for docking area, a taskbar or a panel.
 *
 * @see xxxx
 * @see xxxx
 * @since 5.5
 **/
class KWAYLANDSERVER_EXPORT StrutInterface : public QObject
{
    Q_OBJECT
public:
    explicit StrutInterface(Display *display, QObject *parent = nullptr);
    ~StrutInterface() override;

    static StrutInterface *get(wl_resource *native);

Q_SIGNALS:
    /**
     * Emitted whenever a PlasmaShellSurfaceInterface got created.
     **/
    void setStrut(SurfaceInterface *, struct deepinKwinStrut &);

private:
    QScopedPointer<StrutInterfacePrivate> d;
};

}
#endif
