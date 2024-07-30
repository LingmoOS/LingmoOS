/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>

class GSDXSettingsManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.gtk.Settings")

    Q_PROPERTY(qlonglong FontconfigTimestamp READ FontconfigTimestamp)
    Q_PROPERTY(QString Modules READ Modules)
    Q_PROPERTY(bool EnableAnimations READ EnableAnimations)

public:
    explicit GSDXSettingsManager(QObject *parent);
    ~GSDXSettingsManager() override;

public:
    void modulesChanged();
    void enableAnimationsChanged();

private:
    qlonglong FontconfigTimestamp() const;
    QString Modules() const;
    bool EnableAnimations() const;
};
