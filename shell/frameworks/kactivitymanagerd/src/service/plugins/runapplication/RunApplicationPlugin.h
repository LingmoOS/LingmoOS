/*
 *   SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Plugin.h>

class RunApplicationPlugin : public Plugin
{
    Q_OBJECT

public:
    explicit RunApplicationPlugin(QObject *parent);
    ~RunApplicationPlugin() override;

    bool init(QHash<QString, QObject *> &modules) override;

private Q_SLOTS:
    void currentActivityChanged(const QString &activity);
    void activityStateChanged(const QString &activity, int state);

private:
    QString activityDirectory(const QString &activity) const;
    void executeIn(const QString &directory) const;

    QString m_currentActivity;
    QStringList m_previousActivities;
    QObject *m_activitiesService;
};
