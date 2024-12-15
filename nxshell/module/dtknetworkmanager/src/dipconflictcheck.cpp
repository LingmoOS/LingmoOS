// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dipconflictcheck.h"
#include "dipconflictcheck_p.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DCORE_NAMESPACE::emplace_tag;

DIPConflictCheckPrivate::DIPConflictCheckPrivate(DIPConflictCheck *parent)
    : q_ptr(parent)
    , m_check(new DIPConflictCheckInterface(this))
{
}

DIPConflictCheck::DIPConflictCheck(QObject *parent)
    : QObject(parent)
    , d_ptr(new DIPConflictCheckPrivate(this))
{
    Q_D(const DIPConflictCheck);
    connect(d->m_check,
            &DIPConflictCheckInterface::IPConflict,
            this,
            [this](const QString &ip, const QString &smac, const QString &dmac) {
                Q_EMIT this->IPConflict(ip.toUtf8(), smac.toUtf8(), dmac.toUtf8());
            });
}

DIPConflictCheck::~DIPConflictCheck() = default;

DExpected<QByteArray> DIPConflictCheck::requestIPConflictCheck(const QByteArray &ip, const QByteArray &ifc) const
{
    Q_D(const DIPConflictCheck);
    auto reply = d->m_check->requestIPConflictCheck(ip, ifc);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return reply.value().toUtf8();
}

DNETWORKMANAGER_END_NAMESPACE
