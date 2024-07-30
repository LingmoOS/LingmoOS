/*
    SPDX-FileCopyrightText: 2009-2012 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "actionreply.h"

#include <QDebug>
#include <QIODevice>

namespace KAuth
{
class ActionReplyData : public QSharedData
{
public:
    ActionReplyData()
    {
    }
    ActionReplyData(const ActionReplyData &other)
        : QSharedData(other)
        , data(other.data)
        , errorCode(other.errorCode)
        , errorDescription(other.errorDescription)
        , type(other.type)
    {
    }
    ~ActionReplyData()
    {
    }

    QVariantMap data; // User-defined data for success and helper error replies, empty for kauth errors
    uint errorCode;
    QString errorDescription;
    ActionReply::Type type;
};

// Predefined replies
const ActionReply ActionReply::SuccessReply()
{
    return ActionReply();
}
const ActionReply ActionReply::HelperErrorReply()
{
    ActionReply reply(ActionReply::HelperErrorType);
    reply.setError(-1);
    return reply;
}
const ActionReply ActionReply::HelperErrorReply(int error)
{
    ActionReply reply(ActionReply::HelperErrorType);
    reply.setError(error);
    return reply;
}
const ActionReply ActionReply::NoResponderReply()
{
    return ActionReply(ActionReply::NoResponderError);
}
const ActionReply ActionReply::NoSuchActionReply()
{
    return ActionReply(ActionReply::NoSuchActionError);
}
const ActionReply ActionReply::InvalidActionReply()
{
    return ActionReply(ActionReply::InvalidActionError);
}
const ActionReply ActionReply::AuthorizationDeniedReply()
{
    return ActionReply(ActionReply::AuthorizationDeniedError);
}
const ActionReply ActionReply::UserCancelledReply()
{
    return ActionReply(ActionReply::UserCancelledError);
}
const ActionReply ActionReply::HelperBusyReply()
{
    return ActionReply(ActionReply::HelperBusyError);
}
const ActionReply ActionReply::AlreadyStartedReply()
{
    return ActionReply(ActionReply::AlreadyStartedError);
}
const ActionReply ActionReply::DBusErrorReply()
{
    return ActionReply(ActionReply::DBusError);
}

// Constructors
ActionReply::ActionReply(const ActionReply &reply)
    : d(reply.d)
{
}

ActionReply::ActionReply()
    : d(new ActionReplyData())
{
    d->errorCode = 0;
    d->type = SuccessType;
}

ActionReply::ActionReply(ActionReply::Type type)
    : d(new ActionReplyData())
{
    d->errorCode = 0;
    d->type = type;
}

ActionReply::ActionReply(int error)
    : d(new ActionReplyData())
{
    d->errorCode = error;
    d->type = KAuthErrorType;
}

ActionReply::~ActionReply()
{
}

void ActionReply::setData(const QVariantMap &data)
{
    d->data = data;
}

void ActionReply::addData(const QString &key, const QVariant &value)
{
    d->data.insert(key, value);
}

QVariantMap ActionReply::data() const
{
    return d->data;
}

ActionReply::Type ActionReply::type() const
{
    return d->type;
}

void ActionReply::setType(ActionReply::Type type)
{
    d->type = type;
}

bool ActionReply::succeeded() const
{
    return d->type == SuccessType;
}

bool ActionReply::failed() const
{
    return !succeeded();
}

ActionReply::Error ActionReply::errorCode() const
{
    return (ActionReply::Error)d->errorCode;
}

void ActionReply::setErrorCode(Error errorCode)
{
    d->errorCode = errorCode;
    if (d->type != HelperErrorType) {
        d->type = KAuthErrorType;
    }
}

int ActionReply::error() const
{
    return d->errorCode;
}

void ActionReply::setError(int error)
{
    d->errorCode = error;
}

QString ActionReply::errorDescription() const
{
    return d->errorDescription;
}

void ActionReply::setErrorDescription(const QString &error)
{
    d->errorDescription = error;
}

QByteArray ActionReply::serialized() const
{
    QByteArray data;
    QDataStream s(&data, QIODevice::WriteOnly);

    s << d->data << d->errorCode << static_cast<quint32>(d->type) << d->errorDescription;

    return data;
}

ActionReply ActionReply::deserialize(const QByteArray &data)
{
    ActionReply reply;
    QByteArray a(data);
    QDataStream s(&a, QIODevice::ReadOnly);

    quint32 i;
    s >> reply.d->data >> reply.d->errorCode >> i >> reply.d->errorDescription;
    reply.d->type = static_cast<ActionReply::Type>(i);

    return reply;
}

// Operators
ActionReply &ActionReply::operator=(const ActionReply &reply)
{
    if (this == &reply) {
        // Protect against self-assignment
        return *this;
    }

    d = reply.d;
    return *this;
}

bool ActionReply::operator==(const ActionReply &reply) const
{
    return (d->type == reply.d->type && d->errorCode == reply.d->errorCode);
}

bool ActionReply::operator!=(const ActionReply &reply) const
{
    return (d->type != reply.d->type || d->errorCode != reply.d->errorCode);
}

} // namespace Auth
