// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include "readonlylibarchiveplugin.h"
#include "queries.h"
#include "datamanager.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

class UT_ReadOnlyLibarchivePluginFactory : public QObject, public ::testing::Test
{
public:
    UT_ReadOnlyLibarchivePluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ReadOnlyLibarchivePluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadOnlyLibarchivePluginFactory *m_tester;
};

class UT_ReadOnlyLibarchivePlugin : public QObject, public ::testing::Test
{
public:
    UT_ReadOnlyLibarchivePlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/tar/extract/test.tar";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new ReadOnlyLibarchivePlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadOnlyLibarchivePlugin *m_tester;
};

TEST_F(UT_ReadOnlyLibarchivePluginFactory, initTest)
{

}

TEST_F(UT_ReadOnlyLibarchivePlugin, initTest)
{

}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_list)
{
    EXPECT_EQ(m_tester->list(), PFT_Nomral);
    EXPECT_EQ(m_tester->m_setHasHandlesDirs.isEmpty(), true);
    EXPECT_EQ(m_tester->m_setHasRootDirs.isEmpty(), false);
    EXPECT_EQ(m_tester->m_mapCode.isEmpty(), false);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_testArchive)
{
    EXPECT_EQ(m_tester->testArchive(), PFT_Nomral);
}

bool initializeReader_stub()
{
    return false;
}

void waitForResponse_stub()
{
    return;
}

bool responseSkip_false_stub()
{
    return false;
}

bool responseSkipAll_false_stub()
{
    return false;
}

bool responseOverwriteAll_true_stub()
{
    return true;
}

bool responseCancelled_false_stub()
{
    return false;
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractFiles_001)
{
    Stub stub;
    stub.set(ADDR(LibarchivePlugin, initializeReader), initializeReader_stub);

    QList<FileEntry> files;
    ExtractionOptions options;

    EXPECT_EQ(m_tester->extractFiles(files, options), PFT_Error);
    EXPECT_EQ(m_tester->m_bOverwriteAll, false);
    EXPECT_EQ(m_tester->m_bSkipAll, false);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractFiles_002)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/tar/extract";

    EXPECT_EQ(m_tester->extractFiles(files, options), PFT_Nomral);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractFiles_003)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/tar/extract";

    EXPECT_EQ(m_tester->extractFiles(files, options), PFT_Cancel);
}

bool mkpath_stub(const QString &)
{
    return false;
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractFiles_004)
{
    Stub stub;
    stub.set(ADDR(QDir, mkpath), mkpath_stub);
    QList<FileEntry> files;
    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/tar/extracterror";

    EXPECT_EQ(m_tester->extractFiles(files, options), PFT_Error);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractFiles_005)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), waitForResponse_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), responseCancelled_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), responseSkip_false_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), responseSkipAll_false_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), responseOverwriteAll_true_stub);
    QList<FileEntry> files;
    FileEntry file1;
    file1.isDirectory = false;
    file1.strFullPath = QLatin1String("test.txt");
    files.push_back(file1);
    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/tar/extract";
    m_tester->m_ArchiveEntryCount = 2;

    EXPECT_EQ(m_tester->extractFiles(files, options), PFT_Nomral);

    QFile::remove(options.strTargetPath + "/test.txt");
    QFile::remove(options.strTargetPath + "/test1.txt");
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_addFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Error);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_copyFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    EXPECT_EQ(m_tester->copyFiles(files, options), PFT_Error);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_deleteFiles)
{
    QList<FileEntry> files;
    CompressOptions options;

    EXPECT_EQ(m_tester->deleteFiles(files), PFT_Error);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_addComment)
{
    EXPECT_EQ(m_tester->addComment("comment"), PFT_Error);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_pauseOperation)
{
    m_tester->pauseOperation();
    EXPECT_EQ(m_tester->m_bPause, true);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_continueOperation)
{
    m_tester->continueOperation();
    EXPECT_EQ(m_tester->m_bPause, false);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_doKill)
{
    bool ret = m_tester->doKill();
    EXPECT_EQ(m_tester->m_bPause, false);
    EXPECT_EQ(ret, false);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_initializeReader)
{
    EXPECT_EQ(m_tester->initializeReader(), true);
}

int archive_read_support_filter_all_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_initializeReader_001)
{
    Stub stub;
    stub.set(archive_read_support_filter_all, archive_read_support_filter_all_stub);
    EXPECT_EQ(m_tester->initializeReader(), false);
}

int archive_read_support_format_all_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_initializeReader_002)
{
    Stub stub;
    stub.set(archive_read_support_format_all, archive_read_support_format_all_stub);
    EXPECT_EQ(m_tester->initializeReader(), false);
}

int archive_read_open_filename_stub(struct archive *)
{
    return ARCHIVE_FAILED;
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_initializeReader_003)
{
    Stub stub;
    stub.set(archive_read_open_filename, archive_read_open_filename_stub);
    EXPECT_EQ(m_tester->initializeReader(), false);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_convertCompressionName)
{
    EXPECT_EQ(m_tester->convertCompressionName("gzip_error").isEmpty(), true);
    EXPECT_EQ(m_tester->convertCompressionName("gzip").toStdString(), "GZip");
    EXPECT_EQ(m_tester->convertCompressionName("bzip2").toStdString(), "BZip2");
    EXPECT_EQ(m_tester->convertCompressionName("xz").toStdString(), "XZ");
    EXPECT_EQ(m_tester->convertCompressionName("compress (.Z)").toStdString(), "Compress");
    EXPECT_EQ(m_tester->convertCompressionName("lrzip").toStdString(), "LRZip");
    EXPECT_EQ(m_tester->convertCompressionName("lzip").toStdString(), "LZip");
    EXPECT_EQ(m_tester->convertCompressionName("lz4").toStdString(), "LZ4");
    EXPECT_EQ(m_tester->convertCompressionName("lzop").toStdString(), "lzop");
    EXPECT_EQ(m_tester->convertCompressionName("lzma").toStdString(), "LZMA");
    EXPECT_EQ(m_tester->convertCompressionName("zstd").toStdString(), "Zstandard");
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_updateArchiveData)
{
    UpdateOptions options;
    options.eType = UpdateOptions::Delete;
    FileEntry file1;
    file1.isDirectory = false;
    file1.strFullPath = QLatin1String("test.txt");
    options.listEntry.push_back(file1);
    m_tester->list();
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    int mapFileEntrysizeold = stArchiveData.mapFileEntry.size();
    EXPECT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
    int mapFileEntrysizenew = stArchiveData.mapFileEntry.size();
    EXPECT_EQ(mapFileEntrysizeold, mapFileEntrysizenew + 1);
}

TEST_F(UT_ReadOnlyLibarchivePlugin, test_extractionFlags)
{
    EXPECT_EQ(m_tester->extractionFlags(), 0x0204);
}
