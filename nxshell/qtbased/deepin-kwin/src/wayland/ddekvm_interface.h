/********************************************************************
Copyright 2024  xinbo wang <wangxinbo@uniontech.com>

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
class DDEKvmPointerInterface;
class DDEKvmKeyboardInterface;

class DDEKvmInterfacePrivate;
class DDEKvmPointerInterfacePrivate;
class DDEKvmKeyboardInterfacePrivate;

/** @class DDEKvmInterface
 *
 *
 * @see DDEKvmInterface
 * @since 5.5
 */
class KWIN_EXPORT DDEKvmInterface : public QObject
{
    Q_OBJECT
public:
    explicit DDEKvmInterface(Display *display, QObject *parent = nullptr);
    virtual ~DDEKvmInterface();

    static DDEKvmInterface *get(wl_resource *native);

    QPointF pointerPos() const;

    void pointerMotion(const QPointF &pos);
    void pointerButton(quint32 button, quint32 state, quint32 serial, const QPointF &position);
    void pointerAxis(Qt::Orientation orientation, qint32 delta);

    void updateKey(quint32 key, quint32 serial, quint32 state);
    void updateModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

    void setKvmPointerTimestamp(quint32 time);
    void setkvmKeyboardTimestamp(quint32 time);
    quint32 kvmPointerTimestamp() const;
    quint32 kvmKeyboardTimestamp() const;

Q_SIGNALS:
    /**
     * Emitted whenever a DDEPointerInterface got created.
     **/
    void ddeKvmPointerCreated(KWaylandServer::DDEKvmPointerInterface*);
    void ddeKvmKeyboardCreated(KWaylandServer::DDEKvmKeyboardInterface*);

    void kvmInterfaceEnablePointerRequested(quint32 is_enable);
    void kvmInterfaceEnableCursorRequested(quint32 is_enable);
    void kvmInterfaceSetCursorPosRequested(double x, double y);
    void kvmInterfaceEnableKeyboardRequested(quint32 is_enable);

private:
    QScopedPointer<DDEKvmInterfacePrivate> d;
    friend class DDEKvmInterfacePrivate;
};

/**
 * @brief Resource for the dde_kvm_pointer interface.
 *
 * DDEKvmPointerInterface gets created by DDEKvmInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT DDEKvmPointerInterface : public QObject
{
    Q_OBJECT
public:
    ~DDEKvmPointerInterface() override;

    /**
     * @returns The DDEKvmInterface which created this DDEKvmPointerInterface.
     **/
    DDEKvmInterface *ddeKvm() const;

    /**
     * @returns The DDEPointerInterface for the @p native resource.
     * @since 5.5
     **/
    static DDEKvmPointerInterface *get(wl_resource *native);

Q_SIGNALS:
    void kvmEnablePointerRequested(quint32 is_enable);
    void kvmEnableCursorRequested(quint32 is_enable);
    void kvmSetCursorPosRequested(double x, double y);

private:
    friend class DDEKvmInterface;
    friend class DDEKvmInterfacePrivate;
    friend class DDEKvmPointerInterfacePrivate;

    explicit DDEKvmPointerInterface(DDEKvmInterface *kvm);
    QScopedPointer<DDEKvmPointerInterfacePrivate> d;

    void sendButton(quint32 button, quint32 state, quint32 serial, const QPointF &position);
    void sendAxis(Qt::Orientation orientation, qreal delta);
    void sendMotion(const QPointF &position);
};

/**
 * @brief Resource for the dde_kvm_keyboard interface.
 *
 * DDEKvmKeyboardInterface gets created by DDEKvmInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT DDEKvmKeyboardInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~DDEKvmKeyboardInterface();

    /**
     * @returns The DDEKvmInterface which created this DDEKvmKeyboardInterface.
     **/
    DDEKvmInterface *ddeKvm() const;

    /**
     * @returns The DDEKvmKeyboardInterface for the @p native resource.
     * @since 5.5
     **/
    static DDEKvmKeyboardInterface *get(wl_resource *native);

Q_SIGNALS:
    void kvmEnableKeyboardRequested(uint32_t is_enable);

private:
    friend class DDEKvmInterface;
    friend class DDEKvmInterfacePrivate;
    friend class DDEKvmKeyboardInterfacePrivate;

    explicit DDEKvmKeyboardInterface(DDEKvmInterface *ddeKvm);
    QScopedPointer<DDEKvmKeyboardInterfacePrivate> d;

    void sendKey(quint32 key, quint32 serial, quint32 state);
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

};

}