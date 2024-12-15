// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libminizipplugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QDebug>

#define MAX_FILENAME 512

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_LibminizipPluginFactory : public QObject, public ::testing::Test
{
public:
    UT_LibminizipPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibminizipPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibminizipPluginFactory *m_tester;
};

class UT_LibminizipPlugin : public QObject, public ::testing::Test
{
public:
    UT_LibminizipPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/crx/test.crx";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibminizipPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibminizipPlugin *m_tester;
};


TEST_F(UT_LibminizipPluginFactory, initTest)
{

}

TEST_F(UT_LibminizipPlugin, initTest)
{

}

TEST_F(UT_LibminizipPlugin, test_list)
{
    PluginFinishType eFinishType = m_tester->list();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibminizipPlugin, test_testArchive)
{
    PluginFinishType eFinishType = m_tester->testArchive();
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);
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

TEST_F(UT_LibminizipPlugin, test_extractFiles_001)
{
    QList<FileEntry> files;
    ExtractionOptions options;
    options.bAllExtract = true;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/crx/temp";

    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

    PluginFinishType eFinishType = m_tester->extractFiles(files, options);
    bool bResult = (eFinishType == PFT_Nomral) ? true : false;
    EXPECT_EQ(bResult, true);

    QDir dir(options.strTargetPath);
    dir.removeRecursively();
}

TEST_F(UT_LibminizipPlugin, test_extractFiles_002)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();
    if (stData.listRootEntry.count() > 0) {
        QList<FileEntry> files;
        ExtractionOptions options;
        files << stData.listRootEntry[0];
        options.bAllExtract = false;
        options.strTargetPath = _UTSOURCEDIR;
        options.strTargetPath += "/test_sources/zip/extract/temp";

        Stub stub;
        stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
        stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
        stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
        stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);

        PluginFinishType eFinishType = m_tester->extractFiles(files, options);
        bool bResult = (eFinishType == PFT_Nomral) ? true : false;
        EXPECT_EQ(bResult, true);

        QDir dir(options.strTargetPath);
        dir.removeRecursively();
    }
}

TEST_F(UT_LibminizipPlugin, test_pauseOperation)
{
    m_tester->m_bPause = false;
    m_tester->pauseOperation();
    bool bResult = (m_tester->m_bPause == true) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibminizipPlugin, test_continueOperation)
{
    m_tester->m_bPause = true;
    m_tester->continueOperation();
    bool bResult = (m_tester->m_bPause == false) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibminizipPlugin, test_doKill)
{
    m_tester->m_bPause = true;
    m_tester->m_bCancel = false;
    m_tester->doKill();
    bool bResult = (m_tester->m_bPause == false && m_tester->m_bCancel == true) ? true : false;
    EXPECT_EQ(bResult, true);
}

TEST_F(UT_LibminizipPlugin, test_handleArchiveData)
{
    unzFile zipfile = unzOpen(QFile::encodeName(m_tester->m_strArchiveName).constData());
    bool bResult = m_tester->handleArchiveData(zipfile);
    EXPECT_EQ(bResult, true);
    unzClose(zipfile);
}

TEST_F(UT_LibminizipPlugin, test_getSelFiles)
{
    m_tester->list();
    ArchiveData stData = DataManager::get_instance().archiveData();
    FileEntry entry;
    if (stData.listRootEntry.count() > 0)
        entry = stData.listRootEntry[0];

    QStringList listFiles = m_tester->getSelFiles(QList<FileEntry>() << entry);

    bool bResult = false;
    if (listFiles.count() > 0 && listFiles[0] == entry.strFullPath) {
        bResult = true;
    }

    EXPECT_EQ(bResult, true);
}
