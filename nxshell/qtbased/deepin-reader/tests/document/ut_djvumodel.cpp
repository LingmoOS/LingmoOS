// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DjVuModel.h"
#include "Model.h"
#include "stub.h"

#include <QFile>

#include <libdjvu/ddjvuapi.h>
#include <gtest/gtest.h>
using namespace deepin_reader;

///**********测试DjVuPage*************/
//class TestDjVuPage : public ::testing::Test
//{
//public:
//    virtual void SetUp();

//    virtual void TearDown();

//protected:
//    DjVuPage *m_tester = nullptr;
//    DjVuDocument *m_djvudocument = nullptr;
//    ddjvu_context_t *m_context = nullptr;
//    ddjvu_document_t *m_document = nullptr;
//};
//void TestDjVuPage::SetUp()
//{
//    const char *programname = "test-deepin-reader";
//    m_context = ddjvu_context_create(programname);
//    if (m_context != nullptr) {
//        std::cout << "chendu nullptr" << std::endl;
//    }
//    QString strPath = UTSOURCEDIR;
//    strPath += "/files/normal.djvu";

//    const char *filename = strPath.toLatin1().data();

//    std::cout << "chendu nullptr1" << strPath.toStdString() << std::endl;
//    m_document = ddjvu_document_create_by_filename_utf8(m_context, filename, FALSE);
//    std::cout << "chendu nullptr2" << std::endl;
//    m_djvudocument = new DjVuDocument(m_context, m_document);

//    ddjvu_pageinfo_t pageinfo;
//    pageinfo.width = 500;
//    pageinfo.height = 600;
//    pageinfo.dpi = 100;
//    pageinfo.rotation = 0;
//    pageinfo.version = 25;

//    m_tester = new DjVuPage(m_djvudocument, 0, pageinfo);
//}

//void TestDjVuPage::TearDown()
//{
//    delete m_djvudocument;
//    delete m_tester;
//}

///***********测试用例*************/
//TEST_F(TestDjVuPage, UT_TestDjVuPage_sizeF_001)
//{
//    EXPECT_TRUE(m_tester->sizeF() == QSizeF(500, 600));
//}

//TEST_F(TestDjVuPage, UT_TestDjVuPage_render_001)
//{
//    QRect slice(0, 0, 50, 60);
//    EXPECT_FALSE(m_tester->render(50, 60, slice).isNull());
//}

//TEST_F(TestDjVuPage, UT_TestDjVuPage_text_001)
//{
//    QRectF rect(0, 0, 50, 10);
//    EXPECT_TRUE(m_tester->text(rect).isEmpty());
//}

//TEST_F(TestDjVuPage, UT_TestDjVuPage_search_001)
//{
//    EXPECT_TRUE(m_tester->search("test", false, false).isEmpty());
//}

/**********测试DjVuDocument*************/
class TestDjVuDocument : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    DjVuDocument *m_tester = nullptr;
};

void TestDjVuDocument::SetUp()
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.djvu";
    deepin_reader::Document::Error error;
    m_tester = DjVuDocument::loadDocument(strPath, error);
}

void TestDjVuDocument::TearDown()
{
    delete m_tester;
}

/**************测试用例****************/
TEST_F(TestDjVuDocument, UT_TestDjVuDocument_pageCount_001)
{
    EXPECT_EQ(m_tester->pageCount(), 192);
}

TEST_F(TestDjVuDocument, UT_TestDjVuDocument_page_001)
{
    Page *page = m_tester->page(0);
    EXPECT_TRUE(page->sizeF().isValid());
    EXPECT_TRUE(m_tester->m_pages.size() == 1);
    delete page;
}

TEST_F(TestDjVuDocument, UT_TestDjVuDocument_saveFilter_001)
{
    EXPECT_TRUE(m_tester->saveFilter().contains(QLatin1String("DjVu (*.djvu *.djv)")));
}

TEST_F(TestDjVuDocument, UT_TestDjVuDocument_saveAs_001)
{
    QString strPathcopy = UTSOURCEDIR;
    strPathcopy += "/files/normalcopy.djvu";

    EXPECT_TRUE(m_tester->saveAs(strPathcopy));
    QFile::remove(strPathcopy);
}

TEST_F(TestDjVuDocument, UT_TestDjVuDocument_save_001)
{
    EXPECT_TRUE(m_tester->save());
}

TEST_F(TestDjVuDocument, UT_TestDjVuDocument_properties_001)
{
    EXPECT_TRUE(m_tester->properties().isEmpty());
}
