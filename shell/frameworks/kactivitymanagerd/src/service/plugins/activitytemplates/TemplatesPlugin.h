/*
 *   SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <Plugin.h>

class TemplatesPlugin : public Plugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Templates")

public:
    explicit TemplatesPlugin(QObject *parent);
    ~TemplatesPlugin() override;

    bool init(QHash<QString, QObject *> &modules) override;

    QDBusVariant featureValue(const QStringList &property) const override;

    void setFeatureValue(const QStringList &property, const QDBusVariant &value) override;

public Q_SLOTS:
    void createActivity(const QDBusVariant &values);

private:
    QObject *m_activities;
};
