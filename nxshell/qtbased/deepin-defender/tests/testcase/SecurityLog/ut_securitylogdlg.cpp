// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_securitylogdlg.h"

#include "mock/mock_securitylogdatainterface.h"
#include "window/modules/securitylog/securitylogadaptorinterface.h"
#include "window/modules/securitylog/securitylogdialog.h"
#include "../deepin-defender/src/window/modules/common/common.h"

#include "gtest/gtest.h"

#include <QApplication>
#include <QDebug>

DWIDGET_USE_NAMESPACE

#define CMD_TEMPLATE "INSERT INTO SecurityLog (datetimes, type, info) VALUES ('%1 12:12:12', '%2', '%3')"

SecurityLogDlgTest::SecurityLogDlgTest()
{
}

void SecurityLogDlgTest::SetUpTestCase()
{
    // 数据库插入记录
    MockSecurityLogDataInterface mockInterface;
    QString info = "do not care, only test data";

    // 打开数据库
    ASSERT_TRUE(mockInterface.GetDatabase().open());
    QSqlQuery query(mockInterface.GetDatabase());

    // 删除所有数据
    ASSERT_TRUE(query.exec("delete from SecurityLog"));

    // 在数据库中插入数据
    QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString insertData;
    // 添加数据

    // 插入: TODAY OVERALL,3条记录
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(today).arg(SECURITY_LOG_TYPE_HOME).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(today).arg(SECURITY_LOG_TYPE_HOME).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(today).arg(SECURITY_LOG_TYPE_HOME).arg(info)));

    // 插入: -2days protection,cleanup 各三条
    insertData = QDate::currentDate().addDays(-2).toString("yyyy-MM-dd");
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_PROTECTION).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_PROTECTION).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_PROTECTION).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_CLEANER).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_CLEANER).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_CLEANER).arg(info)));

    // 插入: -5days tools,basic setting 各三条
    insertData = QDate::currentDate().addDays(-5).toString("yyyy-MM-dd");
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_BASIC).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_BASIC).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_BASIC).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_TOOL).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_TOOL).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_TOOL).arg(info)));

    // 插入: -15days tools,basic setting 各三条
    insertData = QDate::currentDate().addDays(-15).toString("yyyy-MM-dd");
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_ANTIAV).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_ANTIAV).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_ANTIAV).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_SAFY).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_SAFY).arg(info)));
    ASSERT_TRUE(query.exec(QString(CMD_TEMPLATE).arg(insertData).arg(SECURITY_LOG_TYPE_SAFY).arg(info)));

    mockInterface.GetDatabase().close();
}

void SecurityLogDlgTest::TearDownTestCase()
{
}

TEST_F(SecurityLogDlgTest, Sample)
{
    MockSecurityLogDataInterface mockInterface;
    SecurityLogAdaptorInterface *dataInterface = &mockInterface;
    SecurityLogDialog dlg(nullptr, dataInterface);
}

TEST_F(SecurityLogDlgTest, Switch_Pages)
{
    MockSecurityLogDataInterface mockInterface;
    SecurityLogAdaptorInterface *dataInterface = &mockInterface;
    SecurityLogDialog dlg(nullptr, dataInterface);
    // 默认选中了TODAY ALL,仅有三条记录,一页
    EXPECT_EQ(1, dlg.GetPageShowIndex());

    dlg.doBeforePage();
    EXPECT_EQ(1, dlg.GetPageShowIndex());

    dlg.doAfterPage();
    EXPECT_EQ(1, dlg.GetPageShowIndex());
}

TEST_F(SecurityLogDlgTest, Query_Log_Amount)
{
    MockSecurityLogDataInterface mockInterface;
    SecurityLogAdaptorInterface *dataInterface = &mockInterface;
    SecurityLogDialog dlg(nullptr, dataInterface);
    // 默认选中了TODAY ALL,仅有三条记录
    // 总数0
    EXPECT_EQ(int(0), dlg.GetLogCount());
    // 本页显示0
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 指定类别
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_PROTECTION);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 指定类别
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_HOME);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 3天内的全部数量
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ALL);
    dlg.doLogDateFilterChanged(LAST_DATE_THREE);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 一周内全部数量
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ALL);
    dlg.doLogDateFilterChanged(LAST_DATE_SEVEN);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());
    dlg.doAfterPage();
    EXPECT_EQ(int(0), dlg.GetRowCount());

    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_BASIC);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 三十天内全部数量
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ALL);
    dlg.doLogDateFilterChanged(LAST_DATE_MONTH);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());
    dlg.doAfterPage();
    dlg.doAfterPage();
    EXPECT_EQ(int(0), dlg.GetRowCount());
}

TEST_F(SecurityLogDlgTest, Delete_Log)
{
    MockSecurityLogDataInterface mockInterface;
    SecurityLogAdaptorInterface *dataInterface = &mockInterface;
    SecurityLogDialog dlg(nullptr, dataInterface);

    // 删除日志
    // 通过发送按键事件触发
    // 默认当日所有日志
    dlg.confirmClearLog();
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 3天内
    dlg.doLogDateFilterChanged(LAST_DATE_THREE);
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_PROTECTION);
    dlg.confirmClearLog();
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ALL);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_CLEANER);
    dlg.confirmClearLog();
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());

    // 7天内
    dlg.doLogDateFilterChanged(LAST_DATE_SEVEN);
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_BASIC);
    dlg.confirmClearLog();
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_TOOL);
    dlg.confirmClearLog();

    // 本月内
    dlg.doLogDateFilterChanged(LAST_DATE_MONTH);
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ANTIAV);
    dlg.confirmClearLog();
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_SAFY);
    dlg.confirmClearLog();

    // 所有日志内容已删除
    dlg.doLogDateFilterChanged(LAST_DATE_MONTH);
    dlg.doLogTypeFilterChanged(SECURITY_LOG_TYPE_ALL);
    EXPECT_EQ(int(0), dlg.GetLogCount());
    EXPECT_EQ(int(0), dlg.GetRowCount());
}

TEST_F(SecurityLogDlgTest, Delegate_Type_Name)
{
    SecurityLogTableDelegate delegate;
    EXPECT_EQ("type error", delegate.getLogTypeName(0));
    EXPECT_EQ("type error", delegate.getLogTypeName(8));

    EXPECT_EQ("Overall checking", delegate.getLogTypeName(SECURITY_LOG_TYPE_HOME));
    EXPECT_EQ("Virus scan", delegate.getLogTypeName(SECURITY_LOG_TYPE_ANTIAV));
    EXPECT_EQ("Protection", delegate.getLogTypeName(SECURITY_LOG_TYPE_PROTECTION));
    EXPECT_EQ("Cleanup", delegate.getLogTypeName(SECURITY_LOG_TYPE_CLEANER));
    EXPECT_EQ("Tools", delegate.getLogTypeName(SECURITY_LOG_TYPE_TOOL));
    EXPECT_EQ("Basic settings", delegate.getLogTypeName(SECURITY_LOG_TYPE_BASIC));
    EXPECT_EQ("Security settings", delegate.getLogTypeName(SECURITY_LOG_TYPE_SAFY));
}
