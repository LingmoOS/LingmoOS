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

#include "lingmo-kwin_export.h"

struct wl_resource;

namespace KWaylandServer
{
class Display;
class OCEANKvmPointerInterface;
class OCEANKvmKeyboardInterface;

class OCEANKvmInterfacePrivate;
class OCEANKvmPointerInterfacePrivate;
class OCEANKvmKeyboardInterfacePrivate;

/** @class OCEANKvmInterface
 *
 *
 * @see OCEANKvmInterface
 * @since 5.5
 */
class KWIN_EXPORT OCEANKvmInterface : public QObject
{
    Q_OBJECT
public:
    explicit OCEANKvmInterface(Display *display, QObject *parent = nullptr);
    virtual ~OCEANKvmInterface();

    static OCEANKvmInterface *get(wl_resource *native);

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
     * Emitted whenever a OCEANPointerInterface got created.
     **/
    void oceanKvmPointerCreated(KWaylandServer::OCEANKvmPointerInterface*);
    void oceanKvmKeyboardCreated(KWaylandServer::OCEANKvmKeyboardInterface*);

    void kvmInterfaceEnablePointerRequested(quint32 is_enable);
    void kvmInterfaceEnableCursorRequested(quint32 is_enable);
    void kvmInterfaceSetCursorPosRequested(double x, double y);
    void kvmInterfaceEnableKeyboardRequested(quint32 is_enable);

private:
    QScopedPointer<OCEANKvmInterfacePrivate> d;
    friend class OCEANKvmInterfacePrivate;
};

/**
 * @brief Resource for the ocean_kvm_pointer interface.
 *
 * OCEANKvmPointerInterface gets created by OCEANKvmInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT OCEANKvmPointerInterface : public QObject
{
    Q_OBJECT
public:
    ~OCEANKvmPointerInterface() override;

    /**
     * @returns The OCEANKvmInterface which created this OCEANKvmPointerInterface.
     **/
    OCEANKvmInterface *oceanKvm() const;

    /**
     * @returns The OCEANPointerInterface for the @p native resource.
     * @since 5.5
     **/
    static OCEANKvmPointerInterface *get(wl_resource *native);

Q_SIGNALS:
    void kvmEnablePointerRequested(quint32 is_enable);
    void kvmEnableCursorRequested(quint32 is_enable);
    void kvmSetCursorPosRequested(double x, double y);

private:
    friend class OCEANKvmInterface;
    friend class OCEANKvmInterfacePrivate;
    friend class OCEANKvmPointerInterfacePrivate;

    explicit OCEANKvmPointerInterface(OCEANKvmInterface *kvm);
    QScopedPointer<OCEANKvmPointerInterfacePrivate> d;

    void sendButton(quint32 button, quint32 state, quint32 serial, const QPointF &position);
    void sendAxis(Qt::Orientation orientation, qreal delta);
    void sendMotion(const QPointF &position);
};

/**
 * @brief Resource for the ocean_kvm_keyboard interface.
 *
 * OCEANKvmKeyboardInterface gets created by OCEANKvmInterface.
 *
 * @since 5.4
 **/
class KWIN_EXPORT OCEANKvmKeyboardInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~OCEANKvmKeyboardInterface();

    /**
     * @returns The OCEANKvmInterface which created this OCEANKvmKeyboardInterface.
     **/
    OCEANKvmInterface *oceanKvm() const;

    /**
     * @returns The OCEANKvmKeyboardInterface for the @p native resource.
     * @since 5.5
     **/
    static OCEANKvmKeyboardInterface *get(wl_resource *native);

Q_SIGNALS:
    void kvmEnableKeyboardRequested(uint32_t is_enable);

private:
    friend class OCEANKvmInterface;
    friend class OCEANKvmInterfacePrivate;
    friend class OCEANKvmKeyboardInterfacePrivate;

    explicit OCEANKvmKeyboardInterface(OCEANKvmInterface *oceanKvm);
    QScopedPointer<OCEANKvmKeyboardInterfacePrivate> d;

    void sendKey(quint32 key, quint32 serial, quint32 state);
    void sendModifiers(quint32 depressed, quint32 latched, quint32 locked, quint32 group, quint32 serial);

};

}