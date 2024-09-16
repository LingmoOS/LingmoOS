/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_PRIMARY_OUTPUT_V1_H
#define KWAYLAND_CLIENT_PRIMARY_OUTPUT_V1_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct kde_primary_output_v1;

namespace KWayland
{
namespace Client
{
class EventQueue;

class KWAYLANDCLIENT_EXPORT PrimaryOutputV1 : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new PrimaryOutputV1.
     * Note: after constructing the PrimaryOutputV1 it is not yet valid and one needs
     * to call setup. In order to get a ready to use PrimaryOutputV1 prefer using
     * Registry::createOutputManagement.
     **/
    explicit PrimaryOutputV1(QObject *parent = nullptr);
    ~PrimaryOutputV1() override;

    /**
     * Setup this PrimaryOutputV1 to manage the @p outputmanagement.
     * When using Registry::createOutputManagement there is no need to call this
     * method.
     **/
    void setup(kde_primary_output_v1 *outputmanagement);
    /**
     * @returns @c true if managing a kde_primary_output_v1.
     **/
    bool isValid() const;
    /**
     * Releases the kde_primary_output_v1 interface.
     * After the interface has been released the PrimaryOutputV1 instance is no
     * longer valid and can be setup with another kde_primary_output_v1 interface.
     **/
    void release();
    /**
     * Destroys the data hold by this PrimaryOutputV1.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new kde_primary_output_v1 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * PrimaryOutputV1 gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this PrimaryOutputV1.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this PrimaryOutputV1.
     **/
    EventQueue *eventQueue();

    operator kde_primary_output_v1 *();
    operator kde_primary_output_v1 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the PrimaryOutputV1 got created by
     * Registry::createOutputManagement
     **/
    void removed();

    void primaryOutputChanged(const QString &outputName);

private:
    class Private;
    QScopedPointer<Private> d;
};
}
}

#endif