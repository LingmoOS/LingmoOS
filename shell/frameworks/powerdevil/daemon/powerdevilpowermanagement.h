/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>

#include "powerdevilcore_export.h"

namespace PowerDevil
{
class POWERDEVILCORE_EXPORT PowerManagement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canSuspend READ canSuspend NOTIFY canSuspendChanged)
    Q_PROPERTY(bool canHibernate READ canHibernate NOTIFY canHibernateChanged)
    Q_PROPERTY(bool canHybridSuspend READ canHybridSuspend NOTIFY canHybridSuspendChanged)
    Q_PROPERTY(bool canSuspendThenHibernate READ canSuspendThenHibernate NOTIFY canSuspendThenHibernateChanged)
public:
    ~PowerManagement() override;

    bool isVirtualMachine();
    bool canSuspend() const;
    bool canHibernate() const;
    bool canHybridSuspend() const;
    bool canSuspendThenHibernate() const;

    static PowerManagement *instance();

public Q_SLOTS:
    void suspend();
    void hibernate();
    void hybridSuspend();
    void suspendThenHibernate();

Q_SIGNALS:
    void canSuspendChanged();
    void canSuspendThenHibernateChanged();
    void canHibernateChanged();
    void canHybridSuspendChanged();

protected:
    explicit PowerManagement();

private:
    class Private;
    QScopedPointer<Private> d;
};

} // namespace PowerDevil
