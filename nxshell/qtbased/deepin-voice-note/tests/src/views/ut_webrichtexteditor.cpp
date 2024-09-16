// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_webrichtexteditor.h"
#include "common/jscontent.h"
#include "stub.h"
#include "dialog/vnotemessagedialog.h"
#include "common/actionmanager.h"
#include "common/vtextspeechandtrmanager.h"

#include <DFileDialog>

#include <QClipboard>
#include <QMimeData>
#include <QWebEngineContextMenuData>

static QWebChannel *webchannel;

static void stub_WebRichTextEditor()
{
}

static int stub_int()
{
    return 1;
}

static QVariant stub_callJsSynchronous()
{
    return QVariant("");
}

static QVariant stub_imageVariant()
{
    return QVariant(QImage());
}

static int stub_fileDialog()
{
    return 0;
}

static QString stub_emptyString()
{
    return "";
}

static QString stub_string()
{
    return "/tmp/1.jpg";
}

static QString stub_sysString()
{
    return "/home/1.jpg";
}

void stub_WebRichTextEditor_setWebChannel(QWebChannel *channel)
{
    webchannel = channel;
}

bool stub_isVoicePaste()
{
    return false;
}

static void stub_findText(const QString &subString, QWebEnginePage::FindFlags options, const QWebEngineCallback<bool> &resultCallback) {
    Q_UNUSED(subString)
        Q_UNUSED(options)
            Q_UNUSED(resultCallback)}

QWebEnginePage *UT_WebRichTextEditor::stub_page()
{
    return (QWebEnginePage *)this;
}

QWebEngineContextMenuData &UT_WebRichTextEditor::stub_contextMenuData()
{
    return m_data;
}

QWebEngineContextMenuData::EditFlags stub_editFlags()
{
    return QWebEngineContextMenuData::CanUndo
           | QWebEngineContextMenuData::CanRedo
           | QWebEngineContextMenuData::CanCut
           | QWebEngineContextMenuData::CanCopy
           | QWebEngineContextMenuData::CanPaste
           | QWebEngineContextMenuData::CanDelete
           | QWebEngineContextMenuData::CanSelectAll
           | QWebEngineContextMenuData::CanTranslate
           | QWebEngineContextMenuData::CanEditRichly;
}

static QWebEnginePage *stub_WebRichTextEditor_page()
{
    return nullptr;
}

static int stub_dialog()
{
    return 1;
}

QWidget *UT_WebRichTextEditor::stub_focusProxy()
{
    return (QWidget *)this;
}

UT_WebRichTextEditor::UT_WebRichTextEditor()
{
}

void UT_WebRichTextEditor::SetUp()
{
    Stub stub;
    stub.set(ADDR(WebRichTextEditor, initWebView), stub_WebRichTextEditor);
    stub.set((QWebEnginePage * (QWebEngineView::*)()) ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set((void (QWebEnginePage::*)(QWebChannel *))ADDR(QWebEnginePage, setWebChannel), stub_WebRichTextEditor_setWebChannel);
    stub.set((void (QWebEngineView::*)(const QUrl &))ADDR(QWebEngineView, load), stub_WebRichTextEditor);
    stub.set((void (QWebEnginePage::*)(const QString &))ADDR(QWebEnginePage, runJavaScript), stub_WebRichTextEditor);
    stub.set((void (QWebEnginePage::*)(const QColor &))ADDR(QWebEnginePage, setBackgroundColor), stub_WebRichTextEditor);
    m_web = new WebRichTextEditor;
}

void UT_WebRichTextEditor::TearDown()
{
    delete m_web;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_initRightMenu_001)
{
    m_web->initRightMenu();
    EXPECT_NE(nullptr, m_web->m_pictureRightMenu);
    EXPECT_NE(nullptr, m_web->m_voiceRightMenu);
    EXPECT_NE(nullptr, m_web->m_txtRightMenu);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_initUpdateTimer_001)
{
    m_web->initUpdateTimer();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_initData_001)
{
    m_web->initData(nullptr, "", false);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_insertVoiceItem_001)
{
    m_web->insertVoiceItem("", 2);
    m_web->insertVoiceItem("/temp/test.mp3", 2);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_updateNote_001)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, page), stub_WebRichTextEditor_page);
    stub.set(ADDR(JsContent, callJsSynchronous), stub_callJsSynchronous);

    VNoteItem *note = new VNoteItem();
    m_web->m_textChange = true;
    m_web->m_noteData = note;
    m_web->updateNote();
    delete note;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_searchText_001)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, findText), stub_findText);
    m_web->searchText("a");
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_unboundCurrentNoteData_001)
{
    m_web->unboundCurrentNoteData();
    EXPECT_FALSE(m_web->m_noteData);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onTextChange_001)
{
    VNoteItem *note = new VNoteItem();
    m_web->m_textChange = false;
    m_web->m_noteData = note;
    m_web->onTextChange();
    EXPECT_TRUE(m_web->m_textChange);
    delete note;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_saveMenuParam_001)
{
    m_web->saveMenuParam(1, QVariant("a"));
    EXPECT_EQ(1, m_web->m_menuType);
    EXPECT_EQ("a", m_web->m_menuJson.toString());
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onSetDataFinsh_001)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QWebEnginePage::triggerAction);
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(A_foo, stub_WebRichTextEditor);
    m_web->m_searchKey = "";
    m_web->onSetDataFinsh();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_showTxtMenu_001)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(ADDR(QWebEnginePage, contextMenuData), ADDR(UT_WebRichTextEditor, stub_contextMenuData));
    stub.set(ADDR(QWebEngineContextMenuData, editFlags), stub_editFlags);
    m_web->showTxtMenu(QPoint());
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtSelectAll)->isEnabled()) << "TxtSelectAll";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtCopy)->isEnabled()) << "TxtCopy";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtCut)->isEnabled()) << "TxtCut";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtDelete)->isEnabled()) << "TxtDelete";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtPaste)->isEnabled()) << "TxtPaste";
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_showTxtMenu_002)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(ADDR(QWebEnginePage, contextMenuData), ADDR(UT_WebRichTextEditor, stub_contextMenuData));
    stub.set(ADDR(QWebEngineContextMenuData, editFlags), stub_editFlags);
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateAISrvAvailable);
    m_web->showTxtMenu(QPoint());
    EXPECT_FALSE(ActionManager::Instance()->getActionById(ActionManager::TxtStopreading)->isVisible()) << "TxtStopreading";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtSpeech)->isVisible()) << "TxtSpeech";
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_showTxtMenu_003)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(ADDR(QWebEnginePage, contextMenuData), ADDR(UT_WebRichTextEditor, stub_contextMenuData));
    stub.set(ADDR(QWebEngineContextMenuData, editFlags), stub_editFlags);
    stub.set(ADDR(VTextSpeechAndTrManager, isTextToSpeechInWorking), stub_int);
    OpsStateInterface::instance()->m_states |= (1 << OpsStateInterface::StateAISrvAvailable);
    m_web->showTxtMenu(QPoint());
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtStopreading)->isVisible()) << "TxtStopreading";
    EXPECT_FALSE(ActionManager::Instance()->getActionById(ActionManager::TxtSpeech)->isVisible()) << "TxtSpeech";
    EXPECT_TRUE(ActionManager::Instance()->getActionById(ActionManager::TxtStopreading)->isEnabled()) << "TxtStopreading";
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_showPictureMenu_001)
{
    m_web->showPictureMenu(QPoint());
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_showVoiceMenu_001)
{
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    Stub stub;
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QWidget, focusProxy), ADDR(UT_WebRichTextEditor, stub_focusProxy));

    m_web->showVoiceMenu(QPoint());
    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    m_web->m_menuJson = metadata;
    m_web->showVoiceMenu(QPoint());
    EXPECT_EQ("20210916 17.19.22", m_web->m_voiceBlock->voiceTitle);
    EXPECT_EQ("/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3", m_web->m_voiceBlock->voicePath);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onMenuActionClicked_001)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QWebEnginePage::triggerAction);
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(A_foo, stub_WebRichTextEditor);

    stub.set(ADDR(QWidget, focusProxy), ADDR(UT_WebRichTextEditor, stub_focusProxy));
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_emptyString);
    stub.set(ADDR(WebRichTextEditor, onPaste), stub_WebRichTextEditor);
    stub.set(ADDR(WebRichTextEditor, isVoicePaste), stub_isVoicePaste);

    ActionManager actionManager;
    QAction *pAction = actionManager.getActionById(ActionManager::VoiceAsSave);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoiceToText);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoiceDelete);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureDelete);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtDelete);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoiceSelectAll);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureSelectAll);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtSelectAll);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtSelectAll);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoiceCopy);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureCopy);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtCopy);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoiceCut);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureCut);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtCut);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::VoicePaste);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PicturePaste);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtPaste);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureView);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::PictureSaveAs);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtSpeech);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtStopreading);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtDictation);
    m_web->onMenuActionClicked(pAction);

    pAction = actionManager.getActionById(ActionManager::TxtTranslate);
    m_web->onMenuActionClicked(pAction);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_savePictureAs_001)
{
    Stub stub;
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_emptyString);

    m_web->m_menuJson = "/tmp/test";
    m_web->savePictureAs();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_saveMP3As_001)
{
    Stub stub;
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_emptyString);
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QWidget, focusProxy), ADDR(UT_WebRichTextEditor, stub_focusProxy));

    m_web->saveMP3As();

    QVariant metadata = "{\"createTime\":\"2021-09-16 17:19:22.065\",\"state\":false,\"text\":\"\",\"title\":\"20210916 17.19.22\",\"transSize\":\"00:01\",\"type\":2,"
                        "\"voicePath\":\"/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3\",\"voiceSize\":1420}";
    m_web->m_menuJson = metadata;
    m_web->showVoiceMenu(QPoint());
    EXPECT_EQ("20210916 17.19.22", m_web->m_voiceBlock->voiceTitle);
    EXPECT_EQ("/home/uos/.local/share/deepin/deepin-voice-note/voicenote/0020210916171920.mp3", m_web->m_voiceBlock->voicePath);
    m_web->saveMP3As();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_saveAsFile_001)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_emptyString);

    EXPECT_TRUE(m_web->saveAsFile("/tmp/test.jpg", "", "").isEmpty());
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_saveAsFile_002)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_string);

    EXPECT_TRUE(m_web->saveAsFile("/tmp/test.jpg", "/temp").isEmpty());
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_saveAsFile_003)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QDialog::exec);
    stub.set(A_foo, stub_dialog);
    stub.set(ADDR(QFileDialog, getSaveFileName), stub_sysString);

    EXPECT_TRUE(m_web->saveAsFile("/tmp/test.jpg", "/temp", "test").isEmpty());
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_viewPicture_001)
{
    m_web->viewPicture("/tmp/test.jpg");
    EXPECT_NE(nullptr, m_web->imgView);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onPaste_001)
{
    QClipboard *clip = QApplication::clipboard();
    QMimeData *data = new QMimeData;
    data->setImageData(QImage());
    clip->setMimeData(data);

    m_web->onPaste();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onPaste_002)
{
    QClipboard *clip = QApplication::clipboard();
    QMimeData *data = new QMimeData;
    QList<QUrl> list;
    list.push_back(QUrl("/test.jpg"));
    data->setUrls(list);
    clip->setMimeData(data);
    m_web->onPaste();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onPaste_003)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QWebEnginePage::triggerAction);
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(A_foo, stub_WebRichTextEditor);
    QClipboard *clip = QApplication::clipboard();
    QMimeData *data = new QMimeData;
    clip->setMimeData(data);
    m_web->onPaste();
}
TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onPaste_004)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QWebEnginePage::triggerAction);
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(A_foo, stub_WebRichTextEditor);
    m_web->onPaste(true);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_contextMenuEvent_001)
{
    Stub stub;
    stub.set(ADDR(WebRichTextEditor, showTxtMenu), stub_WebRichTextEditor);

    QContextMenuEvent *e = new QContextMenuEvent(QContextMenuEvent::Mouse, QPoint(), QPoint());
    m_web->m_menuJson = "";
    m_web->m_menuType = static_cast<WebRichTextEditor::Menu>(0);
    m_web->contextMenuEvent(e);
    m_web->m_menuType = static_cast<WebRichTextEditor::Menu>(1);
    m_web->contextMenuEvent(e);
    m_web->m_menuType = static_cast<WebRichTextEditor::Menu>(2);
    m_web->contextMenuEvent(e);

    delete e;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_dragEnterEvent_001)
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    m_web->dragEnterEvent(e);
    delete e;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_dragLeaveEvent_001)
{
    m_web->dragLeaveEvent(nullptr);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_dragMoveEvent_001)
{
    m_web->dragMoveEvent(nullptr);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_dropEvent_001)
{
    Stub stub;
    stub.set(ADDR(QMimeData, hasUrls), stub_int);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> list;
    list.push_back(QUrl("/test.jpg"));
    mimeData->setUrls(list);
    QDragEnterEvent *e = new QDragEnterEvent(QPoint(), Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    m_web->dropEvent(e);
    delete e;
    delete mimeData;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_deleteSelectText_001)
{
    Stub stub;
    stub.set(ADDR(QWidget, focusProxy), ADDR(UT_WebRichTextEditor, stub_focusProxy));
    m_web->deleteSelectText();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onThemeChanged_001)
{
    Stub stub;
    stub.set(ADDR(QWebEngineView, page), ADDR(UT_WebRichTextEditor, stub_page));
    stub.set(ADDR(QWebEnginePage, setBackgroundColor), stub_WebRichTextEditor);
    m_web->onThemeChanged();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onShowEditToolbar_001)
{
    m_web->onShowEditToolbar(QPoint(600, 200));
    m_web->onShowEditToolbar(QPoint(1920, 1080));
    QPoint menuPoint(0, 0);
    m_web->onShowEditToolbar(menuPoint);
    menuPoint.setX(menuPoint.x() - 9);
    int width = m_web->width() - menuPoint.x() - 320;
    if (width < 0) {
        menuPoint.setX(menuPoint.x() + width);
    }
    menuPoint.setY(menuPoint.y() + 15 + m_web->m_txtRightMenu->height());
    EXPECT_EQ(QRect(menuPoint, QPoint(menuPoint.x() + 290 + 85, menuPoint.y() + 35)), m_web->m_editToolbarRect);
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_onHideEditToolbar_001)
{
    m_web->onHideEditToolbar();
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_setData_001)
{
    Stub stub;
    typedef int (*fptr)();
    fptr A_foo = (fptr)(&QWidget::setVisible);
    stub.set(A_foo, stub_WebRichTextEditor);
    stub.set(ADDR(QWebEngineView, findText), stub_findText);

    VNoteItem *data = new VNoteItem();

    m_web->clearFocus();
    m_web->setData(nullptr, "");

    m_web->setFocus();
    m_web->setData(data, "a");
    EXPECT_EQ("a", m_web->m_searchKey) << "a";
    EXPECT_EQ(data, m_web->m_noteData) << "a";

    data->htmlCode = "<div> </div>";
    m_web->setData(data, "");
    EXPECT_EQ("", m_web->m_searchKey) << "b";
    EXPECT_EQ(data, m_web->m_noteData) << "b";

    m_web->m_noteData = data;
    m_web->setData(data, "a");
    EXPECT_EQ("a", m_web->m_searchKey) << "c";
    EXPECT_EQ(data, m_web->m_noteData) << "c";

    delete data;
}

TEST_F(UT_WebRichTextEditor, UT_WebRichTextEditor_eventFilter_001)
{
    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    m_web->eventFilter(m_web->focusProxy(), event);
    delete event;
}
