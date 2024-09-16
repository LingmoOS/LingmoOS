// Copyright 2022  diguoliang <diguoliang@uniontech.com>
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QObject>
#include <QPointer>
#include <QString>
#include <QMap>

#include <DWayland/Client/kwaylandclient_export.h>

struct wl_surface;
struct dde_globalproperty;

namespace KWayland
{

namespace Client
{

class EventQueue;
class Surface;

class KWAYLANDCLIENT_EXPORT GlobalProperty : public QObject
{
    Q_OBJECT
public:
    explicit GlobalProperty(QObject *parent = nullptr);
    virtual ~GlobalProperty();

    /**
     * Setup this Compositor to manage the @p GlobalProperty.
     * When using Registry::createGlobalProperty there is no need to call this
     * method.
     **/
    void setup(dde_globalproperty *ddeglobalproperty);

    bool isValid() const;

    operator dde_globalproperty*();
    operator dde_globalproperty*() const;
    dde_globalproperty *ddeGlobalProperty();

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

    void setProperty(const QString &module, const QString &function, wl_surface *surface, const int32_t &type, const QString &data);
    void setProperty(const QString &module, const QString &function, Surface *surface, const int32_t &type, const QString &data);
    QString getProperty(const QString &module, const QString &function);

    /**
    * Destroys the data hold by this GlobalProperty.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail.
    *
    * This method is automatically invoked when the Registry which created this
    * GlobalProperty gets destroyed.
    **/
    void destroy();

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
     * This signal gets only emitted if the GlobalProperty got created by
     * Registry::createGlobalProperty
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
