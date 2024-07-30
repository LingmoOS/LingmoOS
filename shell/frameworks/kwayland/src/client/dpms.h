/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_DPMS_H
#define KWAYLAND_CLIENT_DPMS_H

#include <QObject>

#include "KWayland/Client/kwaylandclient_export.h"

struct org_kde_kwin_dpms;
struct org_kde_kwin_dpms_manager;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Dpms;
class Output;

/**
 * @short This class is a factory for Dpms instances.
 *
 * It is a convenience wrapper for the org_kde_kwin_dpms_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the DpmsManager interface:
 * @code
 * DpmsManager *m = registry->createDpmsManager(name, version);
 * @endcode
 *
 * This creates the DpmsManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DpmsManager *m = new DpmsManager;
 * m->setup(registry->bindDpmsManager(name, version));
 * @endcode
 *
 * The DpmsManager can be used as a drop-in replacement for any org_kde_kwin_dpms_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry, Dpms
 * @since 5.5
 **/
class KWAYLANDCLIENT_EXPORT DpmsManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new DpmsManager.
     * Note: after constructing the DpmsManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use DpmsManager prefer using
     * Registry::createDpmsManager.
     **/
    explicit DpmsManager(QObject *parent = nullptr);
    ~DpmsManager() override;

    /**
     * @returns @c true if managing a org_kde_kwin_dpms_manager.
     **/
    bool isValid() const;
    /**
     * Setup this DpmsManager to manage the @p manager.
     * When using Registry::createDpmsManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_dpms_manager *manager);
    /**
     * Releases the org_kde_kwin_dpms_manager interface.
     * After the interface has been released the DpmsManager instance is no
     * longer valid and can be setup with another org_kde_kwin_dpms_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this DpmsManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_dpms_manager interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * DPMS gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Dpms.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Dpms.
     **/
    EventQueue *eventQueue();

    Dpms *getDpms(Output *output, QObject *parent = nullptr);

    operator org_kde_kwin_dpms_manager *();
    operator org_kde_kwin_dpms_manager *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the DpmsManager got created by
     * Registry::createDpmsManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Power management for monitors.
 *
 * Display Power Management Signaling allows power management for monitors.
 * This class is a convenient wrapper for the org_kde_kwin_dpms interface.
 * To create a Dpms call DpmsManager::getDpms.
 *
 * @see DpmsManager
 **/
class KWAYLANDCLIENT_EXPORT Dpms : public QObject
{
    Q_OBJECT
public:
    ~Dpms() override;

    enum class Mode {
        On,
        Standby,
        Suspend,
        Off,
    };

    /**
     * Setup this Dpms to manage the @p dpms.
     * When using DpmsManager::createDpms there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_dpms *dpms);
    /**
     * Releases the org_kde_kwin_dpms interface.
     * After the interface has been released the Dpms instance is no
     * longer valid and can be setup with another org_kde_kwin_dpms interface.
     **/
    void release();
    /**
     * Destroys the data held by this Dpms.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_dpms interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &Dpms::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_kwin_dpms.
     **/
    bool isValid() const;

    /**
     * @returns the Output for which this Dpms got created
     **/
    QPointer<Output> output() const;

    /**
     * Whether Dpms is supported for the Output.
     * Initially set to @c false.
     * @returns whether Dpms is supported for the Output.
     * @see supportedChanged
     **/
    bool isSupported() const;
    /**
     * The current Dpms mode.
     * Initially set to @c Mode::On.
     * @returns the current Dpms mode of the Output
     * @see modeChanged
     **/
    Mode mode() const;

    /**
     * Request to change the Output into Dpms @p mode.
     * The Wayland compositor is not obliged to honor the request.
     * If the mode changes the client is notified and @link modeChanged @endlink gets emitted.
     * @param mode The requested Dpms mode.
     **/
    void requestMode(Mode mode);

    operator org_kde_kwin_dpms *();
    operator org_kde_kwin_dpms *() const;

Q_SIGNALS:
    /**
     * Emitted if the supported state on the Output changes.
     * @see isSupported
     **/
    void supportedChanged();
    /**
     * Emitted if the Dpms mode on the Output changes.
     * @see mode
     **/
    void modeChanged();

private:
    friend class DpmsManager;
    explicit Dpms(const QPointer<Output> &o, QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Dpms::Mode)

#endif
