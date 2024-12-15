/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_OUTPUTMANAGEMENT_V2_H
#define KWAYLAND_CLIENT_OUTPUTMANAGEMENT_V2_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct kde_output_management_v2;
struct kde_output_configuration_v2;

namespace KWayland
{
namespace Client
{
class EventQueue;
class OutputConfigurationV2;

/**
 * @short Wrapper for the kde_output_management_v2 interface.
 *
 * This class provides a convenient wrapper for the kde_output_management_v2 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the OutputManagementV2 interface:
 * @code
 * OutputManagementV2 *c = registry->createOutputManagement(name, version);
 * @endcode
 *
 * This creates the OutputManagementV2 and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * OutputManagementV2 *c = new OutputManagementV2;
 * c->setup(registry->bindOutputManagement(name, version));
 * @endcode
 *
 * The OutputManagementV2 can be used as a drop-in replacement for any kde_output_management_v2
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT OutputManagementV2 : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new OutputManagementV2.
     * Note: after constructing the OutputManagementV2 it is not yet valid and one needs
     * to call setup. In order to get a ready to use OutputManagementV2 prefer using
     * Registry::createOutputManagement.
     **/
    explicit OutputManagementV2(QObject *parent = nullptr);
    ~OutputManagementV2() override;

    /**
     * Setup this OutputManagementV2 to manage the @p outputmanagement.
     * When using Registry::createOutputManagement there is no need to call this
     * method.
     **/
    void setup(kde_output_management_v2 *outputmanagement);
    /**
     * @returns @c true if managing a kde_output_management_v2.
     **/
    bool isValid() const;
    /**
     * Releases the kde_output_management_v2 interface.
     * After the interface has been released the OutputManagementV2 instance is no
     * longer valid and can be setup with another kde_output_management_v2 interface.
     **/
    void release();
    /**
     * Destroys the data hold by this OutputManagementV2.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new kde_output_management_v2 interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * OutputManagementV2 gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this OutputManagementV2.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this OutputManagementV2.
     **/
    EventQueue *eventQueue();

    OutputConfigurationV2 *createConfiguration(QObject *parent = nullptr);

    operator kde_output_management_v2 *();
    operator kde_output_management_v2 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OutputManagementV2 got created by
     * Registry::createOutputManagement
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
