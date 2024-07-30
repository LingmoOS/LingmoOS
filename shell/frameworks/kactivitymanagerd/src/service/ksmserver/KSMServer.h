/*
 *   SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

// Qt
#include <QObject>

// Utils
#include <utils/d_ptr.h>

/**
 * KSMServer
 */
class KSMServer : public QObject
{
    Q_OBJECT
public:
    enum ReturnStatus {
        Started = 0,
        Stopped = 1,
        FailedToStop = 2,
    };

    explicit KSMServer(QObject *parent = nullptr);
    ~KSMServer() override;

public Q_SLOTS:
    void startActivitySession(const QString &activity);
    void stopActivitySession(const QString &activity);

Q_SIGNALS:
    void activitySessionStateChanged(const QString &activity, int status);

private:
    D_PTR;
};
