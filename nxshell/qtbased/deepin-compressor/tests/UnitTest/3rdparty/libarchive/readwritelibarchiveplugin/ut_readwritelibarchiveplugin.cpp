// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gtest/src/stub.h>
#include "readwritelibarchiveplugin.h"

#include <QMimeDatabase>
#include <QFileInfo>


Q_DECLARE_METATYPE(KPluginMetaData)

class UT_ReadWriteLibarchivePluginFactory : public QObject, public ::testing::Test
{
public:
    UT_ReadWriteLibarchivePluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ReadWriteLibarchivePluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadWriteLibarchivePluginFactory *m_tester;
};

class UT_ReadWriteLibarchivePlugin : public QObject, public ::testing::Test
{
public:
    UT_ReadWriteLibarchivePlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/tar/compress/test.tar";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new ReadWriteLibarchivePlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ReadWriteLibarchivePlugin *m_tester;
};

TEST_F(UT_ReadWriteLibarchivePluginFactory, initTest)
{

}

TEST_F(UT_ReadWriteLibarchivePlugin, initTest)
{

}

TEST_F(UT_ReadWriteLibarchivePlugin, test_addFiles_001)
{
    QList<FileEntry> files;
    FileEntry file1;
    file1.strFullPath = _UTSOURCEDIR;
    file1.strFullPath += "/test_sources/tar/compress/test.txt";
    file1.qSize = 4;
    files.push_back(file1);
    CompressOptions options;
    QString strArchive = _UTSOURCEDIR;
    strArchive += "/test_sources/tar/compress/test.tar";
    QFile::remove(strArchive);
    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Nomral);
    QFile::remove(strArchive);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_addFiles_002)
{
    QList<FileEntry> files;
    FileEntry file1;
    file1.strFullPath = _UTSOURCEDIR;
    file1.strFullPath += "/test_sources/tar/compress/dir";
    file1.isDirectory = true;
    file1.qSize = 4;
    files.push_back(file1);
    CompressOptions options;
    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Nomral);
    QFile::remove(m_tester->m_strArchiveName);
}

bool deleteEntry_stub(const QList<FileEntry> &)
{
    return true;
}

bool renameEntry_stub(const QList<FileEntry> &)
{
    return true;
}


static bool g_initializeReader_result;
bool initializeReader_stub()
{
    return g_initializeReader_result;
}

static bool g_initializeWriter_result;
bool initializeWriter_stub()
{
    return g_initializeWriter_result;
}

void finish_stub(const bool)
{
    return;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_deleteFiles_001)
{
    EXPECT_EQ(m_tester->deleteFiles(QList<FileEntry>()), PFT_Error);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_deleteFiles_002)
{
    Stub stub;
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeReader), initializeReader_stub);

    g_initializeReader_result = false;
    QList<FileEntry> files;
    FileEntry file;
    file.strFullPath = "test.txt";
    files.push_back(file);

    EXPECT_EQ(m_tester->deleteFiles(files), PFT_Error);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_deleteFiles_003)
{
    Stub stub;
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeReader), initializeReader_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeWriter), initializeWriter_stub);

    g_initializeReader_result = true;
    g_initializeWriter_result = false;
    QList<FileEntry> files;
    FileEntry file;
    file.strFullPath = "test.txt";
    files.push_back(file);

    EXPECT_EQ(m_tester->deleteFiles(files), PFT_Error);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_deleteFiles_004)
{
    Stub stub;
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeReader), initializeReader_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeWriter), initializeWriter_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, deleteEntry), deleteEntry_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, finish), finish_stub);

    g_initializeReader_result = true;
    g_initializeWriter_result = true;
    QList<FileEntry> files;
    FileEntry file;
    file.strFullPath = "test.txt";
    files.push_back(file);

    EXPECT_EQ(m_tester->deleteFiles(files), PFT_Nomral);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_renameFiles)
{
    Stub stub;
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeReader), initializeReader_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, initializeWriter), initializeWriter_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, renameEntry), renameEntry_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, finish), finish_stub);

    g_initializeReader_result = true;
    g_initializeWriter_result = true;
    QList<FileEntry> files;
    FileEntry file;
    file.strFullPath = "test.txt";
    file.strAlias = "test1.txt";
    files.push_back(file);

    EXPECT_EQ(m_tester->renameFiles(files), PFT_Nomral);
}

bool open_stub(QIODevice::OpenMode flags)
{
    return false;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriter_001)
{
    typedef bool (*fptr)(QIODevice::OpenMode);
    fptr QSaveFile_open = (fptr)(&QSaveFile::open);   //获取虚函数地址
    Stub stub;
    stub.set(QSaveFile_open, open_stub);

    EXPECT_EQ(m_tester->initializeWriter(), false);
}

static int g_archive_filter_code_result;
int archive_filter_code_stub(struct archive *, int)
{
    return g_archive_filter_code_result;
}

int archive_write_add_filter_stub(struct archive *)
{
    return ARCHIVE_OK;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_001)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_gzip, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_GZIP;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_002)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_bzip2, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_BZIP2;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_003)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_xz, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_XZ;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_004)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_lzma, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_LZMA;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_005)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_compress, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_COMPRESS;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_006)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_lzip, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_LZIP;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_007)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_lzop, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_LZOP;
    EXPECT_EQ(m_tester->initializeWriterFilters(), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_008)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_lrzip, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_LRZIP;
    EXPECT_EQ(m_tester->initializeWriterFilters(), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_009)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_lz4, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_LZ4;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_010)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);
    stub.set(archive_write_add_filter_none, archive_write_add_filter_stub);

    g_archive_filter_code_result = ARCHIVE_FILTER_NONE;
    EXPECT_EQ(m_tester->initializeWriterFilters(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeWriterFilters_011)
{
    Stub stub;
    stub.set(archive_filter_code, archive_filter_code_stub);

    g_archive_filter_code_result = 18;
    EXPECT_EQ(m_tester->initializeWriterFilters(), false);
}

int archive_write_set_options_stub(struct archive *, const char *)
{
    return ARCHIVE_OK;
}

int archive_write_set_filter_option_stub(struct archive *, const char *, const char *, const char *)
{
    return ARCHIVE_OK;
}

int archive_write_set_passphrase_stub(struct archive *, const char *)
{
    return ARCHIVE_OK;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_001)
{
    Stub stub;
    stub.set(archive_write_add_filter_gzip, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.gz";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_002)
{
    Stub stub;
    stub.set(archive_write_add_filter_bzip2, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.bz2";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_003)
{
    Stub stub;
    stub.set(archive_write_add_filter_xz, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.xz";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_004)
{
    Stub stub;
    stub.set(archive_write_add_filter_lzma, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.lzma";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_005)
{
    Stub stub;
    stub.set(archive_write_add_filter_compress, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.z";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_006)
{
    Stub stub;
    stub.set(archive_write_add_filter_lzip, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.lz";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_007)
{
    Stub stub;
    stub.set(archive_write_add_filter_lzop, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.lzo";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_008)
{
    Stub stub;
    stub.set(archive_write_add_filter_lrzip, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.lrz";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_009)
{
    Stub stub;
    stub.set(archive_write_add_filter_lz4, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.lz4";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_010)
{
    Stub stub;
    stub.set(archive_write_add_filter_none, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_initializeNewFileWriterFilters_011)
{
    Stub stub;
    stub.set(archive_write_add_filter_gzip, archive_write_add_filter_stub);
    stub.set(archive_write_set_options, archive_write_set_options_stub);
    stub.set(archive_write_set_filter_option, archive_write_set_filter_option_stub);
    stub.set(archive_write_set_passphrase, archive_write_set_passphrase_stub);

    CompressOptions options;
    options.iCompressionLevel = 1;
    options.strPassword = "123";
    m_tester->m_strArchiveName = "1.tar.gzip";
    EXPECT_EQ(m_tester->initializeNewFileWriterFilters(options), true);
}

int archive_write_fail_stub(struct archive *)
{
    return ARCHIVE_OK;
}

int archive_write_close_stub(struct archive *)
{
    return ARCHIVE_OK;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_finish_001)
{
    Stub stub;
    stub.set(archive_write_fail, archive_write_fail_stub);

    m_tester->finish(false);
    EXPECT_EQ(m_tester->m_tempFile.fileName().isEmpty(), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_finish_002)
{
    Stub stub;
    stub.set(archive_write_close, archive_write_close_stub);

    m_tester->finish(true);
    EXPECT_EQ(m_tester->m_tempFile.fileName().isEmpty(), true);
}

bool link_stub(const QString &, const QString &)
{
    return false;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeFileTodestination)
{
    Stub stub;
    stub.set((bool(QFile::*)(const QString &))ADDR(QFile, link), link_stub);

    QString strSourceFileFullPath = _UTSOURCEDIR;
    strSourceFileFullPath += "/test_sources/tar/compress/dir";
    QString strDestination = _UTSOURCEDIR;
    strDestination += "/test_sources/tar/compress";
    EXPECT_EQ(m_tester->writeFileTodestination(strSourceFileFullPath, strDestination, "", 10), false);
}

la_ssize_t archive_read_data_stub(struct archive *, void *, size_t)
{
    static int i = 10;
    return i--;
}

la_ssize_t  archive_write_data_stub(struct archive *, const void *, size_t)
{
    return 1;
}

int archive_errno_stub(struct archive *)
{
    return ARCHIVE_OK;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_copyDataFromSourceAdd)
{
    Stub stub;
    stub.set(archive_read_data, archive_read_data_stub);
    stub.set(archive_write_data, archive_write_data_stub);
    stub.set(archive_errno, archive_errno_stub);
    m_tester->copyDataFromSourceAdd(nullptr, nullptr, 10);
    EXPECT_EQ(m_tester->m_currentAddFilesSize, 55);
}

static int g_archive_write_header_result;
int archive_write_header_stub(struct archive *,  struct archive_entry *)
{
    return g_archive_write_header_result;
}

void copyDataFromSource_stub(struct archive *, struct archive *, const qlonglong &)
{
    return;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryDelete_001)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSource), copyDataFromSource_stub);

    g_archive_write_header_result = ARCHIVE_OK;
    EXPECT_EQ(m_tester->writeEntryDelete(nullptr, 10), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryDelete_002)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSource), copyDataFromSource_stub);

    g_archive_write_header_result = ARCHIVE_FATAL;
    EXPECT_EQ(m_tester->writeEntryDelete(nullptr, 10), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryDelete_003)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSource), copyDataFromSource_stub);

    g_archive_write_header_result = ARCHIVE_WARN;
    EXPECT_EQ(m_tester->writeEntryDelete(nullptr, 10), true);
}

void copyDataFromSourceAdd_stub(struct archive *source, struct archive *dest, const qlonglong &totalsize)
{
    return;
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryAdd_001)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSourceAdd), copyDataFromSourceAdd_stub);

    g_archive_write_header_result = ARCHIVE_OK;
    EXPECT_EQ(m_tester->writeEntryAdd(nullptr, 10), true);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryAdd_002)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSourceAdd), copyDataFromSourceAdd_stub);

    g_archive_write_header_result = ARCHIVE_FATAL;
    EXPECT_EQ(m_tester->writeEntryAdd(nullptr, 10), false);
}

TEST_F(UT_ReadWriteLibarchivePlugin, test_writeEntryAdd_003)
{
    Stub stub;
    stub.set(archive_write_header, archive_write_header_stub);
    stub.set(ADDR(ReadWriteLibarchivePlugin, copyDataFromSourceAdd), copyDataFromSourceAdd_stub);

    g_archive_write_header_result = ARCHIVE_WARN;
    EXPECT_EQ(m_tester->writeEntryAdd(nullptr, 10), true);
}
