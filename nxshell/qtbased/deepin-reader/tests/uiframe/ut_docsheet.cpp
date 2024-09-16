// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DocSheet.h"
#include "SheetBrowser.h"
#include "SheetRenderer.h"
#include "Database.h"
#include "Utils.h"
#include "dpdfannot.h"
#include "PDFModel.h"
#include "CentralDocPage.h"
#include "SheetSidebar.h"
#include "EncryptionPage.h"
#include "PageSearchThread.h"
#include "stub.h"

#include <DPrintPreviewDialog>

#include <QUuid>
#include <QClipboard>
#include <QSignalSpy>
#include <QEvent>
//#include <QStackedLayout>
//#include <QFileDialog>

#include <gtest/gtest.h>

//using namespace deepin_reader;

/********测试DocSheet***********/
static QString g_funcName;
class TestDocSheet : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    CentralDocPage *m_centralDocPage = nullptr;
    DocSheet *m_tester = nullptr;
    DWidget *m_widget = nullptr;
};

void TestDocSheet::SetUp()
{
    m_widget = new DWidget();
    m_centralDocPage = new CentralDocPage(m_widget);
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    m_tester = new DocSheet(Dr::FileType::PDF, strPath, m_centralDocPage);
}

void TestDocSheet::TearDown()
{
    delete m_widget;
    g_funcName.clear();
}

/**********桩函数*************/
namespace {
QImage firstThumbnail_stub(const QString &)
{
    g_funcName = __FUNCTION__;
    return QImage(100, 100, QImage::Format_ARGB32);
}

bool openFileExec_stub(const QString &)
{
    g_funcName = __FUNCTION__;
    return true;
}

void openFileAsync_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

void setCurrentPage_stub(int)
{
    g_funcName = __FUNCTION__;
}


Outline outline_stub()
{
    g_funcName = __FUNCTION__;
    Outline out;
    Section section;
    section.nIndex = 1;
    section.title = "test";
    out.append(section);
    return out;
}

void jumpToOutline_stub(const qreal &, const qreal &, int)
{
    g_funcName = __FUNCTION__;
}

void jumpToHighLight_stub(Annotation *, const int)
{
    g_funcName = __FUNCTION__;
}

void deform_stub(SheetOperation &)
{
    g_funcName = __FUNCTION__;
}

int pageCount_stub()
{
    return 2;
}

void showTips_stub(const QString &, int)
{
    g_funcName = __FUNCTION__;
}

void showTips_stub2(QWidget *, const QString &, int)
{
    g_funcName = __FUNCTION__;
}
int getPageCount_stub()
{
    g_funcName = __FUNCTION__;
    return 2;
}

void setAnnotationInserting_stub(bool)
{
    g_funcName = __FUNCTION__;
}

QImage getImage_stub(int, int, int, const QRect &)
{
    g_funcName = __FUNCTION__;
    return QImage(10, 20, QImage::Format_ARGB32);
}

bool save_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveBookmarks_stub(const QString &, const QSet<int>)
{
    g_funcName = __FUNCTION__;
    return true;
}

void stopSearch_stub()
{
    g_funcName = __FUNCTION__;
}

bool saveAs_stub_true(const QString &)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveAs_stub_false(const QString &)
{
    g_funcName = __FUNCTION__;
    return false;
}

bool copyFile_stub(const QString &, const QString &)
{
    g_funcName = __FUNCTION__;
    return true;
}

QString selectedWordsText_stub()
{
    g_funcName = __FUNCTION__;
    return "test";
}

void setText_stub(const QString &, QClipboard::Mode)
{
    g_funcName = __FUNCTION__;
}

static DPdfTextAnnot *g_dAnnot;
static Annotation *g_annot;
Annotation *addHighLightAnnotation_stub(const QString, const QColor, QPoint &)
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    g_funcName = __FUNCTION__;
    return g_annot;
}

void showNoteEditWidget_stub(Annotation *, const QPoint &)
{
    g_funcName = __FUNCTION__;
}

void openMagnifier_stub()
{
    g_funcName = __FUNCTION__;
}

void closeMagnifier_stub()
{
    g_funcName = __FUNCTION__;
}

bool magnifierOpened_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

QList<Annotation *> annotations_stub()
{
    g_dAnnot = new DPdfTextAnnot;
    g_annot = new PDFAnnotation(g_dAnnot);
    QList<Annotation *> list;
    list.append(g_annot);
    g_funcName = __FUNCTION__;
    return list;
}

bool removeAnnotation_stub(Annotation *)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool removeAllAnnotation_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

Properties properties_stub()
{
    g_funcName = __FUNCTION__;
    Properties p;
    p.insert("Version", QVariant("1.0"));
    return p;
}

QString convertedFileDir_stub()
{
    g_funcName = __FUNCTION__;
    return "test";
}

void openSlide_stub()
{
    g_funcName = __FUNCTION__;
}

void quitSlide_stub()
{
    g_funcName = __FUNCTION__;
}

bool isFullScreen_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

QPoint pos_stub()
{
    g_funcName = __FUNCTION__;
    return QPoint(-1, -1);
}

void showEncryPage_stub()
{
    g_funcName = __FUNCTION__;
}

void wrongPassWordSlot_stub()
{
    g_funcName = __FUNCTION__;
}

void handleOpenSuccess_stub()
{
    g_funcName = __FUNCTION__;
}

void hideSubTipsWidget_stub()
{
    g_funcName = __FUNCTION__;
}

void setSidebarVisible_stub(bool, bool)
{
    g_funcName = __FUNCTION__;
}

void openFullScreen_stub()
{

}

bool quitFullScreen_stub(bool)
{
    return true;
}

Qt::WindowStates windowState_stub()
{
    g_funcName = __FUNCTION__;
    return Qt::WindowFullScreen;
}

void showNormal_stub()
{
    g_funcName = __FUNCTION__;
}

bool pageHasLable_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

void jumpToFirstPage_stub()
{
    g_funcName = __FUNCTION__;
}

void jumpToPrevPage_stub()
{
    g_funcName = __FUNCTION__;
}

void jumpToNextPage_stub()
{
    g_funcName = __FUNCTION__;
}

void jumpToLastPage_stub()
{
    g_funcName = __FUNCTION__;
}

void setBookMark_stub(int, int)
{
    g_funcName = __FUNCTION__;
}

void handleAnntationMsg_stub(const int &, int, deepin_reader::Annotation *)
{
    g_funcName = __FUNCTION__;
}

void handlePrepareSearch_stub()
{
    g_funcName = __FUNCTION__;
}

void handleSearchStart_stub()
{
    g_funcName = __FUNCTION__;
}

void handleSearchStart_stub2(const QString &)
{
    g_funcName = __FUNCTION__;
}

void startSearch_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

void jumpToNextSearchResult_stub()
{
    g_funcName = __FUNCTION__;
}

void jumpToPrevSearchResult_stub()
{
    g_funcName = __FUNCTION__;
}

void handleSearchStop_stub()
{
    g_funcName = __FUNCTION__;
}

void handleSearchResultComming_stub(const SearchResult &)
{
    g_funcName = __FUNCTION__;
}

int handleFindFinished_stub()
{
    g_funcName = __FUNCTION__;
    return 1;
}

void handleFindFinished_stub2(int)
{
    g_funcName = __FUNCTION__;
}

void setIsSearchResultNotEmpty_stub(bool)
{
    g_funcName = __FUNCTION__;
}

void childEvent_stub(QChildEvent *)
{
    g_funcName = __FUNCTION__;
}

bool readOperation_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool readBookmarks_stub(const QString &, QSet<int> &)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveOperation_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
    return true;
}
}
/***********测试用例***********/
TEST_F(TestDocSheet, UT_DocSheet_firstThumbnail_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, firstThumbnail), firstThumbnail_stub);
    DocSheet::g_sheetList.append(m_tester);
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet::firstThumbnail(strPath);
    EXPECT_FALSE(g_funcName == "firstThumbnail_stub");
    DocSheet::g_sheetList.removeAll(m_tester);
}

TEST_F(TestDocSheet, UT_DocSheet_firstThumbnail_002)
{
    Stub s;
    s.set(ADDR(SheetBrowser, firstThumbnail), firstThumbnail_stub);
    QString strPath = UTSOURCEDIR;
    strPath += "/files/nonormal.pdf";
    DocSheet::firstThumbnail(strPath);
    EXPECT_TRUE(g_funcName == "firstThumbnail_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_existFileChanged_001)
{
    m_tester->m_documentChanged = true;
    EXPECT_TRUE(DocSheet::existFileChanged());
}

TEST_F(TestDocSheet, UT_DocSheet_getUuid_001)
{
    EXPECT_TRUE(DocSheet::getUuid(DocSheet::g_sheetList.last()) == DocSheet::g_uuidList.last());
}

TEST_F(TestDocSheet, UT_DocSheet_existSheet_001)
{
    EXPECT_TRUE(DocSheet::existSheet(DocSheet::g_sheetList.last()));
}

TEST_F(TestDocSheet, UT_DocSheet_getSheet_001)
{
    EXPECT_TRUE(DocSheet::getSheet(DocSheet::g_uuidList.last()) == DocSheet::g_sheetList.last());
}

TEST_F(TestDocSheet, UT_DocSheet_getSheetByFilePath_001)
{
    EXPECT_TRUE(DocSheet::getSheetByFilePath(DocSheet::g_sheetList.first()->filePath()) == DocSheet::g_sheetList.first());
}

TEST_F(TestDocSheet, UT_DocSheet_getSheets_001)
{
    EXPECT_TRUE(DocSheet::getSheets() == DocSheet::g_sheetList);
}

TEST_F(TestDocSheet, UT_DocSheet_openFileExec_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, openFileExec), openFileExec_stub);
    EXPECT_TRUE(m_tester->openFileExec("123"));
    EXPECT_TRUE(m_tester->m_password == "123");
}

TEST_F(TestDocSheet, UT_DocSheet_openFileAsync_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, openFileAsync), openFileAsync_stub);
    m_tester->openFileAsync("123");
    EXPECT_TRUE(m_tester->m_password == "123");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToPage_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    m_tester->jumpToPage(1);
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToIndex_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    m_tester->jumpToIndex(0);
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToFirstPage_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    m_tester->jumpToFirstPage();
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToLastPage_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    m_tester->jumpToLastPage();
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToNextPage_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setCurrentPage), setCurrentPage_stub);
    m_tester->jumpToNextPage();
    EXPECT_TRUE(g_funcName == "setCurrentPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_outline_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, outline), outline_stub);
    EXPECT_TRUE(m_tester->outline().size() == 1);
    EXPECT_TRUE(g_funcName == "outline_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToOutline_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, jumpToOutline), jumpToOutline_stub);
    qreal left = 10;
    qreal top = 20;
    int index = 1;
    m_tester->jumpToOutline(left, top, index);
    EXPECT_TRUE(g_funcName == "jumpToOutline_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToHighLight_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, jumpToHighLight), jumpToHighLight_stub);
    m_tester->jumpToHighLight(nullptr, 1);
    EXPECT_TRUE(g_funcName == "jumpToHighLight_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_rotateLeft_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, deform), deform_stub);

    m_tester->m_operation.rotation = Dr::RotateBy0;
    m_tester->rotateLeft();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy270);

    m_tester->m_operation.rotation = Dr::RotateBy270;
    m_tester->rotateLeft();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy180);

    m_tester->m_operation.rotation = Dr::RotateBy180;
    m_tester->rotateLeft();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy90);

    m_tester->m_operation.rotation = Dr::RotateBy90;
    m_tester->rotateLeft();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy0);
}

TEST_F(TestDocSheet, UT_DocSheet_rotateRight_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, deform), deform_stub);

    m_tester->m_operation.rotation = Dr::RotateBy0;
    m_tester->rotateRight();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy90);

    m_tester->m_operation.rotation = Dr::RotateBy270;
    m_tester->rotateRight();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy0);

    m_tester->m_operation.rotation = Dr::RotateBy180;
    m_tester->rotateRight();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy270);

    m_tester->m_operation.rotation = Dr::RotateBy90;
    m_tester->rotateRight();
    EXPECT_TRUE(m_tester->m_operation.rotation == Dr::RotateBy180);
}

TEST_F(TestDocSheet, UT_DocSheet_setBookMark_001)
{
    Stub s;
    s.set(ADDR(DocSheet, pageCount), pageCount_stub);
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    m_tester->setBookMark(2, 1);
    EXPECT_FALSE(g_funcName == "showTips_stub");

    g_funcName.clear();
    m_tester->setBookMark(1, 1);
    EXPECT_FALSE(g_funcName == "showTips_stub");

    g_funcName.clear();
    m_tester->setBookMark(1, 0);
    EXPECT_TRUE(g_funcName == "showTips_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_setBookMarks_001)
{
    Stub s;
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    QList<int> indexlst = {0, 1, 2};
    m_tester->setBookMarks(indexlst, 0);
    EXPECT_TRUE(g_funcName == "showTips_stub");

    g_funcName.clear();
    m_tester->setBookMarks(indexlst, 1);
    EXPECT_FALSE(g_funcName == "showTips_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_pageCount_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, getPageCount), getPageCount_stub);

    EXPECT_TRUE(m_tester->pageCount() == 2);
    EXPECT_TRUE(g_funcName == "getPageCount_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_currentPage_001)
{
    Stub s;
    s.set(ADDR(DocSheet, pageCount), pageCount_stub);

    m_tester->m_operation.currentPage = 0;
    EXPECT_TRUE(m_tester->currentPage() == 1);

    m_tester->m_operation.currentPage = 2;
    EXPECT_TRUE(m_tester->currentPage() == 2);
}

TEST_F(TestDocSheet, UT_DocSheet_currentIndex_001)
{
    Stub s;
    s.set(ADDR(DocSheet, pageCount), pageCount_stub);

    m_tester->m_operation.currentPage = 0;
    EXPECT_TRUE(m_tester->currentIndex() == 0);

    m_tester->m_operation.currentPage = 2;
    EXPECT_TRUE(m_tester->currentIndex() == 1);
}

TEST_F(TestDocSheet, UT_DocSheet_setMouseShape_001)
{
    m_tester->m_operation.mouseShape = Dr::MouseShapeHand;
    m_tester->setMouseShape(Dr::MouseShapeNormal);
    EXPECT_TRUE(m_tester->m_operation.mouseShape == Dr::MouseShapeNormal);
}

TEST_F(TestDocSheet, UT_DocSheet_setAnnotationInserting_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setAnnotationInserting), setAnnotationInserting_stub);

    SheetBrowser *tmp_browser = m_tester->m_browser;
    m_tester->m_browser = nullptr;
    m_tester->setAnnotationInserting(true);
    EXPECT_FALSE(g_funcName == "setAnnotationInserting_stub");

    m_tester->m_browser = tmp_browser;
    m_tester->setAnnotationInserting(true);
    EXPECT_TRUE(g_funcName == "setAnnotationInserting_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_thumbnail_001)
{
    m_tester->m_thumbnailMap.insert(1, QPixmap(10, 20));
    EXPECT_TRUE(m_tester->thumbnail(1).width() == 10);
}

TEST_F(TestDocSheet, UT_DocSheet_setThumbnail_001)
{
    m_tester->setThumbnail(1, QPixmap(10, 20));
    EXPECT_TRUE(m_tester->m_thumbnailMap.size() == 1);
}

TEST_F(TestDocSheet, UT_DocSheet_setScaleMode_001)
{
    m_tester->m_operation.scaleMode = Dr::FitToPageWorHMode;
    m_tester->setScaleMode(Dr::ScaleFactorMode);
    EXPECT_TRUE(m_tester->m_operation.scaleMode == Dr::ScaleFactorMode);
}

TEST_F(TestDocSheet, UT_DocSheet_setScaleFactor_001)
{
    m_tester->m_operation.scaleMode = Dr::FitToPageWidthMode;
    m_tester->m_operation.scaleFactor = 2;
    m_tester->setScaleFactor(1);

    EXPECT_TRUE(m_tester->m_operation.scaleMode == Dr::ScaleFactorMode);
    EXPECT_TRUE(qFuzzyCompare(m_tester->m_operation.scaleFactor, 1));
}

TEST_F(TestDocSheet, UT_DocSheet_getImage_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, getImage), getImage_stub);
    EXPECT_TRUE(m_tester->getImage(0, 10, 20, QRect()).width() == 10);
}

TEST_F(TestDocSheet, UT_DocSheet_fileChanged_001)
{
    m_tester->m_documentChanged = true;
    m_tester->m_bookmarkChanged = false;
    EXPECT_TRUE(m_tester->fileChanged());
}

TEST_F(TestDocSheet, UT_DocSheet_saveData_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, save), save_stub);
    s.set(ADDR(Database, saveBookmarks), saveBookmarks_stub);

    m_tester->m_documentChanged = true;
    m_tester->m_bookmarkChanged = true;
    EXPECT_TRUE(m_tester->saveData());
    EXPECT_FALSE(m_tester->m_documentChanged);
    EXPECT_FALSE(m_tester->m_bookmarkChanged);
}

TEST_F(TestDocSheet, UT_DocSheet_saveAsData_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, saveAs), saveAs_stub_true);
    s.set(ADDR(DocSheet, stopSearch), stopSearch_stub);
    s.set(ADDR(Database, saveBookmarks), saveBookmarks_stub);

    m_tester->m_documentChanged = true;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf.bk";
    EXPECT_TRUE(m_tester->saveAsData(strPath));
}

TEST_F(TestDocSheet, UT_DocSheet_saveAsData_002)
{
    Stub s;
    s.set(ADDR(SheetRenderer, saveAs), saveAs_stub_false);
    s.set(ADDR(DocSheet, stopSearch), stopSearch_stub);
    s.set(ADDR(Database, saveBookmarks), saveBookmarks_stub);

    m_tester->m_documentChanged = true;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf.bk";
    EXPECT_FALSE(m_tester->saveAsData(strPath));
}

TEST_F(TestDocSheet, UT_DocSheet_saveAsData_003)
{
    Stub s;
    s.set(ADDR(SheetRenderer, saveAs), saveAs_stub_true);
    s.set(ADDR(DocSheet, stopSearch), stopSearch_stub);
    s.set(ADDR(Database, saveBookmarks), saveBookmarks_stub);
    s.set(ADDR(Utils, copyFile), copyFile_stub);

    m_tester->m_documentChanged = true;
    m_tester->m_fileType = Dr::DOCX;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf.bk";
    EXPECT_TRUE(m_tester->saveAsData(strPath));
}

TEST_F(TestDocSheet, UT_DocSheet_handlePageModified_001)
{
    QSignalSpy spy(m_tester, SIGNAL(sigPageModified(int)));
    m_tester->handlePageModified(0);
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDocSheet, UT_DocSheet_copySelectedText_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub);
    s.set(ADDR(QClipboard, setText), setText_stub);

    m_tester->copySelectedText();
    EXPECT_TRUE(g_funcName == "setText_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_highlightSelectedText_001)
{
    Stub s;
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    m_tester->highlightSelectedText();
    EXPECT_TRUE(g_funcName == "showTips_stub");

    s.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub);
    s.set(ADDR(SheetBrowser, addHighLightAnnotation), addHighLightAnnotation_stub);
    m_tester->highlightSelectedText();
    EXPECT_TRUE(g_funcName == "addHighLightAnnotation_stub");
    delete g_dAnnot;
    delete g_annot;
}

TEST_F(TestDocSheet, UT_DocSheet_addSelectedTextHightlightAnnotation_001)
{
    Stub s;
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    m_tester->addSelectedTextHightlightAnnotation();
    EXPECT_TRUE(g_funcName == "showTips_stub");

    s.set(ADDR(SheetBrowser, selectedWordsText), selectedWordsText_stub);
    s.set(ADDR(SheetBrowser, addHighLightAnnotation), addHighLightAnnotation_stub);
    s.set(ADDR(SheetBrowser, showNoteEditWidget), showNoteEditWidget_stub);

    m_tester->addSelectedTextHightlightAnnotation();
    EXPECT_TRUE(g_funcName == "showNoteEditWidget_stub");
    delete g_dAnnot;
    delete g_annot;
}

TEST_F(TestDocSheet, UT_DocSheet_openMagnifier_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, openMagnifier), openMagnifier_stub);
    m_tester->openMagnifier();
    EXPECT_TRUE(g_funcName == "openMagnifier_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_closeMagnifier_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, closeMagnifier), closeMagnifier_stub);
    m_tester->closeMagnifier();
    EXPECT_TRUE(g_funcName == "closeMagnifier_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_defaultFocus_001)
{
    m_tester->defaultFocus();
}

TEST_F(TestDocSheet, UT_DocSheet_magnifierOpened_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, magnifierOpened), magnifierOpened_stub);
    m_tester->magnifierOpened();
    EXPECT_TRUE(g_funcName == "magnifierOpened_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_annotations_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, annotations), annotations_stub);
    EXPECT_TRUE(m_tester->annotations().size() == 1);
    EXPECT_TRUE(g_funcName == "annotations_stub");
    delete g_dAnnot;
    delete g_annot;
}

TEST_F(TestDocSheet, UT_DocSheet_removeAnnotation_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, removeAnnotation), removeAnnotation_stub);
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    DPdfTextAnnot *dAnnot = new DPdfTextAnnot;
    PDFAnnotation *annot = new PDFAnnotation(dAnnot);
    EXPECT_TRUE(m_tester->removeAnnotation(annot, true));
    EXPECT_TRUE(g_funcName == "showTips_stub");
    delete dAnnot;
    delete annot;
}

TEST_F(TestDocSheet, UT_DocSheet_removeAllAnnotation_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, removeAllAnnotation), removeAllAnnotation_stub);
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    EXPECT_TRUE(m_tester->removeAllAnnotation());
    EXPECT_TRUE(g_funcName == "showTips_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_scaleFactorList_001)
{
    EXPECT_TRUE(m_tester->scaleFactorList().size() == 12);
}

TEST_F(TestDocSheet, UT_DocSheet_maxScaleFactor_001)
{
    EXPECT_TRUE(qFuzzyCompare(m_tester->maxScaleFactor(), 5));
}

TEST_F(TestDocSheet, UT_DocSheet_filter_001)
{
    m_tester->m_fileType = Dr::PDF;
    EXPECT_TRUE(m_tester->filter() == "Pdf File (*.pdf)");

    m_tester->m_fileType = Dr::DOCX;
    EXPECT_TRUE(m_tester->filter() == "Pdf File (*.pdf)");

    m_tester->m_fileType = Dr::DJVU;
    EXPECT_TRUE(m_tester->filter() == "Djvu files (*.djvu)");

    m_tester->m_fileType = Dr::PPTX;
    EXPECT_TRUE(m_tester->filter().isEmpty());
}

TEST_F(TestDocSheet, UT_DocSheet_format_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, properties), properties_stub);
    s.set(ADDR(DocSheet, showTips), showTips_stub);

    m_tester->m_fileType = Dr::PDF;
    EXPECT_TRUE(m_tester->format() == "PDF 1.0");

    m_tester->m_fileType = Dr::DOCX;
    EXPECT_TRUE(m_tester->format() == "PDF 1.0");

    m_tester->m_fileType = Dr::DJVU;
    EXPECT_TRUE(m_tester->format() == "DJVU");

    m_tester->m_fileType = Dr::PPTX;
    EXPECT_TRUE(m_tester->format().isEmpty());
}

TEST_F(TestDocSheet, UT_DocSheet_getBookMarkList_001)
{
    EXPECT_TRUE(m_tester->getBookMarkList() == m_tester->m_bookmarks);
}

TEST_F(TestDocSheet, UT_DocSheet_operationRef_001)
{
    m_tester->m_operation.currentPage = 2;
    EXPECT_TRUE(m_tester->operationRef().currentPage == 2);
}

TEST_F(TestDocSheet, UT_DocSheet_fileType_001)
{
    EXPECT_TRUE(m_tester->fileType() == m_tester->m_fileType);
}

TEST_F(TestDocSheet, UT_DocSheet_filePath_001)
{
    EXPECT_TRUE(m_tester->filePath() == m_tester->m_filePath);
}

TEST_F(TestDocSheet, UT_DocSheet_openedFilePath_001)
{
    Stub s;
    s.set(ADDR(DocSheet, convertedFileDir), convertedFileDir_stub);

    m_tester->m_fileType = Dr::PDF;
    EXPECT_TRUE(m_tester->openedFilePath() == m_tester->m_filePath);

    m_tester->m_fileType = Dr::DOCX;
    EXPECT_TRUE(m_tester->openedFilePath() == "test/temp.pdf");

}

TEST_F(TestDocSheet, UT_DocSheet_convertedFileDir_001)
{
    EXPECT_FALSE(m_tester->convertedFileDir().isEmpty());
}

TEST_F(TestDocSheet, UT_DocSheet_hasBookMark_001)
{
    m_tester->m_bookmarks.insert(2);
    EXPECT_TRUE(m_tester->hasBookMark(2));
}

TEST_F(TestDocSheet, UT_DocSheet_zoomin_001)
{
    m_tester->m_operation.scaleFactor = 1;
    m_tester->zoomin();
    EXPECT_TRUE(m_tester->m_operation.scaleFactor > 1);
}

TEST_F(TestDocSheet, UT_DocSheet_zoomout_001)
{
    m_tester->m_operation.scaleFactor = 1;
    m_tester->zoomout();
    EXPECT_TRUE(m_tester->m_operation.scaleFactor < 1);
}

TEST_F(TestDocSheet, UT_DocSheet_showTips_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub2);

    m_tester->showTips("test", 1);
    EXPECT_TRUE(g_funcName == "showTips_stub2");
}

//TEST_F(TestDocSheet, UT_DocSheet_onPrintRequested_001)
//{
//    DPrinter *printer = new DPrinter();
//    QVector<int> pageRange;
//    m_tester->onPrintRequested(printer, pageRange);

//    pageRange.append(1);
//    m_tester->onPrintRequested(printer, pageRange);

//    Stub s;
//    s.set(ADDR(CentralDocPage, showTips), showTips_stub2);

//    m_tester->showTips("test", 1);
//    EXPECT_FALSE(g_funcName == "showTips_stub2");
//}

TEST_F(TestDocSheet, UT_DocSheet_openSlide_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, openSlide), openSlide_stub);

    m_tester->openSlide();
    EXPECT_TRUE(g_funcName == "openSlide_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_closeSlide_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitSlide), quitSlide_stub);

    m_tester->closeSlide();
    EXPECT_TRUE(g_funcName == "quitSlide_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_setSidebarVisible_001)
{
    m_tester->m_operation.sidebarVisible = false;
    m_tester->setSidebarVisible(true, true);
    EXPECT_TRUE(m_tester->m_operation.sidebarVisible);
}

TEST_F(TestDocSheet, UT_DocSheet_setSidebarVisible_002)
{
    Stub s;
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
    m_tester->m_operation.sidebarVisible = true;
    m_tester->setSidebarVisible(false, true);
    EXPECT_FALSE(m_tester->m_operation.sidebarVisible);
}

//TEST_F(TestDocSheet, UT_DocSheet_setSidebarVisible_003)
//{
//    Stub s;
//    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);
//    m_tester->m_operation.sidebarVisible = true;
//    m_tester->setSidebarVisible(true, false);
//    EXPECT_TRUE(m_tester->m_operation.sidebarVisible);
//    delete m_tester->m_sideAnimation;
//}

TEST_F(TestDocSheet, UT_DocSheet_onSideAniFinished_001)
{
    Stub s;
    s.set(ADDR(QWidget, pos), pos_stub);
    m_tester->onSideAniFinished();
}

TEST_F(TestDocSheet, UT_DocSheet_onOpened_001)
{
    Stub s;
    s.set(ADDR(DocSheet, showEncryPage), showEncryPage_stub);
    s.set(ADDR(EncryptionPage, wrongPassWordSlot), wrongPassWordSlot_stub);
    s.set(ADDR(SheetSidebar, handleOpenSuccess), handleOpenSuccess_stub);

    m_tester->onOpened(Document::NeedPassword);
    EXPECT_TRUE(g_funcName == "showEncryPage_stub");

    m_tester->m_encryPage = new EncryptionPage(m_tester);
    m_tester->onOpened(Document::WrongPassword);
    EXPECT_TRUE(g_funcName == "wrongPassWordSlot_stub");

    m_tester->m_password = "test";
    m_tester->onOpened(Document::NoError);
    EXPECT_TRUE(g_funcName == "handleOpenSuccess_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_isFullScreen_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, isFullScreen), isFullScreen_stub);

    m_tester->isFullScreen();
    EXPECT_TRUE(g_funcName == "isFullScreen_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_openFullScreen_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, openFullScreen), openFullScreen_stub);
    s.set(ADDR(DocSheet, setSidebarVisible), setSidebarVisible_stub);
    s.set(ADDR(SheetBrowser, hideSubTipsWidget), hideSubTipsWidget_stub);

    m_tester->openFullScreen();
    EXPECT_TRUE(g_funcName == "hideSubTipsWidget_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_closeFullScreen_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitFullScreen), quitFullScreen_stub);
    s.set(ADDR(SheetBrowser, hideSubTipsWidget), hideSubTipsWidget_stub);

    m_tester->closeFullScreen(true);
    EXPECT_TRUE(g_funcName == "hideSubTipsWidget_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_closeFullScreen_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitFullScreen), quitFullScreen_stub);
    s.set(ADDR(QWidget, windowState), windowState_stub);
    s.set(ADDR(QWidget, showNormal), showNormal_stub);
    s.set(ADDR(SheetBrowser, hideSubTipsWidget), hideSubTipsWidget_stub);

    EXPECT_TRUE(m_tester->closeFullScreen(false));
}

TEST_F(TestDocSheet, UT_DocSheet_setDocumentChanged_001)
{
    m_tester->m_documentChanged = false;
    m_tester->setDocumentChanged(true);
    EXPECT_TRUE(m_tester->m_documentChanged);
}

TEST_F(TestDocSheet, UT_DocSheet_setBookmarkChanged_001)
{
    m_tester->m_bookmarkChanged = false;
    m_tester->setBookmarkChanged(true);
    EXPECT_TRUE(m_tester->m_bookmarkChanged);
}

TEST_F(TestDocSheet, UT_DocSheet_setOperationChanged_001)
{
    QSignalSpy spy(m_tester, SIGNAL(sigOperationChanged(DocSheet *)));

    m_tester->setOperationChanged();
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDocSheet, UT_DocSheet_haslabel_001)
{
    Stub s;
    s.set(ADDR(SheetRenderer, pageHasLable), pageHasLable_stub);

    EXPECT_TRUE(m_tester->haslabel());
}

TEST_F(TestDocSheet, UT_DocSheet_docBasicInfo_001)
{
    FileInfo tFileInfo;
    m_tester->docBasicInfo(tFileInfo);
    EXPECT_TRUE(tFileInfo.filePath == m_tester->m_filePath);
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserPageChanged_001)
{
    m_tester->m_operation.currentPage = 2;
    m_tester->onBrowserPageChanged(1);
    EXPECT_TRUE(m_tester->m_operation.currentPage == 1);
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserPageFirst_001)
{
    Stub s;
    s.set(ADDR(DocSheet, jumpToFirstPage), jumpToFirstPage_stub);

    m_tester->onBrowserPageFirst();
    EXPECT_TRUE(g_funcName == "jumpToFirstPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserPagePrev_001)
{
    Stub s;
    s.set(ADDR(DocSheet, jumpToPrevPage), jumpToPrevPage_stub);

    m_tester->onBrowserPagePrev();
    EXPECT_TRUE(g_funcName == "jumpToPrevPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserPageNext_001)
{
    Stub s;
    s.set(ADDR(DocSheet, jumpToNextPage), jumpToNextPage_stub);

    m_tester->onBrowserPageNext();
    EXPECT_TRUE(g_funcName == "jumpToNextPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserPageLast_001)
{
    Stub s;
    s.set(ADDR(DocSheet, jumpToLastPage), jumpToLastPage_stub);

    m_tester->onBrowserPageLast();
    EXPECT_TRUE(g_funcName == "jumpToLastPage_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserBookmark_001)
{
    Stub s;
    s.set(ADDR(DocSheet, setBookMark), setBookMark_stub);

    m_tester->onBrowserBookmark(1, true);
    EXPECT_TRUE(g_funcName == "setBookMark_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onBrowserOperaAnnotation_001)
{
    Stub s;
    s.set(ADDR(SheetSidebar, handleAnntationMsg), handleAnntationMsg_stub);

    m_tester->onBrowserOperaAnnotation(1, 1, nullptr);
    EXPECT_TRUE(g_funcName == "handleAnntationMsg_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_prepareSearch_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, handlePrepareSearch), handlePrepareSearch_stub);

    m_tester->prepareSearch();
    EXPECT_TRUE(g_funcName == "handlePrepareSearch_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_startSearch_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, handleSearchStart), handleSearchStart_stub);
    s.set(ADDR(SheetSidebar, handleSearchStart), handleSearchStart_stub2);
    s.set(ADDR(PageSearchThread, startSearch), startSearch_stub);

    m_tester->startSearch("test");
    EXPECT_TRUE(g_funcName == "startSearch_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToNextSearchResult_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, jumpToNextSearchResult), jumpToNextSearchResult_stub);

    m_tester->jumpToNextSearchResult();
    EXPECT_TRUE(g_funcName == "jumpToNextSearchResult_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_jumpToPrevSearchResult_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, jumpToPrevSearchResult), jumpToPrevSearchResult_stub);

    m_tester->jumpToPrevSearchResult();
    EXPECT_TRUE(g_funcName == "jumpToPrevSearchResult_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_stopSearch_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, handleSearchStop), handleSearchStop_stub);
    s.set(ADDR(SheetSidebar, handleSearchStop), handleSearchStop_stub);
    s.set(ADDR(PageSearchThread, stopSearch), stopSearch_stub);

    m_tester->stopSearch();
    EXPECT_TRUE(g_funcName == "handleSearchStop_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onSearchResultComming_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, handleSearchResultComming), handleSearchResultComming_stub);
    s.set(ADDR(SheetSidebar, handleSearchResultComming), handleSearchResultComming_stub);

    m_tester->onSearchResultComming(SearchResult());
    EXPECT_TRUE(g_funcName == "handleSearchResultComming_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_onSearchFinished_001)
{
    Stub s;
    s.set(ADDR(SheetSidebar, handleFindFinished), handleFindFinished_stub);
    s.set(ADDR(SheetBrowser, handleFindFinished), handleFindFinished_stub2);

    m_tester->onSearchFinished();
    EXPECT_TRUE(g_funcName == "handleFindFinished_stub2");
}

TEST_F(TestDocSheet, UT_DocSheet_onSearchResultNotEmpty_001)
{
    Stub s;
    s.set(ADDR(SheetBrowser, setIsSearchResultNotEmpty), setIsSearchResultNotEmpty_stub);

    m_tester->onSearchResultNotEmpty();
    EXPECT_TRUE(g_funcName == "setIsSearchResultNotEmpty_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_resizeEvent_001)
{
    Stub s;
    s.set(ADDR(DocSheet, isFullScreen), isFullScreen_stub);

    QResizeEvent *event = new QResizeEvent(QSize(10, 10), QSize(30, 30));
    m_tester->m_encryPage = new EncryptionPage(m_tester);
    m_tester->resizeEvent(event);
    EXPECT_TRUE(g_funcName == "isFullScreen_stub");
    delete event;
}

TEST_F(TestDocSheet, UT_DocSheet_childEvent_001)
{
    Stub s;
    typedef void (*fptr)(DSplitter *, QChildEvent *);
    fptr DSplitter_childEvent = (fptr)(&DSplitter::childEvent);
    s.set(DSplitter_childEvent, childEvent_stub);

    QChildEvent *event = new QChildEvent(QEvent::ChildRemoved, nullptr);

    m_tester->childEvent(event);
    EXPECT_TRUE(g_funcName == "childEvent_stub");
    delete event;
}

TEST_F(TestDocSheet, UT_DocSheet_getSheetBrowser_001)
{
    EXPECT_TRUE(m_tester->getSheetBrowser() == m_tester->m_browser);
}

TEST_F(TestDocSheet, UT_DocSheet_setAlive_001)
{
    Stub s;
    s.set(ADDR(Database, readOperation), readOperation_stub);
    s.set(ADDR(Database, readBookmarks), readBookmarks_stub);

    m_tester->setAlive(true);
    EXPECT_TRUE(g_funcName == "readBookmarks_stub");
}

TEST_F(TestDocSheet, UT_DocSheet_setAlive_002)
{
    Stub s;
    s.set(ADDR(DocSheet, stopSearch), stopSearch_stub);
    s.set(ADDR(Database, saveOperation), saveOperation_stub);

    m_tester->m_uuid = DocSheet::g_uuidList.last();
    m_tester->setAlive(false);
    EXPECT_TRUE(g_funcName == "saveOperation_stub");
}
