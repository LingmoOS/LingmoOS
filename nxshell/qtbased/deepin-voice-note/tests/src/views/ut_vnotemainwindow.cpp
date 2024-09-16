// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vnotemainwindow.h"
#include "vnotemainwindow.h"
#include "leftview.h"
#include "middleview.h"
#include "homepage.h"
#include "splashview.h"
#include "middleviewsortfilter.h"
#include "standarditemcommon.h"
#include "vnotedatamanager.h"
#include "vnotea2tmanager.h"
#include "vnoteitem.h"
#include "vnoteforlder.h"
#include "actionmanager.h"
#include "utils.h"
#include "actionmanager.h"
#include "setting.h"
#include "vnotefolderoper.h"
#include "vnoteitemoper.h"
#include "vnotedbmanager.h"
#include "dbuslogin1manager.h"
#include "vnotemessagedialog.h"
#include "vnoterecordbar.h"
#include "vnoteiconbutton.h"
#include "vnmainwnddelayinittask.h"
#include "webrichtexteditor.h"

#include "upgradeview.h"
#include "upgradedbutil.h"

#include "vnoteapplication.h"
#include "stub.h"

#include <DFileDialog>

static QWebChannel *webchannel;

static void stub_vnotemainwindow()
{
}

static int stub_int()
{
    return 1;
}

static int stub_fileDialog()
{
    return 0;
}

void stub_setWeb(QWebChannel *channel)
{
    webchannel = channel;
}

QWebEnginePage *UT_VNoteMainWindow::stub_page()
{
    return (QWebEnginePage *)this;
}

QObject *stub_nullptr()
{
    return nullptr;
}

UT_VNoteMainWindow::UT_VNoteMainWindow()
{
}

void UT_VNoteMainWindow::SetUp()
{
    Stub stub;
    stub.set(ADDR(VNoteMainWindow, initData), stub_vnotemainwindow);
    stub.set(ADDR(VNoteMainWindow, delayInitTasks), stub_vnotemainwindow);
    stub.set((QWebEnginePage * (QWebEngineView::*)()) ADDR(QWebEngineView, page), ADDR(UT_VNoteMainWindow, stub_page));
    stub.set((void (QWebEnginePage::*)(QWebChannel *))ADDR(QWebEnginePage, setWebChannel), stub_setWeb);
    stub.set((void (QWebEngineView::*)(const QUrl &))ADDR(QWebEngineView, load), stub_vnotemainwindow);
    stub.set((void (QWebEnginePage::*)(const QString &))ADDR(QWebEnginePage, runJavaScript), stub_vnotemainwindow);
    stub.set((void (QWebEnginePage::*)(const QColor &))ADDR(QWebEnginePage, setBackgroundColor), stub_vnotemainwindow);

    m_mainWindow = new VNoteMainWindow;
    m_mainWindow->switchWidget(VNoteMainWindow::WndNoteShow);
}

void UT_VNoteMainWindow::TearDown()
{
    delete m_mainWindow;
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initAppSetting_001)
{
    m_mainWindow->initAppSetting();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_changeRightView_001)
{
    m_mainWindow->changeRightView(true);
    EXPECT_FALSE(m_mainWindow->m_imgInsert->isEnabled());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_changeRightView_002)
{
    m_mainWindow->changeRightView(false);
    EXPECT_EQ(m_mainWindow->m_rightViewHolder, m_mainWindow->m_stackedRightMainWidget->currentWidget());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initTitleBar_002)
{
    m_mainWindow->initTitleBar();
    EXPECT_NE(nullptr, m_mainWindow->m_noteSearchEdit);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initA2TManager_002)
{
    m_mainWindow->initA2TManager();
    EXPECT_NE(nullptr, m_mainWindow->m_a2tManager);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_holdHaltLock_001)
{
    m_mainWindow->holdHaltLock();
    m_mainWindow->releaseHaltLock();
    EXPECT_FALSE(m_mainWindow->m_lockFd.isValid());
    m_mainWindow->onVNoteFoldersLoaded();
    EXPECT_FALSE(m_mainWindow->m_stackedWidget->currentIndex() != VNoteMainWindow::WndNoteShow);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteSearch_001)
{
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), stub_int);
    stub.set(ADDR(QWebEngineView, findText), stub_vnotemainwindow);

    m_mainWindow->onVNoteSearch();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteSearch_002)
{
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), stub_int);
    stub.set(ADDR(QWebEngineView, findText), stub_vnotemainwindow);
    m_mainWindow->m_noteSearchEdit->setText("a");
    m_mainWindow->onVNoteSearch();
    EXPECT_EQ("a", m_mainWindow->m_searchKey);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteSearch_003)
{
    Stub stub;
    stub.set(ADDR(QWidget, hasFocus), stub_int);
    stub.set(ADDR(QWebEngineView, findText), stub_vnotemainwindow);
    m_mainWindow->m_noteSearchEdit->setText("a");
    m_mainWindow->m_searchKey = "a";
    m_mainWindow->onVNoteSearch();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteSearchTextChange_001)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, findText), stub_vnotemainwindow);
    m_mainWindow->onVNoteSearchTextChange("");
    EXPECT_EQ(OpsStateInterface::instance()->isSearching(), false);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteFolderChange_001)
{
    m_mainWindow->onVNoteFolderChange(m_mainWindow->m_leftView->restoreNotepadItem(), QModelIndex());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_loadNotepads_001)
{
    EXPECT_TRUE(m_mainWindow->loadNotepads());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_addNotepad_001)
{
    m_mainWindow->addNotepad();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_delNotepad_001)
{
    m_mainWindow->addNotepad();
    m_mainWindow->delNotepad();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_editNotepad_001)
{
    m_mainWindow->editNotepad();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_addNote_001)
{
    m_mainWindow->addNote();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_editNote_001)
{
    m_mainWindow->editNote();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_delNote_001)
{
    m_mainWindow->addNote();
    m_mainWindow->m_middleView->selectAll();
    m_mainWindow->delNote();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_loadNotes_001)
{
    VNoteFolder *vnotefolder = new VNoteFolder;
    vnotefolder->id = 0;
    vnotefolder->category = 1;
    vnotefolder->notesCount = 2;
    vnotefolder->defaultIcon = 3;
    vnotefolder->folder_state = vnotefolder->Normal;
    vnotefolder->name = "test";
    vnotefolder->iconPath = "/home/zhangteng/works/deepin-voice-note/assets/icons/deepin/builtin/default_folder_icons";
    vnotefolder->sortNumber = 4;
    vnotefolder->createTime = QDateTime::currentDateTime();
    vnotefolder->modifyTime = QDateTime::currentDateTime();
    vnotefolder->deleteTime = QDateTime::currentDateTime();
    m_mainWindow->loadNotes(vnotefolder);
    EXPECT_EQ(m_mainWindow->m_middleView->getCurrentId(), vnotefolder->id);
    delete vnotefolder;
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_loadSearchNotes_001)
{
    EXPECT_FALSE(m_mainWindow->loadSearchNotes("本"));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initDeviceExceptionErrMessage_001)
{
    m_mainWindow->initDeviceExceptionErrMessage();
    EXPECT_TRUE(m_mainWindow->m_pDeviceExceptionMsg);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_showAsrErrMessage_001)
{
    m_mainWindow->showAsrErrMessage("error Message");
    EXPECT_TRUE(m_mainWindow->m_asrErrMeassage);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_showDeviceExceptionErrMessage_001)
{
    m_mainWindow->showDeviceExceptionErrMessage();
    EXPECT_TRUE(m_mainWindow->m_pDeviceExceptionMsg);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_closeDeviceExceptionErrMessage_001)
{
    m_mainWindow->closeDeviceExceptionErrMessage();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onSystemDown_001)
{
    m_mainWindow->onSystemDown(true);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_switchWidget_001)
{
    m_mainWindow->switchWidget(VNoteMainWindow::WndNoteShow);
    EXPECT_TRUE(m_mainWindow->m_viewChange->isEnabled());
    EXPECT_TRUE(m_mainWindow->m_noteSearchEdit->isEnabled());
    EXPECT_EQ(VNoteMainWindow::WndNoteShow, m_mainWindow->m_stackedWidget->currentIndex());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_switchWidget_002)
{
    m_mainWindow->switchWidget(VNoteMainWindow::WndHomePage);
    EXPECT_FALSE(m_mainWindow->m_viewChange->isEnabled());
    EXPECT_FALSE(m_mainWindow->m_noteSearchEdit->isEnabled());
    EXPECT_EQ(VNoteMainWindow::WndHomePage, m_mainWindow->m_stackedWidget->currentIndex());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_release_001)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->m_a2tManager = nullptr;
    m_mainWindow->release();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onShowPrivacy_001)
{
    m_mainWindow->onShowPrivacy();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onShowSettingDialog_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_int);
    m_mainWindow->onShowPrivacy();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initMenuExtension_001)
{
    m_mainWindow->initMenuExtension();
    EXPECT_TRUE(m_mainWindow->m_menuExtension);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initTitleIconButton_001)
{
    m_mainWindow->initTitleIconButton();
    EXPECT_TRUE(m_mainWindow->m_viewChange);
    EXPECT_TRUE(m_mainWindow->m_imgInsert);
    EXPECT_FALSE(m_mainWindow->m_imgInsert->isEnabled());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setTabFocus_001)
{
    EXPECT_TRUE(m_mainWindow->setTabFocus(m_mainWindow->m_richTextEdit, nullptr)) << "m_richTextEdit";
    EXPECT_FALSE(m_mainWindow->setTabFocus(m_mainWindow->m_noteSearchEdit->lineEdit(), nullptr)) << "m_noteSearchEdit";
    EXPECT_TRUE(m_mainWindow->setTabFocus(m_mainWindow->m_addNoteBtn, nullptr)) << "m_addNoteBtn";
    EXPECT_FALSE(m_mainWindow->setTabFocus(m_mainWindow->m_middleView, nullptr)) << "m_middleView";
    EXPECT_FALSE(m_mainWindow->setTabFocus(m_mainWindow->m_addNotepadBtn, nullptr)) << "m_addNotepadBtn";
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateSearching);
    EXPECT_TRUE(m_mainWindow->setTabFocus(m_mainWindow->m_leftView, nullptr)) << "default";
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setAddnotepadButtonNext_001)
{
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    m_mainWindow->m_middleView->selectAll();
    EXPECT_TRUE(m_mainWindow->setAddnotepadButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setAddnotepadButtonNext_002)
{
    Stub stub;
    stub.set(ADDR(MiddleView, rowCount), stub_int);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_FALSE(m_mainWindow->setAddnotepadButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setAddnoteButtonNext_001)
{
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setAddnoteButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setAddnoteButtonNext_002)
{
    Stub stub;
    stub.set(ADDR(MiddleView, rowCount), stub_int);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setAddnoteButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setAddnoteButtonNext_003)
{
    Stub stub;
    stub.set(ADDR(MiddleView, rowCount), stub_int);
    m_mainWindow->m_stackedRightMainWidget->setCurrentWidget(m_mainWindow->m_recordBarHolder);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setAddnoteButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setTitleCloseButtonNext_001)
{
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateSearching);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setTitleCloseButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setTitleCloseButtonNext_002)
{
    Stub stub;
    stub.set(ADDR(MiddleView, searchEmpty), stub_int);
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateSearching);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setTitleCloseButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setTitleCloseButtonNext_003)
{
    OpsStateInterface::instance()->m_states = 0;
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setTitleCloseButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setTitleCloseButtonNext_004)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->m_leftViewHolder->setVisible(false);
    m_mainWindow->m_middleViewHolder->setVisible(true);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setTitleCloseButtonNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setMiddleviewNext_001)
{
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateSearching);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setMiddleviewNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setMiddleviewNext_002)
{
    OpsStateInterface::instance()->m_states = (1 << OpsStateInterface::StateRecording);
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_TRUE(m_mainWindow->setMiddleviewNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setMiddleviewNext_003)
{
    OpsStateInterface::instance()->m_states = 0;
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    EXPECT_FALSE(m_mainWindow->setMiddleviewNext(&e));
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_needShowMax_001)
{
    m_mainWindow->m_leftView->setFocus();
    EXPECT_FALSE(m_mainWindow->needShowMax());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onDeleteShortcut_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_int);
    stub.set(ADDR(QWidget, hasFocus), stub_int);

    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->onDeleteShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onEscShortcut_001)
{
    m_mainWindow->onEscShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onPoppuMenuShortcut_001)
{
    m_mainWindow->onPoppuMenuShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onAddNotepadShortcut_001)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->onAddNotepadShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onReNameNotepadShortcut_001)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->onReNameNotepadShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onAddNoteShortcut_001)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->onAddNoteShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onRenameNoteShortcut_001)
{
    m_mainWindow->onAddNoteShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onPlayPauseShortcut_001)
{
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StatePlaying);
    m_mainWindow->onPlayPauseShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onRecordShorcut_001)
{
    m_mainWindow->onRecordShorcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onSaveNoteShortcut_001)
{
    m_mainWindow->onSaveNoteShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onSaveVoicesShortcut_001)
{
    OpsStateInterface::instance()->m_states = 0;
    m_mainWindow->onSaveNoteShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onThemeChanged_001)
{
    m_mainWindow->onThemeChanged();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onViewChangeClicked_001)
{
    m_mainWindow->onViewChangeClicked();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onWebVoicePlay_001)
{
    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateRecording);
    m_mainWindow->onWebVoicePlay(metadata, false);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onWebVoicePlay_002)
{
    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    m_mainWindow->m_currentPlayVoice.reset(nullptr);
    m_mainWindow->onWebVoicePlay(metadata, true);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onWebVoicePlay_003)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_int);

    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    m_mainWindow->onWebVoicePlay(metadata, false);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onWebSearchEmpty_001)
{
    m_mainWindow->onWebSearchEmpty();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_showNotepadList_001)
{
    setting::instance()->setOption(VNOTE_NOTEPAD_LIST_SHOW,
                                   !setting::instance()->getOption(VNOTE_NOTEPAD_LIST_SHOW).toBool());
    m_mainWindow->showNotepadList();

    setting::instance()->setOption(VNOTE_NOTEPAD_LIST_SHOW,
                                   !setting::instance()->getOption(VNOTE_NOTEPAD_LIST_SHOW).toBool());
    m_mainWindow->showNotepadList();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setSpecialStatus_001)
{
    m_mainWindow->setSpecialStatus(m_mainWindow->SearchStart);
    EXPECT_EQ(OpsStateInterface::instance()->isSearching(), true);
    m_mainWindow->setSpecialStatus(m_mainWindow->SearchEnd);
    EXPECT_EQ(OpsStateInterface::instance()->isSearching(), false);
    m_mainWindow->setSpecialStatus(m_mainWindow->PlayVoiceStart);
    EXPECT_EQ(OpsStateInterface::instance()->isPlaying(), true);
    m_mainWindow->setSpecialStatus(m_mainWindow->PlayVoiceEnd);
    EXPECT_EQ(OpsStateInterface::instance()->isPlaying(), false);
    m_mainWindow->setSpecialStatus(m_mainWindow->RecordStart);
    EXPECT_EQ(OpsStateInterface::instance()->isRecording(), true);
    m_mainWindow->setSpecialStatus(m_mainWindow->RecordEnd);
    EXPECT_EQ(OpsStateInterface::instance()->isRecording(), false);
    m_mainWindow->setSpecialStatus(m_mainWindow->VoiceToTextStart);
    EXPECT_EQ(OpsStateInterface::instance()->isVoice2Text(), true);
    m_mainWindow->setSpecialStatus(m_mainWindow->VoiceToTextEnd);
    EXPECT_EQ(OpsStateInterface::instance()->isVoice2Text(), false);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_initAsrErrMessage_001)
{
    m_mainWindow->initAsrErrMessage();
    EXPECT_TRUE(m_mainWindow->m_asrErrMeassage);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onStartRecord_001)
{
    m_mainWindow->onStartRecord("/usr/share/music/bensound-sunny.mp3");
    EXPECT_EQ(OpsStateInterface::instance()->isRecording(), true);
    m_mainWindow->onFinshRecord("/usr/share/music/bensound-sunny.mp3", 2650);
    EXPECT_EQ(OpsStateInterface::instance()->isRecording(), false);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onA2TStart_001)
{
    m_mainWindow->onA2TStart(nullptr);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onA2TStart_002)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_int);
    VNVoiceBlock *voiceBlock = new VNVoiceBlock;
    voiceBlock->voiceSize = MAX_A2T_AUDIO_LEN_MS + 1;
    m_mainWindow->onA2TStart(voiceBlock);

    EXPECT_EQ(voiceBlock, m_mainWindow->m_voiceBlock);
    delete voiceBlock;
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onA2TError_001)
{
    m_mainWindow->onA2TError(8);
    EXPECT_TRUE(m_mainWindow->m_asrErrMeassage) << "err is 8";

    m_mainWindow->onA2TError(2);
    EXPECT_TRUE(m_mainWindow->m_asrErrMeassage) << "err is 2";
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onA2TSuccess_001)
{
    m_mainWindow->onA2TSuccess("/usr/share/music/bensound-sunny.mp3");
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onPreviewShortcut_001)
{
    m_mainWindow->onPreviewShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_closeEvent_001)
{
    Stub stub;
    stub.set(ADDR(VNoteMainWindow, checkIfNeedExit), stub_fileDialog);
    OpsStateInterface::instance()->m_states = (1 << OpsStateInterface::StateRecording);
    QCloseEvent *event = new QCloseEvent();
    m_mainWindow->closeEvent(event);
    delete event;
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_resizeEvent_001)
{
    QSize size;
    QResizeEvent *event = new QResizeEvent(size, size);
    m_mainWindow->resizeEvent(event);
    delete event;
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_checkIfNeedExit_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_fileDialog);
    EXPECT_FALSE(m_mainWindow->checkIfNeedExit());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onVNoteChange_001)
{
    m_mainWindow->onVNoteChange(QModelIndex());
    EXPECT_FALSE(m_mainWindow->m_recordBar->isVisible());
    EXPECT_FALSE(m_mainWindow->m_imgInsert->isEnabled());
    EXPECT_FALSE(m_mainWindow->m_rightViewHasFouse);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onMenuAction_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_int);

    ActionManager actionManager;
    QAction *pAction = actionManager.getActionById(ActionManager::NotebookRename);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NotebookDelete);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NotebookAddNew);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteDelete);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteAddNew);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteRename);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::SaveNoteAsText);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::SaveNoteAsHtml);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteSaveVoice);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteTop);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::NoteMove);
    m_mainWindow->onMenuAction(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureCut);
    m_mainWindow->onMenuAction(pAction);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onMenuAbout2Show_001)
{
    Stub stub;
    stub.set(ADDR(QObject, sender), stub_nullptr);
    stub.set(ADDR(ActionManager, noteContextMenu), stub_nullptr);
    m_mainWindow->onMenuAbout2Show();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onMenuAbout2Show_002)
{
    Stub stub;
    stub.set(ADDR(QObject, sender), stub_nullptr);
    stub.set(ADDR(ActionManager, noteContextMenu), stub_nullptr);
    m_mainWindow->m_middleView->m_currentId = 0;
    m_mainWindow->addNotepad();
    m_mainWindow->addNote();
    m_mainWindow->m_middleView->selectAll();
    m_mainWindow->onMenuAbout2Show();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onMenuAbout2Show_003)
{
    Stub stub;
    stub.set(ADDR(QObject, sender), stub_nullptr);
    stub.set(ADDR(ActionManager, notebookContextMenu), stub_nullptr);
    m_mainWindow->onMenuAbout2Show();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_handleMultipleOption_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&DFileDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_fileDialog);
    int id = 1;
    m_mainWindow->handleMultipleOption(id);
    int id2 = 2;
    m_mainWindow->handleMultipleOption(id2);
    int id3 = 3;
    m_mainWindow->handleMultipleOption(id3);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onDropNote_001)
{
    bool dropCancel = true;
    m_mainWindow->onDropNote(dropCancel);
    EXPECT_EQ(m_mainWindow->m_middleView->m_dragSuccess, false);
    bool dropCancels = false;
    m_mainWindow->onDropNote(dropCancels);
    EXPECT_EQ(m_mainWindow->m_middleView->m_dragSuccess, true);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onShortcut_001)
{
    m_mainWindow->onEscShortcut();
    //m_mainWindow->onDeleteShortcut();

    QScopedPointer<Stub> stub;
    stub.reset(new Stub);
    stub->set(ADDR(LeftView, hasFocus), stub_int);
    //m_mainWindow->onDeleteShortcut();
    m_mainWindow->onPoppuMenuShortcut();
    stub.reset(new Stub);
    stub->set(ADDR(MiddleView, hasFocus), stub_int);
    //m_mainWindow->onDeleteShortcut();
    m_mainWindow->onPoppuMenuShortcut();
    stub.reset(new Stub);
    stub->set(ADDR(QLineEdit, hasFocus), stub_int);
    //m_mainWindow->onDeleteShortcut();
    m_mainWindow->onPoppuMenuShortcut();

    m_mainWindow->onAddNotepadShortcut();
    m_mainWindow->onReNameNotepadShortcut();
    m_mainWindow->onAddNoteShortcut();
    m_mainWindow->onRenameNoteShortcut();
    m_mainWindow->onPlayPauseShortcut();
    m_mainWindow->onSaveNoteShortcut();
    m_mainWindow->onSaveVoicesShortcut();
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_setviewNext_001)
{
    QKeyEvent e(QEvent::KeyPress, 0x58, Qt::ControlModifier, "test");
    m_mainWindow->setTitleBarTabFocus(&e);
    m_mainWindow->setMiddleviewNext(&e);
    m_mainWindow->setTitleCloseButtonNext(&e);
    m_mainWindow->setAddnoteButtonNext(&e);
    m_mainWindow->setAddnotepadButtonNext(&e);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onNewNote_001)
{
    m_mainWindow->onNewNote();
    m_mainWindow->delNote();
    EXPECT_EQ(m_mainWindow->m_middleView->rowCount(), 0);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_canDoShortcutAction_001)
{
    EXPECT_TRUE(m_mainWindow->canDoShortcutAction());
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onNewNotebook_001)
{
    m_mainWindow->onNewNotebook();
    m_mainWindow->delNotepad();
    EXPECT_EQ(m_mainWindow->m_leftView->folderCount(), 0);
}

TEST_F(UT_VNoteMainWindow, UT_VNoteMainWindow_onPlayPlugVoice_001)
{
    m_mainWindow->onPlayPlugVoicePlay(nullptr);
    m_mainWindow->onPlayPlugVoicePause(nullptr);
    m_mainWindow->onPlayPlugVoiceStop(nullptr);
}
