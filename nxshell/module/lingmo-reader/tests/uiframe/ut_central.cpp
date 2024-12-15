// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Central.h"
#include "TitleMenu.h"
#include "MainWindow.h"
#include "CentralDocPage.h"
#include "ShortCutShow.h"
#include "TitleWidget.h"
#include "stub.h"

#include <QFileDialog>
#include <QStackedLayout>
#include <QMimeData>

#include <gtest/gtest.h>

/********测试Central***********/
class TestCentral : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    QWidget *w = nullptr;
    Central *m_tester = nullptr;
};

void TestCentral::SetUp()
{
    w = new QWidget();
    m_tester = new Central(w);
}

void TestCentral::TearDown()
{
    delete w;
}

/************桩函数*************/
static QString g_funcName;
static int exec_stub()
{
    return QDialog::Accepted;
}

QStringList selectedFiles_stub()
{
    return QStringList() << "1.pdf" << "2.pdf";
}

bool activateSheetIfExist_stub(const QString &)
{
    return false;
}

void addFileAsync_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

static void addSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

bool hasSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
    return false;
}

static void showSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

static QList<DocSheet *> getSheets_stub()
{
    g_funcName = __FUNCTION__;
    QList<DocSheet *> l;
    return l;
}

void addFilesWithDialog_stub()
{
    g_funcName = __FUNCTION__;
}

void setSheet_stub(DocSheet *)
{
    g_funcName = __FUNCTION__;
}

static void show_stub()
{
    g_funcName = __FUNCTION__;
}

void handleShortcut_stub(const QString &)
{
    g_funcName = __FUNCTION__;
}

bool closeAllSheets_stub(bool)
{
    g_funcName = __FUNCTION__;
    return false;
}

void setControlEnabled_stub(const bool &)
{
    g_funcName = __FUNCTION__;
}

static bool saveCurrent_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

bool saveAsCurrent_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

static void handleBlockShutdown_stub()
{
    g_funcName = __FUNCTION__;
}

static void openMagnifer_stub()
{
    g_funcName = __FUNCTION__;
}

void openCurFileFolder_stub()
{
    g_funcName = __FUNCTION__;
}

static void prepareSearch_stub()
{
    g_funcName = __FUNCTION__;
}

static DocSheet *getCurSheet_stub()
{
    g_funcName = __FUNCTION__;
    return nullptr;
}

void onPopPrintDialog_stub()
{
    g_funcName = __FUNCTION__;
}

void activateWindow_stub()
{
    g_funcName = __FUNCTION__;
}

int currentIndex_stub()
{
    g_funcName = __FUNCTION__;
    return 1;
}

void zoomIn_stub()
{
    g_funcName = __FUNCTION__;
}


void zoomOut_stub()
{
    g_funcName = __FUNCTION__;
}

bool isActiveWindow_stub()
{
    return true;
}

static QObject *g_object;
static QObject *sender_stub()
{
    g_object = static_cast<QObject *>(new QAction());
    return g_object;
}

bool hasUrls_stub()
{
    return true;
}

bool hasFormat_stub(const QString &)
{
    return true;
}

void resizeEvent_stub(QResizeEvent *)
{
    g_funcName = __FUNCTION__;
}

static MainWindow *g_mainWindow;
static MainWindow *createWindow_stub()
{
    QStringList filePathList;
    g_mainWindow = new MainWindow(filePathList);
    return g_mainWindow;
}

/***********测试用例***********/
TEST_F(TestCentral, UT_Central_titleWidget_001)
{
    EXPECT_TRUE(m_tester->titleWidget() == m_tester->m_widget);
}

TEST_F(TestCentral, UT_Central_docPage_001)
{
    EXPECT_FALSE(m_tester->docPage() == nullptr);
}

TEST_F(TestCentral, UT_Central_setMenu_001)
{
    TitleMenu *menu = new TitleMenu(m_tester);
    m_tester->setMenu(menu);
    EXPECT_TRUE(m_tester->m_menu == menu);
}

TEST_F(TestCentral, UT_Central_addFilesWithDialog_001)
{
    typedef int (*fptr)(QDialog *);
    fptr QDialog_exec = (fptr)(&QDialog::exec);
    Stub s;
    s.set(QDialog_exec, exec_stub);
    s.set(ADDR(QFileDialog, selectedFiles), selectedFiles_stub);
    s.set(ADDR(MainWindow, activateSheetIfExist), activateSheetIfExist_stub);
    s.set(ADDR(CentralDocPage, addFileAsync), addFileAsync_stub);

    m_tester->addFilesWithDialog();

    EXPECT_TRUE(g_funcName == "addFileAsync_stub");
}

TEST_F(TestCentral, UT_Central_addFileAsync_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, addFileAsync), addFileAsync_stub);

    m_tester->addFileAsync("1.pdf");

    EXPECT_TRUE(g_funcName == "addFileAsync_stub");
}

TEST_F(TestCentral, UT_Central_addSheet_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, addSheet), addSheet_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);
    m_tester->addSheet(sheet);

    EXPECT_TRUE(g_funcName == "addSheet_stub");
    delete sheet;
}

TEST_F(TestCentral, UT_Central_hasSheet_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, hasSheet), hasSheet_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);

    EXPECT_FALSE(m_tester->hasSheet(sheet));
    EXPECT_TRUE(g_funcName == "hasSheet_stub");
    delete sheet;
}

TEST_F(TestCentral, UT_Central_showSheet_001)
{
    Stub s;
    s.set(ADDR(CentralDocPage, showSheet), showSheet_stub);

    DocSheet *sheet = new DocSheet(Dr::FileType::PDF, "1.pdf", nullptr);

    m_tester->showSheet(sheet);
    EXPECT_TRUE(g_funcName == "showSheet_stub");
    delete sheet;
}

TEST_F(TestCentral, UT_Central_getSheets_001)
{
    EXPECT_TRUE(m_tester->getSheets().isEmpty());
}

TEST_F(TestCentral, UT_Central_getSheets_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, getSheets), getSheets_stub);

    m_tester->docPage();
    m_tester->getSheets();
    EXPECT_TRUE(g_funcName == "getSheets_stub");
}

TEST_F(TestCentral, UT_Central_handleShortcut_001)
{
    Stub s;
    s.set(ADDR(Central, addFilesWithDialog), addFilesWithDialog_stub);

    QString shortcut(Dr::key_ctrl_o);
    m_tester->handleShortcut(shortcut);

    EXPECT_TRUE(g_funcName == "addFilesWithDialog_stub");
}

TEST_F(TestCentral, UT_Central_handleShortcut_002)
{
    Stub s;
    s.set(ADDR(ShortCutShow, setSheet), setSheet_stub);
    s.set(ADDR(ShortCutShow, show), show_stub);

    QString shortcut(Dr::key_ctrl_shift_slash);
    m_tester->handleShortcut(shortcut);

    EXPECT_TRUE(g_funcName == "show_stub");
}

TEST_F(TestCentral, UT_Central_handleShortcut_003)
{
    Stub s;
    s.set(ADDR(CentralDocPage, handleShortcut), handleShortcut_stub);

    QString shortcut(Dr::key_ctrl_p);
    m_tester->handleShortcut(shortcut);

    EXPECT_TRUE(g_funcName == "handleShortcut_stub");
}

TEST_F(TestCentral, UT_Central_handleClose_001)
{
    EXPECT_TRUE(m_tester->handleClose(false));
}

TEST_F(TestCentral, UT_Central_handleClose_002)
{
    Stub s;
    s.set(ADDR(CentralDocPage, closeAllSheets), closeAllSheets_stub);

    m_tester->docPage();
    EXPECT_FALSE(m_tester->handleClose(false));
}

TEST_F(TestCentral, UT_Central_onSheetCountChanged_001)
{
    Stub s;
    s.set(ADDR(TitleWidget, setControlEnabled), setControlEnabled_stub);

    m_tester->onSheetCountChanged(1);
    EXPECT_FALSE(g_funcName == "setControlEnabled_stub");

    m_tester->onSheetCountChanged(0);
    EXPECT_TRUE(g_funcName == "setControlEnabled_stub");
}

TEST_F(TestCentral, UT_Central_onMenuTriggered_001)
{
    Stub s;
    s.set(ADDR(QWidget, show), show_stub);
    s.set(ADDR(Central, addFilesWithDialog), addFilesWithDialog_stub);
    s.set(ADDR(CentralDocPage, saveCurrent), saveCurrent_stub);
    s.set(ADDR(CentralDocPage, saveAsCurrent), saveAsCurrent_stub);
    s.set(ADDR(CentralDocPage, handleBlockShutdown), handleBlockShutdown_stub);
    s.set(ADDR(CentralDocPage, openMagnifer), openMagnifer_stub);
    s.set(ADDR(CentralDocPage, openCurFileFolder), openCurFileFolder_stub);
    s.set(ADDR(CentralDocPage, prepareSearch), prepareSearch_stub);
    s.set(ADDR(CentralDocPage, getCurSheet), getCurSheet_stub);
    s.set(ADDR(DocSheet, onPopPrintDialog), onPopPrintDialog_stub);
    s.set(static_cast<MainWindow*(*)(QStringList)>(ADDR(MainWindow, createWindow)), createWindow_stub);

    QString action;

    action = "New window";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "show_stub");
    delete g_mainWindow;

    action = "New tab";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "addFilesWithDialog_stub");

    action = "Save";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "handleBlockShutdown_stub");

    action = "Save as";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "saveAsCurrent_stub");

    action = "Magnifer";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "openMagnifer_stub");

    action = "Display in file manager";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "openCurFileFolder_stub");

    action = "Search";
    m_tester->onMenuTriggered(action);
    EXPECT_TRUE(g_funcName == "prepareSearch_stub");

//    action = "Print";
//    m_tester->onMenuTriggered(action);
//    EXPECT_TRUE(g_funcName == "onPopPrintDialog_stub");
}

TEST_F(TestCentral, UT_Central_onOpenFilesExec_001)
{
    Stub s;
    s.set(ADDR(Central, addFilesWithDialog), addFilesWithDialog_stub);

    m_tester->onOpenFilesExec();
    EXPECT_TRUE(g_funcName == "addFilesWithDialog_stub");
}

TEST_F(TestCentral, UT_Central_onNeedActivateWindow_001)
{
    Stub s;
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    m_tester->onNeedActivateWindow();
    EXPECT_TRUE(g_funcName == "activateWindow_stub");
}

TEST_F(TestCentral, UT_Central_onShowTips_001)
{
    m_tester->onShowTips(qobject_cast<QWidget *>(m_tester->parent()), "test", 0);
    m_tester->onShowTips(qobject_cast<QWidget *>(m_tester->parent()), "test", 1);
}

TEST_F(TestCentral, UT_Central_onShowTips_002)
{
    Stub s;
    s.set(ADDR(QStackedLayout, currentIndex), currentIndex_stub);

    m_tester->onShowTips(qobject_cast<QWidget *>(m_tester->parent()), "test", 0);
    m_tester->onShowTips(qobject_cast<QWidget *>(m_tester->parent()), "test", 1);
}

TEST_F(TestCentral, UT_Central_onShowTips_003)
{
    Stub s;
    s.set(ADDR(QStackedLayout, currentIndex), currentIndex_stub);

    m_tester->onShowTips(nullptr, "test", 0);
    m_tester->onShowTips(nullptr, "test", 1);
}

TEST_F(TestCentral, UT_Central_onTouchPadEvent_001)
{
    Stub s;
    s.set(ADDR(QWidget, isActiveWindow), isActiveWindow_stub);
    s.set(ADDR(CentralDocPage, zoomIn), zoomIn_stub);
    s.set(ADDR(CentralDocPage, zoomOut), zoomOut_stub);

    QString name("pinch");
    QString direction("in");
    int fingers = 2;
    m_tester->docPage();

    m_tester->onTouchPadEvent(name, direction, fingers);
    EXPECT_TRUE(g_funcName == "zoomOut_stub");

    direction = "out";
    m_tester->onTouchPadEvent(name, direction, fingers);
    EXPECT_TRUE(g_funcName == "zoomIn_stub");
}

TEST_F(TestCentral, UT_Central_onKeyTriggered_001)
{
    Stub s;
    s.set(ADDR(Central, handleShortcut), handleShortcut_stub);

    m_tester->onKeyTriggered();
    EXPECT_FALSE(g_funcName == "handleShortcut_stub");

    s.set(ADDR(QObject, sender), sender_stub);
    m_tester->onKeyTriggered();
    EXPECT_TRUE(g_funcName == "handleShortcut_stub");

    delete g_object;
}

TEST_F(TestCentral, UT_Central_dragEnterEvent_001)
{
    Stub s;
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data = new QMimeData();
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QDragEnterEvent *event = new QDragEnterEvent(pos, actions, data, buttons, modifiers);

    m_tester->dragEnterEvent(event);
    EXPECT_FALSE(g_funcName == "activateWindow_stub");
    delete data;
    delete event;
}

TEST_F(TestCentral, UT_Central_dragEnterEvent_002)
{
    Stub s;
    s.set(ADDR(QMimeData, hasUrls), hasUrls_stub);
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data = new QMimeData();
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QDragEnterEvent *event = new QDragEnterEvent(pos, actions, data, buttons, modifiers);

    m_tester->dragEnterEvent(event);
    EXPECT_TRUE(g_funcName == "activateWindow_stub");
    delete data;
    delete event;
}

TEST_F(TestCentral, UT_Central_dragEnterEvent_003)
{
    Stub s;
    typedef bool (*fptr)(QMimeData *, const QString &);
    fptr QMimeData_hasFormat = (fptr)(&QMimeData::hasFormat);
    s.set(QMimeData_hasFormat, hasFormat_stub);
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data = new QMimeData();
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QDragEnterEvent *event = new QDragEnterEvent(pos, actions, data, buttons, modifiers);

    m_tester->dragEnterEvent(event);
    EXPECT_TRUE(g_funcName == "activateWindow_stub");
    delete data;
    delete event;
}

TEST_F(TestCentral, UT_Central_dropEvent_001)
{
    Stub s;
    typedef bool (*fptr)(QMimeData *, const QString &);
    fptr QMimeData_hasFormat = (fptr)(&QMimeData::hasFormat);
    s.set(QMimeData_hasFormat, hasFormat_stub);
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data = new QMimeData();
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QDropEvent *event = new QDropEvent(pos, actions, data, buttons, modifiers);

    m_tester->dropEvent(event);
    EXPECT_TRUE(g_funcName == "activateWindow_stub");
    delete data;
    delete event;
}

TEST_F(TestCentral, UT_Central_dropEvent_002)
{
    Stub s;
    s.set(ADDR(QMimeData, hasUrls), hasUrls_stub);
    s.set(ADDR(QWidget, activateWindow), activateWindow_stub);

    QPoint pos;
    Qt::DropActions actions;
    QMimeData *data = new QMimeData();
    QList<QUrl> l;
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    l.append(QUrl(strPath));
    data->setUrls(l);
    Qt::MouseButtons buttons;
    Qt::KeyboardModifiers modifiers;
    QDropEvent *event = new QDropEvent(pos, actions, data, buttons, modifiers);

    g_funcName.clear();
    m_tester->dropEvent(event);
    EXPECT_FALSE(g_funcName == "activateWindow_stub");
    delete data;
    delete event;
}

TEST_F(TestCentral, UT_Central_resizeEvent_001)
{
    Stub s;
    typedef void (*fptr)(BaseWidget *, QResizeEvent *);
    fptr BaseWidget_resizeEvent = (fptr)(&BaseWidget::resizeEvent);
    s.set(BaseWidget_resizeEvent, resizeEvent_stub);

    QSize size(20, 20);
    QSize oldSize(30, 30);
    QResizeEvent *event = new QResizeEvent(size, oldSize);

    m_tester->resizeEvent(event);
    EXPECT_TRUE(g_funcName == "resizeEvent_stub");
    delete event;
}
