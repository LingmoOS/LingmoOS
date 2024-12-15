// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "service_stub.h"

#include "dbusuiopenschedule.h"
#include "dbusnotify.h"

QDBusMessage callWithArgumentList_stub(QDBus::CallMode mode,
                                       const QString &method,
                                       const QList<QVariant> &args)
{
    Q_UNUSED(mode);
    Q_UNUSED(method);
    Q_UNUSED(args);
    QDBusMessage reply;
    reply.createReply("succ");
    return reply;
}

void qDBusAbstractInterface_callWithArgumentList_stub(Stub &stub)
{
    stub.set(ADDR(QDBusAbstractInterface, callWithArgumentList), callWithArgumentList_stub);
    qDBusMessage_type_Stub(stub);
}

QDBusMessage::MessageType type_Stub()
{
    return QDBusMessage::ReplyMessage;
}

void qDBusMessage_type_Stub(Stub &stub)
{
    stub.set(ADDR(QDBusMessage, type), type_Stub);
}
