/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#pragma once

#include <QObject>

class PowerManagement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSuspend READ canSuspend NOTIFY canSuspendChanged)
    Q_PROPERTY(bool canHibernate READ canHibernate NOTIFY canHibernateChanged)
public:
    ~PowerManagement() override;

    bool canSuspend() const;
    bool canHibernate() const;

    static PowerManagement *instance();

public Q_SLOTS:
    void suspend();
    void hibernate();

Q_SIGNALS:
    void canSuspendChanged();
    void canHibernateChanged();

protected:
    explicit PowerManagement();

private:
    class Private;
    QScopedPointer<Private> d;
};
