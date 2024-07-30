/*
 *   SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Plugin.h>

class VirtualDesktopSwitchPlugin : public Plugin
{
    Q_OBJECT

public:
    explicit VirtualDesktopSwitchPlugin(QObject *parent);
    ~VirtualDesktopSwitchPlugin() override;

    bool init(QHash<QString, QObject *> &modules) override;

private Q_SLOTS:
    void currentActivityChanged(const QString &activity);
    void activityRemoved(const QString &activity);

private:
    QString m_currentActivity;
    QObject *m_activitiesService;
};
