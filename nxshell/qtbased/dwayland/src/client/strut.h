// Copyright 2020  Zhang Liang <zhanglianga@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#ifndef KWAYLAND_STRUT_H
#define KWAYLAND_STRUT_H

#include <QObject>
#include <DWayland/Client/kwaylandclient_export.h>

struct wl_surface;
struct com_deepin_kwin_strut;

namespace KWayland
{
namespace Client
{
class EventQueue;

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

    deepinKwinStrut& operator=(const deepinKwinStrut& rhs)
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
 * @short Wrapper for the com_deepin_kwin_strut interface.
 *
 * This class is a convenient wrapper for the com_deepin_kwin_strut interface.
 *
 *
 * The main purpose of this class is to set strut property.This property MUST
 * be set by the Client if the window is to reserve space at the edge of the screen.
 *
 **/
class KWAYLANDCLIENT_EXPORT Strut : public QObject
{
    Q_OBJECT
public:

    explicit Strut(QObject *parent = nullptr);
    virtual ~Strut();

    /**
     * Set the strut area.
     **/
    void setStrutPartial(wl_surface *surface,deepinKwinStrut& sStrut);
    /**
     * @returns @c true if managing a com_deepin_kwin_strut.
     **/
    bool isValid() const;
    /**
     * Setup this com_deepin_kwin_strut to manage the @p strut.
     * When using Registry::createStrut there is no need to call this
     * method.
     **/
    void setup(com_deepin_kwin_strut *strut);
    /**
     * Releases the com_deepin_kwin_strut interface.
     * After the interface has been released the strut instance is no
     * longer valid and can be setup with another com_deepin_kwin_strut interface.
     **/
    void release();
    /**
     * Destroys the data held by this strut.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_blur interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Blur gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Strut.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Strut.
     **/
    EventQueue *eventQueue();

    operator com_deepin_kwin_strut*();
    operator com_deepin_kwin_strut*() const;

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
     * This signal gets only emitted if the Strut got created by
     * Registry::createStrut
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

