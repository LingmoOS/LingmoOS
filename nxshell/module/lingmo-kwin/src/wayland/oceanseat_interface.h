/********************************************************************
Copyright 2022  luochaojiang <luochaojiang@uniontech.com>

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
#pragma once

#include <QObject>
#include <QPointF>

#include "lingmo-kwin_export.h"

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
class KWIN_EXPORT OCEANSeatInterface : public QObject
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
class KWIN_EXPORT OCEANPointerInterface : public QObject
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

    explicit OCEANPointerInterface(OCEANSeatInterface *seat);
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
class KWIN_EXPORT OCEANTouchInterface : public QObject
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

    explicit OCEANTouchInterface(OCEANSeatInterface *oceanSeat);
    QScopedPointer<OCEANTouchInterfacePrivate> d;
};

}