// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PDFModel.h"
#include "dpdfannot.h"
#include "dpdfpage.h"
#include "dpdfdoc.h"
#include "stub.h"

#include <gtest/gtest.h>
using namespace deepin_reader;

/********测试PDFAnnotation***********/
class TestPDFAnnotation : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    PDFAnnotation *m_tester = nullptr;
    DPdfTextAnnot *dAnnot = nullptr;
};

void TestPDFAnnotation::SetUp()
{
    dAnnot = new DPdfTextAnnot;
    m_tester = new PDFAnnotation(dAnnot);
    m_tester->disconnect();
}

void TestPDFAnnotation::TearDown()
{
    delete m_tester;
    delete dAnnot;
}

/***********测试用例***********/
TEST_F(TestPDFAnnotation, UT_PDFAnnotation_boundary_001)
{
    dAnnot->m_type = DPdfAnnot::AText;
    dAnnot->m_rect = QRectF();
    EXPECT_EQ(m_tester->boundary().size(), 1);
}

TEST_F(TestPDFAnnotation, UT_PDFAnnotation_contents_001)
{
    dAnnot->m_text = "test";
    EXPECT_TRUE(m_tester->contents() == "test");

    m_tester->m_dannotation = nullptr;
    EXPECT_TRUE(m_tester->contents().isEmpty());
}

TEST_F(TestPDFAnnotation, UT_PDFAnnotation_type_001)
{
    dAnnot->m_type = DPdfAnnot::AText;
    EXPECT_TRUE(m_tester->type() == DPdfAnnot::AText);

    m_tester->m_dannotation = nullptr;
    EXPECT_TRUE(m_tester->type() == -1);
}

TEST_F(TestPDFAnnotation, UT_PDFAnnotation_ownAnnotation_001)
{
    EXPECT_TRUE(m_tester->ownAnnotation() == dAnnot);
}

/**********测试PDFPage*************/
class TestPDFPage : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    PDFPage *m_tester = nullptr;
//    DPdfTextAnnot *dAnnot = nullptr;
    QMutex *m_docMutex = nullptr;
//    DPdfDocHandler *m_docHandler = nullptr;
    DPdfPage *m_page = nullptr;
};

void TestPDFPage::SetUp()
{
    m_page = new DPdfPage(nullptr, 0, 96, 96);
    m_docMutex = new QMutex;
    m_tester = new PDFPage(m_docMutex, m_page);
}

void TestPDFPage::TearDown()
{
    delete m_tester;
    delete m_docMutex;
    delete m_page;
}

/********桩函数**************/
QSizeF sizeF_stub()
{
    return QSizeF(100, 200);
}

QImage image_stub(void *, int, int, QRect slice)
{
    return QImage(slice.width(), slice.height(), QImage::Format_ARGB32);
}

QList<DPdfAnnot *> empty_links_stub()
{
    QList<DPdfAnnot *> links;
    return links;
}
static DPdfLinkAnnot *g_linkAnnots = nullptr;
QList<DPdfAnnot *> links_stub()
{
    QList<DPdfAnnot *> links;
    g_linkAnnots = new DPdfLinkAnnot;
    g_linkAnnots->setRectF(QRectF(0, 0, 20, 20));
    g_linkAnnots->setPage(1, 30, 40);
    g_linkAnnots->setFilePath("t/e/s/t");
    links.append(g_linkAnnots);
    return links;
}

QList<DPdfAnnot *> links_stub2()
{
    QList<DPdfAnnot *> links;
    g_linkAnnots = new DPdfLinkAnnot;
    g_linkAnnots->setRectF(QRectF(0, 0, 20, 20));
    g_linkAnnots->setPage(1, 30, 40);
    g_linkAnnots->setUrl("http://www.123.com");
    links.append(g_linkAnnots);
    return links;
}

QList<DPdfAnnot *> widgets_stub()
{
    QList<DPdfAnnot *> widgets;
    g_linkAnnots = new DPdfLinkAnnot();
    widgets.append(g_linkAnnots);
    return widgets;
}

QString text_stub(const QRectF &)
{
    return "  test\r\n";
}

void allTextLooseRects_stub(void *, int &charCount, QStringList &texts, QVector<QRectF> &rects)
{
    charCount = 2;
    texts.append("first");
    texts.append("second");
    rects.append(QRectF(0, 0, 20, 10));
    rects.append(QRectF(0, 20, 20, 10));
}

QVector<PageSection> search_stub(const QString &, bool, bool)
{
    PageLine line;
    line.rect = QRectF(0, 0, 12.3, 12.3);

    PageSection section;
    section.append(line);

    QVector<PageSection> results;
    results.append(section);
    return results;
}

static DPdfTextAnnot *g_textAnnots = nullptr;
QList<DPdfAnnot *> annots_stub()
{
    QList<DPdfAnnot *> dannots;
    g_textAnnots = new DPdfTextAnnot();
    dannots.append(g_textAnnots);
    return dannots;
}

static DPdfHightLightAnnot *g_HightLightAnnots = nullptr;
DPdfAnnot *createHightLightAnnot_stub(const QList<QRectF> &, QString, QColor)
{
    g_HightLightAnnots = new DPdfHightLightAnnot;
    return g_HightLightAnnots;
}

bool removeAnnot_stub(DPdfAnnot *dAnnot)
{
    delete dAnnot;
    return true;
}
static QString g_funcName;
bool updateTextAnnot_stub(DPdfAnnot *, QString, QPointF)
{
    g_funcName = __FUNCTION__;
    return true;
}
bool updateHightLightAnnot_stub(DPdfAnnot *, QColor, QString)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool contains_stub(const DPdfAnnot &)
{
    return true;
}

DPdfAnnot *createTextAnnot_stub(QPointF, QString)
{
    g_textAnnots = new DPdfTextAnnot;
    return g_textAnnots;
}
/**********测试用例************/
TEST_F(TestPDFPage, UT_PDFPage_sizeF_001)
{
    Stub s;
    s.set(ADDR(DPdfPage, sizeF), sizeF_stub);
    EXPECT_TRUE(m_tester->sizeF() == QSizeF(100, 200));
}

TEST_F(TestPDFPage, UT_PDFPage_render_001)
{
    Stub s;
    s.set(ADDR(DPdfPage, image), image_stub);
    int width = 1000;
    int height = 2000;
    QRect slice(0, 0, 100, 200);
    EXPECT_TRUE(m_tester->render(width, height, slice).width() == 100);
}

TEST_F(TestPDFPage, UT_PDFPage_getLinkAtPoint_001)
{
    Stub s;
    s.set(ADDR(DPdfPage, links), empty_links_stub);
    EXPECT_TRUE(m_tester->getLinkAtPoint(QPointF(0, 0)).page == -1);

    s.reset(ADDR(DPdfPage, links));
    s.set(ADDR(DPdfPage, links), links_stub);
    Link link = m_tester->getLinkAtPoint(QPointF(5, 5));
    if (g_linkAnnots) {
        delete g_linkAnnots;
        g_linkAnnots = nullptr;
    }
    EXPECT_TRUE(link.page == 2);
    EXPECT_TRUE(link.left == 30.0);
    EXPECT_TRUE(link.top == 40.0);
    EXPECT_TRUE(link.urlOrFileName == "t/e/s/t");

    s.reset(ADDR(DPdfPage, links));
    s.set(ADDR(DPdfPage, links), links_stub2);
    link = m_tester->getLinkAtPoint(QPointF(5, 5));
    if (g_linkAnnots) {
        delete g_linkAnnots;
        g_linkAnnots = nullptr;
    }
    EXPECT_TRUE(link.page == 2);
    EXPECT_TRUE(link.left == 30.0);
    EXPECT_TRUE(link.top == 40.0);
    EXPECT_TRUE(link.urlOrFileName == "http://www.123.com");
}

TEST_F(TestPDFPage, UT_PDFPage_hasWidgetAnnots_001)
{
    Stub s;
    s.set(ADDR(DPdfPage, widgets), widgets_stub);

    EXPECT_TRUE(m_tester->hasWidgetAnnots());
    if (g_linkAnnots) {
        delete g_linkAnnots;
        g_linkAnnots = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_text_001)
{
    Stub s;
    s.set(static_cast<QString(DPdfPage::*)(const QRectF &)>(ADDR(DPdfPage, text)), text_stub);

    EXPECT_TRUE(m_tester->text(QRectF(0, 0, 10, 10)) == "test");
}

TEST_F(TestPDFPage, UT_PDFPage_words_001)
{
    m_tester->m_wordLoaded = true;
    Word w;
    w.text = "test";
    w.boundingBox = QRectF(0, 0, 20, 10);
    m_tester->m_words.append(w);
    EXPECT_TRUE(m_tester->words().size() == 1);

    Stub s;
    s.set(static_cast<void(DPdfPage::*)(int &, QStringList &, QVector<QRectF> &)>(ADDR(DPdfPage, allTextLooseRects)), allTextLooseRects_stub);
    m_tester->m_wordLoaded = false;
    m_tester->words();
    EXPECT_TRUE(m_tester->words().size() == 3);
}

TEST_F(TestPDFPage, UT_PDFPage_search_001)
{
    typedef QVector<PageSection> (*searchPtr)(const QString &, bool, bool);
    Stub s;
    s.set((searchPtr)ADDR(DPdfPage, search), search_stub);

    EXPECT_TRUE(m_tester->search(QString("test"), false, false).size() == 1);
}

TEST_F(TestPDFPage, UT_PDFPage_annotations_001)
{
    Stub s;
    s.set(static_cast<QList<DPdfAnnot *>(DPdfPage::*)()>(ADDR(DPdfPage, annots)), annots_stub);

    QList< Annotation * > annotlist = m_tester->annotations();
    EXPECT_TRUE(annotlist.size() == 1);
    qDeleteAll(annotlist);
    annotlist.clear();
    if (g_textAnnots) {
        delete g_textAnnots;
        g_linkAnnots = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_addHighlightAnnotation_001)
{
    Stub s;
    s.set(static_cast<DPdfAnnot*(DPdfPage::*)(const QList<QRectF> &, QString, QColor)>(ADDR(DPdfPage, createHightLightAnnot)), createHightLightAnnot_stub);
    QList<QRectF> boundaries{QRectF(0, 0, 1.1, 2.2)};
    QString text("test");
    QColor color(Qt::red);
    Annotation *annot = m_tester->addHighlightAnnotation(boundaries, text, color);
    EXPECT_TRUE(annot != nullptr);
    delete annot;
    if (g_HightLightAnnots) {
        delete g_HightLightAnnots;
        g_HightLightAnnots = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_removeAnnotation_001)
{
    PDFAnnotation *annotation = nullptr;
    EXPECT_FALSE(m_tester->removeAnnotation(annotation));

    Stub s;
    s.set(static_cast<bool(DPdfPage::*)(DPdfAnnot *)>(ADDR(DPdfPage, removeAnnot)), removeAnnot_stub);
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    annotation = new PDFAnnotation(dAnnot);
    EXPECT_TRUE(m_tester->removeAnnotation(annotation));
    delete dAnnot;
}

TEST_F(TestPDFPage, UT_PDFPage_updateAnnotation_001)
{
    PDFAnnotation *annotation = nullptr;
    QString text("test");
    QColor color(Qt::red);
    EXPECT_FALSE(m_tester->updateAnnotation(annotation, text, color));

    Stub s;
    s.set(static_cast<bool(DPdfPage::*)(DPdfAnnot *, QString txt, QPointF)>(ADDR(DPdfPage, updateTextAnnot)), updateTextAnnot_stub);
    s.set(ADDR(QList<DPdfAnnot *>, contains), contains_stub);
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    dAnnot->m_type = DPdfAnnot::AText;
    annotation = new PDFAnnotation(dAnnot);

    EXPECT_TRUE(m_tester->updateAnnotation(annotation, text, color));
    EXPECT_TRUE(g_funcName == "updateTextAnnot_stub");
    if (annotation) {
        delete annotation;
        annotation = nullptr;
    }
    if (dAnnot) {
        delete dAnnot;
        dAnnot = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_updateAnnotation_002)
{
    Stub s;
    s.set(static_cast<bool(DPdfPage::*)(DPdfAnnot *, QColor, QString)>(ADDR(DPdfPage, updateHightLightAnnot)), updateHightLightAnnot_stub);
    s.set(ADDR(QList<DPdfAnnot *>, contains), contains_stub);
    QString text("test");
    QColor color(Qt::red);
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    dAnnot->m_type = DPdfAnnot::AHighlight;
    PDFAnnotation *annotation = new PDFAnnotation(dAnnot);

    EXPECT_TRUE(m_tester->updateAnnotation(annotation, text, color));
    EXPECT_TRUE(g_funcName == "updateHightLightAnnot_stub");
    if (annotation) {
        delete annotation;
        annotation = nullptr;
    }
    if (dAnnot) {
        delete dAnnot;
        dAnnot = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_addIconAnnotation_001)
{
    QRectF rect(0, 0, 1.1, 2.2);
    QString text("test");
    DPdfPage *tmpPage = m_tester->m_page;
    m_tester->m_page = nullptr;
    Annotation *annot = m_tester->addIconAnnotation(rect, text);
    EXPECT_TRUE(annot == nullptr);

    m_tester->m_page = tmpPage;
    Stub s;
    s.set(static_cast<DPdfAnnot*(DPdfPage::*)(QPointF pos, QString)>(ADDR(DPdfPage, createTextAnnot)), createTextAnnot_stub);

    annot = m_tester->addIconAnnotation(rect, text);
    EXPECT_TRUE(annot != nullptr);
    delete annot;
    if (g_textAnnots) {
        delete g_textAnnots;
        g_textAnnots = nullptr;
    }
}

TEST_F(TestPDFPage, UT_PDFPage_moveIconAnnotation_001)
{
    QRectF rect(0, 0, 1.1, 2.2);
    QString text("test");
    DPdfPage *tmpPage = m_tester->m_page;
    m_tester->m_page = nullptr;
    PDFAnnotation *annotation = nullptr;
    EXPECT_TRUE(m_tester->moveIconAnnotation(annotation, rect) == nullptr);

    m_tester->m_page = tmpPage;
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    annotation = new PDFAnnotation(dAnnot);
    Stub s;
    s.set(static_cast<bool(DPdfPage::*)(DPdfAnnot *, QString txt, QPointF)>(ADDR(DPdfPage, updateTextAnnot)), updateTextAnnot_stub);
    EXPECT_TRUE(m_tester->moveIconAnnotation(annotation, rect) != nullptr);
    EXPECT_TRUE(g_funcName == "updateTextAnnot_stub");
    if (annotation) {
        delete annotation;
        annotation = nullptr;
    }
    if (dAnnot) {
        delete dAnnot;
        dAnnot = nullptr;
    }
}

/**********测试PDFDocument***********/
class TestPDFDocument : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    PDFDocument *m_tester = nullptr;
//    DPdfTextAnnot *dAnnot = nullptr;
//    QMutex *m_docMutex = nullptr;
//    DPdfDocHandler *m_docHandler = nullptr;
    DPdfDoc *m_document = nullptr;
};
void TestPDFDocument::SetUp()
{
    m_document = new DPdfDoc("test.pdf", "");
    m_tester = new PDFDocument(m_document);
}

void TestPDFDocument::TearDown()
{
    delete m_tester;
}

/*********桩函数**********/
int pageCount_stub()
{
    return 1;
}

static DPdfPage *g_dpdfpage;
DPdfPage *page_stub(void *, int i, qreal xRes, qreal yRes)
{
    g_dpdfpage = new DPdfPage(nullptr, i, xRes, yRes);
    return g_dpdfpage;
}

bool DPdfPage_isValid_stub()
{
    return true;
}

QString label_stub(int)
{
    return "test";
}

bool save_stub()
{
    g_funcName = __FUNCTION__;
    return false;
}

bool saveAs_stub(const QString &)
{
    g_funcName = __FUNCTION__;
    return false;
}

DPdfDoc::Outline outline_stub(qreal, qreal)
{
    DPdfDoc::Section sec;
    sec.nIndex = 0;
    sec.offsetPointF = QPointF(0, 0);
    sec.title = "first";
    DPdfDoc::Section sec1;
    sec1.nIndex = 0;
    sec1.offsetPointF = QPointF(0, 0);
    sec1.title = "first";
    sec1.children.append(sec);
    DPdfDoc::Outline cOutline;
    cOutline.append(sec1);

    return cOutline;
}

DPdfDoc::Properies proeries_stub()
{
    DPdfDoc::Properies properies;
    properies.insert("Version", "1");
    properies.insert("Title", "test");
    properies.insert("Creator", "cd");
    return properies;
}

DPdfDoc::Status status_SUCCESS_stub()
{
    return DPdfDoc::SUCCESS;
}

DPdfDoc::Status status_PASSWORDERROR_stub()
{
    return DPdfDoc::PASSWORD_ERROR;
}

/*********测试用例**********/
TEST_F(TestPDFDocument, UT_PDFDocument_pageCount_001)
{
    Stub s;
    s.set(ADDR(DPdfDoc, pageCount), pageCount_stub);

    EXPECT_EQ(m_tester->pageCount(), 1);
}

TEST_F(TestPDFDocument, UT_PDFDocument_page_001)
{
    EXPECT_TRUE(m_tester->page(0) == nullptr);

    Stub s;
    s.set(ADDR(DPdfDoc, page), page_stub);

    EXPECT_TRUE(m_tester->page(0) == nullptr);
    if (g_dpdfpage) {
        delete g_dpdfpage;
        g_dpdfpage = nullptr;
    }

    s.set(ADDR(DPdfPage, isValid), DPdfPage_isValid_stub);
    Page *page = m_tester->page(0);
    EXPECT_TRUE(page != nullptr);
    delete page;
    if (g_dpdfpage) {
        delete g_dpdfpage;
        g_dpdfpage = nullptr;
    }
}

TEST_F(TestPDFDocument, UT_PDFDocument_label_001)
{
    Stub s;
    s.set(ADDR(DPdfDoc, label), label_stub);

    EXPECT_TRUE(m_tester->label(0) == "test");
}

TEST_F(TestPDFDocument, UT_PDFDocument_saveFilter_001)
{
    EXPECT_TRUE(m_tester->saveFilter().first() == "Portable document format (*.pdf)");
}

TEST_F(TestPDFDocument, UT_PDFDocument_save_001)
{
    Stub s;
    s.set(ADDR(DPdfDoc, save), save_stub);
    m_tester->save();
    EXPECT_TRUE(g_funcName == "save_stub");
}

TEST_F(TestPDFDocument, UT_PDFDocument_saveAs_001)
{
    Stub s;
    s.set(ADDR(DPdfDoc, saveAs), saveAs_stub);
    EXPECT_FALSE(m_tester->saveAs(""));
    EXPECT_TRUE(g_funcName == "saveAs_stub");
}

TEST_F(TestPDFDocument, UT_PDFDocument_outline_001)
{
    Section sec;
    sec.nIndex = 0;
    sec.offsetPointF = QPointF(0, 0);
    sec.title = "first";

    m_tester->m_outline.append(sec);
    EXPECT_EQ(m_tester->outline().size(), 1);
    EXPECT_EQ(m_tester->outline().first().children.size(), 0);

    Stub s;
    s.set(ADDR(DPdfDoc, outline), outline_stub);
    m_tester->m_outline.clear();
    EXPECT_EQ(m_tester->outline().first().children.size(), 1);
}

TEST_F(TestPDFDocument, UT_PDFDocument_properties_001)
{
    Section sec;
    sec.nIndex = 0;
    sec.offsetPointF = QPointF(0, 0);
    sec.title = "first";

    m_tester->m_fileProperties.insert("Version", "1");
    EXPECT_EQ(m_tester->properties().size(), 1);
    EXPECT_TRUE(m_tester->properties().value("Version") == "1");

    Stub s;
    s.set(ADDR(DPdfDoc, proeries), proeries_stub);
    m_tester->m_fileProperties.clear();
    EXPECT_EQ(m_tester->properties().size(), 3);
}

TEST_F(TestPDFDocument, UT_PDFDocument_loadDocument_001)
{
    QString filePath("test.pdf");
    QString password;
    Document::Error error = Document::NoError;

    EXPECT_TRUE(m_tester->loadDocument(filePath, password, error) == nullptr);
    EXPECT_TRUE(error == Document::FileError);

    Stub s;
    s.set(ADDR(DPdfDoc, status), status_SUCCESS_stub);
    PDFDocument *pdfdocument = m_tester->loadDocument(filePath, password, error);
    EXPECT_TRUE(pdfdocument != nullptr);
    EXPECT_TRUE(error == Document::NoError);
    if (pdfdocument) {
        delete pdfdocument;
        pdfdocument = nullptr;
    }

    s.reset(ADDR(DPdfDoc, status));
    s.set(ADDR(DPdfDoc, status), status_PASSWORDERROR_stub);
    EXPECT_TRUE(m_tester->loadDocument(filePath, password, error) == nullptr);
    EXPECT_TRUE(error == Document::NeedPassword);

    password = "123";
    EXPECT_TRUE(m_tester->loadDocument(filePath, password, error) == nullptr);
    EXPECT_TRUE(error == Document::WrongPassword);
}
