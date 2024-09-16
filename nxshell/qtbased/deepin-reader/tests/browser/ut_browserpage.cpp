// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserPage.h"
#include "PageRenderThread.h"
#include "BrowserAnnotation.h"
#include "PDFModel.h"
#include "dpdfannot.h"
#include "SheetRenderer.h"
#include "BrowserWord.h"
#include "stub.h"

#include <DApplicationHelper>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QPainter>
#include <QSignalSpy>
#include <gtest/gtest.h>
DGUI_USE_NAMESPACE
/********测试BrowserPage***********/
class TestBrowserPage : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    DocSheet *m_docsheet = nullptr;
    BrowserPage *m_tester = nullptr;
};

void TestBrowserPage::SetUp()
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    m_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    m_tester = new BrowserPage(m_docsheet->m_browser, 0, m_docsheet);
}

void TestBrowserPage::TearDown()
{
    delete m_docsheet;
    delete m_tester;
}

/**********桩函数*************/
static QString g_funcName;
static void render_stub(const double &, const Dr::Rotation &, const bool &, const bool &)
{
    g_funcName = __FUNCTION__;
}

void drawPixmap_stub(int x, int y, const QPixmap &pm)
{

}

void drawRect_stub(const QRect &)
{
    g_funcName = __FUNCTION__;
}

bool clearImageTasks_stub(DocSheet *sheet, BrowserPage *page, int pixmapId)
{
    return true;
}

void appendTask_stub(DocPageNormalImageTask)
{
    g_funcName = __FUNCTION__;
}

void appendTask_stub(DocPageAnnotationTask)
{
    g_funcName = __FUNCTION__;
}

void appendTask_stub(DocPageSliceImageTask)
{
    g_funcName = __FUNCTION__;
}

void appendTask_stub(DocPageWordTask)
{
    g_funcName = __FUNCTION__;
}
void scaleWords_stub(bool)
{

}

void scaleAnnots_stub(bool)
{

}

QImage getImage_stub(int, int, int, const QRect &)
{
    g_funcName = __FUNCTION__;
    return QImage(20, 20, QImage::Format_RGB32);
}

static bool isSelected_stub()
{
    return true;
}

static QGraphicsScene *g_scene;
QGraphicsScene *scene_stub()
{
    g_scene = new QGraphicsScene;
    return g_scene;
}

void handleAnnotationLoaded_stub(const QList<Annotation *> &)
{
    g_funcName = __FUNCTION__;
}

bool updateAnnotation_stub(int, deepin_reader::Annotation *, const QString &, const QColor &)
{
    g_funcName = __FUNCTION__;
    return true;
}

void renderRect_stub(const QRectF &)
{
    g_funcName = __FUNCTION__;
}

QList<deepin_reader::Word> getWords_stub(int)
{
    Word w1("test", QRectF(0, 0, 20, 10));
    Word w2("test", QRectF(20, 0, 40, 10));
    QList<deepin_reader::Word> words{w1, w2};
    return words;
}

static DPdfTextAnnot *g_dAnnot;
static Annotation *g_annot;
Annotation *addHighlightAnnotation_stub(int, const QList<QRectF> &, const QString &, const QColor &)
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    return g_annot;
}

Annotation *moveIconAnnotation_stub(int, Annotation *, const QRectF &)
{
    return g_annot;
}

static QList<Annotation *> getAnnotations_stub(int)
{
    QList<Annotation *>annots{g_annot};
    return annots;
}

bool removeAnnotation_stub(int, deepin_reader::Annotation *)
{
    return true;
}

QRectF bookmarkMouseRect_stub()
{
    return QRectF(0, 0, 20, 10);
}

static QRectF rect_stub()
{
    return QRectF(10, 20, 30, 40);
}

QPointF pos_stub()
{
    return QPointF(20, 30);
}

Annotation *addIconAnnotation_stub(int, const QRectF &, const QString &)
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    return g_annot;
}

QRectF boundingRect_stub()
{
    return QRectF(0, 0, 20, 10);
}
QRectF boundingRect_stub2()
{
    return QRectF(0, 0, 2000, 1500);
}

static QList<QGraphicsItem *> g_GraphicsItemList;
static QList<QGraphicsItem *> items_stub(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &)
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    g_GraphicsItemList.push_back(new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), g_annot, 2.0));
    return g_GraphicsItemList;
}
static QList<QGraphicsItem *> items_stub2(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &)
{
    g_GraphicsItemList.push_back(new BrowserWord(nullptr, Word("test", QRectF(0, 0, 20, 10))));
    return g_GraphicsItemList;
}

/*********测试用例**********/
TEST_F(TestBrowserPage, UT_BrowserPage_boundingRect_001)
{
    m_tester->m_originSizeF = QSizeF(10, 20);
    m_tester->m_scaleFactor = 1.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->boundingRect().width(), 15));
}

TEST_F(TestBrowserPage, UT_BrowserPage_rect_001)
{
    m_tester->m_originSizeF = QSizeF(10, 20);
    m_tester->m_scaleFactor = 1.5;

    m_tester->m_rotation = Dr::RotateBy90;
    EXPECT_TRUE(qFuzzyCompare(m_tester->rect().width(), 30));

    m_tester->m_rotation = Dr::RotateBy180;
    EXPECT_TRUE(qFuzzyCompare(m_tester->rect().width(), 15));
}

TEST_F(TestBrowserPage, UT_BrowserPage_scaleFactor_001)
{
    m_tester->m_scaleFactor = 1.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->scaleFactor(), 1.5));
}

TEST_F(TestBrowserPage, UT_BrowserPage_bookmarkRect_001)
{
    m_tester->m_originSizeF = QSizeF(100, 200);
    m_tester->m_scaleFactor = 1.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->bookmarkRect().x(), 110));
}

TEST_F(TestBrowserPage, UT_BrowserPage_bookmarkMouseRect_001)
{
    m_tester->m_originSizeF = QSizeF(100, 200);
    m_tester->m_scaleFactor = 1.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->bookmarkMouseRect().x(), 123));
}

TEST_F(TestBrowserPage, UT_BrowserPage_setBookmark_001)
{
    m_tester->setBookmark(true);
    EXPECT_TRUE(m_tester->m_bookmarkState == 3);

    m_tester->setBookmark(false);
    EXPECT_TRUE(m_tester->m_bookmarkState == 0);
}

TEST_F(TestBrowserPage, UT_BrowserPage_updateBookmarkState_001)
{
    m_tester->m_bookmark = true;
    m_tester->updateBookmarkState();
    EXPECT_TRUE(m_tester->m_bookmarkState == 3);

    m_tester->m_bookmark = false;
    m_tester->updateBookmarkState();
    EXPECT_TRUE(m_tester->m_bookmarkState == 0);
}


TEST_F(TestBrowserPage, UT_BrowserPage_paint_001)
{
    Stub s;
    s.set(ADDR(BrowserPage, render), render_stub);
    s.set(static_cast<void (QPainter::*)(const QRect &)>(ADDR(QPainter, drawRect)), drawRect_stub);

    PageSection section = PageSection{PageLine{QString(), QRectF(200, 200, 100, 20)}};

    m_tester->m_renderPixmapScaleFactor = 1.0;
    m_tester->m_scaleFactor = 2.0;
    m_tester->m_rotation = Dr::RotateBy0;
    m_tester->m_viewportRendered = false;
    m_tester->m_pixmapHasRendered = false;
    m_tester->m_originSizeF = QSizeF(1000, 2000);
    m_tester->m_searchLightrectLst.append(section);
    m_tester->m_searchSelectLighRectf = section;
    m_tester->m_drawMoveIconRect = true;

    QPainter *painter = new QPainter;
    m_tester->paint(painter, nullptr, nullptr);
    EXPECT_TRUE(g_funcName == "drawRect_stub");
    delete painter;
}

TEST_F(TestBrowserPage, UT_BrowserPage_render_001)
{
    Stub s;
    s.set(ADDR(PageRenderThread, clearImageTasks), clearImageTasks_stub);
    s.set(static_cast<void (*)(DocPageNormalImageTask)>(ADDR(PageRenderThread, appendTask)), static_cast<void (*)(DocPageNormalImageTask)>(appendTask_stub));
    s.set(static_cast<void (*)(DocPageAnnotationTask)>(ADDR(PageRenderThread, appendTask)), static_cast<void (*)(DocPageAnnotationTask)>(appendTask_stub));
    s.set(ADDR(BrowserPage, scaleWords), scaleWords_stub);
    s.set(ADDR(BrowserPage, scaleAnnots), scaleAnnots_stub);

    m_tester->m_renderPixmapScaleFactor = 1.0;
    m_tester->render(2.0, Dr::RotateBy270, false, false);

    EXPECT_TRUE(qFuzzyCompare(m_tester->m_renderPixmapScaleFactor, 2.0));
    EXPECT_TRUE(m_tester->m_annotatinIsRendering);
}

TEST_F(TestBrowserPage, UT_BrowserPage_renderRect_001)
{
    Stub s;
    s.set(static_cast<void (*)(DocPageSliceImageTask)>(ADDR(PageRenderThread, appendTask)), static_cast<void (*)(DocPageSliceImageTask)>(appendTask_stub));

    m_tester->renderRect(QRectF(100, 200, 500, 500));

    EXPECT_TRUE(g_funcName == "appendTask_stub");
}

TEST_F(TestBrowserPage, UT_BrowserPage_renderViewPort_001)
{
    Stub s;
    s.set(static_cast<void (*)(DocPageSliceImageTask)>(ADDR(PageRenderThread, appendTask)), static_cast<void (*)(DocPageSliceImageTask)>(appendTask_stub));

    m_tester->renderViewPort();

    EXPECT_TRUE(m_tester->m_viewportRendered);
}

TEST_F(TestBrowserPage, UT_BrowserPage_handleRenderFinished_001)
{
    m_tester->m_pixmapHasRendered = false;
    m_tester->handleRenderFinished(0, QPixmap());
    EXPECT_TRUE(m_tester->m_pixmapHasRendered);
}

TEST_F(TestBrowserPage, UT_BrowserPage_handleWordLoaded_001)
{
    m_tester->m_wordIsRendering = true;
    m_tester->m_wordNeeded = true;
    m_tester->m_wordHasRendered = false;
    QList<Word> words{Word("test", QRectF(0, 0, 10, 20))};
    m_tester->handleWordLoaded(words);
    EXPECT_FALSE(m_tester->m_wordIsRendering);
    EXPECT_TRUE(m_tester->m_wordHasRendered);
    EXPECT_TRUE(m_tester->m_words.size() == 1);
}

TEST_F(TestBrowserPage, UT_BrowserPage_handleAnnotationLoaded_001)
{
    m_tester->m_wordIsRendering = true;
    m_tester->m_wordNeeded = true;
    m_tester->m_wordHasRendered = false;
    QList<Word> words{Word("test", QRectF(0, 0, 10, 20))};
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    QList<Annotation *> annots{annot};
    m_tester->handleAnnotationLoaded(annots);
    EXPECT_TRUE(m_tester->m_annotations.size() == 1);
    EXPECT_TRUE(m_tester->m_annotationItems.size() == 1);
    EXPECT_TRUE(m_tester->m_hasLoadedAnnotation);
    delete dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_getCurrentImage_001)
{
    m_tester->m_pixmap = QPixmap(50, 50);
    EXPECT_FALSE(m_tester->getCurrentImage(50, 50).isNull());
}

TEST_F(TestBrowserPage, UT_BrowserPage_getImagePoint_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, getImage), getImage_stub);
    EXPECT_FALSE(m_tester->getImagePoint(2.0, QPoint(0, 0)).isNull());
}

TEST_F(TestBrowserPage, UT_BrowserPage_getCurImagePoint_001)
{
    m_tester->m_renderPixmap = QPixmap(50, 50);
    EXPECT_FALSE(m_tester->getCurImagePoint(QPoint(100, 100)).isNull());
}

TEST_F(TestBrowserPage, UT_BrowserPage_itemIndex_001)
{
    m_tester->m_index = 1;
    EXPECT_TRUE(m_tester->itemIndex() == 1);
}

TEST_F(TestBrowserPage, UT_BrowserPage_selectedWords_001)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, isSelected), isSelected_stub);
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);
    EXPECT_TRUE(m_tester->selectedWords() == "firstsecond");
}

TEST_F(TestBrowserPage, UT_BrowserPage_setWordSelectable_001)
{
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);
    m_tester->setWordSelectable(true);
    EXPECT_TRUE(m_tester->m_wordSelectable);
}

TEST_F(TestBrowserPage, UT_BrowserPage_loadWords_001)
{
    m_tester->m_wordIsRendering = false;
    m_tester->m_wordHasRendered = true;
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);
    m_tester->m_wordScaleFactor = 1;
    m_tester->m_scaleFactor = 2;
    m_tester->loadWords();
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_wordScaleFactor, 2));
}

TEST_F(TestBrowserPage, UT_BrowserPage_loadWords_002)
{
    Stub s;
    s.set(static_cast<void (*)(DocPageWordTask)>(ADDR(PageRenderThread, appendTask)), static_cast<void (*)(DocPageWordTask)>(appendTask_stub));

    m_tester->m_wordIsRendering = false;
    m_tester->m_wordHasRendered = false;

    m_tester->loadWords();
    EXPECT_TRUE(g_funcName == "appendTask_stub");
}

TEST_F(TestBrowserPage, UT_BrowserPage_clearPixmap_001)
{
    m_tester->m_renderPixmapScaleFactor = 1;
    m_tester->m_pixmapId = 1;
    m_tester->m_pixmapHasRendered = true;
    m_tester->m_viewportRendered = true;
    m_tester->clearPixmap();
    EXPECT_TRUE(m_tester->m_pixmapId == 2);
    EXPECT_FALSE(m_tester->m_pixmapHasRendered);
    EXPECT_FALSE(m_tester->m_viewportRendered);
}

TEST_F(TestBrowserPage, UT_BrowserPage_clearWords_001)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, scene), scene_stub);
    m_tester->m_wordNeeded = true;
    m_tester->m_wordHasRendered = true;
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);

    m_tester->clearWords();
    delete g_scene;
    EXPECT_FALSE(m_tester->m_pixmapHasRendered);
    EXPECT_FALSE(m_tester->m_wordNeeded);
}

TEST_F(TestBrowserPage, UT_BrowserPage_scaleAnnots_001)
{
    m_tester->m_annotScaleFactor = 1.0;
    m_tester->m_scaleFactor = 2.0;
    m_tester->m_annotatinIsRendering = true;
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    BrowserAnnotation *annotationItem = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), annot, 2.0);
    m_tester->m_annotationItems.append(annotationItem);

    m_tester->scaleAnnots();
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_wordScaleFactor, 2));
    delete dAnnot;
    delete annot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_scaleWords_001)
{
    m_tester->m_wordScaleFactor = 1.0;
    m_tester->m_scaleFactor = 2.0;
    m_tester->m_wordHasRendered = true;
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);

    m_tester->scaleWords(true);
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_wordScaleFactor, 2));
}

TEST_F(TestBrowserPage, UT_BrowserPage_annotations_001)
{
    Stub s;
    s.set(ADDR(BrowserPage, handleAnnotationLoaded), handleAnnotationLoaded_stub);

    m_tester->m_hasLoadedAnnotation = false;

    EXPECT_TRUE(m_tester->annotations() == m_tester->m_annotations);
}

TEST_F(TestBrowserPage, UT_BrowserPage_updateAnnotation_001)
{
    Annotation *annotation = nullptr;
    QString text("test");
    QColor color;
    EXPECT_FALSE(m_tester->updateAnnotation(annotation, text, color));

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    m_tester->m_annotations.append(annot);
    annotation = new PDFAnnotation(dAnnot);
    EXPECT_FALSE(m_tester->updateAnnotation(annotation, text, color));

    delete annotation;
    annotation = annot;
    EXPECT_FALSE(m_tester->updateAnnotation(annotation, text, color));

    Stub s;
    s.set(ADDR(SheetRenderer, updateAnnotation), updateAnnotation_stub);
    s.set(ADDR(BrowserPage, renderRect), renderRect_stub);
    EXPECT_TRUE(m_tester->updateAnnotation(annotation, text, color));
    EXPECT_TRUE(g_funcName == "renderRect_stub");
    delete dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_addHighlightAnnotation_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, getWords), getWords_stub);
    s.set(ADDR(QGraphicsItem, isSelected), isSelected_stub);
    s.set(ADDR(SheetRenderer, addHighlightAnnotation), addHighlightAnnotation_stub);
    BrowserWord *w1 = new BrowserWord(nullptr, Word("first", QRectF(0, 0, 20, 10)));
    BrowserWord *w2 = new BrowserWord(nullptr, Word("second", QRectF(20, 0, 40, 10)));
    m_tester->m_words.append(w1);
    m_tester->m_words.append(w2);

    EXPECT_FALSE(m_tester->addHighlightAnnotation("", QColor()) == nullptr);
    delete g_dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_hasAnnotation_001)
{
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    m_tester->m_annotations.append(annot);
    EXPECT_TRUE(m_tester->hasAnnotation(annot));
    delete dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_setSelectIconRect_001)
{
    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    m_tester->m_annotations.append(annot);
    BrowserAnnotation *annotationItem = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), annot, 2.0);
    m_tester->m_annotationItems.append(annotationItem);

    m_tester->setSelectIconRect(true, annot);
    EXPECT_FALSE(m_tester->m_lastClickIconAnnotationItem == nullptr);

    m_tester->m_lastClickIconAnnotationItem = annotationItem;
    m_tester->setSelectIconRect(true, nullptr);
    EXPECT_FALSE(m_tester->m_lastClickIconAnnotationItem == nullptr);
    delete dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_setDrawMoveIconRect_001)
{
    m_tester->m_drawMoveIconRect = false;
    m_tester->setDrawMoveIconRect(true);
    EXPECT_TRUE(m_tester->m_drawMoveIconRect);
}

TEST_F(TestBrowserPage, UT_BrowserPage_iconMovePos_001)
{
    EXPECT_TRUE(m_tester->iconMovePos() == m_tester->m_drawMoveIconPoint);
}

TEST_F(TestBrowserPage, UT_BrowserPage_setIconMovePos_001)
{
    m_tester->m_drawMoveIconPoint = QPointF(0, 0);
    m_tester->setIconMovePos(QPointF(1, 1));
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_drawMoveIconPoint.x(), 1));
}

TEST_F(TestBrowserPage, UT_BrowserPage_moveIconAnnotation_001)
{
    EXPECT_FALSE(m_tester->moveIconAnnotation(QRectF(0, 0, 20, 10)));

    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);

    m_tester->m_annotations.append(g_annot);
    BrowserAnnotation *annotationItem = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), g_annot, 2.0);
    m_tester->m_annotationItems.append(annotationItem);
    m_tester->m_lastClickIconAnnotationItem = annotationItem;

    Stub s;
    s.set(ADDR(SheetRenderer, moveIconAnnotation), moveIconAnnotation_stub);

    EXPECT_TRUE(m_tester->moveIconAnnotation(QRectF(0, 0, 20, 10)));
    delete g_dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_removeAllAnnotation_001)
{
    m_tester->m_hasLoadedAnnotation = false;
    g_dAnnot = new DPdfTextAnnot;
    g_dAnnot->m_text = "test";
    g_annot = new PDFAnnotation(g_dAnnot);
    m_tester->m_annotations.append(g_annot);
    BrowserAnnotation *annotationItem = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), g_annot, 2.0);
    m_tester->m_annotationItems.append(annotationItem);

    Stub s;
    s.set(ADDR(SheetRenderer, getAnnotations), getAnnotations_stub);
    s.set(ADDR(SheetRenderer, removeAnnotation), removeAnnotation_stub);

    EXPECT_TRUE(m_tester->removeAllAnnotation());
    delete g_dAnnot;
    delete g_annot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_setPageBookMark_001)
{
    Stub s;
    s.set(ADDR(BrowserPage, bookmarkMouseRect), bookmarkMouseRect_stub);

    m_tester->m_bookmark = false;
    m_tester->setPageBookMark(QPointF(5, 5));
    EXPECT_TRUE(m_tester->m_bookmarkState == 1);

    m_tester->m_bookmark = true;
    m_tester->setPageBookMark(QPointF(5, 5));
    EXPECT_TRUE(m_tester->m_bookmarkState == 3);
}

TEST_F(TestBrowserPage, UT_BrowserPage_getTopLeftPos_001)
{
    Stub s;
    s.set(ADDR(BrowserPage, rect), rect_stub);
    s.set(ADDR(QGraphicsItem, pos), pos_stub);

    m_tester->m_rotation = Dr::RotateBy0;
    EXPECT_TRUE(m_tester->getTopLeftPos() == QPointF(20, 30));

    m_tester->m_rotation = Dr::RotateBy90;
    EXPECT_TRUE(m_tester->getTopLeftPos() == QPointF(-10.0, 30.0));

    m_tester->m_rotation = Dr::RotateBy270;
    EXPECT_TRUE(m_tester->getTopLeftPos() == QPointF(20.0, -10.0));

    m_tester->m_rotation = Dr::RotateBy180;
    EXPECT_TRUE(m_tester->getTopLeftPos() == QPointF(-10.0, -10.0));
}

TEST_F(TestBrowserPage, UT_BrowserPage_removeAnnotation_001)
{

    EXPECT_FALSE(m_tester->removeAnnotation(nullptr));

    Stub s;
    s.set(ADDR(SheetRenderer, removeAnnotation), removeAnnotation_stub);


    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    m_tester->m_annotations.append(annot);
    BrowserAnnotation *annotationItem = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), annot, 2.0);
    m_tester->m_annotationItems.append(annotationItem);
    EXPECT_TRUE(m_tester->removeAnnotation(annot));

    delete dAnnot;
    delete annot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_addIconAnnotation_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, addIconAnnotation), addIconAnnotation_stub);
    QRectF rect(0, 0, 20, 20);
    QString text("test");
    EXPECT_FALSE(m_tester->addIconAnnotation(rect, text) == nullptr);
    delete g_dAnnot;
}

TEST_F(TestBrowserPage, UT_BrowserPage_sceneEvent_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, addIconAnnotation), addIconAnnotation_stub);
    s.set(ADDR(BrowserPage, bookmarkMouseRect), bookmarkMouseRect_stub);
    QEvent *event = new QGraphicsSceneHoverEvent(QEvent::GraphicsSceneHoverMove);
    m_tester->m_bookmark = false;
    m_tester->sceneEvent(event);
    EXPECT_TRUE(m_tester->m_bookmarkState == 1);

    m_tester->m_bookmark = true;
    m_tester->sceneEvent(event);
    EXPECT_TRUE(m_tester->m_bookmarkState == 3);
    delete event;
}

TEST_F(TestBrowserPage, UT_BrowserPage_setSearchHighlightRectf_001)
{
    PageSection section = PageSection{PageLine{QString(), QRectF(0, 0, 20, 10)}};

    m_tester->setSearchHighlightRectf(QVector<PageSection>{section});
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_searchSelectLighRectf.value(0).rect.width(), 20));
    EXPECT_TRUE(m_tester->m_searchLightrectLst.size() == 1);
}

TEST_F(TestBrowserPage, UT_BrowserPage_clearSearchHighlightRects_001)
{
    m_tester->clearSearchHighlightRects();
    EXPECT_TRUE(m_tester->m_searchLightrectLst.size() == 0);
}

TEST_F(TestBrowserPage, UT_BrowserPage_clearSelectSearchHighlightRects_001)
{
    PageSection section = PageSection{PageLine{QString(), QRectF(0, 0, 20, 10)}};
    m_tester->m_searchSelectLighRectf = section;
    m_tester->clearSelectSearchHighlightRects();
    EXPECT_FALSE(qFuzzyCompare(m_tester->m_searchSelectLighRectf.value(0).rect.width(), 20));
}

TEST_F(TestBrowserPage, UT_BrowserPage_searchHighlightRectSize_001)
{
    PageSection section = PageSection{PageLine{QString(), QRectF(0, 0, 20, 10)}};

    m_tester->m_searchLightrectLst.append(section);
    EXPECT_TRUE(m_tester->searchHighlightRectSize() == 1);
}

TEST_F(TestBrowserPage, UT_BrowserPage_findSearchforIndex_001)
{
    PageSection section = PageSection{PageLine{QString(), QRectF(0, 0, 20, 10)}};

    m_tester->m_searchLightrectLst.append(section);
    m_tester->findSearchforIndex(0);
    EXPECT_TRUE(qFuzzyCompare(m_tester->findSearchforIndex(0).value(0).rect.width(), 20));

    m_tester->findSearchforIndex(1);
    EXPECT_TRUE(m_tester->findSearchforIndex(1).isEmpty());
}

TEST_F(TestBrowserPage, UT_BrowserPage_getNorotateRect_001)
{
    m_tester->m_scaleFactor = 1.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->getNorotateRect(QRectF(0, 0, 20, 10)).width(), 30));
}

TEST_F(TestBrowserPage, UT_BrowserPage_translateRect_001)
{
    m_tester->m_scaleFactor = 1.5;
    m_tester->m_originSizeF = QSizeF(200, 200);

    m_tester->m_rotation = Dr::RotateBy0;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translateRect(QRectF(0, 0, 20, 10)).width(), 30));

    Stub s;
    typedef QRectF(*fptr)(BrowserPage *);
    fptr A_foo = (fptr)(&BrowserPage::boundingRect);
    s.set(A_foo, boundingRect_stub);

    m_tester->m_rotation = Dr::RotateBy90;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translateRect(QRectF(0, 0, 20, 10)).width(), 15));

    m_tester->m_rotation = Dr::RotateBy180;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translateRect(QRectF(0, 0, 20, 10)).width(), 30));

    m_tester->m_rotation = Dr::RotateBy270;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translateRect(QRectF(0, 0, 20, 10)).width(), 15));
}

TEST_F(TestBrowserPage, UT_BrowserPage_getBrowserAnnotation_001)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, scene), scene_stub);

    EXPECT_TRUE(m_tester->getBrowserAnnotation(QPointF(0, 0)) == nullptr);
    delete g_scene;
}

TEST_F(TestBrowserPage, UT_BrowserPage_getBrowserAnnotation_002)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, scene), scene_stub);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub);

    EXPECT_FALSE(m_tester->getBrowserAnnotation(QPointF(0, 0)) == nullptr);
    delete g_scene;
    delete g_annot;
    delete g_dAnnot;
    qDeleteAll(g_GraphicsItemList);
    g_GraphicsItemList.clear();
}

TEST_F(TestBrowserPage, UT_BrowserPage_getBrowserWord_001)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, scene), scene_stub);

    EXPECT_TRUE(m_tester->getBrowserWord(QPointF(0, 0)) == nullptr);
    delete g_scene;
}

TEST_F(TestBrowserPage, UT_BrowserPage_getBrowserWord_002)
{
    Stub s;
    s.set(ADDR(QGraphicsItem, scene), scene_stub);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub2);

    EXPECT_FALSE(m_tester->getBrowserWord(QPointF(0, 0)) == nullptr);
    delete g_scene;
    qDeleteAll(g_GraphicsItemList);
    g_GraphicsItemList.clear();
}

TEST_F(TestBrowserPage, UT_BrowserPage_isBigDoc_001)
{
    EXPECT_FALSE(m_tester->isBigDoc());

    Stub s;
    typedef QRectF(*fptr)(BrowserPage *);
    fptr A_foo = (fptr)(&BrowserPage::boundingRect);
    s.set(A_foo, boundingRect_stub2);
    EXPECT_TRUE(m_tester->isBigDoc());
}
