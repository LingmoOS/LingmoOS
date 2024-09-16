// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_trashcleanwidget.h"
#include "window/modules/cleaner/trashcleanwidget.h"
#include "../deepin-defender/src/widgets/cleaneritem.h"

#include "gtest/gtest.h"
#include "mock/mock_cleanerdbusinterface.h"

#include <DPushButton>
#include <QTimer>
#include <QThread>
#include <QDir>

DWIDGET_USE_NAMESPACE

const QString LOG_DIR = "PATH/cleanerTestDir/log";
const QString DEB_DIR = "PATH/cleanerTestDir/deb";
const QString TRASH_DIR = "PATH/cleanerTestDir/trash";
const QString HISTORY_DIR = "PATH/cleanerTestDir/history";

const QString APP_TRASH_JSON = "{\"appArray\":[{\"appCachePath\":[\"PATH/cleanerTestDir/app/uninstallNotEmpty/cache\"],\"appConfigPath\":[\"PATH/cleanerTestDir/app/uninstallNotEmpty/config\"],\"appInstallFlag\":false,\"appName\":\"uninstallNotEmpty\",\"appTip\":\"uninstallNotEmpty\",\"comName\":\"uninstallNotEmpty\"},{\"appCachePath\":[],\"appConfigPath\":[],\"appInstallFlag\":false,\"appName\":\"uninstallEmpty\",\"appTip\":\"uninstallEmpty\",\"comName\":\"uninstallEmpty\"},{\"appCachePath\":[\"PATH/cleanerTestDir/app/installNotEmpty/cache\"],\"appConfigPath\":[\"PATH/cleanerTestDir/app/installNotEmpty/config\"],\"appInstallFlag\":true,\"appName\":\"installNotEmpty\",\"appTip\":\"installNotEmpty\",\"comName\":\"installNotEmpty\"},{\"appCachePath\":[],\"appConfigPath\":[],\"appInstallFlag\":true,\"appName\":\"installEmpty\",\"appTip\":\"installEmpty\",\"comName\":\"installEmpty\"}]}";
const QString APP_TRASH_JSON_CLEANED = "{\"appArray\":[{\"appCachePath\":[],\"appConfigPath\":[],\"appInstallFlag\":false,\"appName\":\"uninstallNotEmpty\",\"appTip\":\"uninstallNotEmpty\",\"comName\":\"uninstallNotEmpty\"},{\"appCachePath\":[],\"appConfigPath\":[],\"appInstallFlag\":false,\"appName\":\"uninstallEmpty\",\"appTip\":\"uninstallEmpty\",\"comName\":\"uninstallEmpty\"},{\"appCachePath\":[],\"appConfigPath\":[\"PATH/cleanerTestDir/app/installNotEmpty/config\"],\"appInstallFlag\":true,\"appName\":\"installNotEmpty\",\"appTip\":\"installNotEmpty\",\"comName\":\"installNotEmpty\"},{\"appCachePath\":[],\"appConfigPath\":[],\"appInstallFlag\":true,\"appName\":\"installEmpty\",\"appTip\":\"installEmpty\",\"comName\":\"installEmpty\"}]}";

const QString BROWSER_COOKIES_JSON = "{\"browserCookiesArray\":[{\"appName\":\"browser\",\"appTip\":\"browser\",\"comName\":\"browser\",\"cookiesPath\":[\"PATH/cleanerTestDir/cookies/browser/cookies\"]}]}";

enum CHECK_ITEM_INDEX {
    SYS_CHECK = 0,
    APP_CHECK,
    HISTORY_CHECK,
    COOKIES_CHECK
};

TrashCleanWidgetTest::TrashCleanWidgetTest()
{
}

void TrashCleanWidgetTest::SetUp()
{
}

void TrashCleanWidgetTest::TearDown()
{
}

TEST_F(TrashCleanWidgetTest, Sample)
{
    TrashCleanWidget widget(nullptr);
    QString path = QDir::currentPath();
}

// 垃圾清理-配置页
// 默认初始的检查项配置
// 仅勾选系统垃圾与应用垃圾
TEST_F(TrashCleanWidgetTest, DefaultConfigCheckItem)
{
    TrashCleanWidget widget(nullptr);

    CleanerItem *ci_sys = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(SYS_CHECK));
    CleanerItem *ci_app = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(APP_CHECK));
    CleanerItem *ci_history = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(HISTORY_CHECK));
    CleanerItem *ci_browser = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(COOKIES_CHECK));

    EXPECT_EQ(true, ci_sys->getCheckBoxStatus());
    EXPECT_EQ(true, ci_app->getCheckBoxStatus());
    EXPECT_EQ(false, ci_history->getCheckBoxStatus());
    EXPECT_EQ(false, ci_browser->getCheckBoxStatus());
}

// 扫描文件名称随系统字体变大时，中间变成显示号
TEST_F(TrashCleanWidgetTest, Scan_Filename_Elided)
{
    TrashCleanWidget widget(nullptr);
    // 需要设置一个超长的字符以确何中间被省略
    QString input = APP_TRASH_JSON + APP_TRASH_JSON_CLEANED;
    QString rst = widget.getElidedScanTitle(input);
    EXPECT_TRUE(rst.length() < input.length());
}

// 垃圾清理-配置页
// 默认初始的检查项配置
// 全部不勾选
// 左按钮置灰
TEST_F(TrashCleanWidgetTest, NoneConfigItemChecked)
{
    TrashCleanWidget widget(nullptr);

    CleanerItem *ci_sys = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(SYS_CHECK));
    CleanerItem *ci_app = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(APP_CHECK));
    CleanerItem *ci_history = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(HISTORY_CHECK));
    CleanerItem *ci_browser = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(COOKIES_CHECK));

    DPushButton *leftButton = widget.findChild<DPushButton *>("leftButton");
    EXPECT_EQ(true, leftButton->isEnabled());

    ci_sys->setCheckBoxStatus(false);
    ci_app->setCheckBoxStatus(false);

    // 工作状态变化 流程测试，使用ASSERT_EQ在状态错误时退出用例

    ASSERT_EQ(false, ci_sys->getCheckBoxStatus());
    ASSERT_EQ(false, ci_app->getCheckBoxStatus());
    ASSERT_EQ(false, ci_history->getCheckBoxStatus());
    ASSERT_EQ(false, ci_browser->getCheckBoxStatus());

    ASSERT_EQ(false, leftButton->isEnabled());
}

// 状态切换
// 随状态变化，按钮事件响应，主界面的显示内容切换
// 流程1 ： 扫描-返回
// 流程2 ： 扫描-清理-返回
// 流程3 ： 扫描-清理-返回-再次扫描
TEST_F(TrashCleanWidgetTest, Default_Check_Stage_Change)
{
    TrashCleanWidget widget(nullptr);
    QTimer timer;

    DPushButton *leftButton = widget.findChild<DPushButton *>("leftButton");
    DPushButton *rightButton = widget.findChild<DPushButton *>("rightButton");
    EXPECT_EQ(true, leftButton->isEnabled());
    EXPECT_EQ(false, rightButton->isVisible());

    ASSERT_EQ(TrashCleanWidget::ScanStages::PREPARING, widget.GetCurrentStage());

    // 左键发生点击后，应当立即进入“扫描结束”状态
    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());

    // 未配置接口情况下，扫描大小与内容应当都为0
    ASSERT_EQ(int(0), widget.GetScanedSize());
    ASSERT_EQ(int(0), widget.GetScanedAcount());
    ASSERT_EQ(int(0), widget.GetCleanedSize());
    ASSERT_EQ(int(0), widget.GetCleanedAcount());

    // 流程2
    // 扫描完成后，左键返回到配置页
    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::PREPARING != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::PREPARING, widget.GetCurrentStage());

    // 扫描
    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());

    // 0224 修改用例,内容大小均为0时,右键被disable,仅提供左键以返回
    /*******
    // 清理
    rightButton->click();
    QThread::msleep(200);
    // 由于具体实现，开始清理后，短暂时间内即进入清理完成状态
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::CLEAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::CLEAN_FINISHED, widget.GetCurrentStage());
    // 右键返回配置页
    rightButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::PREPARING != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::PREPARING, widget.GetCurrentStage());
    ********/
    // 内容为空,禁用清理按键
    rightButton->click();
    QThread::msleep(200);
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());

    /*** 待修改,因为右键现在在扫描大小为0时,被禁用了
    // 流程3
    // 扫描
    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());
    // 清理
    rightButton->click();
    while (TrashCleanWidget::ScanStages::CLEAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    ASSERT_EQ(TrashCleanWidget::ScanStages::CLEAN_FINISHED, widget.GetCurrentStage());

    // 左键重新扫描
    leftButton->click();
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());
    ***/
}

// 错误的JSON输入处理
TEST_F(TrashCleanWidgetTest, Mock_Dbus_Interface_error_json_format)
{
    TrashCleanWidget widget(nullptr);
    QTimer timer;

    // 根据当前目录位置，修改绝对路径
    QString path = QDir::currentPath();

    QString replace_str = "PATH";
    QString log_dir = QString(LOG_DIR).replace(QRegularExpression(replace_str), path);
    QString deb_dir = QString(DEB_DIR).replace(QRegularExpression(replace_str), path);
    QString trash_dir = QString(TRASH_DIR).replace(QRegularExpression(replace_str), path);
    QString his_dir = QString(HISTORY_DIR).replace(QRegularExpression(replace_str), path);
    QString app_json = QString(APP_TRASH_JSON).replace(QRegularExpression(replace_str), path);
    QString app_json_cleaned = QString(APP_TRASH_JSON_CLEANED).replace(QRegularExpression(replace_str), path);
    QString cookies_json = QString(BROWSER_COOKIES_JSON).replace(QRegularExpression(replace_str), path);

    MockCleanerDbusInterface dbusInterface;
    EXPECT_CALL(dbusInterface, GetTrashInfoList()).WillRepeatedly(testing::Return(QStringList() << trash_dir)); // 文件4
    EXPECT_CALL(dbusInterface, GetCacheInfoList()).WillRepeatedly(testing::Return(QStringList() << deb_dir)); // 文件2
    EXPECT_CALL(dbusInterface, GetLogInfoList()).WillRepeatedly(testing::Return(QStringList() << log_dir)); // 文件3
    EXPECT_CALL(dbusInterface, GetHistoryInfoList()).WillRepeatedly(testing::Return(QStringList() << his_dir)); // 文件1

    EXPECT_CALL(dbusInterface, GetAppTrashInfoList()).WillRepeatedly(testing::Return("err or"));
    EXPECT_CALL(dbusInterface, GetBrowserCookiesInfoList()).WillRepeatedly(testing::Return("err or")); // 文件1

    widget.setServerInterface(&dbusInterface);

    CleanerItem *ci_sys = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(SYS_CHECK));
    CleanerItem *ci_app = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(APP_CHECK));
    CleanerItem *ci_history = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(HISTORY_CHECK));
    CleanerItem *ci_browser = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(COOKIES_CHECK));
    ci_sys->setCheckBoxStatus(true);
    ci_app->setCheckBoxStatus(true);
    ci_history->setCheckBoxStatus(true);
    ci_browser->setCheckBoxStatus(true);

    DPushButton *leftButton = widget.findChild<DPushButton *>("leftButton");

    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());

    // 总计有15个文件，但是已安装应用的config文件不清理，所以不会计入
    // 此处总共扫描14个文件
    const int fileAcount = 10;
    ASSERT_EQ(int(fileAcount), widget.GetScanedAcount());
    ASSERT_EQ(int(fileAcount * 1024), widget.GetScanedSize());
}

// 引入MOCK的dbus接口，获取大小进行测试
TEST_F(TrashCleanWidgetTest, Mock_Dbus_Interface)
{
    TrashCleanWidget widget(nullptr);
    QTimer timer;

    // 根据当前目录位置，修改绝对路径
    QString path = QDir::currentPath();

    QString replace_str = "PATH";
    QString log_dir = QString(LOG_DIR).replace(QRegularExpression(replace_str), path);
    QString deb_dir = QString(DEB_DIR).replace(QRegularExpression(replace_str), path);
    QString trash_dir = QString(TRASH_DIR).replace(QRegularExpression(replace_str), path);
    QString his_dir = QString(HISTORY_DIR).replace(QRegularExpression(replace_str), path);
    QString app_json = QString(APP_TRASH_JSON).replace(QRegularExpression(replace_str), path);
    QString app_json_cleaned = QString(APP_TRASH_JSON_CLEANED).replace(QRegularExpression(replace_str), path);
    QString cookies_json = QString(BROWSER_COOKIES_JSON).replace(QRegularExpression(replace_str), path);

    MockCleanerDbusInterface dbusInterface;
    EXPECT_CALL(dbusInterface, GetTrashInfoList()).WillRepeatedly(testing::Return(QStringList() << trash_dir)); // 文件4
    EXPECT_CALL(dbusInterface, GetCacheInfoList()).WillRepeatedly(testing::Return(QStringList() << deb_dir)); // 文件2
    EXPECT_CALL(dbusInterface, GetLogInfoList()).WillRepeatedly(testing::Return(QStringList() << log_dir)); // 文件3
    EXPECT_CALL(dbusInterface, GetHistoryInfoList()).WillRepeatedly(testing::Return(QStringList() << his_dir)); // 文件1

    EXPECT_CALL(dbusInterface, GetAppTrashInfoList())
        .Times(testing::AtMost(3))
        .WillOnce(testing::Return(app_json))
        .WillOnce(testing::Return(app_json))
        .WillOnce(testing::Return(app_json_cleaned)); // 文件4
    EXPECT_CALL(dbusInterface, GetBrowserCookiesInfoList()).WillRepeatedly(testing::Return(cookies_json)); // 文件1

    widget.setServerInterface(&dbusInterface);

    CleanerItem *ci_sys = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(SYS_CHECK));
    CleanerItem *ci_app = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(APP_CHECK));
    CleanerItem *ci_history = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(HISTORY_CHECK));
    CleanerItem *ci_browser = widget.findChild<CleanerItem *>(QString("cleanerItem%1").arg(COOKIES_CHECK));
    ci_sys->setCheckBoxStatus(true);
    ci_app->setCheckBoxStatus(true);
    ci_history->setCheckBoxStatus(true);
    ci_browser->setCheckBoxStatus(true);

    DPushButton *leftButton = widget.findChild<DPushButton *>("leftButton");
    DPushButton *rightButton = widget.findChild<DPushButton *>("rightButton");

    // 第一次扫描，注意gmock的查询返回
    leftButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::SCAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::SCAN_FINISHED, widget.GetCurrentStage());

    // 总计有15个文件，但是已安装应用的config文件不清理，所以不会计入
    // 此处总共扫描14个文件
    const int fileAcount = 14;
    ASSERT_EQ(int(fileAcount), widget.GetScanedAcount());
    ASSERT_EQ(int(fileAcount * 1024), widget.GetScanedSize());

    // 返回
    leftButton->click();
    QThread::msleep(200);
    // 第二次扫描，注意gmock的查询返回
    leftButton->click();
    QThread::msleep(200);

    // 对比第二次扫描结果
    ASSERT_EQ(int(fileAcount), widget.GetScanedAcount());
    ASSERT_EQ(int(fileAcount * 1024), widget.GetScanedSize());

    // 清理所有文件，再次扫描
    // 全选，立即清理所有内容
    rightButton->click();
    timer.start(3000);
    while (TrashCleanWidget::ScanStages::CLEAN_FINISHED != widget.GetCurrentStage() && timer.remainingTime()) {
        QThread::msleep(200);
    }
    timer.stop();
    ASSERT_EQ(TrashCleanWidget::ScanStages::CLEAN_FINISHED, widget.GetCurrentStage());

    // 核对删除文件数量
    // 并没有通过MOCK接口真正删除，但是需要比对记录值

    // MOCK接口被调用时计划删除数量
    ASSERT_EQ(fileAcount, dbusInterface.m_deletedFiles.size());

    // 业务类计划删除数量
    ASSERT_EQ(int(fileAcount), widget.GetCleanedAcount());
    ASSERT_EQ(int(fileAcount * 1024), widget.GetCleanedSize());

    // 第三次扫描--清理后重新扫描
    // 注意gmock的查询返回，GetAppTrashInfoList此时返回已被删除列表
    // 已经删除的
    leftButton->click();
    QThread::msleep(200);
    ASSERT_EQ(int(11), widget.GetScanedAcount());
    ASSERT_EQ(int(11 * 1024), widget.GetScanedSize());
}
