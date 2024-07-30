/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "testinterface.h"
#include "fakebluez.h"

#include <QDBusConnection>

TestInterface::TestInterface(FakeBluez *parent)
    : QDBusAbstractAdaptor(parent)
    , m_fakeBluez(parent)
{
}

void TestInterface::runTest(const QString &testName)
{
    m_fakeBluez->runTest(testName);
}

void TestInterface::runAction(const QString &object, const QString &actionName, const QVariantMap &properties, const QDBusMessage &msg)
{
    m_msg = msg;
    m_msg.setDelayedReply(true);

    m_fakeBluez->runAction(object, actionName, properties);
}

void TestInterface::emitActionFinished()
{
    QDBusConnection::sessionBus().send(m_msg.createReply());
}

#include "moc_testinterface.cpp"
