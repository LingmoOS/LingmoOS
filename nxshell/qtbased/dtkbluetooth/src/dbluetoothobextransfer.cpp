// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothobextransfer_p.h"
#include "dobextransferinterface.h"
#include "dbluetoothutils.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::emplace_tag;

DObexTransferPrivate::DObexTransferPrivate(const ObexSessionInfo &info, quint64 transferId, DObexTransfer *parent)
    : DObjectPrivate(parent)
    , m_obextransfer(new DObexTransferInterface(transferIdToDBusPath(sessionInfoToDBusPath(info), transferId)))
{
}

DObexTransferPrivate::~DObexTransferPrivate()
{
    delete m_obextransfer;
}

DObexTransfer::DObexTransfer(const ObexSessionInfo &info, quint64 transferId, QObject *parent)
    : QObject(parent)
    , DObject(*new DObexTransferPrivate(info, transferId, this))
{
    D_DC(DObexTransfer);
    connect(d->m_obextransfer, &DObexTransferInterface::statusChanged, this, [this](const QString &statusStr) {
        Q_EMIT this->statusChanged(stringToTransferstatus(statusStr));
    });
    connect(d->m_obextransfer, &DObexTransferInterface::transferredChanged, this, &DObexTransfer::transferredChanged);
    connect(d->m_obextransfer, &DObexTransferInterface::removed, this, &DObexTransfer::removed);
}

DObexTransfer::~DObexTransfer() = default;

DObexTransfer::TransferStatus DObexTransfer::status() const
{
    D_DC(DObexTransfer);
    return stringToTransferstatus(d->m_obextransfer->status());
}

ObexSessionInfo DObexTransfer::session() const
{
    D_DC(DObexTransfer);
    return dBusPathToSessionInfo(d->m_obextransfer->session());
}

QString DObexTransfer::name() const
{
    D_DC(DObexTransfer);
    return d->m_obextransfer->name();
}

quint64 DObexTransfer::size() const
{
    D_DC(DObexTransfer);
    return d->m_obextransfer->size();
}

quint64 DObexTransfer::transferred() const
{
    D_DC(DObexTransfer);
    return d->m_obextransfer->transferred();
}

QFileInfo DObexTransfer::filename() const
{
    D_DC(DObexTransfer);
    return QFileInfo{d->m_obextransfer->filename()};
}

QString DObexTransfer::type() const
{
    D_DC(DObexTransfer);
    return d->m_obextransfer->type();
}

DExpected<void> DObexTransfer::cancel() const
{
    D_DC(DObexTransfer);
    auto reply = d->m_obextransfer->cancel();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DBLUETOOTH_END_NAMESPACE
