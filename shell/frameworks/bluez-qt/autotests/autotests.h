/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef AUTOTESTS_H
#define AUTOTESTS_H

#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QSignalSpy>
#include <QTest>

class FakeBluez
{
public:
    static void start();
    static void stop();

    static bool isRunning();
    static void runTest(const QString &testName);
    static void runAction(const QString &object, const QString &actionName, const QVariantMap &properties = QVariantMap());

    static QProcess *s_process;
};

namespace Autotests
{
void registerMetatypes();
void verifyPropertiesChangedSignal(const QSignalSpy &spy, const QString &propertyName, const QVariant &propertyValue);

}

#endif // AUTOTESTS_H
