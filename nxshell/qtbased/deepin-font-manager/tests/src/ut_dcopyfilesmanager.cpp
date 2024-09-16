// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcopyfilesmanager.h"

#include "fontmanagercore.h"
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>
#include <QThreadPool>
#include <QSignalSpy>
#include <QDir>
#include <QDebug>

namespace {
class TestDCopyFilesManager : public testing::Test
{

protected:
    void SetUp()
    {
        fmd = new DCopyFilesManager();
    }
    void TearDown()
    {
        delete fmd;
    }

    // Some expensive resource shared by all tests.
    DCopyFilesManager *fmd;
};

QString stub_getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath)
{
    Q_UNUSED(inPath);
    Q_UNUSED(srcPath);
    Q_UNUSED(targetPath);

    return QString();

}

void stub_return()
{
}

static QString g_funcname;
bool stub_copy(void *, const QString &)
{
    g_funcname = __FUNCTION__;
    return true;
}
bool stub_removeRecursively()
{
    g_funcname = __FUNCTION__;
    return true;
}
bool stub_mkpath(const QString &)
{
    g_funcname = __FUNCTION__;
    return true;
}
void stub_run()
{
    g_funcname = __FUNCTION__;
}
}

TEST_F(TestDCopyFilesManager, checkRunInstall)
{
    CopyFontThread *t = new CopyFontThread(CopyFontThread::INSTALL, 0);

    QSignalSpy spy(t, SIGNAL(fileInstalled(const QString &, const QString &)));

    t->m_srcFiles.clear();
    t->run();

    Stub s;
    s.set(ADDR(DCopyFilesManager, getTargetPath), stub_getTargetPath);
    t->appendFile("first");
    DCopyFilesManager::m_installCanceled = false;
    t->run();
    EXPECT_TRUE(t->m_targetFiles.count() == 1);
    EXPECT_TRUE(spy.count() == 1) << spy.count();
    delete  t;
}

TEST_F(TestDCopyFilesManager, checkRunExport)
{
    Stub s;
    s.set((bool(QFile::*)(const QString &))ADDR(QFile, copy), stub_copy);

    CopyFontThread *t = new CopyFontThread(CopyFontThread::EXPORT, 0);
    t->appendFile("first");
    t->run();
    t->deleteLater();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_copy"));
}


TEST_F(TestDCopyFilesManager, checkCopyFilesNULL)
{
    QStringList list;
    fmd->m_type = CopyFontThread::INVALID;
    fmd->copyFiles(CopyFontThread::INSTALL, list);
    EXPECT_TRUE(fmd->m_type == CopyFontThread::INVALID);
}

TEST_F(TestDCopyFilesManager, checkCopyFilesLess)
{
    QStringList filelist;
    filelist << "first";

    Stub s;
    s.set(ADDR(DCopyFilesManager, getTargetPath), stub_getTargetPath);
    typedef void (*fptr)();
    fptr CopyFontThread_run = (fptr)(&CopyFontThread::run);
    s.set(CopyFontThread_run, stub_run);

    fmd->m_type = CopyFontThread::INSTALL;
    fmd->copyFiles(CopyFontThread::INVALID, filelist);
    EXPECT_TRUE(fmd->m_type == CopyFontThread::INVALID);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_run"));
}

//getTargetPath
TEST_F(TestDCopyFilesManager, checkGetTargetPath)
{
    Stub s;
    s.set(ADDR(QDir, mkpath), stub_mkpath);

    QString bstr;
    QString astr;
    QString familyname;

    familyname = fmd->getTargetPath("1|2|3", astr, bstr);
    EXPECT_TRUE(familyname == "2");

    familyname = fmd->getTargetPath("1|2/|3", astr, bstr);
    EXPECT_TRUE(familyname == "2-");

    familyname = fmd->getTargetPath("1||3", astr, bstr);
    EXPECT_TRUE(familyname == "");
}

TEST_F(TestDCopyFilesManager, checkDeleteFiles)
{
    Stub s;
    s.set(ADDR(QDir, removeRecursively), stub_removeRecursively);
    s.set(ADDR(QDir, mkpath), stub_mkpath);

    QStringList filelist;
    filelist << "1|2|3";

    fmd->deleteFiles(filelist, false);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_removeRecursively"));
}


TEST_F(TestDCopyFilesManager, getPool)
{
    QStringList strlist;
    strlist << "123" << "1234";
    fmd->sortFontList(strlist);
    EXPECT_TRUE(strlist.first() == "1234");
}








