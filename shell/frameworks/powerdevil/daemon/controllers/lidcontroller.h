/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#pragma once

#include <QObject>

#include "powerdevilcore_export.h"

#include "upower_interface.h"

class POWERDEVILCORE_EXPORT LidController : public QObject
{
    Q_OBJECT
public:
    LidController();

    /**
     * @returns whether the lid is closed or not.
     */
    bool isLidClosed() const;
    /**
     * @returns whether the a lid is present
     */
    bool isLidPresent() const;

Q_SIGNALS:
    /**
     * This signal is emitted when the laptop lid is closed or opened
     *
     * @param closed Whether the lid is now closed or not
     */
    void lidClosedChanged(bool closed);

private Q_SLOTS:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    OrgFreedesktopUPowerInterface *m_upowerInterface = nullptr;
    bool m_isLidClosed = false;
    bool m_isLidPresent = false;
};
