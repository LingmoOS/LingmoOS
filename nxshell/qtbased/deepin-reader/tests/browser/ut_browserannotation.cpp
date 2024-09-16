// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserAnnotation.h"
#include "DocSheet.h"
#include "SheetRenderer.h"
#include "BrowserPage.h"
#include "SheetBrowser.h"
#include "dpdfannot.h"
#include "stub.h"

#include <gtest/gtest.h>
using namespace deepin_reader;

/*************桩函数***************/
static DPdfTextAnnot *g_dAnnot;
static QList<Annotation *> getAnnotations_stub(int)
{
    g_dAnnot = new DPdfTextAnnot;
    g_dAnnot->m_text = "test";
    Annotation *annot = new PDFAnnotation(g_dAnnot);
    QList<Annotation *> list;
    list.append(annot);
    return list;
}
/********测试BrowserAnnotation***********/
class TestBrowserAnnotation : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    DocSheet *m_docsheet = nullptr;
//    SheetBrowser *m_sheetbrowser = nullptr;
    BrowserPage *m_browserpage = nullptr;
    BrowserAnnotation *m_tester = nullptr;
    QList<deepin_reader::Annotation *> m_annots;
    deepin_reader::Document *m_document = nullptr;
//    DPdfTextAnnot *dAnnot = nullptr;
};
void TestBrowserAnnotation::SetUp()
{
    Stub s;
    s.set(ADDR(SheetRenderer, getAnnotations), getAnnotations_stub);

    QRectF rect(4.67, 1.67, 26.67, 26.67);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    m_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
//    m_sheetbrowser = new  SheetBrowser(m_docsheet);
    m_browserpage = new BrowserPage(m_docsheet->m_browser, 0, m_docsheet);
    Document::Error error;
    m_document = DocumentFactory::getDocument(m_docsheet->fileType(), strPath, "", "", nullptr, error);
    m_docsheet->renderer()->m_pages.append(m_document->page(0));
    m_annots = m_docsheet->renderer()->getAnnotations(0);
    qInfo() << "chendu" << m_annots;
    m_tester = new BrowserAnnotation(m_browserpage, m_annots.first()->boundary().first(), m_annots.first(), m_browserpage->m_scaleFactor);
}

void TestBrowserAnnotation::TearDown()
{
    delete m_tester;
    delete g_dAnnot;
    delete m_docsheet;
//    delete m_sheetbrowser;
    delete m_browserpage;
    qDeleteAll(m_annots);
    m_annots.clear();
    delete m_document;
}

/*********测试用例**********/
TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_annotationType_001)
{
    EXPECT_EQ(m_tester->annotationType(), 1);
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_annotationText_001)
{
    EXPECT_FALSE(m_tester->annotationText().isEmpty());
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_boundingRect_001)
{
    m_tester->m_rect = QRectF(10, 20, 30, 40);
    m_tester->m_scaleFactor = 0.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->boundingRect().x(), 5.0));
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_paint_001)
{
    m_tester->m_drawSelectRect = true;
    QPainter painter;
    QStyleOptionGraphicsItem option;
    m_tester->paint(&painter, &option);
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_annotation_001)
{
    EXPECT_TRUE(m_tester->annotation() == m_tester->m_annotation);
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_isSame_001)
{
    EXPECT_TRUE(m_tester->isSame(m_annots.first()));
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_setDrawSelectRect_001)
{
    m_tester->setDrawSelectRect(true);
    EXPECT_TRUE(m_tester->m_drawSelectRect);
    PDFAnnotation *tmp = dynamic_cast<PDFAnnotation *>(m_tester->m_annotation);
    tmp->m_dannotation->m_type = DPdfAnnot::AnnotType::AHighlight;
    m_tester->setDrawSelectRect(true);
    EXPECT_FALSE(m_tester->m_drawSelectRect);
}

TEST_F(TestBrowserAnnotation, UT_BrowserAnnotation_setScaleFactor_001)
{
    PDFAnnotation *tmp = dynamic_cast<PDFAnnotation *>(m_tester->m_annotation);
    m_tester->m_annotation = nullptr;
    m_tester->m_scaleFactor = 1.0;
    m_tester->setScaleFactor(2.5);
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_scaleFactor, 1.0));

    m_tester->m_annotation = static_cast<Annotation *>(tmp);
    m_tester->setScaleFactor(2.5);
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_scaleFactor, 2.5));
}
