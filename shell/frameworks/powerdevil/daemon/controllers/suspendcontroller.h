/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <QDBusInterface>
#include <QObject>
#include <QPointer>

#include <KJob>

#include <sessionmanagement.h>

#include "powerdevilcore_export.h"

class POWERDEVILCORE_EXPORT SuspendController : public QObject
{
    Q_OBJECT
public:
    SuspendController();

    /**
     * This enum type defines the different suspend methods.
     *
     * - UnknownSuspendMethod: The name says it all
     * - Standby: Processes are stopped, some hardware is deactivated (ACPI S1)
     * - ToRam: Most devices are deactivated, only RAM is powered (ACPI S3)
     * - ToDisk: State of the machine is saved to disk, and it's powered down (ACPI S4)
     * - SuspendThenHibernate: Same as ToRam, but after a delay it switches to ToDisk
     */
    enum SuspendMethod {
        UnknownSuspendMethod = 0,
        Standby = 1,
        ToRam = 2,
        ToDisk = 4,
        HybridSuspend = 8,
        SuspendThenHibernate = 16,
    };
    Q_ENUM(SuspendMethod)

    /**
     * This type stores an OR combination of SuspendMethod values.
     */
    Q_DECLARE_FLAGS(SuspendMethods, SuspendMethod)

    bool canSuspend() const;
    void suspend();

    bool canHibernate() const;
    void hibernate();

    bool canHybridSuspend() const;
    void hybridSuspend();

    bool canSuspendThenHibernate() const;
    void suspendThenHibernate();

Q_SIGNALS:
    /**
     * This signal is emitted when the PC is resuming from suspension
     */
    void resumeFromSuspend();

    /**
     * This signal is emitted when the PC is about to suspend
     */
    void aboutToSuspend();

private Q_SLOTS:
    void slotLogin1PrepareForSleep(bool active);

private:
    SessionManagement m_sessionManagement;

    // login1 interface
    QPointer<QDBusInterface> m_login1Interface;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SuspendController::SuspendMethods)
