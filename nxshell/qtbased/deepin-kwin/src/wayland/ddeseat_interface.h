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

#include "deepin-kwin_export.h"

struct wl_resource;

namespace KWaylandServer
{
class Display;
class DDEPointerInterface;
class DDEKeyboardInterface;
class DDETouchInterface;

class DDESeatInterfacePrivate;
class DDEPointerInterfacePrivate;
class DDETouchInterfacePrivate;

/** @class DDESeatInterface
 *
 *
 * @see DDESeatInterface
 * @since 5.5
 */
class KWIN_EXPORT DDESeatInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDESeatInterface(Display *display, QObject *parent = nullptr);
    virtual ~DDESeatInterface();

    static DDESeatInterface *get(wl_resource *native);

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
     * Emitted whenever a DDEPointerInterface got created.
     **/
    void ddePointerCreated(KWaylandServer::DDEPointerInterface*);
    void pointerPosChanged(const QPointF &pos);

    void ddeKeyboardCreated(KWaylandServer::DDEKeyboardInterface*);

    void ddeTouchCreated(KWaylandServer::DDETouchInterface*);

private:
    QScopedPointer<DDESeatInterfacePrivate> d;
    friend class DDESeatInterfacePrivate;
};

/**
 * @brief Resource for the dde_pointer interface.
 *
 * DDEPointerInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT DDEPointerInterface : public QObject
{
    Q_OBJECT
public:
    ~DDEPointerInterface() override;

    /**
     * @returns The DDESeatInterface which created this DDEPointerInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    /**
     * @returns The DDEPointerInterface for the @p native resource.
     * @since 5.5
     **/
    static DDEPointerInterface *get(wl_resource *native);

private:
    friend class DDESeatInterface;
    friend class DDESeatInterfacePrivate;
    friend class DDEPointerInterfacePrivate;

    explicit DDEPointerInterface(DDESeatInterface *seat);
    QScopedPointer<DDEPointerInterfacePrivate> d;

    void buttonPressed(quint32 button);
    void buttonReleased(quint32 button);
    void axis(Qt::Orientation orientation, qint32 delta);
    void sendMotion(const QPointF &position);
};

/**
 * @brief Resource for the dde_touch interface.
 *
 * DDEPointerInterface gets created by DDESeatInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT DDETouchInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~DDETouchInterface();

    /**
     * @returns The DDESeatInterface which created this DDETouchInterface.
     **/
    DDESeatInterface *ddeSeat() const;

    /**
     * @returns The DDETouchInterface for the @p native resource.
     * @since 5.5
     **/
    static DDETouchInterface *get(wl_resource *native);

private:
    friend class DDESeatInterface;
    friend class DDESeatInterfacePrivate;
    friend class DDETouchInterfacePrivate;

    void touchDown(qint32 id, const QPointF &pos);
    void touchMotion(qint32 id, const QPointF &pos);
    void touchUp(qint32 id);

    explicit DDETouchInterface(DDESeatInterface *ddeSeat);
    QScopedPointer<DDETouchInterfacePrivate> d;
};

}