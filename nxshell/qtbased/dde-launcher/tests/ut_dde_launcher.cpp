// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_dde_launcher.h"
#include "dbuslauncherframe.h"
#include "dbustartmanager.h"
#include "dbusdock.h"
#include "dbuslauncher.h"
#include "calculate_util.h"
#include "dbusdockinterface.h"
#include "launchersys.h"
#include "dbuslauncherservice.h"

#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusMessage>
#include <QDBusArgument>
#include <QGSettings>
#include <QTest>

/**
 * @brief LauncherUnitTest::testDockDBus 测试任务栏Dbus服务
 */
TEST_F(LauncherUnitTest, testDockDBus)
{
    DBusDock dockInterface(this);
    QVERIFY(dockInterface.isValid());

    uint activeWindow = dockInterface.activeWindow();
    qDebug() << QString("Test dockInterface activeWindow:%1").arg(activeWindow);

    QList<QDBusObjectPath> entries = dockInterface.entries();
    qDebug() << QString("Test dockInterface entrie count:%1").arg(entries.count());

    int displayMode = dockInterface.displayMode();
    qDebug() << QString("Test dockInterface displayMode:%1").arg(displayMode);

    int hideMode = dockInterface.hideMode();
    qDebug() << QString("Test dockInterface hideMode:%1").arg(hideMode);

    int hideState = dockInterface.hideState();
    qDebug() << QString("Test dockInterface hideState:%1").arg(hideState);

    int position = dockInterface.position();
    qDebug() << QString("Test dockInterface position:%1").arg(position);

    quint32 iconSize = dockInterface.iconSize();
    qDebug() << QString("Test dockInterface iconSize:%1").arg(iconSize);

    QRect frontendRect = dockInterface.frontendRect();
    qDebug() << QString("Test dockInterface frontendRect:(%1,%2,%3,%4").arg(frontendRect.left()).arg(frontendRect.top()).arg(frontendRect.right()).arg(frontendRect.bottom());
}

/**
 * @brief LauncherUnitTest::testLauncherDBus 测试启动器Dbus服务
 */
TEST_F(LauncherUnitTest, testLauncherDBus)
{
    DBusLauncher launcherInterface(this);
    QVERIFY(launcherInterface.isValid());

    bool fullscreen = launcherInterface.fullscreen();
    qDebug() << QString("Test launcherInterface fullscreen:%1").arg(fullscreen);

    int displaymode = launcherInterface.displaymode();
    qDebug() << QString("Test launcherInterface displaymode:%1").arg(displaymode);
}

TEST_F(LauncherUnitTest, CalculateUtil)
{
    auto m_calculateUtil = CalculateUtil::instance();
    m_calculateUtil->getAppBoxSize();
    m_calculateUtil->setFullScreen(false);
    m_calculateUtil->setDisplayMode(1);
}

/**
 * @brief LauncherUnitTest::testStartManagerDBus  测试StartManagerDBus
 */
TEST_F(LauncherUnitTest, testStartManagerDBus)
{
    DBusStartManager startManagerInterface(this);
    QVERIFY(startManagerInterface.isValid());

    // 检测是否自启动
    QString appName = QString("deepin-editor");
    QVERIFY(startManagerInterface.IsAutostart(appName));

    // 设置自启动
    startManagerInterface.AddAutostart(appName);

    // 启动应用
    startManagerInterface.Launch(appName);

    // 解除自启动
    startManagerInterface.RemoveAutostart(appName);
}

/**
 * @brief LauncherUnitTest::testDockPos
 * 测试启动器的位置是否正确,有时候启动器不知道在哪里
 *
 */
TEST_F(LauncherUnitTest, testDockPos)
{
    DBusDock dockInterface(this);
    QVERIFY(dockInterface.isValid());

    QRect r = dockInterface.frontendRect();
    qDebug() << "frontendRect:" << r;
    //时尚模式
    if (dockInterface.position() != 1) {
        if (dockInterface.displayMode() == 1) {
            QCOMPARE(r.x(), 0);
        } else {
            QCOMPARE(r.x(), 10);
        }
    } else {
        //dock右侧
        if (dockInterface.displayMode() == 1) {
            QCOMPARE(r.y(), 0);
        } else {
            QCOMPARE(r.y(), 10);
        }
    }

    if (r == QRect(0, 0, 0, 0)) {
        QFAIL("dock pos error");
    }
}

/**
 * @brief LauncherUnitTest::case10_testLauncher
 * 检查Lanucher接口是否正常，Show,Hide,IsVisible和UninstallApp是否正常
 */
TEST_F(LauncherUnitTest, testLauncher)
{
    QDBusInterface inter("com.deepin.dde.Lanucher",
                         "/com/deepin/dde/Lanucher",
                         "com.deepin.dde.Lanucher",
                         QDBusConnection::sessionBus());
    QVERIFY(inter.isValid());

    inter.call("Hide");
    QDBusPendingReply<> replyHide = inter.call("IsVisible");
    replyHide.waitForFinished();
    QTest::qWait(200);

    QDBusPendingReply<> replyShow = inter.call("Show");
    replyShow.waitForFinished();
    QTest::qWait(200);

    QDBusPendingReply<> replyVisible = inter.call("IsVisible");
    replyVisible.waitForFinished();
    QTest::qWait(200);

    QDBusPendingReply<> windowedReply = inter.call("ShowByMode", 0);
    replyVisible.waitForFinished();
    QTest::qWait(200);

    QDBusPendingReply<> fullscreenReply = inter.call("ShowByMode", 1);
    replyVisible.waitForFinished();
    QTest::qWait(200);

    QDBusPendingReply<> toggleReply = inter.call("Toggle");
    toggleReply.waitForFinished();
    QTest::qWait(200);
}

/**
 * @brief LauncherUnitTest::checkSendToDesktop
 * 测试发送到桌面是否正常显示
 */
TEST_F(LauncherUnitTest, checkSendToDesktop)
{
    DBusLauncher launcher(this);
    QVERIFY(launcher.isValid());

    QString appName = "deepin-music";
    launcher.RequestSendToDesktop(appName);
    QTest::qWait(50);
    bool value = launcher.IsItemOnDesktop(appName);
    QCOMPARE(value, true);
    launcher.RequestRemoveFromDesktop(appName);
    QTest::qWait(50);
    value = launcher.IsItemOnDesktop(appName);
    QCOMPARE(value, false);
}

/**
 * @brief LauncherUnitTest::check_gsettings_default
 * 验证启动器的默认配置值是否正确，后续如果还有类似验证，应一并放到这里
 * 1.图标默认大小比例为0.5
 * 2.全屏模式的排序模式：默认为free自由排序模式
 * 3.显示模式：默认为false窗口模式
 */
TEST_F(LauncherUnitTest, check_gsettings_default)
{
    // 服务器环境没有gsetting配置，加检验防止崩溃
    if (!QGSettings::isSchemaInstalled("com.deepin.dde.launcher"))
        return;

    QGSettings setting("com.deepin.dde.launcher", "/com/deepin/dde/launcher/");
    if (setting.keys().contains("appsIconRatio")) {
        double ratio = setting.get("apps-icon-ratio").toDouble();
        QVERIFY(qAbs(ratio - 0.5) < 0.000001);
    }

    // 0 free, 1 category
    if (setting.keys().contains("displayMode")) {
        int ratio = setting.get("display-mode").toInt();
        QCOMPARE(ratio, 0);
    }

    if (setting.keys().contains("fullscreen")) {
        bool ratio = setting.get("fullscreen").toBool();
        QCOMPARE(ratio, false);
    }
}

TEST_F(LauncherUnitTest, check_toggleLauncher)
{
    QDBusInterface launcherBusInter("com.deepin.dde.daemon.Launcher"
                            ,"/com/deepin/dde/daemon/Launcher"
                            ,"org.freedesktop.DBus.Properties"
                            ,QDBusConnection::sessionBus(), this);
    QVERIFY(launcherBusInter.isValid());

    QDBusReply<QVariant> reply = launcherBusInter.call("Get", "com.deepin.dde.daemon.Launcher", "Fullscreen");

    bool fullScreen = QVariant(reply).toBool();
    if (fullScreen == false) {
        launcherBusInter.call("Set", "com.deepin.dde.daemon.Launcher", "Fullscreen", true);
        QTest::qWait(100);

        QDBusReply<QVariant> replyGetFullscreen = launcherBusInter.call("Get", "com.deepin.dde.daemon.Launcher", "Fullscreen");
        fullScreen = QVariant(replyGetFullscreen).toBool();

        QCOMPARE(fullScreen, true);
    } else {
        launcherBusInter.call("Set", "com.deepin.dde.daemon.Launcher", "Fullscreen", false);
        QTest::qWait(100);

        QDBusReply<QVariant> replyGetFullscreen = launcherBusInter.call("Get", "com.deepin.dde.daemon.Launcher", "Fullscreen");
        fullScreen = QVariant(replyGetFullscreen).toBool();

        QCOMPARE(fullScreen, false);
    }

    QDBusPendingReply<CategoryInfoList> categoryInfoListReply = launcherBusInter.call("GetAllCategoryInfos");
    categoryInfoListReply.waitForFinished();

    QDBusPendingReply<FrequencyInfoList> frequencyListReply = launcherBusInter.call("GetAllFrequency");
    frequencyListReply.waitForFinished();

    QDBusPendingReply<InstalledTimeInfoList> installedTimeInfoListReply = launcherBusInter.call("GetAllTimeInstalled");
    installedTimeInfoListReply.waitForFinished();

    QDBusPendingReply<CategoryInfo> categoryInfoReply = launcherBusInter.call("GetCategoryInfo");
    categoryInfoReply.waitForFinished();

    QDBusPendingReply<ItemInfo> getItemInfoReply = launcherBusInter.call("GetItemInfo");
    categoryInfoReply.waitForFinished();

    QDBusPendingReply<> recordFrequency = launcherBusInter.call("RecordFrequency");
    recordFrequency.waitForFinished();

    QDBusPendingReply<> recordRotate = launcherBusInter.call("RecordRate");
    recordRotate.waitForFinished();

    QDBusPendingReply<> reqUninstall = launcherBusInter.call("RequestUninstall", "deepin-editor", false);
    reqUninstall.waitForFinished();

    QDBusPendingReply<> setProxyReply = launcherBusInter.call("SetUseProxy", "deepin-editor", false);
    setProxyReply.waitForFinished();

    QDBusPendingReply<bool> getProxyReply = launcherBusInter.call("GetUseProxy", "deepin-editor");
    getProxyReply.waitForFinished();

    QVERIFY(getProxyReply.argumentAt(0).toBool());

    QDBusPendingReply<> setDisableScaleReply = launcherBusInter.call("SetDisableScaling", "deepin-editor", false);
    setDisableScaleReply.waitForFinished();

    QDBusPendingReply<bool> getScaleReply = launcherBusInter.call("GetDisableScaling", "deepin-editor");
    getScaleReply.waitForFinished();

    QVERIFY(getScaleReply.argumentAt(0).toBool());
}

TEST_F(LauncherUnitTest, check_dbusDockInterface)
{
    DBusDockInterface dockInter;
    QVERIFY(dockInter.isValid());

    qDebug() << "dock rect:" << dockInter.geometry();
}
