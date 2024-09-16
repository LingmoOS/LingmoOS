// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bz2plugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_LibBzip2InterfaceFactory : public QObject, public ::testing::Test
{
public:
    UT_LibBzip2InterfaceFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibBzip2InterfaceFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibBzip2InterfaceFactory *m_tester;
};

class UT_LibBzip2Interface : public QObject, public ::testing::Test
{
public:
    UT_LibBzip2Interface(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/bz2/test.txt.bz2";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibBzip2Interface(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibBzip2Interface *m_tester;
};


TEST_F(UT_LibBzip2InterfaceFactory, initTest)
{

}

TEST_F(UT_LibBzip2Interface, initTest)
{

}

TEST_F(UT_LibBzip2Interface, test_list)
{
    PluginFinishType eType = m_tester->list();
    EXPECT_EQ(eType, PFT_Nomral);
}

TEST_F(UT_LibBzip2Interface, test_inittestArchive)
{
    PluginFinishType eType = m_tester->testArchive();
    EXPECT_EQ(eType, PFT_Nomral);
}



void waitForResponse_stub()
{
    return;
}

bool responseCancelled_true_stub()
{
    return true;
}

bool responseSkip_true_stub()
{
    return true;
}

bool responseSkipAll_true_stub()
{
    return true;
}

bool responseOverwriteAll_true_stub()
{
    return true;
}

bool responseCancelled_false_stub()
{
    return false;
}

bool responseSkip_false_stub()
{
    return false;
}

bool responseSkipAll_false_stub()
{
    return false;
}

bool responseOverwriteAll_false_stub()
{
    return false;
}

QStringList qString_split_stub(QChar, QString::SplitBehavior, Qt::CaseSensitivity)
{
    return QStringList() << "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
           "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
           "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
}

TEST_F(UT_LibBzip2Interface, test_extractFiles_001)
{
    ExtractionOptions options;
    options.bExistList = false;
    options.bAllExtract = false;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";
    QDir dir(options.strTargetPath);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_true_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);
    EXPECT_EQ(eType, PFT_Nomral);
}

TEST_F(UT_LibBzip2Interface, test_extractFiles_002)
{
    ExtractionOptions options;
    options.bAllExtract = false;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";
    QDir dir(options.strTargetPath);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_true_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);
    EXPECT_EQ(eType, PFT_Cancel);
}

TEST_F(UT_LibBzip2Interface, test_extractFiles_003)
{
    ExtractionOptions options;
    options.bAllExtract = false;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";
    QDir dir(options.strTargetPath);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_true_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);
    EXPECT_EQ(eType, PFT_Cancel);
}

TEST_F(UT_LibBzip2Interface, test_extractFiles_004)
{
    ExtractionOptions options;
    options.bAllExtract = false;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";
    QDir dir(options.strTargetPath);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);
    EXPECT_EQ(eType, PFT_Nomral);
}

TEST_F(UT_LibBzip2Interface, test_extractFiles_005)
{
    ExtractionOptions options;
    options.bAllExtract = false;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/bz2/temp";
    QDir dir(options.strTargetPath);

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    typedef QStringList(QString::*fptr)(QChar, QString::SplitBehavior, Qt::CaseSensitivity) const;
    fptr A_foo = (fptr)(&QString::split);   //获取虚函数地址
    stub.set(A_foo, qString_split_stub);

    PluginFinishType eType = m_tester->extractFiles(QList<FileEntry>(), options);
    EXPECT_EQ(eType, PFT_Cancel);
    dir.removeRecursively();
}

TEST_F(UT_LibBzip2Interface, testpauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibBzip2Interface, test_continueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibBzip2Interface, test_doKill)
{
    m_tester->m_bPause = true;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibBzip2Interface, test_uncompressedFileName_001)
{
    QString str = m_tester->uncompressedFileName();
    EXPECT_EQ(str, "test.txt");
}

TEST_F(UT_LibBzip2Interface, test_uncompressedFileName_002)
{
    m_tester->m_strArchiveName = "test.svgz";
    QString str = m_tester->uncompressedFileName();
    EXPECT_EQ(str, "test.svg");
}
