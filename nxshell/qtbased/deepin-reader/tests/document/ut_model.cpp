// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Model.h"
#include "PDFModel.h"
#include "DjVuModel.h"
#include "stub.h"

#include <QProcess>
#include <QDir>

#include <gtest/gtest.h>
using namespace deepin_reader;

/***********桩函数**********/
PDFDocument *loadpdfDocument_stub(const QString &, const QString &, deepin_reader::Document::Error &error)
{
    error = Document::FileError;
    return nullptr;
}

DjVuDocument *loaddjvuDocument_stub(const QString &, deepin_reader::Document::Error &error)
{
    error = Document::FileError;
    return nullptr;
}

bool copy_stub(void *, const QString &)
{
    return true;
}

void start_stub(const QString &, QProcess::OpenMode)
{
}

bool waitForStarted_true_stub(int)
{
    return true;
}

bool waitForStarted_false_stub(int)
{
    return false;
}

bool waitForFinished_true_stub(int)
{
    return true;
}

bool waitForFinished_false_stub(int)
{
    return false;
}

bool exists_stub()
{
    return true;
}

/***********测试用例***********/
TEST(UT_DocumentFactory_getDocument, UT_DocumentFactory_getDocument_001)
{
    int fileType = Dr::Unknown;
    QString filePath = "test.txt";
    QString convertedFileDir = "";
    QString password = "";
    QProcess **pprocess = nullptr;
    deepin_reader::Document::Error error;
    deepin_reader::Document *pdocument = nullptr;

    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, pprocess, error);
    EXPECT_EQ(pdocument, nullptr);
}

TEST(UT_DocumentFactory_getDocument, UT_DocumentFactory_getDocument_002)
{
    Stub s;
    s.set(ADDR(PDFDocument, loadDocument), loadpdfDocument_stub);

    int fileType = Dr::PDF;
    QString filePath = "test.pdf";
    QString convertedFileDir;
    QString password;
    QProcess **pprocess = nullptr;
    deepin_reader::Document::Error error = Document::NoError;
    deepin_reader::Document *pdocument = nullptr;

    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, pprocess, error);
    EXPECT_EQ(pdocument, nullptr);
    EXPECT_EQ(error, Document::FileError);
}

TEST(UT_DocumentFactory_getDocument, UT_DocumentFactory_getDocument_003)
{
    Stub s;
    s.set(ADDR(DjVuDocument, loadDocument), loaddjvuDocument_stub);

    int fileType = Dr::DJVU;
    QString filePath = "test.djvu";
    QString convertedFileDir;
    QString password;
    QProcess **pprocess = nullptr;
    deepin_reader::Document::Error error = Document::NoError;
    deepin_reader::Document *pdocument = nullptr;

    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, pprocess, error);
    EXPECT_EQ(pdocument, nullptr);
    EXPECT_EQ(error, Document::FileError);
}


TEST(UT_DocumentFactory_getDocument, UT_DocumentFactory_getDocument_004)
{
    int fileType = Dr::DOCX;
    QString filePath = "test.docx";
    QString convertedFileDir = QCoreApplication::applicationDirPath();
    QString password;
    QProcess p;
    QProcess *process = nullptr;
    deepin_reader::Document::Error error = Document::NoError;
    deepin_reader::Document *pdocument = nullptr;

    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_EQ(pdocument, nullptr);
    EXPECT_EQ(error, Document::ConvertFailed);

    process = &p;
    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_EQ(pdocument, nullptr);
    EXPECT_EQ(error, Document::ConvertFailed);

    Stub s;
    s.set(static_cast<bool(QFile::*)(const QString &)>(ADDR(QFile, copy)), copy_stub);
    s.set(static_cast<void(QProcess::*)(const QString &, QProcess::OpenMode)>(ADDR(QProcess, start)), start_stub);
    Stub s1;

    s1.set(ADDR(QProcess, waitForStarted), waitForStarted_false_stub);
    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_TRUE(pdocument == nullptr);
    EXPECT_TRUE(process == nullptr);
    EXPECT_EQ(error, Document::ConvertFailed);

    s1.reset(ADDR(QProcess, waitForStarted));
    s1.set(ADDR(QProcess, waitForStarted), waitForStarted_true_stub);
    s1.set(ADDR(QProcess, waitForFinished), waitForFinished_false_stub);
    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_TRUE(pdocument == nullptr);
    EXPECT_TRUE(process == nullptr);
    EXPECT_EQ(error, Document::ConvertFailed);

    s1.reset(ADDR(QProcess, waitForFinished));
    s1.set(ADDR(QProcess, waitForFinished), waitForFinished_true_stub);
    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_TRUE(pdocument == nullptr);
    EXPECT_TRUE(process == nullptr);
    EXPECT_EQ(error, Document::ConvertFailed);

    s1.set(static_cast<bool(QDir::*)()const>(ADDR(QDir, exists)), exists_stub);
    s1.set(static_cast<bool(QFile::*)()const>(ADDR(QFile, exists)), exists_stub);
    s1.set(ADDR(PDFDocument, loadDocument), loadpdfDocument_stub);
    pdocument = DocumentFactory::getDocument(fileType, filePath, convertedFileDir, password, &process, error);
    EXPECT_TRUE(pdocument == nullptr);
    EXPECT_TRUE(process == nullptr);
    EXPECT_EQ(error, Document::FileError);
}
