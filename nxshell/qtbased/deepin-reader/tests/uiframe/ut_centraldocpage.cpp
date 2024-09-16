// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CentralDocPage.h"
#include "DBusObject.h"
#include "DocTabBar.h"
#include "SaveDialog.h"
#include "SlideWidget.h"
#include "ReaderImageThreadPoolManager.h"
#include "MainWindow.h"
#include "stub.h"

#include <DDialog>

#include <QProcess>
#include <QDesktopServices>
#include <QSignalSpy>
#include <QLayout>
#include <QStackedLayout>
#include <QFileDialog>

#include <gtest/gtest.h>

using namespace deepin_reader;

/********测试CentralDocPage***********/
class TestCentralDocPage : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    QWidget *w = nullptr;
    CentralDocPage *m_tester = nullptr;
};
void TestCentralDocPage::SetUp()
{
    w = new QWidget();
    m_tester = new CentralDocPage(w);
}

void TestCentralDocPage::TearDown()
{
    delete w;
}
/**********桩函数*************/
static QString g_funcName;
static DocSheet *g_docsheet;
static DocSheet *getCurSheet_stub()
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    if (!g_docsheet) {
        g_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    }
    return g_docsheet;
}

static DocSheet *getCurSheet_stub2()
{
    return g_docsheet;
}

static DocSheet *getCurSheet_stub_nullptr()
{
    return nullptr;
}

bool startDetached_stub(const QString &)
{
    g_funcName = __FUNCTION__;
    return false;
}

static bool openUrl_stub(const QUrl &)
{
    g_funcName = __FUNCTION__;
    return true;
}

bool existFileChanged_stub_true()
{
    g_funcName = __FUNCTION__;
    return true;
}

bool existFileChanged_stub_false()
{
    g_funcName = __FUNCTION__;
    return false;
}

void blockShutdown_stub()
{
    g_funcName = __FUNCTION__;
}

void unBlockShutdown_stub()
{
    g_funcName = __FUNCTION__;
}

void insertSheet_stub(DocSheet *, int)
{
    g_funcName = __FUNCTION__;
}

void enterSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

int indexOfFilePath_stub(const QString &)
{
    return 1;
}

int count_stub()
{
    return 2;
}

void setCurrentIndex_stub(int)
{
    g_funcName = __FUNCTION__;
}

void showTips_stub(QWidget *, const QString &, int)
{
    g_funcName = __FUNCTION__;
}

static void openFileAsync_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

void defaultFocus_stub()
{
    g_funcName = __FUNCTION__;
}

bool closeSheet_stub(DocSheet *, bool)
{
    g_funcName = __FUNCTION__;
    return true;
}

void leaveSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

static void show_stub()
{
    g_funcName = __FUNCTION__;
}

static void addSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

void removeWidget_stub(QWidget *)
{
    g_funcName = __FUNCTION__;
}

int showExitDialog_stub_0(QString, QWidget *)
{
    g_funcName = __FUNCTION__;
    return 0;
}

int showExitDialog_stub_2(QString, QWidget *)
{
    g_funcName = __FUNCTION__;
    return 2;
}

bool saveAsCurrent_stub_true()
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveAsCurrent_stub_false()
{
    g_funcName = __FUNCTION__;
    return false;
}

bool saveData_stub_true()
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveData_stub_false()
{
    g_funcName = __FUNCTION__;
    return false;
}

static QList<DocSheet *> g_listDocSheet;
static QList<DocSheet *> getSheets_stub()
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_listDocSheet.append(sheet);
    return g_listDocSheet;
}

static void showSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

QWidget *currentWidget_stub()
{
    if (g_docsheet) {
        g_docsheet->m_documentChanged = true;
        g_docsheet->m_fileType = Dr::PDF;
    }
    return static_cast<QWidget *>(g_docsheet);
}

static int exec_stub()
{
    g_funcName = __FUNCTION__;
    return QDialog::Accepted;
}

QString getSaveFileName_stub(QWidget *, const QString &, const QString &, QString *, QFileDialog::Options)
{
    return "/.pdf";
}

QString getSaveFileName_stub1(QWidget *, const QString &, const QString &, QString *, QFileDialog::Options)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    return strPath;
}

QString getSaveFileName_stub2(QWidget *, const QString &, const QString &, QString *, QFileDialog::Options)
{
    return "/.djvu";
}

Dr::FileType fileType_stub()
{
    return Dr::DJVU;
}

Dr::FileType fileType_stub1()
{
    return Dr::DOCX;
}

void quitSlide_stub()
{
    g_funcName = __FUNCTION__;
}

void quitMagnifer_stub()
{
    g_funcName = __FUNCTION__;
}

void addgetDocImageTask_stub(const ReaderImageParam_t &)
{

}

static bool magnifierOpened_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

void handleKeyPressEvent_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

static bool saveCurrent_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

static void handleBlockShutdown_stub()
{
    g_funcName = __FUNCTION__;
}

void openSlide_stub()
{
    g_funcName = __FUNCTION__;
}

static void openMagnifer_stub()
{
    g_funcName = __FUNCTION__;
}

void openFullScreen_stub()
{
    g_funcName = __FUNCTION__;
}

static void openMagnifier_stub()
{
    g_funcName = __FUNCTION__;
}

void closeMagnifier_stub()
{
    g_funcName = __FUNCTION__;
}

bool opened_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

static void prepareSearch_stub()
{
    g_funcName = __FUNCTION__;
}

void zoomin_stub()
{
    g_funcName = __FUNCTION__;
}

void zoomout_stub()
{
    g_funcName = __FUNCTION__;
}

bool saveAsData_stub(QString)
{
    g_funcName = __FUNCTION__;
    return true;
}

static MainWindow *g_mainWindow;
MainWindow *createWindow_stub(DocSheet *sheet)
{
    g_mainWindow = new MainWindow(sheet);
    return g_mainWindow;
}
/***********测试用例***********/
TEST_F(TestCentralDocPage, UT_CentralDocPage_firstThumbnail_001)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.djvu";
    QString thumbnailPath = UTSOURCEDIR;
    thumbnailPath += "/files/firstThumbnail.png";
    EXPECT_TRUE(CentralDocPage::firstThumbnail(strPath, thumbnailPath));
    QFile::remove(thumbnailPath);
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_openCurFileFolder_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(static_cast<bool (*)(const QString &)>(ADDR(QProcess, startDetached)), startDetached_stub);
    s.set(ADDR(QDesktopServices, openUrl), openUrl_stub);
    m_tester->openCurFileFolder();

    qDebug() << "g_funcName" << g_funcName;
//    EXPECT_TRUE(g_funcName == "openUrl_stub");
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onSheetFileChanged_001)
{
    Stub s;
    s.set(ADDR(DocSheet, existFileChanged), existFileChanged_stub_true);
    s.set(ADDR(DBusObject, blockShutdown), blockShutdown_stub);
    s.set(ADDR(DBusObject, unBlockShutdown), unBlockShutdown_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    QSignalSpy spy(m_tester, SIGNAL(sigCurSheetChanged(DocSheet *)));

    m_tester->onSheetFileChanged(sheet);

    EXPECT_TRUE(g_funcName == "blockShutdown_stub");
    EXPECT_TRUE(spy.count() == 1);
    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onSheetFileChanged_002)
{
    Stub s;
    s.set(ADDR(DocSheet, existFileChanged), existFileChanged_stub_false);
    s.set(ADDR(DBusObject, blockShutdown), blockShutdown_stub);
    s.set(ADDR(DBusObject, unBlockShutdown), unBlockShutdown_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    QSignalSpy spy(m_tester, SIGNAL(sigCurSheetChanged(DocSheet *)));

    m_tester->onSheetFileChanged(sheet);

    EXPECT_TRUE(g_funcName == "unBlockShutdown_stub");
    EXPECT_TRUE(spy.count() == 1);

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onSheetFileChanged_003)
{
    Stub s;
    s.set(ADDR(DocSheet, existFileChanged), existFileChanged_stub_false);
    s.set(ADDR(DBusObject, blockShutdown), blockShutdown_stub);
    s.set(ADDR(DBusObject, unBlockShutdown), unBlockShutdown_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);

    QSignalSpy spy(m_tester, SIGNAL(sigCurSheetChanged(DocSheet *)));

    m_tester->onSheetFileChanged(nullptr);

    EXPECT_TRUE(spy.count() == 0);
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onSheetOperationChanged_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = nullptr;

    QSignalSpy spy(m_tester, SIGNAL(sigCurSheetChanged(DocSheet *)));

    m_tester->onSheetFileChanged(sheet);
    EXPECT_TRUE(spy.count() == 0);

    sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    m_tester->onSheetFileChanged(sheet);
    EXPECT_TRUE(spy.count() == 1);

    delete sheet;
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_addSheet_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, insertSheet), insertSheet_stub);
    s.set(ADDR(CentralDocPage, enterSheet), enterSheet_stub);
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    m_tester->addSheet(sheet);
    EXPECT_TRUE(g_funcName == "enterSheet_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_addFileAsync_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, indexOfFilePath), indexOfFilePath_stub);
    s.set(ADDR(DTabBar, count), count_stub);
    s.set(ADDR(DTabBar, setCurrentIndex), setCurrentIndex_stub);
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    m_tester->addFileAsync(strPath);

    EXPECT_TRUE(g_funcName == "setCurrentIndex_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_addFileAsync_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.png";
    m_tester->addFileAsync(strPath);

    EXPECT_TRUE(g_funcName == "showTips_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_addFileAsync_003)
{
    Stub s;
    s.set(ADDR(DocSheet, openFileAsync), openFileAsync_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";

    m_tester->disconnect();
    m_tester->addFileAsync(strPath);

    EXPECT_TRUE(g_funcName == "openFileAsync_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onOpened_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    Document::Error error = Document::FileError;

    m_tester->onOpened(sheet, error);

    EXPECT_TRUE(g_funcName == "showTips_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onOpened_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    Document::Error error = Document::FileDamaged;

    m_tester->onOpened(sheet, error);

    EXPECT_TRUE(g_funcName == "showTips_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onOpened_003)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    Document::Error error = Document::ConvertFailed;

    m_tester->onOpened(sheet, error);

    EXPECT_TRUE(g_funcName == "showTips_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onOpened_004)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    Document::Error error = Document::NoError;

    m_tester->onOpened(nullptr, error);

    g_funcName.clear();
    EXPECT_FALSE(g_funcName == "showTips_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onOpened_005)
{
    Stub s;
    s.set(ADDR(DocSheet, defaultFocus), defaultFocus_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    Document::Error error = Document::NoError;

    m_tester->onOpened(sheet, error);

    EXPECT_TRUE(g_funcName == "defaultFocus_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onTabChanged_001)
{
    Stub s;
    s.set(ADDR(DocSheet, defaultFocus), defaultFocus_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->onTabChanged(sheet);

    EXPECT_TRUE(g_funcName == "defaultFocus_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onTabMoveIn_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, enterSheet), enterSheet_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->onTabMoveIn(nullptr);
    EXPECT_FALSE(g_funcName == "enterSheet_stub");

    g_funcName.clear();
    m_tester->onTabMoveIn(sheet);
    EXPECT_TRUE(g_funcName == "enterSheet_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onTabClosed_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, closeSheet), closeSheet_stub);

    g_funcName.clear();
    m_tester->onTabClosed(nullptr);
    EXPECT_TRUE(g_funcName == "closeSheet_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onTabMoveOut_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, leaveSheet), leaveSheet_stub);


    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->onTabMoveOut(nullptr);
    EXPECT_FALSE(g_funcName == "leaveSheet_stub");

    g_funcName.clear();
    m_tester->onTabMoveOut(sheet);
    EXPECT_TRUE(g_funcName == "leaveSheet_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onTabNewWindow_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, leaveSheet), leaveSheet_stub);
    s.set(ADDR(QWidget, show), show_stub);
    s.set(static_cast<MainWindow*(*)(DocSheet *)>(ADDR(MainWindow, createWindow)), createWindow_stub);
//    s.set(ADDR(MainWindow, createWindow), createWindow_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->onTabNewWindow(sheet);
    EXPECT_TRUE(g_funcName == "show_stub");

    delete sheet;
    delete g_mainWindow;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onCentralMoveIn_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, addSheet), addSheet_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->addSheet(sheet);
    EXPECT_TRUE(g_funcName == "addSheet_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_leaveSheet_001)
{
    Stub s;
    s.set(ADDR(QLayout, removeWidget), removeWidget_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    g_funcName.clear();
    m_tester->leaveSheet(nullptr);
    EXPECT_FALSE(g_funcName == "removeWidget_stub");

    g_funcName.clear();
    m_tester->leaveSheet(sheet);
    EXPECT_TRUE(g_funcName == "removeWidget_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeSheet_001)
{
    Stub s;
    s.set(ADDR(SaveDialog, showExitDialog), showExitDialog_stub_0);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    sheet->m_documentChanged = true;

    g_funcName.clear();
    EXPECT_FALSE(m_tester->closeSheet(nullptr, true));

    g_funcName.clear();
    EXPECT_FALSE(m_tester->closeSheet(sheet, true));
    EXPECT_TRUE(g_funcName == "showExitDialog_stub_0");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeSheet_002)
{
    Stub s;
    s.set(ADDR(SaveDialog, showExitDialog), showExitDialog_stub_2);
    s.set(ADDR(CentralDocPage, saveAsCurrent), saveAsCurrent_stub_false);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.docx";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    sheet->m_documentChanged = true;
    sheet->m_fileType = Dr::DOCX;

    g_funcName.clear();
    EXPECT_FALSE(m_tester->closeSheet(sheet, true));
    EXPECT_TRUE(g_funcName == "saveAsCurrent_stub_false");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeSheet_003)
{
    Stub s;
    s.set(ADDR(SaveDialog, showExitDialog), showExitDialog_stub_2);
    s.set(ADDR(CentralDocPage, saveAsCurrent), saveAsCurrent_stub_true);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.docx";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    sheet->m_documentChanged = true;
    sheet->m_fileType = Dr::DOCX;

    g_funcName.clear();
    EXPECT_TRUE(m_tester->closeSheet(sheet, true));
    EXPECT_TRUE(g_funcName == "saveAsCurrent_stub_true");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeSheet_004)
{
    Stub s;
    s.set(ADDR(SaveDialog, showExitDialog), showExitDialog_stub_2);
    s.set(ADDR(DocSheet, saveData), saveData_stub_false);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    sheet->m_documentChanged = true;
    sheet->m_fileType = Dr::PDF;

    g_funcName.clear();
    EXPECT_FALSE(m_tester->closeSheet(sheet, true));
    EXPECT_TRUE(g_funcName == "saveData_stub_false");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeSheet_005)
{
    Stub s;
    s.set(ADDR(SaveDialog, showExitDialog), showExitDialog_stub_2);
    s.set(ADDR(DocSheet, saveData), saveData_stub_true);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    sheet->m_documentChanged = true;
    sheet->m_fileType = Dr::PDF;

    g_funcName.clear();
    EXPECT_TRUE(m_tester->closeSheet(sheet, true));
    EXPECT_TRUE(g_funcName == "saveData_stub_true");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_closeAllSheets_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, getSheets), getSheets_stub);
    s.set(ADDR(CentralDocPage, showSheet), showSheet_stub);
    s.set(ADDR(CentralDocPage, closeSheet), closeSheet_stub);

    g_funcName.clear();
    EXPECT_TRUE(m_tester->closeAllSheets(true));
    EXPECT_TRUE(g_funcName == "closeSheet_stub");

    qDeleteAll(g_listDocSheet);
    g_listDocSheet.clear();
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_enterSheet_001)
{
    Stub s;
    s.set(ADDR(DocSheet, defaultFocus), defaultFocus_stub);

    g_funcName.clear();
    m_tester->enterSheet(nullptr);
    EXPECT_FALSE(g_funcName == "defaultFocus_stub");

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    g_funcName.clear();
    m_tester->disconnect();
    m_tester->enterSheet(sheet);
    EXPECT_TRUE(g_funcName == "defaultFocus_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_hasSheet_001)
{
    EXPECT_FALSE(m_tester->hasSheet(nullptr));

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    EXPECT_TRUE(m_tester->hasSheet(sheet));

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_showSheet_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, showSheet), showSheet_stub);

    g_funcName.clear();
    m_tester->showSheet(nullptr);
    EXPECT_FALSE(g_funcName == "showSheet_stub");

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    g_funcName.clear();
    m_tester->showSheet(sheet);
    EXPECT_TRUE(g_funcName == "showSheet_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_getSheets_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, getSheets), getSheets_stub);

    EXPECT_TRUE(m_tester->getSheets().count() == 1);

    qDeleteAll(g_listDocSheet);
    g_listDocSheet.clear();
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveCurrent_001)
{
    Stub s;
    s.set(ADDR(DocSheet, saveData), saveData_stub_false);
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    g_funcName.clear();
    EXPECT_FALSE(m_tester->saveCurrent());
    EXPECT_FALSE(g_funcName == "showTips_stub");

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    g_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    s.set(ADDR(QStackedLayout, currentWidget), currentWidget_stub);
    g_funcName.clear();
    EXPECT_FALSE(m_tester->saveCurrent());
    EXPECT_TRUE(g_funcName == "showTips_stub");
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveCurrent_002)
{
    Stub s;
    s.set(ADDR(DocSheet, saveData), saveData_stub_true);
    s.set(ADDR(CentralDocPage, showTips), showTips_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    g_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    s.set(ADDR(QStackedLayout, currentWidget), currentWidget_stub);
    g_funcName.clear();
    EXPECT_TRUE(m_tester->saveCurrent());
    EXPECT_TRUE(g_funcName == "showTips_stub");

    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveAsCurrent_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    typedef int (*fptr)(DDialog *);
    fptr DDialog_exec = (fptr)(&DDialog::exec);   //获取虚函数地址
    s.set(DDialog_exec, exec_stub);
    s.set(ADDR(QFileDialog, getSaveFileName), getSaveFileName_stub);

    EXPECT_FALSE(m_tester->saveAsCurrent());

    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveAsCurrent_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    typedef int (*fptr)(DDialog *);
    fptr DDialog_exec = (fptr)(&DDialog::exec);   //获取虚函数地址
    s.set(DDialog_exec, exec_stub);
    s.set(ADDR(QFileDialog, getSaveFileName), getSaveFileName_stub2);
    s.set(ADDR(DocSheet, fileType), fileType_stub);

    EXPECT_FALSE(m_tester->saveAsCurrent());

    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveAsCurrent_003)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(CentralDocPage, saveCurrent), saveCurrent_stub);
    s.set(ADDR(CentralDocPage, handleBlockShutdown), handleBlockShutdown_stub);
    s.set(ADDR(DocSheet, saveAsData), saveAsData_stub);
    typedef int (*fptr)(DDialog *);
    fptr DDialog_exec = (fptr)(&DDialog::exec);
    s.set(DDialog_exec, exec_stub);
    s.set(ADDR(QFileDialog, getSaveFileName), getSaveFileName_stub1);

    EXPECT_TRUE(m_tester->saveAsCurrent());
    EXPECT_TRUE(g_funcName == "handleBlockShutdown_stub");
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_saveAsCurrent_004)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(CentralDocPage, saveCurrent), saveCurrent_stub);
    s.set(ADDR(CentralDocPage, handleBlockShutdown), handleBlockShutdown_stub);
    s.set(ADDR(DocSheet, saveAsData), saveAsData_stub);
    typedef int (*fptr)(DDialog *);
    fptr DDialog_exec = (fptr)(&DDialog::exec);
    s.set(DDialog_exec, exec_stub);
    s.set(ADDR(QFileDialog, getSaveFileName), getSaveFileName_stub1);
    s.set(ADDR(DocSheet, fileType), fileType_stub1);

    EXPECT_TRUE(m_tester->saveAsCurrent());
    EXPECT_TRUE(g_funcName == "saveAsData_stub");
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_getCurSheet_001)
{
    EXPECT_TRUE(m_tester->getCurSheet() == nullptr);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    g_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    Stub s;
    s.set(ADDR(QStackedLayout, currentWidget), currentWidget_stub);
    EXPECT_FALSE(m_tester->getCurSheet() == nullptr);
    delete g_docsheet;
    g_docsheet = nullptr;
}


TEST_F(TestCentralDocPage, UT_CentralDocPage_getSheet_001)
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";

    EXPECT_TRUE(m_tester->getSheet(strPath) == nullptr);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    EXPECT_FALSE(m_tester->getSheet(strPath) == nullptr);

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitSlide), quitSlide_stub);
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    SlideWidget *slide = new SlideWidget(sheet);
    m_tester->m_slideWidget = slide;

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_esc);

    EXPECT_TRUE(g_funcName == "quitSlide_stub");

    delete sheet;
    delete slide;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitSlide), quitSlide_stub);
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    SlideWidget *slide = new SlideWidget(sheet);
    m_tester->m_slideWidget = slide;

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_f11);

    EXPECT_FALSE(g_funcName == "quitSlide_stub");

    delete sheet;
    delete slide;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_003)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitMagnifer), quitMagnifer_stub);
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);
    s.set(ADDR(DocSheet, magnifierOpened), magnifierOpened_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    m_tester->m_magniferSheet = sheet;

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_esc);

    EXPECT_TRUE(g_funcName == "quitMagnifer_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_004)
{
    Stub s;
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);
    s.set(ADDR(SlideWidget, handleKeyPressEvent), handleKeyPressEvent_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    SlideWidget *slide = new SlideWidget(sheet);
    m_tester->m_slideWidget = slide;

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_f5);

    EXPECT_TRUE(g_funcName == "handleKeyPressEvent_stub");

    delete sheet;
    delete slide;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_005)
{
    Stub s;
    s.set(ADDR(CentralDocPage, saveCurrent), saveCurrent_stub);
    s.set(ADDR(CentralDocPage, saveAsCurrent), saveAsCurrent_stub_true);
    s.set(ADDR(CentralDocPage, handleBlockShutdown), handleBlockShutdown_stub);
    s.set(ADDR(CentralDocPage, openSlide), openSlide_stub);
    s.set(ADDR(CentralDocPage, openMagnifer), openMagnifer_stub);
    s.set(ADDR(SlideWidget, handleKeyPressEvent), handleKeyPressEvent_stub);

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_ctrl_s);
    EXPECT_TRUE(g_funcName == "handleBlockShutdown_stub");

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_ctrl_shift_s);
    EXPECT_TRUE(g_funcName == "saveAsCurrent_stub_true");

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_f5);
    EXPECT_TRUE(g_funcName == "openSlide_stub");

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_alt_z);
    EXPECT_TRUE(g_funcName == "openMagnifer_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleShortcut_006)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(DocSheet, openFullScreen), openFullScreen_stub);

    g_funcName.clear();
    m_tester->handleShortcut(Dr::key_f11);
    EXPECT_TRUE(g_funcName == "openFullScreen_stub");

    delete g_docsheet;
    g_docsheet = nullptr;

}

TEST_F(TestCentralDocPage, UT_CentralDocPage_showTips_001)
{
    QSignalSpy spy(m_tester, SIGNAL(sigNeedShowTips(QWidget *, const QString &, int)));

    m_tester->showTips(static_cast<QWidget *>(m_tester->parent()), "test", 0);

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_openMagnifer_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, quitMagnifer), quitMagnifer_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(DocSheet, openMagnifier), openMagnifier_stub);

    m_tester->openMagnifer();

    EXPECT_TRUE(g_funcName == "openMagnifier_stub");

    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_quitMagnifer_001)
{
    Stub s;
    s.set(ADDR(DocSheet, closeMagnifier), closeMagnifier_stub);
    s.set(ADDR(DocSheet, magnifierOpened), magnifierOpened_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, m_tester);
    m_tester->m_magniferSheet = sheet;

    m_tester->quitMagnifer();

    EXPECT_TRUE(g_funcName == "closeMagnifier_stub");

    delete sheet;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_openSlide_001)
{
    Stub s;
    s.set(ADDR(DocSheet, opened), opened_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub2);
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    g_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);

    m_tester->openSlide();
    EXPECT_FALSE(m_tester->m_slideWidget == nullptr);

    delete m_tester->m_slideWidget;
    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_quitSlide_001)
{
    Stub s;
    s.set(ADDR(DocSheet, opened), opened_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub2);
    s.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), addgetDocImageTask_stub);

    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    SlideWidget *slide = new SlideWidget(sheet);
    m_tester->m_slideWidget = slide;

    m_tester->quitSlide();
    EXPECT_TRUE(m_tester->m_slideWidget == nullptr);

    delete sheet;
    delete slide;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_isSlide_001)
{
    EXPECT_FALSE(m_tester->isSlide());
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_prepareSearch_001)
{
    Stub s;
    s.set(ADDR(DocSheet, prepareSearch), prepareSearch_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);

    m_tester->prepareSearch();
    EXPECT_TRUE(g_funcName == "prepareSearch_stub");

    delete g_docsheet;
    g_docsheet = nullptr;
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_onUpdateTabLabelText_001)
{
    m_tester->m_tabBar->addTab("one");
    m_tester->onUpdateTabLabelText();
    EXPECT_TRUE(m_tester->m_tabLabel->text() == "one");
}


TEST_F(TestCentralDocPage, UT_CentralDocPage_getTitleLabel_001)
{
    EXPECT_TRUE(m_tester->getTitleLabel() == m_tester->m_tabLabel);
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleBlockShutdown_001)
{
    Stub s;
    s.set(ADDR(DocTabBar, getSheets), getSheets_stub);
    s.set(ADDR(DocSheet, existFileChanged), existFileChanged_stub_true);
    s.set(ADDR(DBusObject, blockShutdown), blockShutdown_stub);

    m_tester->handleBlockShutdown();
    EXPECT_TRUE(g_funcName == "blockShutdown_stub");

    qDeleteAll(g_listDocSheet);
    g_listDocSheet.clear();
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_handleBlockShutdown_002)
{
    Stub s;
    s.set(ADDR(DBusObject, unBlockShutdown), unBlockShutdown_stub);

    m_tester->handleBlockShutdown();
    EXPECT_TRUE(g_funcName == "unBlockShutdown_stub");
}

TEST_F(TestCentralDocPage, UT_CentralDocPage_zoomIn_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(DocSheet, zoomin), zoomin_stub);

    m_tester->zoomIn();
    EXPECT_TRUE(g_funcName == "zoomin_stub");

    delete g_docsheet;
    g_docsheet = nullptr;
}


TEST_F(TestCentralDocPage, UT_CentralDocPage_zoomOut_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(DocSheet, zoomout), zoomout_stub);

    m_tester->zoomOut();
    EXPECT_TRUE(g_funcName == "zoomout_stub");

    delete g_docsheet;
    g_docsheet = nullptr;
}
