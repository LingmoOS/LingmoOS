// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusObject.h"
#include "MainWindow.h"
#include "stub.h"

#include <QWidget>

#include <gtest/gtest.h>

/********测试DBusObject***********/
class TestDBusObject : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    DBusObject *m_tester = nullptr;
};

void TestDBusObject::SetUp()
{
    m_tester = DBusObject::instance();
}

void TestDBusObject::TearDown()
{
    m_tester->destory();
}

/*************桩函数**************/
static QString g_funcName;
bool registerService_stub(const QString &)
{
    return true;
}

bool registerService_stub2(const QString &)
{
    return false;
}

bool registerObject_stub(const QString &, QObject *, QDBusConnection::RegisterOptions)
{
    return true;
}

bool connect_stub(const QString &, const QString &, const QString &, const QString &, QObject *, const char *)
{
    return true;
}

QDBusMessage callWithArgumentList_stub(QDBus::CallMode, const QString, const QList<QVariant> &)
{
    return QDBusMessage();
}

bool unregisterService_stub(const QString &)
{
    return true;
}

void addFile_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

static void show_stub()
{
    g_funcName = __FUNCTION__;
}

static MainWindow *g_mainWindow;
static MainWindow *createWindow_stub()
{
    QStringList filePathList;
    g_mainWindow = new MainWindow(filePathList);
    return g_mainWindow;
}
/***********测试用例***********/
TEST_F(TestDBusObject, UT_DBusObject_registerOrNotify_001)
{
    Stub s;
    s.set(ADDR(QDBusConnection, registerService), registerService_stub);
    s.set(static_cast<bool(QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions)>(ADDR(QDBusConnection, registerObject)), registerObject_stub);
    s.set(static_cast<bool(QDBusConnection::*)(const QString &, const QString &, const QString &, const QString &, QObject *, const char *)>(ADDR(QDBusConnection, connect)), registerObject_stub);

    QStringList arguments;
    EXPECT_TRUE(m_tester->registerOrNotify(arguments));
}

TEST_F(TestDBusObject, UT_DBusObject_registerOrNotify_002)
{
    Stub s;
    s.set(ADDR(QDBusConnection, registerService), registerService_stub2);
    s.set(ADDR(QDBusAbstractInterface, callWithArgumentList), callWithArgumentList_stub);

    QStringList arguments;
    EXPECT_FALSE(m_tester->registerOrNotify(arguments));
}

TEST_F(TestDBusObject, UT_DBusObject_unRegister_001)
{
    Stub s;
    s.set(ADDR(QDBusConnection, unregisterService), unregisterService_stub);

    m_tester->unRegister();
}

TEST_F(TestDBusObject, UT_DBusObject_blockShutdown_001)
{
    Stub s;
    s.set(ADDR(QDBusAbstractInterface, callWithArgumentList), callWithArgumentList_stub);

    m_tester->m_isBlockShutdown = true;
    m_tester->blockShutdown();
    EXPECT_TRUE(m_tester->m_blockShutdownInterface == nullptr);


    m_tester->m_isBlockShutdown = false;
    m_tester->blockShutdown();
    EXPECT_FALSE(m_tester->m_blockShutdownInterface == nullptr);
}

TEST_F(TestDBusObject, UT_DBusObject_unBlockShutdown_001)
{
    m_tester->m_isBlockShutdown = true;
    m_tester->unBlockShutdown();
    EXPECT_FALSE(m_tester->m_isBlockShutdown);
}

TEST_F(TestDBusObject, UT_DBusObject_handleFiles_001)
{
    Stub s;
    s.set(ADDR(MainWindow, addFile), addFile_stub);
    s.set(ADDR(QWidget, show), show_stub);
    s.set(static_cast<MainWindow*(*)(QStringList)>(ADDR(MainWindow, createWindow)), createWindow_stub);

    m_tester->m_isBlockShutdown = true;
    QStringList filePathList;
    m_tester->handleFiles(filePathList);
    EXPECT_FALSE(g_funcName == "addFile_stub");
    delete g_mainWindow;
}

TEST_F(TestDBusObject, UT_DBusObject_handleFiles_002)
{
    Stub s;
    s.set(ADDR(MainWindow, addFile), addFile_stub);
    s.set(ADDR(QWidget, show), show_stub);
    s.set(static_cast<MainWindow*(*)(QStringList)>(ADDR(MainWindow, createWindow)), createWindow_stub);

    m_tester->m_isBlockShutdown = true;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    QStringList filePathList = {strPath};
    m_tester->handleFiles(filePathList);
    EXPECT_FALSE(g_funcName == "show_stub");
    delete g_mainWindow;
}

