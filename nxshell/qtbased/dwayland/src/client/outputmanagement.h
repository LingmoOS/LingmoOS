/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_OUTPUTMANAGEMENT_H
#define KWAYLAND_CLIENT_OUTPUTMANAGEMENT_H

#include <QObject>

#include <DWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_outputmanagement;
struct org_kde_kwin_outputconfiguration;

namespace KWayland
{
namespace Client
{
class EventQueue;
class OutputConfiguration;

/**
 * @short Wrapper for the org_kde_kwin_outputmanagement interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_outputmanagement interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the OutputManagement interface:
 * @code
 * OutputManagement *c = registry->createOutputManagement(name, version);
 * @endcode
 *
 * This creates the OutputManagement and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * OutputManagement *c = new OutputManagement;
 * c->setup(registry->bindOutputManagement(name, version));
 * @endcode
 *
 * The OutputManagement can be used as a drop-in replacement for any org_kde_kwin_outputmanagement
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT OutputManagement : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new OutputManagement.
     * Note: after constructing the OutputManagement it is not yet valid and one needs
     * to call setup. In order to get a ready to use OutputManagement prefer using
     * Registry::createOutputManagement.
     **/
    explicit OutputManagement(QObject *parent = nullptr);
    ~OutputManagement() override;

    /**
     * Setup this OutputManagement to manage the @p outputmanagement.
     * When using Registry::createOutputManagement there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_outputmanagement *outputmanagement);
    /**
     * @returns @c true if managing a org_kde_kwin_outputmanagement.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_outputmanagement interface.
     * After the interface has been released the OutputManagement instance is no
     * longer valid and can be setup with another org_kde_kwin_outputmanagement interface.
     **/
    void release();
    /**
     * Destroys the data hold by this OutputManagement.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_kwin_outputmanagement interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * OutputManagement gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this OutputManagement.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this OutputManagement.
     **/
    EventQueue *eventQueue();

    OutputConfiguration *createConfiguration(QObject *parent = nullptr);

    operator org_kde_kwin_outputmanagement *();
    operator org_kde_kwin_outputmanagement *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OutputManagement got created by
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
