// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPCONFLICTCHECK_H
#define DIPCONFLICTCHECK_H

#include "dnetworkmanagertypes.h"
#include <QScopedPointer>
#include <DExpected>

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;

class DIPConflictCheckPrivate;

class DIPConflictCheck : public QObject
{
    Q_OBJECT
public:
    explicit DIPConflictCheck(QObject *parent = nullptr);
    ~DIPConflictCheck() override;

public Q_SLOTS:
    DExpected<QByteArray> requestIPConflictCheck(const QByteArray &ip, const QByteArray &ifc) const;

Q_SIGNALS:
    void IPConflict(const QByteArray &ip, const QByteArray &smac, const QByteArray &dmac);

private:
    QScopedPointer<DIPConflictCheckPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DIPConflictCheck)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
