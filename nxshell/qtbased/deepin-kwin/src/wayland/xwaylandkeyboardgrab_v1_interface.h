/*
    SPDX-FileCopyrightText: 2022 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "deepin-kwin_export.h"

#include <QObject>

#include <memory>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class SeatInterface;
class SurfaceInterface;
class XWaylandKeyboardGrabV1InterfacePrivate;
class XWaylandKeyboardGrabManagerV1InterfacePrivate;
class XWaylandKeyboardGrabV1Interface;

class KWIN_EXPORT XWaylandKeyboardGrabManagerV1Interface : public QObject
{
    Q_OBJECT
public:
    explicit XWaylandKeyboardGrabManagerV1Interface(Display *display, QObject *parent = nullptr);
    ~XWaylandKeyboardGrabManagerV1Interface() override;
    bool hasGrab(SurfaceInterface *surface, SeatInterface *seat) const;

Q_SIGNALS:
    void XWaylandKeyboardGrabV1Created(XWaylandKeyboardGrabV1Interface *grab);
    void XWaylandKeyboardGrabV1Destroyed();

private:
    friend class XWaylandKeyboardGrabManagerV1InterfacePrivate;
    std::unique_ptr<XWaylandKeyboardGrabManagerV1InterfacePrivate> d;
};

class KWIN_EXPORT XWaylandKeyboardGrabV1Interface : public QObject
{
    Q_OBJECT
public:
    ~XWaylandKeyboardGrabV1Interface() override;

    SurfaceInterface *surface() const;
    SeatInterface *seat() const;

private:
    friend class XWaylandKeyboardGrabManagerV1InterfacePrivate;
    XWaylandKeyboardGrabV1Interface(wl_resource *resource, SurfaceInterface *surface, SeatInterface *seat);
    std::unique_ptr<XWaylandKeyboardGrabV1InterfacePrivate> d;
};

}
