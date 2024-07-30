/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#include <QDBusAbstractAdaptor>
#include <QDBusMessage>

class FakeBluez;

class TestInterface : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.bluezqt.fakebluez.Test")

public:
    explicit TestInterface(FakeBluez *parent);

public Q_SLOTS:
    void runTest(const QString &testName);
    void runAction(const QString &object, const QString &actionName, const QVariantMap &properties, const QDBusMessage &msg);

    void emitActionFinished();

private:
    FakeBluez *m_fakeBluez;
    QDBusMessage m_msg;
};

#endif // TESTINTERFACE_H
