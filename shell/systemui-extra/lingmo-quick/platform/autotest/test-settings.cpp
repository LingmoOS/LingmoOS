/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */
#include <QtTest>
#include "settings.h"
#include <QtDBus/QDBusInterface>
#include <QtGui/QGuiApplication>
#include <QDBusPendingReply>

using namespace LingmoUIQuick;
static const QString STATUS_MANAGER_SERVICE = QStringLiteral("com.lingmo.statusmanager.interface");
static const QString STATUS_MANAGER_PATH = QStringLiteral("/");
static const QString STATUS_MANAGER_INTERFACE = QStringLiteral("com.lingmo.statusmanager.interface");
const QString USD_SERVICE = QStringLiteral("org.lingmo.SettingsDaemon");
const QString USD_GLOBAL_SIGNAL_PATH = QStringLiteral("/GlobalSignal");
const QString USD_GLOBAL_SIGNAL_INTERFACE = QStringLiteral("org.lingmo.SettingsDaemon.GlobalSignal");
class settings_test : public QObject
{
    Q_OBJECT

private slots:
    void test_liteAnimation();
    void test_liteFunction();
    void test_tabletMode();
    void test_platformName();
};

void settings_test::test_liteAnimation()
{
    QDBusInterface usdGlobalSignalIface(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                QDBusConnection::sessionBus(), this);
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> replyAppList = usdGlobalSignalIface.callWithArgumentList(QDBus::Block, "getLINGMOLiteAnimation", argumentList);
    if (!replyAppList.isValid() || replyAppList.isError()) {
        return;
    }
    QString test = replyAppList.value();
    QCOMPARE(Settings::instance()->liteAnimation(), test);
}

void settings_test::test_liteFunction()
{
    QDBusInterface usdGlobalSignalIface(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                QDBusConnection::sessionBus(), this);
    QList<QVariant> argumentList;
    QDBusPendingReply<QString> replyAppList = usdGlobalSignalIface.callWithArgumentList(QDBus::Block, "getLINGMOLiteFunction", argumentList);
    if (!replyAppList.isValid() || replyAppList.isError()) {
        return;
    }
    QString test = replyAppList.value();
    QCOMPARE(Settings::instance()->liteFunction(), "normal");
}

void settings_test::test_tabletMode()
{
    QDBusInterface statusManagerIface(STATUS_MANAGER_SERVICE, STATUS_MANAGER_PATH, STATUS_MANAGER_INTERFACE,
                                              QDBusConnection::sessionBus(), this);
    QList<QVariant> argumentList;
    QDBusPendingReply<bool> replyAppList = statusManagerIface.callWithArgumentList(QDBus::Block, "getCurrentTabletMode", argumentList);
    if (!replyAppList.isValid() || replyAppList.isError()) {
        return;
    }
    bool test = replyAppList.value();

    QCOMPARE(Settings::instance()->tabletMode(), test);
}

void settings_test::test_platformName()
{
    QCOMPARE(Settings::instance()->platformName(), QGuiApplication::platformName());
}

QTEST_MAIN(settings_test)

#include "test-settings.moc"
