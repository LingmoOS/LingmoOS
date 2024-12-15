// Copyright 2022  luochaojiang <luochaojiang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>
#include <QPointF>

#include <DWayland/Server/kwaylandserver_export.h>

struct wl_resource;

namespace KWaylandServer
{
class Display;
class OCEANPointerInterface;
class OCEANKeyboardInterface;
class OCEANTouchInterface;

class OCEANSeatInterfacePrivate;
class OCEANPointerInterfacePrivate;
class OCEANTouchInterfacePrivate;

/** @class OCEANSeatInterface
 *
 *
 * @see OCEANSeatInterface
 * @since 5.5
 */
class KWAYLANDSERVER_EXPORT OCEANSeatInterface : public QObject
{
    Q_OBJECT
public:
    explicit OCEANSeatInterface(Display *display, QObject *parent = nullptr);
    virtual ~OCEANSeatInterface();

    static OCEANSeatInterface *get(wl_resource *native);

    void setPointerPos(const QPointF &pos);
    QPointF pointerPos() const;

    void pointerButtonPressed(quint32 button);
    void pointerButtonReleased(quint32 button);

    void pointerAxis(Qt::Orientation orientation, qint32 delta);

    void setTimestamp(quint32 time);
    void setTouchTimestamp(quint32 time);
    quint32 timestamp() const;
    quint32 touchtimestamp() const;

    void setKeymap(int fd, quint32 size);
    void keyPressed(quint32 key);
    void keyReleased(quint32 key);
    void updateKeyboardModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group);

    quint32 depressedModifiers() const;
    quint32 latchedModifiers() const;
    quint32 lockedModifiers() const;
    quint32 groupModifiers() const;
    quint32 lastModifiersSerial() const;

    void touchDown(qint32 id, const QPointF &pos);
    void touchMotion(qint32 id, const QPointF &pos);
    void touchUp(qint32 id);

    void setHasPointer(bool has);
    void setHasKeyboard(bool has);
    void setHasTouch(bool has);

Q_SIGNALS:
    /**
     * Emitted whenever a OCEANPointerInterface got created.
     **/
    void oceanPointerCreated(KWaylandServer::OCEANPointerInterface*);
    void pointerPosChanged(const QPointF &pos);

    void oceanKeyboardCreated(KWaylandServer::OCEANKeyboardInterface*);

    void oceanTouchCreated(KWaylandServer::OCEANTouchInterface*);

private:
    QScopedPointer<OCEANSeatInterfacePrivate> d;
    friend class OCEANSeatInterfacePrivate;
};

/**
 * @brief Resource for the ocean_pointer interface.
 *
 * OCEANPointerInterface gets created by OCEANSeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT OCEANPointerInterface : public QObject
{
    Q_OBJECT
public:
    ~OCEANPointerInterface() override;

    /**
     * @returns The OCEANSeatInterface which created this OCEANPointerInterface.
     **/
    OCEANSeatInterface *oceanSeat() const;

    /**
     * @returns The OCEANPointerInterface for the @p native resource.
     * @since 5.5
     **/
    static OCEANPointerInterface *get(wl_resource *native);

private:
    friend class OCEANSeatInterface;
    friend class OCEANSeatInterfacePrivate;
    friend class OCEANPointerInterfacePrivate;

    explicit OCEANPointerInterface(OCEANSeatInterface *seat, wl_resource *resource);
    QScopedPointer<OCEANPointerInterfacePrivate> d;

    void buttonPressed(quint32 button);
    void buttonReleased(quint32 button);
    void axis(Qt::Orientation orientation, qint32 delta);
    void sendMotion(const QPointF &position);
};

/**
 * @brief Resource for the ocean_touch interface.
 *
 * OCEANPointerInterface gets created by OCEANSeatInterface.
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT OCEANTouchInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~OCEANTouchInterface();

    /**
     * @returns The OCEANSeatInterface which created this OCEANTouchInterface.
     **/
    OCEANSeatInterface *oceanSeat() const;

    /**
     * @returns The OCEANTouchInterface for the @p native resource.
     * @since 5.5
     **/
    static OCEANTouchInterface *get(wl_resource *native);

private:
    friend class OCEANSeatInterface;
    friend class OCEANSeatInterfacePrivate;
    friend class OCEANTouchInterfacePrivate;

    void touchDown(qint32 id, const QPointF &pos);
    void touchMotion(qint32 id, const QPointF &pos);
    void touchUp(qint32 id);

    explicit OCEANTouchInterface(OCEANSeatInterface *oceanSeat, wl_resource *resource);
    QScopedPointer<OCEANTouchInterfacePrivate> d;
};

}