/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_POWER_H
#define SOLID_POWER_H

#include "solid_export.h"
#include <QObject>

namespace Solid
{
class StatesJob;
class AcPluggedJob;
class InhibitionJob;
class RequestStateJob;
class SOLID_EXPORT Power : public QObject
{
    Q_OBJECT
public:
    /**
     * List of states a device can be in
     *
     * This list of states can be used to either put the device
     * running the code into a specific state (for example put a
     * laptop to sleep) or to avoid (inhibit) that state to happen,
     * for example to prevent the screen to be dimed automatically
     *
     * Since this is quite specific to each platform some backends might
     * not support all states.
     *
     * Some States are more complex or contain sub-states, those are usually
     * set using a specific job (for example Sleep can be done either using suspend to ram,
     * suspend to disk or an hybrid approach).
     *
     * @see InhibitionTypes
     */
    enum InhibitionType {
        None = 0,
        Sleep = 1 << 0,
        Screen = 1 << 1,
        Shutdown = 1 << 3,
    };
    /*
     * Stores a combination of #InhibitionType values.
     */
    Q_DECLARE_FLAGS(InhibitionTypes, InhibitionType)
    Q_FLAG(InhibitionTypes)
    /**
     * Returns an instance of Power
     *
     * Having an instance of Power is useful to monitor any changes
     * in the power management system by connecting to its signals, like
     * acPluggedChanged().
     *
     * If you are interested in Power during the entire life cycle of your
     * application, then you want to use this singleton. If instead you are
     * only interested for a short period of time, consider instantiating your own
     * Solid::Power so you can free the memory at any point.
     */
    static Power *self();

    /**
     * Returns an AcPluggedJob
     *
     * The returned AcPluggedJob has to be started, when finished
     * the Job::result() signal will be emitted.
     */
    static AcPluggedJob *isAcPlugged(QObject *parent = nullptr);

    /**
     * Returns an InhibitionJob
     *
     * The returned job is initialized with the given @p states and @p description
     */
    static InhibitionJob *inhibit(Power::InhibitionTypes states, const QString &description, QObject *parent = nullptr);

    /**
     * Query the supported states (like Sleep or Hibernation)
     * @return a StatesJob
     */
    static StatesJob *supportedStates(QObject *parent = nullptr);

    /**
     * Set the computer in a desired @p state (like Sleep or Hibernation)
     * @return a RequestStateJob
     */
    static RequestStateJob *requestState(Power::InhibitionType state, QObject *parent = nullptr);

    /**
     * If you are not going to destroy this object for the entire
     * application life cycle, you might want to use self() singleton.
     *
     * The only reason to instantiate your own Power object is for when an
     * application is only interested in power management during a small
     * period of time and you want to free the memory after using it.
     */
    explicit Power(QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * Emitted when the system changes the power source
     * @param plugged whether the system runs on AC
     */
    void acPluggedChanged(bool plugged);

    /**
     * Emitted when the system is about to suspend/hibernate
     *
     * @since 5.6
     */
    void aboutToSuspend();

    /**
     * Emitted when the system has just resumed from being suspended/hibernated
     */
    void resumeFromSuspend();

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Power::InhibitionTypes)

}

#endif // SOLID_POWER_H
