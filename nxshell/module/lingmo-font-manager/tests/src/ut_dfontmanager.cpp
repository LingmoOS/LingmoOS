// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontmanagercore.h"
#include "dcopyfilesmanager.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include "signalmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QSignalSpy>

const QString sysDir = QDir::homePath() + "/.local/share/fonts";

namespace {
class TestDfontmanager : public testing::Test
{

protected:
    void SetUp()
    {
        fm = FontManagerCore::instance();
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    FontManagerCore *fm;
};

static QString g_funcname;
void stub_handleInstall()
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_handleUnInstall()
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_doInstall()
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_doUninstall()
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_copyFiles(CopyFontThread::OPType, QStringList &)
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_doCmd(QStringList &)
{
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
void stub_start(void *, const QString &, QIODevice::OpenMode)
{
    qDebug() << __FUNCTION__;
    g_funcname.clear();
    g_funcname = __FUNCTION__;
}
bool stub_waitForFinished(void *, int)
{
    qDebug() << __FUNCTION__;
    g_funcname.clear();
    g_funcname = __FUNCTION__;
    return true;
}
}

TEST_F(TestDfontmanager, checkSetType)
{
    fm->setType(FontManagerCore::Install);
    EXPECT_EQ(fm->m_type, FontManagerCore::Install);
}

TEST_F(TestDfontmanager, checkSetInstallFileList)
{
    QStringList list;
    list << "first" << "second" << "third" << "fouth";
    fm->setInstallFileList(list);
    EXPECT_EQ(fm->m_instFileList.count(), 4);
    EXPECT_EQ(true, fm->m_instFileList.contains("first"));

    list.clear();
    list << "fifth";
    fm->setInstallFileList(list);
    EXPECT_EQ(fm->m_instFileList.count(), 1);
    EXPECT_EQ(true, fm->m_instFileList.contains("fifth"));
}

TEST_F(TestDfontmanager, checkSetUnInstallFile)
{
    QStringList list;
    list << "first" << "second" << "third" << "fouth";
    fm->setUnInstallFile(list);
    EXPECT_EQ(fm->m_uninstFile.count(), 4);
    EXPECT_EQ(true, fm->m_uninstFile.contains("first"));
}


TEST_F(TestDfontmanager, checkRunInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, handleInstall), stub_handleInstall);

    fm->setType(FontManagerCore::Install);
    fm->run();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_handleInstall"));

    fm->setType(FontManagerCore::HalfwayInstall);
    fm->run();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_handleInstall"));

    fm->setType(FontManagerCore::ReInstall);
    fm->run();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_handleInstall"));
}

TEST_F(TestDfontmanager, checkRunUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, handleUnInstall), stub_handleUnInstall);

    fm->setType(FontManagerCore::UnInstall);
    fm->run();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_handleUnInstall"));

    fm->setType(FontManagerCore::DefaultNullType);
    g_funcname.clear();
    fm->run();
    EXPECT_TRUE(g_funcname.isEmpty());
}

TEST_F(TestDfontmanager, checkDoCmdInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, doInstall), stub_doInstall);
    QStringList list;

    fm->setType(FontManagerCore::Install);
    fm->doCmd(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_doInstall"));

    fm->setType(FontManagerCore::HalfwayInstall);
    fm->doCmd(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_doInstall"));

    fm->setType(FontManagerCore::ReInstall);
    fm->doCmd(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_doInstall"));
}

TEST_F(TestDfontmanager, checkDoCmdUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, doUninstall), stub_doUninstall);

    QStringList list;
    fm->setType(FontManagerCore::UnInstall);
    fm->doCmd(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_doUninstall"));

    fm->setType(FontManagerCore::DefaultNullType);
    g_funcname.clear();
    fm->doCmd(list);
    EXPECT_TRUE(g_funcname.isEmpty());
}

TEST_F(TestDfontmanager, checkHandleInstallAndDoInstall)
{
    Stub s;
    s.set(ADDR(DCopyFilesManager, copyFiles), stub_copyFiles);

    QSignalSpy spy(fm, SIGNAL(requestCancelInstall));

    fm->setType(FontManagerCore::Install);
    fm->m_instFileList << "first";
    fm->handleInstall();
    EXPECT_TRUE(spy.count() == 0);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_copyFiles"));

    fm->cancelInstall();
    fm->handleInstall();
    EXPECT_TRUE(spy.count() == 0);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_copyFiles"));
}

TEST_F(TestDfontmanager, checkHandleUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, doCmd), stub_doCmd);
    QStringList list;
    list << "first" << "endl";

    fm->setUnInstallFile(list);
    fm->handleUnInstall();
    EXPECT_EQ(true, fm->m_uninstFile.isEmpty());
    EXPECT_TRUE(g_funcname == QLatin1String("stub_doCmd"));
}

TEST_F(TestDfontmanager, checkDoUnstall)
{
    QSignalSpy spy(fm, SIGNAL(uninstallFontFinished(QStringList)));
    QSignalSpy spys(fm, SIGNAL(uninstallFcCacheFinish()));

    QStringList list;

    QString str = QDir::homePath() + "/.local/share/fonts/Addictype";
    QString filePathOrig = QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf";

    QDir d;
    //新建文件夹并确认创建成功
    d.mkdir(str);
    EXPECT_EQ(true, d.exists());
    QFile::copy(filePathOrig, str + "/Addictype-Regular.otf");

    list << str + "/Addictype-Regular.otf";
    fm->doUninstall(list);
    QDir dd(str);
    EXPECT_EQ(false, dd.exists());

}

TEST_F(TestDfontmanager, checkOnInstallResultInstall)
{
    QSignalSpy spy(fm, SIGNAL(installFinished(int, const QStringList)));

    fm->m_instFileList.clear();
    fm->m_instFileList << "first";
    fm->setType(FontManagerCore::Install);
    fm->setCacheStatus(FontManagerCore::CacheNow);
    fm->onInstallResult("first", "");

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDfontmanager, checkOnInstallResultReInstall)
{
    QSignalSpy spy(fm, SIGNAL(reInstallFinished(int, const QStringList)));

    fm->m_instFileList.clear();
    fm->m_instFileList << "first";
    fm->setType(FontManagerCore::ReInstall);
    fm->setCacheStatus(FontManagerCore::CacheNow);
    fm->onInstallResult("first", "");

    EXPECT_TRUE(spy.count() == 1);

}


TEST_F(TestDfontmanager, checkSetCacheStatus)
{
    fm->setCacheStatus(FontManagerCore::CacheNow);
    EXPECT_EQ(fm->m_CacheStatus, FontManagerCore::Install);
}

TEST_F(TestDfontmanager, checkCancelInstall)
{
    fm->cancelInstall();
    EXPECT_TRUE(fm->m_installCanceled);
}

TEST_F(TestDfontmanager, checkDoCache)
{
    Stub s;
    s.set((void(QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), stub_start);
    s.set(/*(bool(QProcess::*)(int))*/ADDR(QProcess, waitForFinished), stub_waitForFinished);
    QSignalSpy spy(fm, SIGNAL(cacheFinish()));

    fm->doCache();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_waitForFinished"));
    EXPECT_EQ(spy.count(), 1);

    fm->m_type = FontManagerCore::Install;
    fm->m_CacheStatus = FontManagerCore::CacheNow;
    fm->m_installCanceled = false;
    EXPECT_TRUE(fm->needCache());
    delete fm;
//    fm->deleteLater();
}



