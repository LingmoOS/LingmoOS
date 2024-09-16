// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SheetBrowser.h"
#include "BrowserPage.h"
#include "BrowserWord.h"
#include "BrowserMenu.h"
#include "BrowserMagniFier.h"
#include "BrowserAnnotation.h"
#include "SheetRenderer.h"
#include "PDFModel.h"
#include "dpdfannot.h"
#include "dpdfpage.h"
#include "TipsWidget.h"
#include "TextEditWidget.h"
#include "stub.h"

#include <DDialog>

#include <QTest>
#include <QSignalSpy>
#include <QScroller>
#include <QDesktopServices>

#include <gtest/gtest.h>

///*******************************函数打桩************************************/
static QString g_funcName;
QString selectedWordsText_stub()
{
    return "test";
}

QString selectedWordsText_stub_empty()
{
    return "";
}

void initActions_stub(DocSheet *sheet, int index, SheetMenuType_e type, const QString &copytext)
{
    Q_UNUSED(sheet)
    Q_UNUSED(index)
    Q_UNUSED(type)
    Q_UNUSED(copytext)
}

static QAction *exec_stub(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos)
    Q_UNUSED(at)
    return nullptr;
}

void clearSelectIconAnnotAfterMenu_stub()
{

}

QPoint mapToGlobal_stub(const QPoint &)
{
    return QPoint();
}

QPointF getAnnotPosInPage_stub(const QPointF &pos, BrowserPage *page)
{
    return QPointF(-1, -1);
}

static QList<BrowserPage *> g_pBrowserPage;
QGraphicsItem *parentItem_stub()
{
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    g_pBrowserPage.push_back(new BrowserPage(nullptr, 0, &sheet));
    return g_pBrowserPage.last();
}

Annotation *addHighlightAnnotation_stub(QString text, QColor color)
{
    g_funcName = __FUNCTION__;
    return nullptr;
}

bool calcIconAnnotRect_stub_true(BrowserPage *page, const QPointF &point, QRectF &iconRect)
{
    return true;
}

bool calcIconAnnotRect_stub_false(BrowserPage *page, const QPointF &point, QRectF &iconRect)
{
    return false;
}

int  searchHighlightRectSize_stub_0()
{
    return 0;
}

int  searchHighlightRectSize_stub_1()
{
    return 1;
}

void clearSelectSearchHighlightRects_stub()
{

}

PageSection findSearchforIndex_stub(int)
{
    PageLine line;
    line.rect = QRectF(-1, -1, -1, -1);

    PageSection section;
    section.append(line);

    return section;
}

QRectF translateRect_stub(const QRectF &rect)
{
    Q_UNUSED(rect)
    return QRectF(-1, -1, -1, -1);
}

void render_stub(const double &scaleFactor, const Dr::Rotation &rotation, const bool &renderLater, const bool &force)
{
    Q_UNUSED(scaleFactor)
    Q_UNUSED(rotation)
    Q_UNUSED(renderLater)
    Q_UNUSED(force)
}

QImage render_stub2(int, int, const QRect &)
{
    return QImage(100, 100, QImage::Format_ARGB32);
}
void beginViewportChange_stub()
{
    g_funcName = __FUNCTION__;
}

QList<QRectF> boundary_stub()
{
    return QList<QRectF>();
}

QList<QRectF> boundary_stub2()
{
    QList<QRectF> rectFList;
    QRectF rectf(-1, -1, -1, -1);
    rectFList.append(rectf);
    return rectFList;
}

QList<QRectF> boundary_stub3()
{
    QList<QRectF> rectFList;
    QRectF rectf(0, 0, 400, 400);
    rectFList.append(rectf);
    return rectFList;
}

SheetOperation operation_stub()
{
    SheetOperation operation;
    operation.rotation = Dr::RotateBy90;
    return operation;
}

SheetOperation operation_stub2()
{
    SheetOperation operation;
    operation.rotation = Dr::RotateBy180;
    return operation;
}

SheetOperation operation_stub3()
{
    SheetOperation operation;
    operation.rotation = Dr::RotateBy270;
    return operation;
}

bool setDocTapGestrue_stub(QPoint mousePos)
{
    Q_UNUSED(mousePos)
    return true;
}

Qt::MouseEventSource source_stub()
{
    return Qt::MouseEventSynthesizedByQt;
}

Qt::MouseEventSource source_stub2()
{
    return Qt::MouseEventSynthesizedBySystem;
}

static BrowserPage *g_pBrowserPage2 = nullptr;
BrowserPage *getBrowserPageForPoint_stub(QPointF &)
{
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    g_pBrowserPage2 = new BrowserPage(nullptr, 0, &sheet);
    g_pBrowserPage2->m_index = 3;
    return g_pBrowserPage2;
}

static PDFAnnotation *g_pPDFAnnotation = nullptr;
static DPdfTextAnnot *g_pDPdfAnnot = nullptr;
Annotation *getClickAnnot_stub(BrowserPage *page, const QPointF clickPoint, bool drawRect)
{
    Q_UNUSED(page)
    Q_UNUSED(clickPoint)
    Q_UNUSED(drawRect)
    g_pDPdfAnnot = new DPdfTextAnnot();
    g_pPDFAnnotation = new PDFAnnotation(g_pDPdfAnnot);
    g_pPDFAnnotation->m_dannotation->m_type = DPdfAnnot::AText;
    return g_pPDFAnnotation;
}

void mousePressEvent_stub(DGraphicsView *, QMouseEvent *)
{
}

static QList<QGraphicsItem *> g_QGraphicsItemList;
static QList<QGraphicsItem *> items_stub(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &)
{
    g_QGraphicsItemList.push_back((new QGraphicsLineItem(nullptr)));
    return g_QGraphicsItemList;
}

static QList<QGraphicsItem *> items_stub2(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &)
{
    g_QGraphicsItemList.push_front((new BrowserWord(nullptr, Word("test", QRectF(5, 5, 20, 10)))));
    return g_QGraphicsItemList;
}

static QList<QGraphicsItem *> items_stub3(const QRectF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &)
{
    g_QGraphicsItemList.push_front((new BrowserWord(nullptr, Word("test", QRectF(5, 5, 20, 10)))));
    return g_QGraphicsItemList;
}

static QList<QGraphicsItem *> items_stub4(const QPoint &)
{
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    g_QGraphicsItemList.push_back((new BrowserPage(nullptr, 0, &sheet)));
    return g_QGraphicsItemList;
}
void deform_stub(SheetOperation &)
{

}

int currentScrollValueForPage_stub()
{
    return 1;
}

void currentIndexRange_stub(void *, int &fromIndex, int &toIndex)
{
    fromIndex = 3;
    toIndex = 5;
}

QString selectedWords_stub()
{
    return "test";
}

static DPdfTextAnnot *g_dAnnot;
static QList<DPdfTextAnnot *> g_dAnnotlsit;
static Annotation *g_annot;
QList<Annotation *> annotations_stub()
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    QList<Annotation *> annotlist;
    annotlist.append(g_annot);
    g_dAnnotlsit.append(g_dAnnot);
    return annotlist;
}

bool hasAnnotation_stub(Annotation *)
{
    return true;
}

bool removeAnnotation_stub(Annotation *)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool removeAllAnnotation_stub(Annotation *)
{
    return true;
}

bool updateAnnotation_stub(Annotation *, const QString &, const QColor &)
{
    g_funcName = __FUNCTION__;
    return true;
}

void setCurrentPage_stub(int)
{
    g_funcName = __FUNCTION__;
}

void onViewportChanged_stub()
{
    g_funcName = __FUNCTION__;
}

Qt::KeyboardModifiers keyboardModifiers_stub()
{
    g_funcName = __FUNCTION__;
    return Qt::ControlModifier;
}

bool isHidden_stub()
{
    g_funcName = __FUNCTION__;
    return false;
}

void showMenu_stub()
{
    g_funcName = __FUNCTION__;
}

bool gestureEvent_stub(QGestureEvent *)
{
    g_funcName = __FUNCTION__;
    return true;
}

static QGesture *g_gesture;
QGesture *gesture_stub(Qt::GestureType)
{
    g_gesture = new QGesture();
    g_funcName = __FUNCTION__;
    return g_gesture;
}

void pinchTriggered_stub(QPinchGesture *)
{
    g_funcName = __FUNCTION__;
}

Qt::GestureState state_stub()
{
    g_funcName = __FUNCTION__;
    return Qt::GestureStarted;
}

Qt::GestureState state_stub2()
{
    g_funcName = __FUNCTION__;
    return Qt::GestureFinished;
}

QPinchGesture::ChangeFlags changeFlags_stub()
{
    return QPinchGesture::RotationAngleChanged;
}

QPinchGesture::ChangeFlags changeFlags_stub2()
{
    return QPinchGesture::ScaleFactorChanged;
}

qreal rotationAngle_stub()
{
    return 40;
}

qreal rotationAngle_stub2()
{
    return -40;
}

void rotateLeft_stub()
{
    g_funcName = __FUNCTION__;
}

void rotateRight_stub()
{
    g_funcName = __FUNCTION__;
}

void setScaleFactor_stub(qreal)
{
    g_funcName = __FUNCTION__;
}

static bool isVisible_stub()
{
    return true;
}

static void openMagnifier_stub()
{
    g_funcName = __FUNCTION__;
}

QGraphicsView::DragMode dragMode_stub()
{
    return QGraphicsView::NoDrag;
}

bool handleInput_stub(QScroller::Input, const QPointF &, qint64)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool isFullScreen_stub()
{
    return true;
}

static bool magnifierOpened_stub()
{
    return true;
}

bool magnifierOpened_stub2()
{
    return false;
}

void showMagnigierImage_stub(const QPoint &)
{
    g_funcName = __FUNCTION__;
}

void loadWords_stub()
{
    g_funcName = __FUNCTION__;
}

void setSelected_stub(bool)
{
    g_funcName = __FUNCTION__;
}

static BrowserAnnotation *g_pBrowserAnnotation = nullptr;
BrowserAnnotation *getBrowserAnnotation_stub(const QPointF &)
{
    g_pDPdfAnnot = new DPdfTextAnnot;
    g_pPDFAnnotation = new PDFAnnotation(g_pDPdfAnnot);
    g_pBrowserAnnotation = new BrowserAnnotation(nullptr, QRectF(0, 0, 50, 50), g_pPDFAnnotation, 2.0);

    return g_pBrowserAnnotation;
}

BrowserAnnotation *getBrowserAnnotation_stub2(const QPointF &)
{
    return nullptr;
}

QString contents_stub()
{
    return "test";
}
static void show_stub()
{
    g_funcName = __FUNCTION__;
}

void showNoteEditWidget_stub(deepin_reader::Annotation *, const QPoint &)
{

}

qreal manhattanLength_stub()
{
    return 1;
}

bool jump2Link_stub(QPointF)
{
    g_funcName = __FUNCTION__;
    return true;
}

int timerId_stub()
{
    return 1;
}

void curpageChanged_stub(int)
{
    g_funcName = __FUNCTION__;
}

QImage getCurrentImage_stub(int, int)
{
    QImage image(10, 10, QImage::Format_RGB32);
    return image;
}

bool contains_stub(const QPointF &)
{
    return true;
}

bool calcIconAnnotRect_stub(BrowserPage *, const QPointF &, QRectF &)
{
    return true;
}

Annotation *addIconAnnotation_stub(const QRectF &, const QString &)
{
    g_pDPdfAnnot = new DPdfTextAnnot();
    g_pPDFAnnotation = new PDFAnnotation(g_pDPdfAnnot);
    g_pPDFAnnotation->m_dannotation->m_type = DPdfAnnot::AText;

    return nullptr;
}

void ignore_stub()
{
    g_funcName = __FUNCTION__;
}

void onInit_stub()
{
    g_funcName = __FUNCTION__;
}

void showPosition_stub(const int &)
{
    g_funcName = __FUNCTION__;
}

void clearSearchHighlightRects_stub()
{
    g_funcName = __FUNCTION__;
}

void setSearchHighlightRectf_stub(const QVector< QRectF > &)
{
    g_funcName = __FUNCTION__;
}

void jumpToNextSearchResult_stub()
{
    g_funcName = __FUNCTION__;
}

void setEditAlert_stub(const int &)
{
    g_funcName = __FUNCTION__;
}

bool inLink_stub(int, const QPointF &)
{
    return true;
}

Link getLinkAtPoint_stub(int, const QPointF &)
{
    Link link;
    link.page = 1;
    return link;
}

Link getLinkAtPoint_stub2(int, const QPointF &)
{
    Link link;
    link.page = 0;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    link.urlOrFileName = strPath;
    return link;
}


void setSelectIconRect_stub(const bool, Annotation *)
{
    g_funcName = __FUNCTION__;
}

void jump2PagePos_stub(BrowserPage *, const qreal, const qreal)
{
    g_funcName = __FUNCTION__;
}

int exec_stub2()
{
    return DDialog::Accepted;
}

static bool openUrl_stub(const QUrl &)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool isLink_stub(QPointF)
{
    return false;
}

static QRectF rect_stub()
{
    return QRectF(0, 0, 50, 50);
}

void setSidebarVisible_stub(bool, bool)
{
    g_funcName = __FUNCTION__;
}

void mouseMoveEvent_stub(QMouseEvent *)
{
    return;
}

void mouseReleaseEvent_stub(QMouseEvent *)
{
    return;
}

void showWidget_stub(const QPoint &)
{
    g_funcName = __FUNCTION__;
}
///*******************************函数打桩************************************/


// 测试SheetBrowser
class TestSheetBrowser : public ::testing::Test
{
public:
    TestSheetBrowser(): m_tester(nullptr) {}

public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    SheetBrowser *m_tester;
};

void TestSheetBrowser::SetUp()
{
    m_tester = new SheetBrowser;
    m_tester->disconnect();
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *p1 = new BrowserPage(nullptr, 0, &sheet);
    BrowserPage *p2 = new BrowserPage(nullptr, 1, &sheet);
    m_tester->m_items.push_back(p1);
    m_tester->m_items.push_back(p2);
}

void TestSheetBrowser::TearDown()
{
    delete m_tester;
}

/****************测试用例********************/
TEST_F(TestSheetBrowser, initTest)
{

}

TEST_F(TestSheetBrowser, testmouseMoveEvent)
{
    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = true;
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
    EXPECT_TRUE(m_tester->m_bHandAndLink == true);
}

TEST_F(TestSheetBrowser, testshowMenu001)
{
    Stub stub;
    stub.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub);
    stub.set(ADDR(BrowserMenu, initActions), initActions_stub);
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), exec_stub);
    stub.set(ADDR(SheetBrowser, clearSelectIconAnnotAfterMenu), clearSelectIconAnnotAfterMenu_stub);

    BrowserWord *p = new BrowserWord(nullptr, Word());
    m_tester->m_selectEndWord = p;
    DocSheet *p2 = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = p2;
    m_tester->showMenu();
    delete p;
    delete p2;
    EXPECT_TRUE(m_tester->m_selectIconAnnotation == false);
}

TEST_F(TestSheetBrowser, testshowMenu002)
{
    Stub stub;
    stub.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub_empty);
    stub.set(ADDR(BrowserMenu, initActions), initActions_stub);
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), exec_stub);
    stub.set(ADDR(SheetBrowser, clearSelectIconAnnotAfterMenu), clearSelectIconAnnotAfterMenu_stub);

    BrowserWord *p = new BrowserWord(nullptr, Word());
    m_tester->m_selectEndWord = p;
    PDFAnnotation *p2 = new PDFAnnotation(nullptr);
    m_tester->m_iconAnnot = p2;
    DocSheet *p3 = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = p3;
    m_tester->showMenu();

    delete p;
    delete p2;
    delete p3;

    EXPECT_TRUE(m_tester->m_selectIconAnnotation == false);
}

TEST_F(TestSheetBrowser, testshowMenu003)
{
    Stub stub;
    stub.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub_empty);
    stub.set(ADDR(BrowserMenu, initActions), initActions_stub);
    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), exec_stub);
    stub.set(ADDR(SheetBrowser, clearSelectIconAnnotAfterMenu), clearSelectIconAnnotAfterMenu_stub);
    stub.set(ADDR(QWidget, mapToGlobal), mapToGlobal_stub);

    BrowserWord *p = new BrowserWord(nullptr, Word());
    m_tester->m_selectEndWord = p;
    DocSheet *p2 = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = p2;
    m_tester->showMenu();

    delete p;
    delete p2;

    EXPECT_TRUE(m_tester->m_selectIconAnnotation == false);
}

TEST_F(TestSheetBrowser, testcalcIconAnnotRect001)
{
    QPointF point;
    QRectF iconRect;
    EXPECT_TRUE(m_tester->calcIconAnnotRect(nullptr, point, iconRect) == false);
}

TEST_F(TestSheetBrowser, testcalcIconAnnotRect002)
{
    QPointF point;
    QRectF iconRect;
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage page(nullptr, 0, &sheet);
    m_tester->m_sheet = &sheet;
    EXPECT_TRUE(m_tester->calcIconAnnotRect(&page, point, iconRect) == true);
}

TEST_F(TestSheetBrowser, testcalcIconAnnotRect003)
{
    QPointF point;
    QRectF iconRect;
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage page(nullptr, 0, &sheet);
    m_tester->m_sheet = &sheet;
    Stub stub;
    stub.set(ADDR(SheetBrowser, getAnnotPosInPage), getAnnotPosInPage_stub);
    EXPECT_TRUE(m_tester->calcIconAnnotRect(&page, point, iconRect) == false);
}

TEST_F(TestSheetBrowser, testaddHighLightAnnotation001)
{
    QString contains;
    QPoint showPoint;
    EXPECT_TRUE(m_tester->addHighLightAnnotation(contains, QColor(), showPoint) == nullptr);

    BrowserWord *pStart = new BrowserWord(nullptr, Word());
    BrowserWord *pEnd = new BrowserWord(nullptr, Word());
    m_tester->m_selectStartWord = pStart;
    m_tester->m_selectEndWord = pEnd;
    EXPECT_TRUE(m_tester->addHighLightAnnotation(contains, QColor(), showPoint) == nullptr);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndPos == QPointF());
    EXPECT_TRUE(m_tester->m_selectStartPos == QPointF());

    m_tester->m_selectStartWord = pStart;
    m_tester->m_selectEndWord = pEnd;
    EXPECT_TRUE(m_tester->addHighLightAnnotation(contains, QColor(), showPoint) == nullptr);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndPos == QPointF());
    EXPECT_TRUE(m_tester->m_selectStartPos == QPointF());

    delete pStart;
    delete pEnd;
}

TEST_F(TestSheetBrowser, testaddHighLightAnnotation002)
{
    QString contains;
    QPoint showPoint;
    BrowserWord *pStart = new BrowserWord(nullptr, Word());
    BrowserWord *pEnd = new BrowserWord(nullptr, Word());
    m_tester->m_selectStartWord = pStart;
    m_tester->m_selectEndWord = pEnd;
    EXPECT_TRUE(m_tester->addHighLightAnnotation(contains, QColor(), showPoint) == nullptr);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndPos == QPointF());
    EXPECT_TRUE(m_tester->m_selectStartPos == QPointF());

    delete pStart;
    delete pEnd;
}

TEST_F(TestSheetBrowser, testaddHighLightAnnotation003)
{
    QString contains;
    QPoint showPoint;
    BrowserWord *pStart = new BrowserWord(nullptr, Word());
    BrowserWord *pEnd = new BrowserWord(nullptr, Word());
    m_tester->m_selectStartWord = pStart;
    m_tester->m_selectEndWord = pEnd;
    Stub stub;
    stub.set(ADDR(QGraphicsItem, parentItem), parentItem_stub);
    stub.set(ADDR(BrowserPage, addHighlightAnnotation), addHighlightAnnotation_stub);
    EXPECT_TRUE(m_tester->addHighLightAnnotation(contains, QColor(), showPoint) == nullptr);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectEndPos == QPointF());
    EXPECT_TRUE(m_tester->m_selectStartPos == QPointF());

    delete pStart;
    delete pEnd;
    qDeleteAll(g_pBrowserPage);
}



TEST_F(TestSheetBrowser, testmoveIconAnnot001)
{
    QPointF point;
    EXPECT_TRUE(m_tester->moveIconAnnot(nullptr, point) == false);
}

TEST_F(TestSheetBrowser, testmoveIconAnnot002)
{
    QPointF point;
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage page(nullptr, 0, &sheet);
    Stub stub;
    stub.set(ADDR(SheetBrowser, calcIconAnnotRect), calcIconAnnotRect_stub_false);
    EXPECT_TRUE(m_tester->moveIconAnnot(&page, point) == false);
}

TEST_F(TestSheetBrowser, testmoveIconAnnot003)
{
    QPointF point;
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage page(nullptr, 0, &sheet);
    Stub stub;
    stub.set(ADDR(SheetBrowser, calcIconAnnotRect), calcIconAnnotRect_stub_true);
    EXPECT_TRUE(m_tester->moveIconAnnot(&page, point) == false);
}

TEST_F(TestSheetBrowser, testjumpToPrevSearchResult001)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, searchHighlightRectSize), searchHighlightRectSize_stub_0);

    m_tester->jumpToPrevSearchResult();
    EXPECT_TRUE(m_tester->m_searchCurIndex == 0);
}

TEST_F(TestSheetBrowser, testjumpToPrevSearchResult002)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, searchHighlightRectSize), searchHighlightRectSize_stub_1);
    stub.set(ADDR(BrowserPage, clearSelectSearchHighlightRects), clearSelectSearchHighlightRects_stub);
    stub.set(ADDR(BrowserPage, findSearchforIndex), findSearchforIndex_stub);
    stub.set(ADDR(BrowserPage, translateRect), translateRect_stub);
    m_tester->m_isSearchResultNotEmpty = true;

    m_tester->jumpToPrevSearchResult();
    EXPECT_TRUE(m_tester->m_sheet == nullptr);
    EXPECT_TRUE(m_tester->m_searchCurIndex == 1);
    EXPECT_TRUE(m_tester->m_changSearchFlag == false);
    EXPECT_TRUE(m_tester->m_searchPageTextIndex == 0);
}

TEST_F(TestSheetBrowser, testjumpToNextSearchResult001)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, searchHighlightRectSize), searchHighlightRectSize_stub_0);

    m_tester->jumpToNextSearchResult();
    EXPECT_TRUE(m_tester->m_searchCurIndex == 0);
    EXPECT_TRUE(m_tester->m_searchPageTextIndex == 0);
    EXPECT_TRUE(m_tester->m_changSearchFlag == false);
    EXPECT_TRUE(m_tester->m_lastFindPage == nullptr);
}

TEST_F(TestSheetBrowser, testjumpToNextSearchResult002)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, searchHighlightRectSize), searchHighlightRectSize_stub_1);
    stub.set(ADDR(BrowserPage, clearSelectSearchHighlightRects), clearSelectSearchHighlightRects_stub);
    stub.set(ADDR(BrowserPage, findSearchforIndex), findSearchforIndex_stub);
    stub.set(ADDR(BrowserPage, translateRect), translateRect_stub);
    m_tester->m_isSearchResultNotEmpty = true;

    m_tester->jumpToNextSearchResult();
    EXPECT_TRUE(m_tester->m_searchCurIndex == 1);
    EXPECT_TRUE(m_tester->m_searchPageTextIndex == 0);
    EXPECT_TRUE(m_tester->m_changSearchFlag == false);
    EXPECT_TRUE(m_tester->m_lastFindPage != nullptr);
}

TEST_F(TestSheetBrowser, testdeform001)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageWidthMode;
    operation.layoutMode = Dr::SinglePageMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testdeform002)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageWidthMode;
    operation.layoutMode = Dr::TwoPagesMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testdeform003)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageHeightMode;
    operation.layoutMode = Dr::SinglePageMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testdeform004)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageHeightMode;
    operation.layoutMode = Dr::SinglePageMode;
    operation.rotation = Dr::RotateBy90;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 1);
}

TEST_F(TestSheetBrowser, testdeform005)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageDefaultMode;
    operation.layoutMode = Dr::SinglePageMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testdeform006)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageWorHMode;
    operation.layoutMode = Dr::SinglePageMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testdeform007)
{
    Stub stub;
    stub.set(ADDR(BrowserPage, render), render_stub);
    stub.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    SheetOperation operation;
    operation.scaleMode = Dr::FitToPageWorHMode;
    operation.layoutMode = Dr::TwoPagesMode;

    m_tester->deform(operation);
    EXPECT_TRUE(m_tester->m_lastScaleFactor == 1.0);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
    EXPECT_TRUE(m_tester->m_tipsWidget != nullptr);
    EXPECT_TRUE(m_tester->m_lastrotation == 0);
}

TEST_F(TestSheetBrowser, testjumpToHighLight001)
{
    m_tester->jumpToHighLight(nullptr, 0);
    EXPECT_TRUE(m_tester->m_sheet == nullptr);
}

TEST_F(TestSheetBrowser, testjumpToHighLight002)
{
    typedef QList<QRectF> (*fptr)(PDFAnnotation *);
    fptr A_foo = (fptr)(&PDFAnnotation::boundary);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, boundary_stub);

    DocSheet p1(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = &p1;
    PDFAnnotation p2(nullptr);
    m_tester->jumpToHighLight(&p2, 0);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
}

TEST_F(TestSheetBrowser, testjumpToHighLight003)
{
    typedef QList<QRectF> (*fptr)(PDFAnnotation *);
    fptr A_foo = (fptr)(&PDFAnnotation::boundary);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, boundary_stub2);
    stub.set(ADDR(DocSheet, operation), operation_stub);

    DocSheet p1(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = &p1;
    PDFAnnotation p2(nullptr);
    m_tester->jumpToHighLight(&p2, 0);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
}

TEST_F(TestSheetBrowser, testjumpToHighLight004)
{
    typedef QList<QRectF> (*fptr)(PDFAnnotation *);
    fptr A_foo = (fptr)(&PDFAnnotation::boundary);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, boundary_stub2);
    stub.set(ADDR(DocSheet, operation), operation_stub2);

    DocSheet p1(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = &p1;
    PDFAnnotation p2(nullptr);
    m_tester->jumpToHighLight(&p2, 0);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
}

TEST_F(TestSheetBrowser, testjumpToHighLight005)
{
    typedef QList<QRectF> (*fptr)(PDFAnnotation *);
    fptr A_foo = (fptr)(&PDFAnnotation::boundary);   //获取虚函数地址
    Stub stub;
    stub.set(A_foo, boundary_stub2);
    stub.set(ADDR(DocSheet, operation), operation_stub3);

    DocSheet p1(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = &p1;
    PDFAnnotation p2(nullptr);
    m_tester->jumpToHighLight(&p2, 0);
    EXPECT_TRUE(m_tester->m_sheet != nullptr);
    EXPECT_TRUE(m_tester->m_items.count() == 2);
}

TEST_F(TestSheetBrowser, testmousePressEvent001)
{
    Stub stub;
    stub.set(ADDR(SheetBrowser, setDocTapGestrue), setDocTapGestrue_stub);
    stub.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    stub.set(ADDR(QMouseEvent, source), source_stub);
    stub.set(ADDR(SheetBrowser, getClickAnnot), getClickAnnot_stub);
    typedef void (*fptr)(DGraphicsView *, QMouseEvent *);
    fptr DGraphicsView_mousePressEvent = (fptr)(&DGraphicsView::mousePressEvent);
    stub.set(DGraphicsView_mousePressEvent, mousePressEvent_stub);
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage p1(nullptr, 0, &sheet);
    m_tester->m_lastSelectIconAnnotPage = &p1;
    QPointF localPos;
    QMouseEvent event(QEvent::MouseButtonPress, localPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(&event);

    delete g_pBrowserPage2;
    delete g_pPDFAnnotation;
    delete g_pDPdfAnnot;

    EXPECT_TRUE(m_tester->m_annotationInserting == false);
    EXPECT_TRUE(m_tester->m_canTouchScreen == true);
    EXPECT_TRUE(m_tester->m_iconAnnot != nullptr);
    EXPECT_TRUE(m_tester->m_selectIconAnnotation == true);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectWord == nullptr);
}


TEST_F(TestSheetBrowser, testmousePressEvent002)
{
    QPointF localPos;
    QMouseEvent event(QEvent::MouseButtonPress, localPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    m_tester->mousePressEvent(&event);

    EXPECT_TRUE(m_tester->m_annotationInserting == false);
    EXPECT_TRUE(m_tester->m_canTouchScreen == false);
    EXPECT_TRUE(m_tester->m_iconAnnot == nullptr);
    EXPECT_TRUE(m_tester->m_selectIconAnnotation == false);
    EXPECT_TRUE(m_tester->m_selectStartWord == nullptr);
    EXPECT_TRUE(m_tester->m_selectWord == nullptr);
}

TEST_F(TestSheetBrowser, testinit001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, deform), deform_stub);
    QSet<int> bookmarks;
    bookmarks.insert(0);
    SheetOperation operation;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    DPdfPage *dpdfpage = new DPdfPage(nullptr, 0, 96, 96);
    QMutex *docMutex = new QMutex;
    docsheet->m_renderer->m_pages.append(new PDFPage(docMutex, dpdfpage));
    m_tester->m_sheet = docsheet;
    m_tester->m_hasLoaded = false;
    int size = m_tester->m_items.size();
    m_tester->init(operation, bookmarks);
    EXPECT_TRUE(m_tester->m_items.size() == size + 1);
    EXPECT_TRUE(m_tester->m_hasLoaded);
    delete docsheet;
    delete dpdfpage;
    delete docMutex;
}

TEST_F(TestSheetBrowser, testsetMouseShape001)
{
    Dr::MouseShape shape = Dr::MouseShapeHand;
    m_tester->setMouseShape(shape);
    EXPECT_TRUE(m_tester->dragMode() == QGraphicsView::ScrollHandDrag);
    shape = Dr::MouseShapeNormal;
    m_tester->setMouseShape(shape);
    EXPECT_TRUE(m_tester->dragMode() == QGraphicsView::NoDrag);
}

TEST_F(TestSheetBrowser, testsetBookMark001)
{
    m_tester->setBookMark(0, 0);
    EXPECT_FALSE(m_tester->m_items.at(0)->m_bookmark);
}

TEST_F(TestSheetBrowser, testsetAnnotationInserting001)
{
    m_tester->m_annotationInserting = false;
    m_tester->setAnnotationInserting(true);
    EXPECT_TRUE(m_tester->m_annotationInserting);
}

TEST_F(TestSheetBrowser, testonVerticalScrollBarValueChanged001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);
    s.set(ADDR(SheetBrowser, currentScrollValueForPage), currentScrollValueForPage_stub);
    m_tester->m_bNeedNotifyCurPageChanged = true;
    m_tester->m_currentPage = 2;

    m_tester->onVerticalScrollBarValueChanged(0);
    EXPECT_TRUE(m_tester->m_currentPage == 1);
}

TEST_F(TestSheetBrowser, testonHorizontalScrollBarValueChanged001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, beginViewportChange), beginViewportChange_stub);

    m_tester->onHorizontalScrollBarValueChanged(0);
    EXPECT_TRUE(g_funcName == "beginViewportChange_stub");
}

TEST_F(TestSheetBrowser, testbeginViewportChanged001)
{
    m_tester->beginViewportChange();
    EXPECT_FALSE(m_tester->m_viewportChangeTimer == nullptr);
}

TEST_F(TestSheetBrowser, testhideSubTipsWidget001)
{
    m_tester->hideSubTipsWidget();
    EXPECT_TRUE(m_tester->cursor() == Qt::ArrowCursor);
}

TEST_F(TestSheetBrowser, testonViewportChanged001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, currentIndexRange), currentIndexRange_stub);

    m_tester->onViewportChanged();
    EXPECT_FALSE(m_tester->m_items.first()->m_wordNeeded);
}

TEST_F(TestSheetBrowser, testonAddHighLightAnnot001)
{
    Stub s;
    s.set(ADDR(BrowserPage, addHighlightAnnotation), addHighlightAnnotation_stub);

    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 0, &sheet);

    m_tester->onAddHighLightAnnot(page, "test", QColor(Qt::red));
    EXPECT_TRUE(g_funcName == "addHighlightAnnotation_stub");
    delete page;
}

TEST_F(TestSheetBrowser, testshowNoteEditWidget001)
{
    Stub s;
    s.set(ADDR(TextEditShadowWidget, showWidget), showWidget_stub);

    DPdfTextAnnot *dPdfAnnot = new DPdfTextAnnot();
    Annotation *annotation = new PDFAnnotation(dPdfAnnot);
    QPoint point(0, 0);

    m_tester->m_bHandAndLink = true;
    m_tester->showNoteEditWidget(annotation, point);
    EXPECT_TRUE(m_tester->m_noteEditWidget == nullptr);

    m_tester->m_bHandAndLink = false;
    m_tester->showNoteEditWidget(annotation, point);
    EXPECT_FALSE(m_tester->m_noteEditWidget == nullptr);
    delete dPdfAnnot;
    delete annotation;
}

TEST_F(TestSheetBrowser, testtranslate2Local001)
{
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->m_sheet->m_operation.scaleFactor = 0;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translate2Local(QPointF(100, 100)).x(), 0));

    m_tester->m_sheet->m_operation.scaleFactor = 0.5;
    EXPECT_TRUE(qFuzzyCompare(m_tester->translate2Local(QPointF(100, 100)).x(), 200));

    delete sheet;
}

TEST_F(TestSheetBrowser, testgetClickAnnot001)
{
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 0, sheet);

    QPointF clickPoint(100, 100);
    bool drawRect = true;

    EXPECT_TRUE(m_tester->getClickAnnot(page, clickPoint, drawRect) == nullptr);

    m_tester->m_sheet = sheet;
    m_tester->m_sheet->m_operation.scaleFactor = 0.5;

    EXPECT_TRUE(m_tester->getClickAnnot(page, clickPoint, drawRect) == nullptr);

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    page->m_annotations.append(annot);
    page->m_hasLoadedAnnotation = true;
    typedef QList<QRectF>(*fptr)(PDFAnnotation *);
    fptr A_foo = (fptr)(&PDFAnnotation::boundary);
    Stub s;
    s.set(A_foo, boundary_stub3);
    EXPECT_FALSE(m_tester->getClickAnnot(page, clickPoint, drawRect) == nullptr);

    delete sheet;
    delete page;
    delete dAnnot;
}

TEST_F(TestSheetBrowser, testjump2PagePos001)
{
    BrowserPage *jumpPage = nullptr;
    qreal posLeft = 10;
    qreal posTop = 100;
    m_tester->jump2PagePos(jumpPage, posLeft, posTop);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    jumpPage = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_sheet = sheet;

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy0;
    m_tester->jump2PagePos(jumpPage, posLeft, posTop);
    EXPECT_TRUE(m_tester->m_bNeedNotifyCurPageChanged);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy90;
    m_tester->jump2PagePos(jumpPage, posLeft, posTop);
    EXPECT_TRUE(m_tester->m_bNeedNotifyCurPageChanged);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy180;
    m_tester->jump2PagePos(jumpPage, posLeft, posTop);
    EXPECT_TRUE(m_tester->m_bNeedNotifyCurPageChanged);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy270;
    m_tester->jump2PagePos(jumpPage, posLeft, posTop);
    EXPECT_TRUE(m_tester->m_bNeedNotifyCurPageChanged);

    delete sheet;
    delete jumpPage;
}

TEST_F(TestSheetBrowser, testcurrentIndexRange001)
{
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_sheet = sheet;
    int fromIndex = 0;
    int toIndex = 2;

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy0;
    m_tester->currentIndexRange(fromIndex, toIndex);
    EXPECT_EQ(fromIndex, 0);
    EXPECT_EQ(toIndex, 1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy180;
    m_tester->currentIndexRange(fromIndex, toIndex);
    EXPECT_EQ(fromIndex, 0);
    EXPECT_EQ(toIndex, 1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy270;
    m_tester->currentIndexRange(fromIndex, toIndex);
    EXPECT_EQ(fromIndex, 0);
    EXPECT_EQ(toIndex, 1);

    delete sheet;
    delete page;
}

TEST_F(TestSheetBrowser, testselectedWordsText001)
{
    Stub s;
    s.set(ADDR(BrowserPage, selectedWords), selectedWords_stub);

    EXPECT_TRUE(m_tester->selectedWordsText() == "testtest");
}

TEST_F(TestSheetBrowser, testannotations001)
{
    Stub s;
    s.set(ADDR(BrowserPage, annotations), annotations_stub);

    QList<Annotation *> annotlist = m_tester->annotations();
    EXPECT_TRUE(annotlist.size() == 2);

    qDeleteAll(annotlist);
    annotlist.clear();
    qDeleteAll(g_dAnnotlsit);
    g_dAnnotlsit.clear();
}

TEST_F(TestSheetBrowser, testremoveAnnotation001)
{
    Stub s;
    s.set(ADDR(BrowserPage, hasAnnotation), hasAnnotation_stub);
    s.set(ADDR(BrowserPage, removeAnnotation), removeAnnotation_stub);

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);

    EXPECT_TRUE(m_tester->removeAnnotation(annot));
    delete dAnnot;
    delete annot;
}

TEST_F(TestSheetBrowser, testremoveAllAnnotation001)
{
    Stub s;
    s.set(ADDR(BrowserPage, removeAllAnnotation), removeAllAnnotation_stub);

    EXPECT_TRUE(m_tester->removeAllAnnotation());
}

TEST_F(TestSheetBrowser, testupdateAnnotation001)
{
    Stub s;
    s.set(ADDR(BrowserPage, hasAnnotation), hasAnnotation_stub);
    s.set(ADDR(BrowserPage, updateAnnotation), updateAnnotation_stub);

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    QString text;
    QColor color;

    EXPECT_TRUE(m_tester->updateAnnotation(annot, text, color));

    text = "test";
    EXPECT_TRUE(m_tester->updateAnnotation(annot, text, color));
    delete dAnnot;
    delete annot;
}

TEST_F(TestSheetBrowser, testonRemoveAnnotation001)
{
    Stub s;
    s.set(ADDR(DocSheet, removeAnnotation), removeAnnotation_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->onRemoveAnnotation(nullptr, true);
    EXPECT_TRUE(g_funcName == "removeAnnotation_stub");
    delete sheet;
}

TEST_F(TestSheetBrowser, testonUpdateAnnotation001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, updateAnnotation), updateAnnotation_stub);

    m_tester->onUpdateAnnotation(nullptr, "test");
    EXPECT_TRUE(g_funcName == "updateAnnotation_stub");
}

TEST_F(TestSheetBrowser, testonSetDocSlideGesture001)
{
    m_tester->onSetDocSlideGesture();
}

TEST_F(TestSheetBrowser, testonRemoveDocSlideGesture001)
{
    m_tester->onRemoveDocSlideGesture();
}

TEST_F(TestSheetBrowser, testonRemoveIconAnnotSelect001)
{
    m_tester->m_selectIconAnnotation = true;
    m_tester->onRemoveIconAnnotSelect();
    EXPECT_FALSE(m_tester->m_selectIconAnnotation);
}

TEST_F(TestSheetBrowser, testonInit001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    s.set(ADDR(SheetBrowser, onViewportChanged), onViewportChanged_stub);

    m_tester->m_initPage = 2;
    m_tester->onInit();
    EXPECT_TRUE(m_tester->m_initPage == 1);
}

TEST_F(TestSheetBrowser, testjumpToOutline001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);

    qreal linkLeft = 10;
    qreal linkTop = 100;
    int index = 1;

    m_tester->jumpToOutline(linkLeft, linkTop, index);
    EXPECT_FALSE(g_funcName == "setCurrentPage_stub");

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy90;
    m_tester->jumpToOutline(linkLeft, linkTop, index);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy0;
    m_tester->jumpToOutline(linkLeft, linkTop, index);
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
    delete sheet;
}

TEST_F(TestSheetBrowser, testwheelEvent001)
{
    Stub s;
    s.set(ADDR(QApplication, keyboardModifiers), keyboardModifiers_stub);

    QPointF pos;
    int delta;
    Qt::MouseButtons buttons = Qt::LeftButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QWheelEvent *event = new QWheelEvent(pos, delta, buttons, modifiers);
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->wheelEvent(event);
    EXPECT_TRUE(g_funcName == "keyboardModifiers_stub");

    s.set(ADDR(QWidget, isHidden), isHidden_stub);
    m_tester->wheelEvent(event);
    EXPECT_FALSE(g_funcName == "isHidden_stub");

    delete sheet;
    delete event;
}

TEST_F(TestSheetBrowser, testevent001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, showMenu), showMenu_stub);
    s.set(ADDR(SheetBrowser, gestureEvent), gestureEvent_stub);

    QEvent::Type type = QEvent::KeyPress;
    int key = Qt::Key_Tab;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QKeyEvent *event = new QKeyEvent(type, key, modifiers);

    m_tester->event(event);
    EXPECT_FALSE(g_funcName == "showMenu_stub");

    m_tester->m_lastSelectIconAnnotPage = m_tester->m_items.first();
    m_tester->event(event);
    EXPECT_FALSE(g_funcName == "showMenu_stub");
    delete event;

    key = Qt::Key_Menu;
    event = new QKeyEvent(type, key, modifiers);
    m_tester->event(event);
    EXPECT_TRUE(g_funcName == "showMenu_stub");
    delete event;

    key = Qt::Key_M;
    modifiers = Qt::AltModifier;
    event = new QKeyEvent(type, key, modifiers);
    m_tester->event(event);
    EXPECT_TRUE(g_funcName == "showMenu_stub");
    delete event;

    type = QEvent::Gesture;
    event = new QKeyEvent(type, key, modifiers);
    m_tester->event(event);
    EXPECT_TRUE(g_funcName == "gestureEvent_stub");
    delete event;
}

TEST_F(TestSheetBrowser, testgestureEvent001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, pinchTriggered), pinchTriggered_stub);
    s.set(ADDR(QGestureEvent, gesture), gesture_stub);

    QGesture *gesture = new QGesture();
    QList<QGesture *> gestures = {gesture};
    QGestureEvent *event = new QGestureEvent(gestures);

    m_tester->gestureEvent(event);
    EXPECT_TRUE(g_funcName == "pinchTriggered_stub");

    delete g_gesture;
    qDeleteAll(gestures);
    delete event;
}

TEST_F(TestSheetBrowser, testpinchTriggered001)
{
    Stub s;
    s.set(ADDR(QGesture, state), state_stub);
    s.set(ADDR(QPinchGesture, changeFlags), changeFlags_stub);
    s.set(ADDR(QPinchGesture, rotationAngle), rotationAngle_stub);
    s.set(ADDR(DocSheet, rotateLeft), rotateLeft_stub);
    s.set(ADDR(DocSheet, rotateRight), rotateRight_stub);

    QPinchGesture *gesture = new QPinchGesture();
    m_tester->m_startPinch = false;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->pinchTriggered(gesture);

    EXPECT_TRUE(g_funcName == "rotateRight_stub");

    delete gesture;
    delete sheet;
}

TEST_F(TestSheetBrowser, testpinchTriggered002)
{
    Stub s;
    s.set(ADDR(QGesture, state), state_stub);
    s.set(ADDR(QPinchGesture, changeFlags), changeFlags_stub);
    s.set(ADDR(QPinchGesture, rotationAngle), rotationAngle_stub2);
    s.set(ADDR(DocSheet, rotateLeft), rotateLeft_stub);
    s.set(ADDR(DocSheet, rotateRight), rotateRight_stub);

    QPinchGesture *gesture = new QPinchGesture();
    m_tester->m_startPinch = false;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->pinchTriggered(gesture);

    EXPECT_TRUE(g_funcName == "rotateLeft_stub");

    delete gesture;
    delete sheet;
}

TEST_F(TestSheetBrowser, testpinchTriggered003)
{
    Stub s;
    s.set(ADDR(QGesture, state), state_stub2);
    s.set(ADDR(QPinchGesture, changeFlags), changeFlags_stub2);
    s.set(ADDR(QPinchGesture, rotationAngle), rotationAngle_stub2);
    s.set(ADDR(DocSheet, setScaleFactor), setScaleFactor_stub);

    QPinchGesture *gesture = new QPinchGesture();
    m_tester->m_startPinch = false;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->pinchTriggered(gesture);

    EXPECT_TRUE(g_funcName == "setScaleFactor_stub");

    delete gesture;
    delete sheet;
}

TEST_F(TestSheetBrowser, testhasLoaded001)
{
    EXPECT_TRUE(m_tester->hasLoaded() == m_tester->m_hasLoaded);
}

TEST_F(TestSheetBrowser, testresizeEvent001)
{
//    Stub s;
//    s.set(ADDR(SheetBrowser, deform), deform_stub);
//    s.set(ADDR(SheetBrowser, openMagnifier), openMagnifier_stub);
//    s.set(ADDR(QWidget, isVisible), isVisible_stub);

//    QSize size(10, 20);
//    QSize oldSize(30, 60);
//    QResizeEvent *event = new QResizeEvent(size, oldSize);
//    m_tester->m_hasLoaded = true;

//    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
//    m_tester->m_sheet = sheet;
//    m_tester->m_sheet->m_operation.scaleMode = Dr::FitToPageDefaultMode;
//    m_tester->m_findWidget = new FindWidget(m_tester);
//    m_tester->m_magnifierLabel = new BrowserMagniFier(m_tester);
//    m_tester->resizeEvent(event);

//    QEventLoop eventloop;
//    QTimer::singleShot(5, &eventloop, SLOT(quit()));
//    eventloop.exec();

//    EXPECT_TRUE(g_funcName == "openMagnifier_stub");

//    delete event;
//    delete sheet;
}

TEST_F(TestSheetBrowser, testmouseMoveEvent001)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(QMouseEvent, source), source_stub);
    s.set(ADDR(QScroller, handleInput), handleInput_stub);
    typedef void (*fptr)(QGraphicsView *, QMouseEvent *);
    fptr A_foo = (fptr)(&QGraphicsView::mouseMoveEvent);   //获取虚函数地址
    s.set(A_foo, mouseMoveEvent_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(50, 50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
    m_tester->m_selectPressedPos = QPointF(30, 30);

    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(g_funcName == "handleInput_stub");

    delete event;
}

TEST_F(TestSheetBrowser, testmouseMoveEvent002)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
//    s.set(ADDR(QMouseEvent, source), source_stub);
//    s.set(ADDR(QScroller, handleInput), handleInput_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(50, 50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
//    m_tester->m_selectPressedPos = QPointF(40, 40);
    m_tester->m_selectIconAnnotation = true;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *browserPage = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_lastSelectIconAnnotPage = browserPage;
    m_tester->m_iconAnnotationMovePos = QPointF(1, 1);


    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(m_tester->m_iconAnnotationMovePos == QPointF(0, 0));

    delete event;
    delete browserPage;
    delete sheet;
}

TEST_F(TestSheetBrowser, testmouseMoveEvent003)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
    s.set(ADDR(DocSheet, setSidebarVisible), setSidebarVisible_stub);
    s.set(ADDR(SheetBrowser, magnifierOpened), magnifierOpened_stub);
    s.set(ADDR(SheetBrowser, showMagnigierImage), showMagnigierImage_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(50, 50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(g_funcName == "showMagnigierImage_stub");

    delete event;
    delete sheet;
}

TEST_F(TestSheetBrowser, testmouseMoveEvent004)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
    s.set(ADDR(DocSheet, setSidebarVisible), setSidebarVisible_stub);
    s.set(ADDR(SheetBrowser, magnifierOpened), magnifierOpened_stub2);
    s.set(ADDR(SheetBrowser, showMagnigierImage), showMagnigierImage_stub);
    s.set(ADDR(BrowserPage, loadWords), loadWords_stub);
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub2);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QRectF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub3);
    s.set(ADDR(QGraphicsItem, setSelected), setSelected_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(-50, -50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->m_selectIndex = 1;
    m_tester->m_selectPressedPos = QPointF(10, 10);

    BrowserPage *browserPage = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_items.append(browserPage);


    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(g_funcName == "setSelected_stub");

    delete event;
    delete sheet;
    delete g_pBrowserPage2;
    qDeleteAll(g_QGraphicsItemList);
    g_QGraphicsItemList.clear();
}

TEST_F(TestSheetBrowser, testmouseMoveEvent005)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
    s.set(ADDR(DocSheet, setSidebarVisible), setSidebarVisible_stub);
    s.set(ADDR(SheetBrowser, magnifierOpened), magnifierOpened_stub2);
    s.set(ADDR(SheetBrowser, showMagnigierImage), showMagnigierImage_stub);
    s.set(ADDR(BrowserPage, getBrowserAnnotation), getBrowserAnnotation_stub);
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QPointF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub2);
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsScene::*)(const QRectF &, Qt::ItemSelectionMode, Qt::SortOrder, const QTransform &) const >(ADDR(QGraphicsScene, items)), items_stub3);
    s.set(ADDR(QGraphicsItem, setSelected), setSelected_stub);
    s.set(ADDR(SheetBrowser, getLinkAtPoint), getLinkAtPoint_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(-50, -50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->m_selectIndex = 1;
//    m_tester->m_selectPressedPos = QPointF(10, 10);

    BrowserPage *browserPage = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_items.append(browserPage);


    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(g_funcName == "setSidebarVisible_stub");

    delete event;
    delete sheet;
    delete g_pBrowserPage2;
    qDeleteAll(g_QGraphicsItemList);
    g_QGraphicsItemList.clear();

    delete g_pDPdfAnnot;
    delete g_pPDFAnnotation;
    delete g_pBrowserAnnotation;
}

TEST_F(TestSheetBrowser, testmouseMoveEvent006)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
    s.set(ADDR(QMouseEvent, source), source_stub2);
    s.set(ADDR(SheetBrowser, magnifierOpened), magnifierOpened_stub2);
    s.set(ADDR(SheetBrowser, showMagnigierImage), showMagnigierImage_stub);
    s.set(ADDR(BrowserPage, getBrowserAnnotation), getBrowserAnnotation_stub);
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetBrowser, getLinkAtPoint), getLinkAtPoint_stub);
    s.set(ADDR(DocSheet, setSidebarVisible), setSidebarVisible_stub);

    typedef QString(*fptr)(PDFAnnotation *);
    fptr PDFAnnotation_contents = (fptr)(&PDFAnnotation::contents);
    s.set(PDFAnnotation_contents, contents_stub);
    s.set(ADDR(QWidget, show), show_stub);

    typedef void (*fptr1)(QGraphicsView *, QMouseEvent *);
    fptr1 A_foo = (fptr1)(&QGraphicsView::mouseMoveEvent);   //获取虚函数地址
    s.set(A_foo, mouseMoveEvent_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(-50, -50);
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_bHandAndLink = false;
    m_tester->m_canTouchScreen = true;
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;
    m_tester->m_selectIndex = 1;

    BrowserPage *browserPage = new BrowserPage(nullptr, 0, sheet);
    m_tester->m_items.append(browserPage);


    m_tester->mouseMoveEvent(event);
    EXPECT_TRUE(m_tester->m_tipsWidget->m_text == "test");

    delete event;
    delete sheet;
    delete g_pBrowserPage2;
    qDeleteAll(g_QGraphicsItemList);
    g_QGraphicsItemList.clear();

    delete g_pDPdfAnnot;
    delete g_pPDFAnnotation;
    delete g_pBrowserAnnotation;
}

TEST_F(TestSheetBrowser, testmouseReleaseEvent001)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(QMouseEvent, source), source_stub);
    s.set(ADDR(QScroller, handleInput), handleInput_stub);
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetBrowser, getClickAnnot), getClickAnnot_stub);
    s.set(ADDR(SheetBrowser, updateAnnotation), updateAnnotation_stub);
    s.set(ADDR(SheetBrowser, showNoteEditWidget), showNoteEditWidget_stub);
    s.set(ADDR(QPointF, manhattanLength), manhattanLength_stub);
    s.set(ADDR(SheetBrowser, jump2Link), jump2Link_stub);
    s.set(ADDR(QWidget, isHidden), isHidden_stub);
    s.set(ADDR(SheetBrowser, getLinkAtPoint), getLinkAtPoint_stub);

    typedef void (*fptr)(QGraphicsView *, QMouseEvent *);
    fptr A_foo = (fptr)(&QGraphicsView::mouseReleaseEvent);   //获取虚函数地址
    s.set(A_foo, mouseReleaseEvent_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(50, 50);
    Qt::MouseButton button = Qt::LeftButton;
    Qt::MouseButtons buttons = Qt::LeftButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_selectIconAnnotation = false;
    m_tester->m_canTouchScreen = true;
    m_tester->m_annotationInserting = true;
    m_tester->m_bHandAndLink = true;

    m_tester->mouseReleaseEvent(event);
    EXPECT_TRUE(g_funcName == "jump2Link_stub");
    EXPECT_FALSE(m_tester->m_annotationInserting);

    delete event;
    delete g_pBrowserPage2;
    delete g_pPDFAnnotation;
    delete g_pDPdfAnnot;
}

TEST_F(TestSheetBrowser, testmouseReleaseEvent002)
{
    Stub s;
    s.set(ADDR(QGraphicsView, dragMode), dragMode_stub);
    s.set(ADDR(QMouseEvent, source), source_stub);
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetBrowser, getClickAnnot), getClickAnnot_stub);
    s.set(ADDR(QPointF, manhattanLength), manhattanLength_stub);
    s.set(ADDR(SheetBrowser, jump2Link), jump2Link_stub);
    s.set(ADDR(QWidget, isHidden), isHidden_stub);

    typedef void (*fptr)(QGraphicsView *, QMouseEvent *);
    fptr A_foo = (fptr)(&QGraphicsView::mouseReleaseEvent);   //获取虚函数地址
    s.set(A_foo, mouseReleaseEvent_stub);

    QEvent::Type type = QEvent::MouseMove;
    QPointF localPos(50, 50);
    Qt::MouseButton button = Qt::LeftButton;
    Qt::MouseButtons buttons = Qt::LeftButton;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QMouseEvent *event = new QMouseEvent(type, localPos, button, buttons, modifiers);

    m_tester->m_startPinch = false;
    m_tester->m_selectIconAnnotation = true;
    m_tester->m_canTouchScreen = false;
    m_tester->m_annotationInserting = true;
    m_tester->m_bHandAndLink = true;
    m_tester->m_lastSelectIconAnnotPage = m_tester->m_items.first();

    m_tester->mouseReleaseEvent(event);
    EXPECT_TRUE(g_funcName == "jump2Link_stub");
    EXPECT_FALSE(m_tester->m_selectIconAnnotation);

    delete event;
    delete g_pBrowserPage2;
    delete g_pPDFAnnotation;
    delete g_pDPdfAnnot;
}

TEST_F(TestSheetBrowser, testfocusOutEvent001)
{
    QEvent::Type type = QEvent::FocusOut;
    QFocusEvent *event = new QFocusEvent(type);

    m_tester->m_bHandAndLink = true;
    m_tester->focusOutEvent(event);

    EXPECT_FALSE(m_tester->m_bHandAndLink);
    delete event;
}

//TEST_F(TestSheetBrowser, testtimerEvent001)
//{
//    Stub s;
//    s.set(ADDR(QTimerEvent, timerId), timerId_stub);
//    s.set(ADDR(QBasicTimer, timerId), timerId_stub);

//    QTimerEvent *event = new QTimerEvent(1000);

//    m_tester->m_canTouchScreen = true;
//    m_tester->timerEvent(event);

//    EXPECT_FALSE(m_tester->m_canTouchScreen);
//    delete event;
//}

TEST_F(TestSheetBrowser, testallPages001)
{
    EXPECT_TRUE(m_tester->allPages() == 2);
}

TEST_F(TestSheetBrowser, testcurrentPage001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, currentScrollValueForPage), currentScrollValueForPage_stub);

    m_tester->m_currentPage = 2;
    EXPECT_TRUE(m_tester->currentPage() == 2);

    m_tester->m_currentPage = 0;
    EXPECT_TRUE(m_tester->currentPage() == 1);
}

TEST_F(TestSheetBrowser, testcurrentScrollValueForPage001)
{
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy0;
    EXPECT_TRUE(m_tester->currentScrollValueForPage() == 1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy90;
    EXPECT_TRUE(m_tester->currentScrollValueForPage() == 1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy180;
    EXPECT_TRUE(m_tester->currentScrollValueForPage() == 1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy270;
    EXPECT_TRUE(m_tester->currentScrollValueForPage() == 1);

    delete sheet;
}

TEST_F(TestSheetBrowser, testsetCurrentPage001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, curpageChanged), curpageChanged_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->setCurrentPage(0);
    EXPECT_FALSE(g_funcName == "curpageChanged_stub");

    m_tester->setCurrentPage(1);

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy0;
    EXPECT_TRUE(g_funcName == "curpageChanged_stub");

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy90;
    EXPECT_TRUE(g_funcName == "curpageChanged_stub");

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy180;
    EXPECT_TRUE(g_funcName == "curpageChanged_stub");

    m_tester->m_sheet->m_operation.rotation = Dr::RotateBy270;
    EXPECT_TRUE(g_funcName == "curpageChanged_stub");

    delete sheet;
}

TEST_F(TestSheetBrowser, testgetExistImage001)
{
    Stub s;
    s.set(ADDR(BrowserPage, getCurrentImage), getCurrentImage_stub);

    QImage image;
    EXPECT_FALSE(m_tester->getExistImage(10, image, 20, 10));
    EXPECT_TRUE(m_tester->getExistImage(0, image, 20, 10));
}

TEST_F(TestSheetBrowser, testgetBrowserPageForPoint001)
{
    QPointF viewPoint;
    EXPECT_TRUE(m_tester->getBrowserPageForPoint(viewPoint) == nullptr);

    Stub s;
    s.set(static_cast<QList<QGraphicsItem *>(QGraphicsView::*)(const QPoint &pos) const>(ADDR(QGraphicsView, items)), items_stub4);
    typedef bool (*fptr)(QGraphicsItem *, const QPointF &);
    fptr QGraphicsItem_contains = (fptr)(&QGraphicsItem::contains);   //获取虚函数地址
    s.set(QGraphicsItem_contains, contains_stub);
    EXPECT_FALSE(m_tester->getBrowserPageForPoint(viewPoint) == nullptr);

    qDeleteAll(g_QGraphicsItemList);
    g_QGraphicsItemList.clear();
}

TEST_F(TestSheetBrowser, testaddIconAnnotation001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, calcIconAnnotRect), calcIconAnnotRect_stub);
    s.set(ADDR(BrowserPage, addIconAnnotation), addIconAnnotation_stub);

    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *browserPage = new BrowserPage(nullptr, 0, &sheet);
    QPointF clickPoint;
    QString contents("test");

    EXPECT_TRUE(m_tester->addIconAnnotation(browserPage, clickPoint, contents) == nullptr);

    qDeleteAll(g_QGraphicsItemList);
    g_QGraphicsItemList.clear();
    delete g_pDPdfAnnot;
    delete g_pPDFAnnotation;
    delete browserPage;
}

TEST_F(TestSheetBrowser, testopenMagnifier001)
{
    Stub s;
    s.set(ADDR(QWidget, show), show_stub);

    m_tester->openMagnifier();
    EXPECT_TRUE(g_funcName == "show_stub");

    m_tester->openMagnifier();
    EXPECT_TRUE(g_funcName == "show_stub");
}

TEST_F(TestSheetBrowser, testcloseMagnifier001)
{
    Stub s;
    s.set(ADDR(QWidget, show), show_stub);

    m_tester->m_magnifierLabel = new BrowserMagniFier(m_tester);
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    m_tester->m_sheet->m_operation.mouseShape = Dr::MouseShapeHand;
    m_tester->closeMagnifier();
    EXPECT_TRUE(m_tester->dragMode() == QGraphicsView::ScrollHandDrag);

    m_tester->m_sheet->m_operation.mouseShape = Dr::MouseShapeNormal;
    m_tester->closeMagnifier();
    EXPECT_TRUE(m_tester->dragMode() == QGraphicsView::NoDrag);

    delete m_tester->m_magnifierLabel;
    delete sheet;
}

TEST_F(TestSheetBrowser, testmagnifierOpened001)
{
    EXPECT_FALSE(m_tester->magnifierOpened());
}

TEST_F(TestSheetBrowser, testmaxHeight001)
{
    EXPECT_TRUE(qFuzzyCompare(m_tester->maxHeight(), m_tester->m_maxWidth));
}

TEST_F(TestSheetBrowser, testneedBookmark001)
{
    QSignalSpy spy(m_tester, SIGNAL(sigNeedBookMark(int, bool)));
    m_tester->needBookmark(0, true);
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestSheetBrowser, testdragEnterEvent001)
{
    Stub s;
    s.set(static_cast<void(QDragMoveEvent::*)()>(ADDR(QDragMoveEvent, ignore)), ignore_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data;
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;

    QDragEnterEvent *event = new QDragEnterEvent(pos, actions, data, buttons, modifiers);
    m_tester->dragEnterEvent(event);
    EXPECT_TRUE(g_funcName == "ignore_stub");

    delete event;
}

TEST_F(TestSheetBrowser, testshowEvent001)
{
//    Stub s;
//    s.set(ADDR(SheetBrowser, onInit), onInit_stub);

//    QShowEvent *event = new QShowEvent();
//    m_tester->showEvent(event);

//    QEventLoop eventloop;
//    QTimer::singleShot(100, &eventloop, SLOT(quit()));
//    eventloop.exec();

//    EXPECT_TRUE(g_funcName == "onInit_stub");

//    delete event;
}

TEST_F(TestSheetBrowser, testhandlePrepareSearch001)
{
    Stub s;
    s.set(ADDR(FindWidget, updatePosition), showPosition_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    sheet->m_fileType = Dr::FileType::PDF;
    m_tester->m_sheet = sheet;

    m_tester->handlePrepareSearch();

    EXPECT_TRUE(g_funcName == "showPosition_stub");

    delete sheet;
}

TEST_F(TestSheetBrowser, testhandleSearchStart001)
{
    Stub s;
    s.set(ADDR(BrowserPage, clearSearchHighlightRects), clearSearchHighlightRects_stub);

    m_tester->handleSearchStart();

    EXPECT_TRUE(g_funcName == "clearSearchHighlightRects_stub");
}

TEST_F(TestSheetBrowser, testhandleSearchStop001)
{
    Stub s;
    s.set(ADDR(BrowserPage, clearSearchHighlightRects), clearSearchHighlightRects_stub);

    m_tester->handleSearchStop();

    EXPECT_TRUE(g_funcName == "clearSearchHighlightRects_stub");
}

TEST_F(TestSheetBrowser, testhandleSearchResultComming001)
{
    Stub s;
    s.set(ADDR(BrowserPage, setSearchHighlightRectf), setSearchHighlightRectf_stub);
    s.set(ADDR(SheetBrowser, jumpToNextSearchResult), jumpToNextSearchResult_stub);

    SearchResult res;
    res.page = 1;
    m_tester->m_searchPageTextIndex = -1;

    m_tester->handleSearchResultComming(res);

    EXPECT_TRUE(g_funcName == "jumpToNextSearchResult_stub");
}

TEST_F(TestSheetBrowser, testhandleFindFinished001)
{
    Stub s;
    s.set(ADDR(FindWidget, setEditAlert), setEditAlert_stub);

    m_tester->m_findWidget = new FindWidget(m_tester);

    m_tester->handleFindFinished(0);

    EXPECT_TRUE(g_funcName == "setEditAlert_stub");
}

TEST_F(TestSheetBrowser, testcurpageChanged001)
{
    m_tester->m_currentPage = 0;

    m_tester->curpageChanged(1);

    EXPECT_TRUE(m_tester->m_currentPage == 1);
}

TEST_F(TestSheetBrowser, testisLink001)
{
    EXPECT_FALSE(m_tester->isLink(QPointF(20, 20)));

    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetRenderer, inLink), inLink_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    EXPECT_TRUE(m_tester->isLink(QPointF(20, 20)));

    delete sheet;
    delete g_pBrowserPage2;
}

TEST_F(TestSheetBrowser, testgetLinkAtPoint001)
{
    EXPECT_TRUE(m_tester->getLinkAtPoint(QPointF(20, 20)).page == -1);

    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetRenderer, getLinkAtPoint), getLinkAtPoint_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    EXPECT_TRUE(m_tester->getLinkAtPoint(QPointF(20, 20)).page == 1);

    delete sheet;
    delete g_pBrowserPage2;
}

TEST_F(TestSheetBrowser, testsetIconAnnotSelect001)
{
    Stub s;
    s.set(ADDR(BrowserPage, setSelectIconRect), setSelectIconRect_stub);

    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 1, &sheet);
    m_tester->m_lastSelectIconAnnotPage = page;
    m_tester->setIconAnnotSelect(true);

    EXPECT_TRUE(g_funcName == "setSelectIconRect_stub");

    delete page;
}

TEST_F(TestSheetBrowser, testsetDocTapGestrue001)
{
    EXPECT_TRUE(m_tester->setDocTapGestrue(QPoint(20, 20)));
}

TEST_F(TestSheetBrowser, testsetDocTapGestrue002)
{
    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(BrowserPage, getBrowserAnnotation), getBrowserAnnotation_stub2);
    s.set(ADDR(SheetBrowser, isLink), isLink_stub);

    EXPECT_TRUE(m_tester->setDocTapGestrue(QPoint(20, 20)));

    delete g_pBrowserPage2;
}

TEST_F(TestSheetBrowser, testsetDocTapGestrue003)
{
    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(BrowserPage, getBrowserAnnotation), getBrowserAnnotation_stub);

    EXPECT_FALSE(m_tester->setDocTapGestrue(QPoint(20, 20)));

    delete g_pBrowserPage2;
    delete g_pDPdfAnnot;
    delete g_pPDFAnnotation;
    delete g_pBrowserAnnotation;
}

TEST_F(TestSheetBrowser, testclearSelectIconAnnotAfterMenu001)
{
    m_tester->m_selectIconAnnotation = true;
    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 1, &sheet);
    m_tester->m_lastSelectIconAnnotPage = page;

    m_tester->clearSelectIconAnnotAfterMenu();

    EXPECT_FALSE(m_tester->m_selectIconAnnotation);
    delete page;
}

TEST_F(TestSheetBrowser, testjump2Link001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetRenderer, getLinkAtPoint), getLinkAtPoint_stub);
    s.set(ADDR(SheetBrowser, jump2PagePos), jump2PagePos_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    EXPECT_TRUE(m_tester->jump2Link(QPointF(20, 20)));
    EXPECT_TRUE(g_funcName == "jump2PagePos_stub");

    delete sheet;
    delete g_pBrowserPage2;
}

TEST_F(TestSheetBrowser, testjump2Link002)
{
    Stub s;
    s.set(ADDR(SheetBrowser, getBrowserPageForPoint), getBrowserPageForPoint_stub);
    s.set(ADDR(SheetRenderer, getLinkAtPoint), getLinkAtPoint_stub2);
    s.set(ADDR(SheetBrowser, jump2PagePos), jump2PagePos_stub);
    s.set(ADDR(QDesktopServices, openUrl), openUrl_stub);
    typedef int (*fptr)(DDialog *);
    fptr DDialog_exec = (fptr)(&DDialog::exec);   //获取虚函数地址
    s.set(DDialog_exec, exec_stub2);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->m_sheet = sheet;

    EXPECT_TRUE(m_tester->jump2Link(QPointF(20, 20)));
    EXPECT_TRUE(g_funcName == "openUrl_stub");

    delete sheet;
    delete g_pBrowserPage2;
}

TEST_F(TestSheetBrowser, testpages001)
{
    EXPECT_TRUE(m_tester->pages() == m_tester->m_items);
}

TEST_F(TestSheetBrowser, testshowMagnigierImage001)
{
    Stub s;
    s.set(ADDR(BrowserMagniFier, showMagnigierImage), showMagnigierImage_stub);

    m_tester->setFixedSize(150, 150);
    m_tester->m_magnifierLabel = new BrowserMagniFier(m_tester);
    m_tester->showMagnigierImage(QPoint(100, 100));

    EXPECT_TRUE(g_funcName == "showMagnigierImage_stub");
}

TEST_F(TestSheetBrowser, testgetAnnotPosInPage001)
{
    Stub s;
    s.set(ADDR(BrowserPage, rect), rect_stub);

    QPointF pos(30, 30);

    DocSheet sheet(Dr::FileType::PDF, "1.pdf", nullptr);
    BrowserPage *page = new BrowserPage(nullptr, 0, &sheet);
    page->m_scaleFactor = 2;

    EXPECT_TRUE(m_tester->getAnnotPosInPage(pos, page) == QPointF(20, 20));

    EXPECT_TRUE(g_funcName == "showMagnigierImage_stub");

    delete page;
}

TEST_F(TestSheetBrowser, testsetIsSearchResultNotEmpty001)
{
    m_tester->m_isSearchResultNotEmpty = false;
    m_tester->m_findWidget = new FindWidget(m_tester);

    m_tester->setIsSearchResultNotEmpty(true);
    EXPECT_TRUE(m_tester->m_isSearchResultNotEmpty);
}

TEST_F(TestSheetBrowser, testgetNoteEditWidget001)
{
    EXPECT_TRUE(m_tester->getNoteEditWidget() == m_tester->m_noteEditWidget);
}

TEST_F(TestSheetBrowser, testfirstThumbnail001)
{
    Stub s;
    typedef QImage(*fptr)(PDFPage *, int, int, const QRect &);
    fptr PDFPage_render = (fptr)(&PDFPage::render);
    s.set(PDFPage_render, render_stub2);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";

    EXPECT_FALSE(m_tester->firstThumbnail(strPath).isNull());
}
