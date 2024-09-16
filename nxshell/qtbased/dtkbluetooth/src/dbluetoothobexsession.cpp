// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbluetoothobexsession_p.h"
#include "dbluetoothutils.h"
#include "dbluetoothtypes.h"
#include "dbluetoothobextransfer_p.h"
#include "btobexdispatcher.h"

DBLUETOOTH_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DUnexpected;
using DTK_CORE_NAMESPACE::emplace_tag;

DObexSessionPrivate::DObexSessionPrivate(const QString &path, DObexSession *parent)
    : DObjectPrivate(parent)
    , m_obexsession(new DObexSessionInterface(path))
    , m_obexobjectpush(new DObexObjectPushInterface(path))
{
}

DObexSessionPrivate::~DObexSessionPrivate()
{
    delete m_obexsession;
    delete m_obexobjectpush;
}

DObexSession::DObexSession(const ObexSessionInfo &info, QObject *parent)
    : QObject(parent)
    , DObject(*new DObexSessionPrivate(sessionInfoToDBusPath(info), this))
{
    D_DC(DObexSession);
    connect(d->m_obexsession, &DObexSessionInterface::removed, this, &DObexSession::removed);
    connect(d->m_obexobjectpush, &DObexObjectPushInterface::transferAdded, this, &DObexSession::transferAdded);
    connect(d->m_obexobjectpush, &DObexObjectPushInterface::transferRemoved, this, &DObexSession::transferRemoved);
}

QString DObexSession::source() const
{
    D_DC(DObexSession);
    return d->m_obexsession->source();
}

QString DObexSession::destination() const
{
    D_DC(DObexSession);
    return d->m_obexsession->destination();
}

QBluetoothUuid DObexSession::target() const
{
    D_DC(DObexSession);
    return QBluetoothUuid{d->m_obexsession->target()};
}

QDir DObexSession::root() const
{
    D_DC(DObexSession);
    return d->m_obexsession->root();
}

ObexSessionInfo DObexSession::currentSession() const
{
    D_DC(DObexSession);
    return dBusPathToSessionInfo(QDBusObjectPath(d->m_obexsession->dBusPath()));
}

DExpected<QList<quint64>> DObexSession::transfers() const
{
    auto reply = BluetoothObexDispatcher::instance().getManagedObjects();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    }
    const auto &transferList = getSpecificObject(
        reply.value(),
        {QString(BlueZObexTransferInterface)},
        {{QString(BlueZObexTransferInterface),
          QVariantMap{{QString("Session"),
                       QVariant::fromValue<QDBusObjectPath>(QDBusObjectPath(sessionInfoToDBusPath(currentSession())))}}}});
    QList<quint64> ret;
    for (const auto &transfer : transferList)
        ret.append(dBusPathToTransferId(transfer.path()));
    return ret;
}

DExpected<QSharedPointer<DObexTransfer>> DObexSession::transferFromId(quint64 transferId) const
{
    const auto &reply = transfers();
    if (!reply)
        return DUnexpected(reply.error());
    const auto &transferList = reply.value();
    if (!transferList.contains(transferId))
        return DUnexpected(emplace_tag::USE_EMPLACE, -1, "no such transfer in current session");
    return QSharedPointer<DObexTransfer>(new DObexTransfer(currentSession(), transferId));
}

DExpected<QString> DObexSession::capabilities()
{
    D_DC(DObexSession);
    auto reply = d->m_obexsession->getCapabilities();
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return {};
}

DExpected<quint64> DObexSession::sendFile(const QFileInfo &file) const
{
    D_DC(DObexSession);
    if (!file.exists() or !file.isFile())
        return DUnexpected{emplace_tag::USE_EMPLACE, -1, "File does not exist or is not a file"};
    auto reply = d->m_obexobjectpush->sendFile(file);
    reply.waitForFinished();
    if (!reply.isValid())
        return DUnexpected{emplace_tag::USE_EMPLACE, reply.error().type(), reply.error().message()};
    return dBusPathToTransferId(reply.argumentAt<0>().path());
}

DBLUETOOTH_END_NAMESPACE
