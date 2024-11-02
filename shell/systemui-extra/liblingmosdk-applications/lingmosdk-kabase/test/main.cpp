/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <unistd.h>

#include <QApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

#include "single_application.hpp"
#include "currency.hpp"
#include "buried_point.hpp"
#include "log.hpp"
#include "session_management.hpp"
#include "dbus.hpp"
#include "system_information.hpp"
#include "user_manual.hpp"

#include "widget_test.hpp"
#include "gsettings_test.hpp"
#include "dbus_test.hpp"
#include "image_test.hpp"

/* 埋点接口测试 */
void buriedPointTest(void)
{
    kdk::kabase::BuriedPoint buriedPointTest;

    QJsonObject obj;
    obj.insert("TestBool", QJsonValue(true));
    obj.insert("TestInt", QJsonValue(7));
    obj.insert("TestFloat", QJsonValue(3.14));

    if (buriedPointTest.uploadMessage(kdk::kabase::AppName::LingmoFontViewer,
                                      kdk::kabase::BuriedPoint::BuriedPointType::FunctionType, obj)) {
        qCritical() << "Error : buried point fail !";
    }

    return;
}

/* 功能埋点测试 */
void functionBuriedPointTest(void)
{
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::LingmoOsManager,
                                            kdk::kabase::BuriedPoint::PT::LingmoOsManagerGarbageClean)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}

/* lingmosdk 日志接口测试 */
void logKysdkTest(void)
{
    QString a = "TestQString";
    std::string b = "TestStd::String";
    char *c = "TestC";
    const char *d = "TestConstC";
    int e = 7;
    float f = 3.14;
    double g = 3.14;
    bool h = true;

    int i = 10;
    int *j = &i;

    QString k("aaaaaa");
    QString *l = &k;

    kdk::kabase::debug << a << b << c << d << e << f << g << h << j << l;
    kdk::kabase::info << a << b << c << d << e << f << g << h << j << l;
    kdk::kabase::warning << a << b << c << d << e << f << g << h << j << l;
    kdk::kabase::error << a << b << c << d << e << f << g << h << j << l;
    kdk::kabase::fatal << a << b << c << d << e << f << g << h << j << l;

    return;
}

/* Qt 日志注册函数接口测试 */
void logRegisterTest(void)
{
    qInstallMessageHandler(kdk::kabase::Log::logOutput);

    QString a = "QStringTest";
    QJsonObject b;
    b.insert("Key1", QJsonValue("StringTest"));
    b.insert("Key2", QJsonValue(2));
    b.insert("Key3", QJsonValue(3.14));
    int c = 777;
    float d = 3.14;
    const char *p = "Point";

    qDebug() << a << b << c << d << p;
    qInfo() << a << b << c << d << p;
    qWarning() << a << b << c << d << p;
    qCritical() << a << b << c << d << p;
    // qFatal("++++++++++++++++++++++");

    return;
}

/* 锁屏接口测试 */
void inhibitLockScreenTest(void)
{
    kdk::kabase::SessionManagement sessionManagementTest;
    quint32 flag =
        sessionManagementTest.setInhibitLockScreen(kdk::kabase::AppName::LingmoFontViewer, QString("TestReason"));
    if (flag == 0) {
        qCritical() << "set inhibit lock screen fail";
        return;
    }

    qInfo() << "set inhibit lock screen success ! flag = " << flag;

    sleep(10);

    /* 取消禁止锁屏 */
    sessionManagementTest.unInhibitLockScreen(flag);

    return;
}

/* 用户手册测试 */
void userManualTest(void)
{
    //kdk::kabase::UserManualManagement userManualTest;
    //if (!userManualTest.callUserManual(kdk::kabase::AppName::LingmoIpmsg)) {
    //    qCritical() << "user manual call fail!";
    //}

    kdk::kabase::UserManualManagement userManualTest;
    if (!userManualTest.callUserManual("lingmo-calculator")) {
        qCritical() << "user manual call fail!";
    }

    return;
}

/* D-Bus 接口测试 */
void dbusTest(void)
{
    DBusTest *dbus = new DBusTest();

    kdk::kabase::DBus::registerService("com.test.service");
    kdk::kabase::DBus::registerObject("/test/object", dbus);

    QList<QVariant> args;
    args << 3 << 2;
    QList<QVariant> ret =
        kdk::kabase::DBus::callMethod("com.test.service", "/test/object", "com.test.service.interface", "add", args);
    if (!ret.isEmpty()) {
        qInfo() << ret.at(0).toInt();
    }
    delete dbus;

    return;
}

/* 获取系统信息接口测试 */
void systemInformationTest(void)
{
    kdk::kabase::SystemInformation systemInformationTest;

    qDebug() << systemInformationTest.getLsbReleaseInformation(QString("DISTRIB_DESCRIPTION"));
    qDebug() << systemInformationTest.getOsReleaseInformation(QString("VERSION"));
    qDebug() << systemInformationTest.getProjectCodeName();
    qDebug() << systemInformationTest.getCpuModelName();
    qDebug() << systemInformationTest.getHdPlatform();

    return;
}

int main(int argc, char *argv[])
{
    /* 设置缩放属性测试· */
    // kdk::kabase::WindowManagement::setScalingProperties();

    /* 应用单例测试 */
    //kdk::kabase::QtSingleApplication app(argc, argv);

    //if (app.isRunning()) {
    //    qDebug() << "is running";
    //    app.sendMessage("running , 4000");
    //    return 0;
    //}

    //QWidget widget;
    //app.setActivationWindow(&widget);
    //widget.show();

    //return app.exec();

    QApplication app(argc, argv);

    /* 日志输出测试 */
    // logKysdkTest();

    /* 日志注册测试 */
    // logRegisterTest();

    /* 埋点测试 */
    buriedPointTest();

    /* 功能埋点测试 */
    // functionBuriedPointTest();

    /* 窗口测试 */
    // Widget widgetTest;
    // widgetTest.show();

    /* Gsettings 测试 */
    // GsettingTest gsettingsTest;
    // Q_UNUSED(gsettingsTest);

    /* 锁屏测试 */
    // inhibitLockScreenTest();

    /* 用户手册测试 */
    //userManualTest();

    /* D-Bus 测试 */
    // dbusTest();

    /* 系统信息接口测试 */
    // systemInformationTest();

    /* 图片测试 */
    // ImageTest imageTest;
    // imageTest.show();

    return app.exec();
}
