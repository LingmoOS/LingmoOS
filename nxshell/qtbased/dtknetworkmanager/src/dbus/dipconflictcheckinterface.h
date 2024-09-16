// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPCONFLICTCHECKINTERFACE_H
#define DIPCONFLICTCHECKINTERFACE_H

#include "dnetworkmanager_global.h"
#include "dnetworkmanagertypes.h"
#include <DDBusInterface>
#include <QDBusPendingReply>
#include <QString>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DDBusInterface;

class DIPConflictCheckInterface : public QObject
{
    Q_OBJECT
public:
    explicit DIPConflictCheckInterface(QObject *parent = nullptr);
    ~DIPConflictCheckInterface() override = default;

public Q_SLOTS:
    QDBusPendingReply<QString> requestIPConflictCheck(const QByteArray &ip, const QByteArray &ifc) const;

Q_SIGNALS:
    void IPConflict(const QString &ip, const QString &smac, const QString &dmac);

private:
    DDBusInterface *m_inter{nullptr};
};

DNETWORKMANAGER_END_NAMESPACE

#endif
