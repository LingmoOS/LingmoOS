/*
    SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <QStringList>

#include <qqmlregistration.h>

class LaunchApp : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit LaunchApp(QObject *parent = nullptr);

public Q_SLOTS:
    void launchWizard();
    void launchSendFile(const QString &ubi);
};
