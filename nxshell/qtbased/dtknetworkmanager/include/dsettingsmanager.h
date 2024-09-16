// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSETTINGMANAGER_H
#define DSETTINGMANAGER_H

#include "dnetworkmanagertypes.h"
#include <DExpected>
#include <QUuid>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;

class DSettingsManagerPrivate;

class DSettingsManager : public QObject
{
    Q_OBJECT

public:
    explicit DSettingsManager(QObject *parent = nullptr);
    ~DSettingsManager() override;

public Q_SLOTS:
    DExpected<QList<quint64>> listConnections() const;
    DExpected<quint64> getConnectionByUUID(const QUuid &UUID) const;
    DExpected<quint64> addConnection(const SettingDesc &conn) const;

Q_SIGNALS:
    void ConnectionRemoved(const quint64 connId);
    void NewConnection(const quint64 connId);

private:
    QScopedPointer<DSettingsManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DSettingsManager)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
