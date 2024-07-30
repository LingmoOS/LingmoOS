/*
    SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "FakeHelperProxy.h"

namespace KAuth
{
FakeHelperProxy::FakeHelperProxy()
    : HelperProxy()
{
}

FakeHelperProxy::~FakeHelperProxy()
{
}

void FakeHelperProxy::sendProgressStepData(const QVariantMap &step)
{
    Q_UNUSED(step)
}

void FakeHelperProxy::sendProgressStep(int step)
{
    Q_UNUSED(step)
}

void FakeHelperProxy::sendDebugMessage(int level, const char *msg)
{
    Q_UNUSED(level)
    Q_UNUSED(msg)
}

bool FakeHelperProxy::hasToStopAction()
{
    return false;
}

void FakeHelperProxy::setHelperResponder(QObject *o)
{
    Q_UNUSED(o)
}

bool FakeHelperProxy::initHelper(const QString &name)
{
    Q_UNUSED(name)
    return false;
}

void FakeHelperProxy::stopAction(const QString &action, const QString &helperID)
{
    Q_UNUSED(action)
    Q_UNUSED(helperID)
}

void FakeHelperProxy::executeAction(const QString &action, const QString &helperID, const DetailsMap &details, const QVariantMap &arguments, int timeout)
{
    Q_UNUSED(helperID)
    Q_UNUSED(details)
    Q_UNUSED(arguments)
    Q_UNUSED(timeout)
    Q_EMIT actionPerformed(action, KAuth::ActionReply::NoSuchActionReply());
}

int FakeHelperProxy::callerUid() const
{
    return -1;
}

}

#include "moc_FakeHelperProxy.cpp"
