// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>

#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;
class QRect;

namespace KWaylandServer
{
class Display;
class SurfaceInterface;

class DDEShellInterface;
class DDEShellInterfacePrivate;

class DDEShellSurfaceInterface;
class DDEShellSurfaceInterfacePrivate;

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
/** @class DDEShellInterface
 *
 *
 * @see DDEShellInterface
 * @since 5.5
 */
class KWAYLANDSERVER_EXPORT DDEShellInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDEShellInterface(Display *display, QObject *parent);
    virtual ~DDEShellInterface();

Q_SIGNALS:
    void shellSurfaceCreated(DDEShellSurfaceInterface*);

private:
    QScopedPointer<DDEShellInterfacePrivate> d;
};

class KWAYLANDSERVER_EXPORT DDEShellSurfaceInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~DDEShellSurfaceInterface();

    DDEShellInterface *ddeShell() const;
    SurfaceInterface *surface() const;

    static DDEShellSurfaceInterface *get(wl_resource *native);
    static DDEShellSurfaceInterface *get(SurfaceInterface *surface);

    void sendGeometry(const QRect &geom);
    void sendSplitable(int splitable);

    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setMovable(bool set);
    void setResizable(bool set);
    void setAcceptFocus(bool set);
    void setModal(bool set);

Q_SIGNALS:
    void activationRequested();
    void activeRequested(bool set);
    void minimizedRequested(bool set);
    void maximizedRequested(bool set);
    void fullscreenRequested(bool set);
    void keepAboveRequested(bool set);
    void keepBelowRequested(bool set);
    void onAllDesktopsRequested(bool set);
    void closeableRequested(bool set);
    void minimizeableRequested(bool set);
    void maximizeableRequested(bool set);
    void fullscreenableRequested(bool set);
    void movableRequested(bool set);
    void resizableRequested(bool set);
    void acceptFocusRequested(bool set);
    void modalityRequested(bool set);

    void noTitleBarPropertyRequested(qint32 value);
    void windowRadiusPropertyRequested(QPointF windowRadius);
    void nonStandardWindowPropertyRequested(const QMap<QString, QVariant> &mapNonStandardWindowPropertyData);
    void splitWindowRequested(SplitType splitType, int splitMode);
private:
    friend class DDEShellInterfacePrivate;
    explicit DDEShellSurfaceInterface(SurfaceInterface *surface, wl_resource *resource);
    QScopedPointer<DDEShellSurfaceInterfacePrivate> d;
};

}
