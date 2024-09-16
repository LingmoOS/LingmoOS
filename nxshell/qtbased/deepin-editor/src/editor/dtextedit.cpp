// SPDX-FileCopyrightText: 2011-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later


#include "../common/utils.h"
#include "../widgets/window.h"
#include "../widgets/bottombar.h"
#include "dtextedit.h"
#include "leftareaoftextedit.h"
#include "editwrapper.h"
#include "showflodcodewidget.h"
#include "deletebackcommond.h"
#include "replaceallcommond.h"
#include "insertblockbytextcommond.h"
#include "indenttextcommond.h"
#include "undolist.h"
#include "changemarkcommand.h"
#include "endlineformatcommond.h"

#include <KSyntaxHighlighting/definition.h>
#include <KSyntaxHighlighting/syntaxhighlighter.h>
#include <KSyntaxHighlighting/theme.h>

#include <QAbstractTextDocumentLayout>
#include <QTextDocumentFragment>
#include <QInputMethodEvent>
#include <DDesktopServices>
#include <QApplication>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettings>
#include <QClipboard>
#include <QFileInfo>
#include <QDebug>
#include <QPainter>
#include <QScroller>
#include <QScrollBar>
#include <QStyleFactory>
#include <QTextBlock>
#include <QMimeData>
#include <QTimer>
#include <QGesture>
#include <QStyleHints>
#include <DSysInfo>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>
#include <QtSvg/qsvgrenderer.h>

TextEdit::TextEdit(QWidget *parent)
    : DPlainTextEdit(parent),
      m_wrapper(nullptr)
{
    // 更新单独添加的高亮格式文件
    m_repository.addCustomSearchPath(KF5_HIGHLIGHT_PATH);

    setUndoRedoEnabled(false);
    //撤销重做栈
    m_pUndoStack = new QUndoStack();

    m_nLines = 0;
    m_nBookMarkHoverLine = -1;
    m_bIsFileOpen = false;
    m_qstrCommitString = "";
    m_bIsShortCut = false;
    m_bIsInputMethod = false;
    m_isSelectAll = false;
    m_touchTapDistance = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::TouchDoubleTapDistance).toInt();
    m_fontLineNumberArea.setFamily("SourceHanSansSC-Normal");

    m_pLeftAreaWidget = new LeftAreaTextEdit(this);
    m_pLeftAreaWidget->m_pFlodArea->setAttribute(Qt::WA_Hover, true); //开启悬停事件
    m_pLeftAreaWidget->m_pBookMarkArea->setAttribute(Qt::WA_Hover, true);
    m_pLeftAreaWidget->m_pFlodArea->installEventFilter(this);
    m_pLeftAreaWidget->m_pBookMarkArea->installEventFilter(this);
    m_foldCodeShow = new ShowFlodCodeWidget(this);
    m_foldCodeShow->setVisible(false);

    viewport()->installEventFilter(this);
    viewport()->setCursor(Qt::IBeamCursor);

    // Don't draw frame around editor widget.
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setContentsMargins(0, 0, 0, 0);
    setEditPalette(palette().foreground().color().name(), palette().foreground().color().name());

    grabGesture(Qt::TapGesture);
    grabGesture(Qt::TapAndHoldGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    // Init widgets.
    //左边栏控件　滑动条滚动跟新行号 折叠标记
    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &TextEdit::slotValueChanged);
    connect(this, &QPlainTextEdit::textChanged, this, &TextEdit::updateLeftAreaWidget);
    connect(this, &QPlainTextEdit::textChanged, this, [this]() {
        this->m_wrapper->UpdateBottomBarWordCnt(this->characterCount());
    });

    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &TextEdit::cursorPositionChanged);
    connect(this, &QPlainTextEdit::selectionChanged, this, &TextEdit::onSelectionArea);

    connect(document(), &QTextDocument::contentsChange, this, &TextEdit::updateMark);
    connect(document(), &QTextDocument::contentsChange, this, &TextEdit::checkBookmarkLineMove);
    connect(document(), &QTextDocument::contentsChange, this, &TextEdit::onTextContentChanged);

    connect(m_pUndoStack, &QUndoStack::canRedoChanged, this, &TextEdit::slotCanRedoChanged);
    connect(m_pUndoStack, &QUndoStack::canUndoChanged, this, &TextEdit::slotCanUndoChanged);

    QDBusConnection dbus = QDBusConnection::sessionBus();
    switch (Utils::getSystemVersion()) {
        case Utils::V23:
            dbus.systemBus().connect("org.deepin.dde.Gesture1",
                                    "/org/deepin/dde/Gesture1", "org.deepin.dde.Gesture1",
                                    "Event",
                                    this, SLOT(fingerZoom(QString, QString, int)));
            break;
        default:
            dbus.systemBus().connect("com.deepin.daemon.Gesture",
                                    "/com/deepin/daemon/Gesture", "com.deepin.daemon.Gesture",
                                    "Event",
                                    this, SLOT(fingerZoom(QString, QString, int)));
            break;
    }

    //初始化右键菜单
    initRightClickedMenu();

    // Init scroll animation.
    m_scrollAnimation = new QPropertyAnimation(verticalScrollBar(), "value");
    m_scrollAnimation->setEasingCurve(QEasingCurve::InOutExpo);
    m_scrollAnimation->setDuration(300);

    m_cursorMode = Insert;

    connect(m_scrollAnimation, &QPropertyAnimation::finished, this, &TextEdit::handleScrollFinish, Qt::QueuedConnection);

    // Monitor cursor mark status to update in line number area.
    connect(this, &TextEdit::cursorMarkChanged, this, &TextEdit::handleCursorMarkChanged);

    // configure content area
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(verticalScrollBar(), &QScrollBar::rangeChanged, this, &TextEdit::adjustScrollbarMargins, Qt::QueuedConnection);
    // Don't blink the cursor when selecting text
    // Recover blink when not selecting text.
    connect(this, &TextEdit::selectionChanged, this, &TextEdit::slotSelectionChanged, Qt::QueuedConnection);
}

TextEdit::~TextEdit()
{
    if (m_scrollAnimation != nullptr) {
        if (m_scrollAnimation->state() != QAbstractAnimation::Stopped) {
            m_scrollAnimation->stop();
        }
        delete m_scrollAnimation;
        m_scrollAnimation = nullptr;
    }
    if (m_colorMarkMenu != nullptr) {
        delete m_colorMarkMenu;
        m_colorMarkMenu = nullptr;
    }
    if (m_convertCaseMenu != nullptr) {
        delete m_convertCaseMenu;
        m_convertCaseMenu = nullptr;
    }
    if (m_rightMenu != nullptr) {
        delete m_rightMenu;
        m_rightMenu = nullptr;
    }

    if (m_pUndoStack != nullptr) {
        m_pUndoStack->deleteLater();
    }
}

void TextEdit::insertTextEx(QTextCursor cursor, QString text)
{
    QUndoCommand *pInsertStack = new InsertTextUndoCommand(cursor, text, this);
    m_pUndoStack->push(pInsertStack);
    ensureCursorVisible();
}

/**
 * @brief 将多组插入信息 \a multiText 压入单个撤销栈，便于撤销栈管理。
 * @param multiText 多组插入信息，每组含插入光标位置和插入文本。
 */
void TextEdit::insertMultiTextEx(const QList<QPair<QTextCursor, QString> > &multiText)
{
    if (multiText.isEmpty()) {
        return;
    }

    QUndoCommand *pMultiCommand = new QUndoCommand;
    // 将所有的插入信息添加到单个撤销项中，便于单次处理
    for (auto pairText : multiText) {
        // pMultiCommand 析构时会自动析构子撤销项
        (void)new InsertTextUndoCommand(pairText.first, pairText.second, this, pMultiCommand);
    }
    m_pUndoStack->push(pMultiCommand);
    ensureCursorVisible();
}

void TextEdit::deleteSelectTextEx(QTextCursor cursor)
{
    if (cursor.hasSelection()) {
        QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(cursor, this);
        m_pUndoStack->push(pDeleteStack);
    }
}

void TextEdit::deleteSelectTextEx(QTextCursor cursor, QString text, bool currLine)
{
    QUndoCommand *pDeleteStack = new DeleteTextUndoCommand2(cursor, text, this, currLine);
    m_pUndoStack->push(pDeleteStack);
}

void TextEdit::deleteTextEx(QTextCursor cursor)
{
    QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(cursor, this);
    m_pUndoStack->push(pDeleteStack);
}

/**
 * @brief 将多组删除信息 \a multiText 压入单个撤销栈，便于撤销栈管理。
 * @param multiText 多组删除信息，每组含删除光标信息(删除位置和选取区域)。
 */
void TextEdit::deleteMultiTextEx(const QList<QTextCursor> &multiText)
{
    if (multiText.isEmpty()) {
        return;
    }

    QUndoCommand *pMultiCommand = new QUndoCommand;
    // 将所有的插入信息添加到单个撤销项中，便于单次处理
    for (auto textCursor : multiText) {
        // pMultiCommand 析构时会自动析构子撤销项
        (void)new DeleteTextUndoCommand(textCursor, this, pMultiCommand);
    }
    m_pUndoStack->push(pMultiCommand);
}

void TextEdit::insertSelectTextEx(QTextCursor cursor, QString text)
{
    QUndoCommand *pInsertStack = new InsertTextUndoCommand(cursor, text, this);
    m_pUndoStack->push(pInsertStack);
    ensureCursorVisible();
}

void TextEdit::insertColumnEditTextEx(QString text)
{
    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    for (int i = 0; i < m_altModSelections.size(); i++) {
        if (m_altModSelections[i].cursor.hasSelection()) deleteTextEx(m_altModSelections[i].cursor);
    }
    QUndoCommand *pInsertStack = new InsertTextUndoCommand(m_altModSelections, text, this);
    m_pUndoStack->push(pInsertStack);
    ensureCursorVisible();
}

void TextEdit::initRightClickedMenu()
{
    // Init menu.
    m_rightMenu = new DMenu();
    m_undoAction = new QAction(tr("Undo"), this);
    m_redoAction = new QAction(tr("Redo"), this);
    m_cutAction = new QAction(tr("Cut"), this);
    m_copyAction = new QAction(tr("Copy"), this);
    m_pasteAction = new QAction(tr("Paste"), this);
    m_deleteAction = new QAction(tr("Delete"), this);
    m_selectAllAction = new QAction(tr("Select All"), this);
    m_findAction = new QAction(tr("Find"), this);
    m_replaceAction = new QAction(tr("Replace"), this);
    m_jumpLineAction = new QAction(tr("Go to Line"), this);
    m_enableReadOnlyModeAction = new QAction(tr("Turn on Read-Only mode"), this);
    m_disableReadOnlyModeAction = new QAction(tr("Turn off Read-Only mode"), this);
    m_fullscreenAction = new QAction(tr("Fullscreen"), this);
    m_exitFullscreenAction = new QAction(tr("Exit fullscreen"), this);
    m_openInFileManagerAction = new QAction(tr("Display in file manager"), this);
    m_toggleCommentAction = new QAction(tr("Add Comment"), this);
    m_voiceReadingAction = new QAction(tr("Text to Speech"), this);
    m_stopReadingAction = new QAction(tr("Stop reading"), this);
    m_dictationAction = new QAction(tr("Speech to Text"), this);
    m_translateAction = new QAction(tr("Translate"), this);
    m_columnEditAction = new QAction(tr("Column Mode"), this);
    m_addBookMarkAction = new QAction(tr("Add bookmark"), this);
    m_cancelBookMarkAction = new QAction(tr("Remove Bookmark"), this);
    m_preBookMarkAction = new QAction(tr("Previous bookmark"), this);
    m_nextBookMarkAction = new QAction(tr("Next bookmark"), this);
    m_clearBookMarkAction = new QAction(tr("Remove All Bookmarks"), this);
    m_flodAllLevel = new QAction(tr("Fold All"), this);
    m_flodCurrentLevel = new QAction(tr("Fold Current Level"), this);
    m_unflodAllLevel = new QAction(tr("Unfold All"), this);
    m_unflodCurrentLevel = new QAction(tr("Unfold Current Level"), this);

    //yanyuhan
    //颜色标记、折叠/展开、书签、列编辑、设置注释、取消注释;
    //点击颜色标记菜单，显示二级菜单，包括：标记、清除上次标记、清除标记、标记所有;
    m_colorMarkMenu = new DMenu(tr("Color Mark"));

    // 为颜色标记Menu，增加事件过滤
    m_colorMarkMenu->installEventFilter(this);
    m_cancleMarkAllLine = new QAction(tr("Clear All Marks"), this);
    m_cancleLastMark = new QAction(tr("Clear Last Mark"), this);

    //添加当前颜色选择控件　梁卫东
    ColorSelectWdg *pColorsSelectWdg = new ColorSelectWdg(QString(), this);
    connect(pColorsSelectWdg, &ColorSelectWdg::sigColorSelected, this, &TextEdit::slotSigColorSelected);

    m_actionColorStyles = new QWidgetAction(this);
    m_actionColorStyles->setDefaultWidget(pColorsSelectWdg);

    m_markCurrentAct = new QAction(tr("Mark"), this);
    connect(m_markCurrentAct, &QAction::triggered, this, [this, pColorsSelectWdg]() {
        isMarkCurrentLine(true, pColorsSelectWdg->getDefaultColor().name());
        renderAllSelections();
    });

    //添加全部颜色选择控件　梁卫东
    ColorSelectWdg *pColorsAllSelectWdg = new ColorSelectWdg(QString(), this);
    connect(pColorsAllSelectWdg, &ColorSelectWdg::sigColorSelected, this, &TextEdit::slotSigColorAllSelected);
    m_actionAllColorStyles = new QWidgetAction(this);
    m_actionAllColorStyles->setDefaultWidget(pColorsAllSelectWdg);

    m_markAllAct = new QAction(tr("Mark All"), this);
    connect(m_markAllAct, &QAction::triggered, this, [this, pColorsAllSelectWdg]() {
        m_strMarkAllLineColorName = pColorsAllSelectWdg->getDefaultColor().name();
        isMarkAllLine(true, pColorsAllSelectWdg->getDefaultColor().name());
        renderAllSelections();
    });

    m_addComment = new QAction(tr("Add Comment"), this);
    m_cancelComment = new QAction(tr("Remove Comment"), this);

    connect(m_rightMenu, &DMenu::aboutToHide, this, &TextEdit::removeHighlightWordUnderCursor);
    connect(m_undoAction, &QAction::triggered, this, [ = ]() {
        this->undo_();
    });
    connect(m_redoAction, &QAction::triggered, this, [ = ]() {
        this->redo_();
    });
    connect(m_cutAction, &QAction::triggered, this, &TextEdit::slotCutAction);
    connect(m_copyAction, &QAction::triggered, this, &TextEdit::slotCopyAction);
    connect(m_pasteAction, &QAction::triggered, this, &TextEdit::slotPasteAction);
    connect(m_deleteAction, &QAction::triggered, this, &TextEdit::slotDeleteAction);
    connect(m_selectAllAction, &QAction::triggered, this, &TextEdit::slotSelectAllAction);
    connect(m_findAction, &QAction::triggered, this, &TextEdit::clickFindAction);
    connect(m_replaceAction, &QAction::triggered, this, &TextEdit::clickReplaceAction);
    connect(m_jumpLineAction, &QAction::triggered, this, &TextEdit::clickJumpLineAction);
    connect(m_fullscreenAction, &QAction::triggered, this, &TextEdit::clickFullscreenAction);
    connect(m_exitFullscreenAction, &QAction::triggered, this, &TextEdit::clickFullscreenAction);
    connect(m_enableReadOnlyModeAction, &QAction::triggered, this, &TextEdit::toggleReadOnlyMode);
    connect(m_disableReadOnlyModeAction, &QAction::triggered, this, &TextEdit::toggleReadOnlyMode);
    connect(m_openInFileManagerAction, &QAction::triggered, this, &TextEdit::slotOpenInFileManagerAction);
    connect(m_addComment, &QAction::triggered, this, &TextEdit::slotAddComment);
    connect(m_cancelComment, &QAction::triggered, this, &TextEdit::slotCancelComment);
    connect(m_voiceReadingAction, &QAction::triggered, this, &TextEdit::slotVoiceReadingAction);
    connect(m_stopReadingAction, &QAction::triggered, this, &TextEdit::slotStopReadingAction);
    connect(m_dictationAction, &QAction::triggered, this, &TextEdit::slotdictationAction);
    connect(m_translateAction, &QAction::triggered, this, &TextEdit::slot_translate);
    connect(m_columnEditAction, &QAction::triggered, this, &TextEdit::slotColumnEditAction);
    connect(m_addBookMarkAction, &QAction::triggered, this, &TextEdit::addOrDeleteBookMark);
    connect(m_cancelBookMarkAction, &QAction::triggered, this, &TextEdit::addOrDeleteBookMark);
    connect(m_preBookMarkAction, &QAction::triggered, this, &TextEdit::slotPreBookMarkAction);
    connect(m_nextBookMarkAction, &QAction::triggered, this, &TextEdit::slotNextBookMarkAction);
    connect(m_clearBookMarkAction, &QAction::triggered, this, &TextEdit::slotClearBookMarkAction);
    connect(m_flodAllLevel, &QAction::triggered, this, &TextEdit::slotFlodAllLevel);
    connect(m_unflodAllLevel, &QAction::triggered, this, &TextEdit::slotUnflodAllLevel);
    connect(m_flodCurrentLevel, &QAction::triggered, this, &TextEdit::slotFlodCurrentLevel);
    connect(m_unflodCurrentLevel, &QAction::triggered, this, &TextEdit::slotUnflodCurrentLevel);
    connect(m_cancleMarkAllLine, &QAction::triggered, this, &TextEdit::slotCancleMarkAllLine);
    connect(m_cancleLastMark, &QAction::triggered, this, &TextEdit::slotCancleLastMark);

    // Init convert case sub menu.
    m_haveWordUnderCursor = false;
    m_convertCaseMenu = new DMenu(tr("Change Case"));
    m_upcaseAction = new QAction(tr("Upper Case"), this);
    m_downcaseAction = new QAction(tr("Lower Case"), this);
    m_capitalizeAction = new QAction(tr("Capitalize"), this);
    m_convertCaseMenu->addAction(m_upcaseAction);
    m_convertCaseMenu->addAction(m_downcaseAction);
    m_convertCaseMenu->addAction(m_capitalizeAction);

    connect(m_upcaseAction, &QAction::triggered, this, &TextEdit::upcaseWord);
    connect(m_downcaseAction, &QAction::triggered, this, &TextEdit::downcaseWord);
    connect(m_capitalizeAction, &QAction::triggered, this, &TextEdit::capitalizeWord);

    m_canUndo = false;
    m_canRedo = false;

    connect(this, &TextEdit::undoAvailable, this, &TextEdit::slotUndoAvailable);
    connect(this, &TextEdit::redoAvailable, this, &TextEdit::slotRedoAvailable);
}

void TextEdit::popRightMenu(QPoint pos)
{
    //清除菜单分割线残影
    if (m_rightMenu != nullptr) {
        delete m_rightMenu;
        m_rightMenu = nullptr;
    }
    m_rightMenu = new DMenu;

    m_rightMenu->clear();
    QTextCursor selectionCursor = textCursor();
    selectionCursor.movePosition(QTextCursor::StartOfBlock);
    selectionCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString text = selectionCursor.selectedText();

    // init base.
    bool isBlankLine = text.trimmed().isEmpty();

    bool isAddUndoRedo = false;
    if (m_pUndoStack->canUndo() && m_bReadOnlyPermission == false && m_readOnlyMode == false) {
        m_rightMenu->addAction(m_undoAction);
        isAddUndoRedo = true;
    }

    if (m_pUndoStack->canRedo() && m_bReadOnlyPermission == false && m_readOnlyMode == false) {
        m_rightMenu->addAction(m_redoAction);
        isAddUndoRedo = true;
    }

    if (isAddUndoRedo) {
        m_rightMenu->addSeparator();
    }

    if (textCursor().hasSelection() || m_hasColumnSelection) {
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addAction(m_cutAction);
        }
        m_rightMenu->addAction(m_copyAction);
    }

    if (canPaste()) {
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addAction(m_pasteAction);
        }
    }

    if (textCursor().hasSelection() || m_hasColumnSelection) {
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addAction(m_deleteAction);
        }

    }

    if (!document()->isEmpty()) {
        m_rightMenu->addAction(m_selectAllAction);
    }

    m_rightMenu->addSeparator();

    if (!document()->isEmpty()) {
        m_rightMenu->addAction(m_findAction);
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addAction(m_replaceAction);
        }
        m_rightMenu->addAction(m_jumpLineAction);
        m_rightMenu->addSeparator();
    }

    if (textCursor().hasSelection()) {
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addMenu(m_convertCaseMenu);
        }
    } else {
        m_convertCaseMenu->hide();
    }

    // intelligent judge whether to support comments.
    const auto def = m_repository.definitionForFileName(QFileInfo(m_sFilePath).fileName());
    if (characterCount() &&
            (textCursor().hasSelection() || !isBlankLine) &&
            !def.filePath().isEmpty()) {

        /*
         * 不支持注释的文件类型，右键菜单不显示“添加注释/取消注释”
         * 不支持注释的文件类型：Markdown(.d)/vCard(.vcf)/JSON(.json)
         */
        if (def.name() != "Markdown"
                && !def.name().contains(QString("vCard"))
                && !def.name().contains(QString("JSON"))) {
            m_rightMenu->addAction(m_addComment);
            m_rightMenu->addAction(m_cancelComment);
        }
    }

    if (m_bReadOnlyPermission || m_readOnlyMode) {
        m_addComment->setEnabled(false);
        m_cancelComment->setEnabled(false);
    } else {
        m_addComment->setEnabled(true);
        m_cancelComment->setEnabled(true);
    }

    m_rightMenu->addSeparator();
    if (m_bReadOnlyPermission == false) {
        if (m_readOnlyMode) {
            m_rightMenu->addAction(m_disableReadOnlyModeAction);
        } else {
            m_rightMenu->addAction(m_enableReadOnlyModeAction);
        }
    }

    m_rightMenu->addAction(m_openInFileManagerAction);
    m_rightMenu->addSeparator();
    if (static_cast<Window *>(this->window())->isFullScreen()) {
        m_rightMenu->addAction(m_exitFullscreenAction);
    } else {
        m_rightMenu->addAction(m_fullscreenAction);
    }

    /* 专业版/家庭版/教育版鼠标右键菜单支持语音读写 */
    /* 更换成只要发现有com.iflytek.aiassistant服务已经注册开启，则开启支持语音读写功能 */
    /*if ((DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosProfessional) ||
        (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosHome) ||
        (DSysInfo::uosEditionType() == DSysInfo::UosEdition::UosEducation)) {*/

    if (m_wrapper->window()->getIsRegistIflytekAiassistant()) {
        bool stopReadingState = false;
        QDBusMessage stopReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                     "/aiassistant/tts",
                                                                     "com.iflytek.aiassistant.tts",
                                                                     "isTTSInWorking");

        QDBusReply<bool> stopReadingStateRet = QDBusConnection::sessionBus().asyncCall(stopReadingMsg, 100);
        if (stopReadingStateRet.isValid()) {
            stopReadingState = stopReadingStateRet.value();
        }
        if (!stopReadingState) {
            m_rightMenu->addAction(m_voiceReadingAction);
            m_voiceReadingAction->setEnabled(false);
        } else {
            m_rightMenu->removeAction(m_voiceReadingAction);
            m_rightMenu->addAction(m_stopReadingAction);
        }
        bool voiceReadingState = false;
        QDBusMessage voiceReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                      "/aiassistant/tts",
                                                                      "com.iflytek.aiassistant.tts",
                                                                      "getTTSEnable");

        QDBusReply<bool> voiceReadingStateRet = QDBusConnection::sessionBus().asyncCall(voiceReadingMsg, 100);
        //没有收到返回就加载配置文件数据
        if (voiceReadingStateRet.isValid()) {
            voiceReadingState = voiceReadingStateRet.value();
        } else {
            voiceReadingState = m_wrapper->window()->getIflytekaiassistantConfig("aiassistant-tts");
        }
        if ((textCursor().hasSelection() && voiceReadingState) || m_hasColumnSelection) {
            m_voiceReadingAction->setEnabled(true);
        }

        m_rightMenu->addAction(m_dictationAction);
        bool dictationState = false;
        QDBusMessage dictationMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                   "/aiassistant/iat",
                                                                   "com.iflytek.aiassistant.iat",
                                                                   "getIatEnable");

        QDBusReply<bool> dictationStateRet = QDBusConnection::sessionBus().asyncCall(dictationMsg, 100);
        //没有收到返回就加载配置文件数据
        if (dictationStateRet.isValid()) {
            dictationState = dictationStateRet.value();
        } else {
            dictationState = m_wrapper->window()->getIflytekaiassistantConfig("aiassistant-iat");
        }
        m_dictationAction->setEnabled(dictationState);
        if (m_bReadOnlyPermission || m_readOnlyMode) {
            m_dictationAction->setEnabled(false);
        }

        m_rightMenu->addAction(m_translateAction);
        m_translateAction->setEnabled(false);
        bool translateState = false;
        QDBusMessage translateReadingMsg = QDBusMessage::createMethodCall("com.iflytek.aiassistant",
                                                                          "/aiassistant/trans",
                                                                          "com.iflytek.aiassistant.trans",
                                                                          "getTransEnable");

        QDBusReply<bool> translateStateRet = QDBusConnection::sessionBus().asyncCall(translateReadingMsg, 100);
        //没有收到返回就加载配置文件数据
        if (translateStateRet.isValid()) {
            translateState = translateStateRet.value();
        } else {
            translateState = m_wrapper->window()->getIflytekaiassistantConfig("aiassistant-trans");
        }
        if ((textCursor().hasSelection() && translateState) || m_hasColumnSelection) {
            m_translateAction->setEnabled(translateState);
        }
    }


    if (!this->document()->isEmpty()) {

        m_colorMarkMenu->clear();
        // 清空 tab order list
        m_MarkColorMenuTabOrder.clear();

        // 增加 Mark Color Menu Item
        m_colorMarkMenu->addAction(m_markCurrentAct);
        m_colorMarkMenu->addAction(m_actionColorStyles);
        // 将对应 Mark Color Menu Item 加入 Tab Order
        // QPair<QAction*, bool> , bool决定tab过程中是否可以获取focus
        m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_markCurrentAct, true));
        m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_actionColorStyles, false));

        // 增加 Mark Color Menu Item
        m_colorMarkMenu->addSeparator();
        m_colorMarkMenu->addAction(m_markAllAct);
        m_colorMarkMenu->addAction(m_actionAllColorStyles);
        m_colorMarkMenu->addSeparator();
        // 将对应 Mark Color Menu Item 加入 Tab Order
        m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_markAllAct, true));
        m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_actionAllColorStyles, false));

        if (m_markOperations.size() > 0) {
            // 增加 Mark Color Menu Item
            m_colorMarkMenu->addAction(m_cancleLastMark);
            m_colorMarkMenu->addSeparator();
            m_colorMarkMenu->addAction(m_cancleMarkAllLine);
            // 将对应 Mark Color Menu Item 加入 Tab Order
            m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_cancleLastMark, true));
            m_MarkColorMenuTabOrder.append(QPair<QAction *, bool>(m_cancleMarkAllLine, true));
        }

        m_rightMenu->addSeparator();
        if (m_bReadOnlyPermission == false && m_readOnlyMode == false) {
            m_rightMenu->addAction(m_columnEditAction);
        }
        m_rightMenu->addMenu(m_colorMarkMenu);
    }

    QPoint pos1 = cursorRect().bottomRight();
    //当全选大文本 坐标超出屏幕外显示不了 梁卫东　２０２０－０８－１９　１０：２３：２９
    if (pos1.y() > this->rect().height()) {
        pos1.setY((this->rect().height()) / 2);
        pos1.setX((this->rect().width()) / 2);
    }
    if (pos.isNull()) {
        m_rightMenu->exec(mapToGlobal(pos1));
    } else {
        m_rightMenu->exec(pos);
    }
}

void TextEdit::setWrapper(EditWrapper *w)
{
    m_wrapper = w;
}

EditWrapper *TextEdit::getWrapper()
{
    return m_wrapper;
}

bool TextEdit::isUndoRedoOpt()
{
    return (m_pUndoStack->canRedo() || m_pUndoStack->canUndo());
}

bool TextEdit::getModified()
{
    return (document()->isModified() && (m_pUndoStack->canUndo() || m_pUndoStack->index() != m_lastSaveIndex));
}


int TextEdit::getCurrentLine()
{
    return textCursor().blockNumber() + 1;
}

int TextEdit::getCurrentColumn()
{
    return textCursor().columnNumber();
}

int TextEdit::getPosition()
{
    return textCursor().position();
}

int TextEdit::getScrollOffset()
{
    return verticalScrollBar()->value();
}

void TextEdit::forwardChar()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::NextCharacter);
    }
}

void TextEdit::backwardChar()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::PreviousCharacter);
    }
}

void TextEdit::forwardWord()
{
    QTextCursor cursor = textCursor();

    if (m_cursorMark) {
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
    } else {
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::MoveAnchor);
    }

    setTextCursor(cursor);
}

void TextEdit::backwardWord()
{
    QTextCursor cursor = textCursor();

    if (m_cursorMark) {
        // cursor.setPosition(getPrevWordPosition(cursor, QTextCursor::KeepAnchor), QTextCursor::KeepAnchor);
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
    } else {
        // cursor.setPosition(getPrevWordPosition(cursor, QTextCursor::MoveAnchor), QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::MoveAnchor);
    }

    setTextCursor(cursor);
}

void TextEdit::forwardPair()
{
    // Record cursor and seleciton position before move cursor.
    int actionStartPos = textCursor().position();
    int selectionStartPos = textCursor().selectionStart();
    int selectionEndPos = textCursor().selectionEnd();

    // Because find always search start from selection end position.
    // So we need clear selection to make search start from cursor.
    QTextCursor removeSelectionCursor = textCursor();
    removeSelectionCursor.clearSelection();
    setTextCursor(removeSelectionCursor);

    // Start search.
    if (find(QRegExp("[\]>)}]"))) {
        int findPos = textCursor().position();

        QTextCursor cursor = textCursor();
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

        if (actionStartPos == selectionStartPos) {
            cursor.setPosition(selectionEndPos, QTextCursor::MoveAnchor);
            cursor.setPosition(findPos, moveMode);
        } else {
            cursor.setPosition(selectionStartPos, QTextCursor::MoveAnchor);
            cursor.setPosition(findPos, moveMode);
        }

        setTextCursor(cursor);
    }
}

void TextEdit::backwardPair()
{
    // Record cursor and seleciton position before move cursor.
    int actionStartPos = textCursor().position();
    int selectionStartPos = textCursor().selectionStart();
    int selectionEndPos = textCursor().selectionEnd();

    // Because find always search start from selection end position.
    // So we need clear selection to make search start from cursor.
    QTextCursor removeSelectionCursor = textCursor();
    removeSelectionCursor.clearSelection();
    setTextCursor(removeSelectionCursor);

    QTextDocument::FindFlags options;
    options |= QTextDocument::FindBackward;

    if (find(QRegExp("[\[<({]"), options)) {
        QTextCursor cursor = textCursor();
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

        cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);

        int findPos = cursor.position();

        if (actionStartPos == selectionStartPos) {
            cursor.setPosition(selectionEndPos, QTextCursor::MoveAnchor);
            cursor.setPosition(findPos, moveMode);
            setTextCursor(cursor);
        } else {
            cursor.setPosition(selectionStartPos, QTextCursor::MoveAnchor);
            cursor.setPosition(findPos, moveMode);
            setTextCursor(cursor);
        }
    }
}

int TextEdit::blockCount() const
{
    return document()->blockCount();
}

int TextEdit::characterCount() const
{
    return document()->characterCount();
}

QTextBlock TextEdit::firstVisibleBlock()
{
    return document()->findBlockByLineNumber(getFirstVisibleBlockId());
}

void TextEdit::moveToStart()
{
    verticalScrollBar()->setValue(0);
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::Start);
    }

    // 移动展示区域，手动高亮文本
    m_wrapper->OnUpdateHighlighter();
}

void TextEdit::moveToEnd()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::End);
    }

    // 移动展示区域，手动高亮文本
    m_wrapper->OnUpdateHighlighter();
}

void TextEdit::moveToStartOfLine()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::StartOfBlock);
    }
}

void TextEdit::moveToEndOfLine()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::EndOfBlock);
    }
}

void TextEdit::moveToLineIndentation()
{
    // Init cursor and move type.
    QTextCursor cursor = textCursor();
    // Get line start position.
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    int startColumn = cursor.columnNumber();
    // Get line end position.
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    int endColumn = cursor.columnNumber();
    // Move the cursor to line start first while keep the anchor to end of block.
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    int column = startColumn;
    while (column < endColumn)
    {
        QChar currentChar = *cursor.selection().toPlainText().data();
        if (!currentChar.isSpace())
        {
            //stop and reset anchor while be at row indentation.
            cursor.setPosition(cursor.position(), QTextCursor::MoveAnchor);
            break;
        }
        else
        {
            //while including 'space',just move ahead.
            cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);
        }
        column++;
    }
    cursor.clearSelection();
    setTextCursor(cursor);
}

void TextEdit::nextLine()
{
    m_isSelectAll = false;
    if (!characterCount())
        return;

    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::Down);
    }

    if (m_wrapper != nullptr) {
        m_wrapper->OnUpdateHighlighter();
        if ((m_wrapper->window()->findBarIsVisiable() || m_wrapper->window()->replaceBarIsVisiable()) &&
                (QString::compare(m_wrapper->window()->getKeywordForSearchAll(), m_wrapper->window()->getKeywordForSearch(), Qt::CaseInsensitive) == 0)) {
            highlightKeywordInView(m_wrapper->window()->getKeywordForSearchAll());
        }

        markAllKeywordInView();
    }
}

void TextEdit::prevLine()
{
    m_isSelectAll = false;
    if (!characterCount())
        return;

    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
        setTextCursor(cursor);
    } else {
        moveCursorNoBlink(QTextCursor::Up);
    }

    if (m_wrapper != nullptr) {
        m_wrapper->OnUpdateHighlighter();
        if ((m_wrapper->window()->findBarIsVisiable() || m_wrapper->window()->replaceBarIsVisiable()) &&
                (QString::compare(m_wrapper->window()->getKeywordForSearchAll(), m_wrapper->window()->getKeywordForSearch(), Qt::CaseInsensitive) == 0)) {
            highlightKeywordInView(m_wrapper->window()->getKeywordForSearchAll());
        }

        markAllKeywordInView();
    }
}

void TextEdit::moveCursorNoBlink(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode)
{
    // Function moveCursorNoBlink will blink cursor when move cursor.
    // But function movePosition won't, so we use movePosition to avoid that cursor link when moving cursor.
    QTextCursor cursor = textCursor();
    cursor.movePosition(operation, mode);
    setTextCursor(cursor);
}

void TextEdit::jumpToLine(int line, bool keepLineAtCenter)
{
    QTextCursor cursor(document()->findBlockByNumber(line - 1)); // line - 1 because line number starts from 0
    //verticalScrollBar()->setValue(fontMetrics().height() * line - height());
    // Update cursor.
    setTextCursor(cursor);

    if (keepLineAtCenter) {
        keepCurrentLineAtCenter();
    }
    m_pLeftAreaWidget->m_pLineNumberArea->update();
}

void TextEdit::newline()
{
    // Stop mark if mark is set.
    tryUnsetMark();

    QTextCursor cursor = textCursor();
    auto com = new InsertTextUndoCommand(cursor, "\n", this);
    m_pUndoStack->push(com);
    setTextCursor(cursor);
}

void TextEdit::openNewlineAbove()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    InsertTextUndoCommand *com = new InsertTextUndoCommand(cursor, "\n", this);
    m_pUndoStack->push(com);
    cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
}

void TextEdit::openNewlineBelow()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    InsertTextUndoCommand *com = new InsertTextUndoCommand(cursor, "\n", this);
    m_pUndoStack->push(com);

    //make the vertical scroll bar change together.
    this->setTextCursor(cursor);
}

/*
 * swap tow lines.
 * firstly,combine the contents of the current line with the contents of the previous or next line.
 * then,insert the combined content.
 * */
void TextEdit::moveLineDownUp(bool up)
{
    if (up) {
        QTextCursor cursor = this->textCursor();
        //current line isn't the first line of this document
        if (0 != cursor.blockNumber()) {
            int startpos = 0;
            int endpos = 0;
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            endpos = cursor.position();
            QString curtext = cursor.selectedText();

            cursor.movePosition(QTextCursor::PreviousBlock);
            startpos = cursor.position();
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            QString uptext = cursor.selectedText();

            cursor.setPosition(startpos);
            cursor.setPosition(endpos, QTextCursor::KeepAnchor);
            InsertTextUndoCommand *com = new InsertTextUndoCommand(cursor, curtext + "\n" + uptext, this);
            m_pUndoStack->push(com);

            //ensure that this operation can be performed multiple times in succession.
            //and make the vertical scroll bar change together at the same time.
            cursor.setPosition(startpos);
            this->setTextCursor(cursor);
        }
    } else {
        QTextCursor cursor = this->textCursor();
        //current line isn't the last line of this document
        if (cursor.blockNumber() + 1 != this->document()->blockCount()) {
            int startpos = 0;
            int endpos = 0;
            cursor.movePosition(QTextCursor::StartOfLine);
            startpos = cursor.position();
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            QString curtext = cursor.selectedText();

            cursor.movePosition(QTextCursor::NextBlock);
            cursor.movePosition(QTextCursor::StartOfLine);
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            endpos = cursor.position();
            QString downtext = cursor.selectedText();

            cursor.setPosition(startpos);
            cursor.setPosition(endpos, QTextCursor::KeepAnchor);
            InsertTextUndoCommand *com = new InsertTextUndoCommand(cursor, downtext + "\n" + curtext, this);
            m_pUndoStack->push(com);

            //make the vertical scroll bar change together.
            cursor.setPosition(endpos);
            this->setTextCursor(cursor);
        }
    }


}

void TextEdit::scrollLineUp()
{
    QScrollBar *scrollbar = verticalScrollBar();

    scrollbar->setValue(scrollbar->value() - 1);

    if (cursorRect().y() > rect().height() - fontMetrics().height()) {
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Up, moveMode);
        setTextCursor(cursor);
    }
}

void TextEdit::scrollLineDown()
{
    QScrollBar *scrollbar = verticalScrollBar();

    scrollbar->setValue(scrollbar->value() + 1);

    if (cursorRect().y() < 0) {
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Down, moveMode);
        setTextCursor(cursor);
    }
}

void TextEdit::scrollUp()
{
    QScrollBar *scrollbar = verticalScrollBar();
    scrollbar->triggerAction(QAbstractSlider::SliderPageStepSub);

    m_pLeftAreaWidget->m_pLineNumberArea->update();
    //m_pLeftAreaWidget->m_pFlodArea->update();
    //m_pLeftAreaWidget->m_pBookMarkArea->update();

    if (verticalScrollBar()->maximum() > 0) {
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
        QPoint startPoint = QPointF(0, fontMetrics().height()).toPoint();
        QTextCursor cur = cursorForPosition(startPoint);
        QTextCursor cursor = textCursor();
        cursor.setPosition(cur.position(), moveMode);
        setTextCursor(cursor);
    }

    if (m_wrapper != nullptr) {
        m_wrapper->OnUpdateHighlighter();
        if ((m_wrapper->window()->findBarIsVisiable() || m_wrapper->window()->replaceBarIsVisiable()) &&
                (QString::compare(m_wrapper->window()->getKeywordForSearchAll(), m_wrapper->window()->getKeywordForSearch(), Qt::CaseInsensitive) == 0)) {
            highlightKeywordInView(m_wrapper->window()->getKeywordForSearchAll());
        }

        markAllKeywordInView();
    }
}

void TextEdit::scrollDown()
{
    QScrollBar *scrollbar = verticalScrollBar();
    scrollbar->triggerAction(QAbstractSlider::SliderPageStepAdd);

    m_pLeftAreaWidget->m_pLineNumberArea->update();
    //m_pLeftAreaWidget->m_pFlodArea->update();
    //m_pLeftAreaWidget->m_pBookMarkArea->update();

    if (verticalScrollBar()->maximum() > 0) {
        auto moveMode = m_cursorMark ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
        QPoint endPoint = QPointF(0, height() - fontMetrics().height()).toPoint();
        QTextCursor cur = cursorForPosition(endPoint);
        QTextCursor cursor = textCursor();
        cursor.setPosition(cur.position(), moveMode);
        setTextCursor(cursor);
    }

    if (m_wrapper != nullptr) {
        m_wrapper->OnUpdateHighlighter();
        if ((m_wrapper->window()->findBarIsVisiable() || m_wrapper->window()->replaceBarIsVisiable()) &&
                (QString::compare(m_wrapper->window()->getKeywordForSearchAll(), m_wrapper->window()->getKeywordForSearch(), Qt::CaseInsensitive) == 0)) {
            highlightKeywordInView(m_wrapper->window()->getKeywordForSearchAll());
        }

        markAllKeywordInView();
    }
}

/*
 * copy the current line
 * firstly,get the text of current line.
 * then,insert the text with '\n' at the end of current line.
 * */
void TextEdit::duplicateLine()
{
    auto cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    auto text = cursor.selectedText();

    text = "\n" + text;
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
    auto com = new InsertTextUndoCommand(cursor, text, this);
    m_pUndoStack->push(com);

    //make the vertical scroll bar change together.
    this->setTextCursor(cursor);
}

void TextEdit::copyLines()
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!Utils::enableClipCopy(getFilePath())) {
        return;
    }

    // Record current cursor and build copy cursor.
    QTextCursor currentCursor = textCursor();
    QTextCursor copyCursor = textCursor();

    if (textCursor().hasSelection()) {
        // Sort selection bound cursors.
        int startPos = textCursor().anchor();
        int endPos = textCursor().position();

        if (startPos > endPos) {
            std::swap(startPos, endPos);
        }

        // Selectoin multi-lines.
        QTextCursor startCursor = textCursor();
        startCursor.setPosition(startPos, QTextCursor::MoveAnchor);
        startCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

        QTextCursor endCursor = textCursor();
        endCursor.setPosition(endPos, QTextCursor::MoveAnchor);
        endCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);

        copyCursor.setPosition(startCursor.position(), QTextCursor::MoveAnchor);
        copyCursor.setPosition(endCursor.position(), QTextCursor::KeepAnchor);

        popupNotify(tr("Selected line(s) copied"));
    } else {
        // Selection current line.
        copyCursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        copyCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        popupNotify(tr("Current line copied"));
    }

    // Copy lines to system clipboard.
    setTextCursor(copyCursor);
    copySelectedText(true);

    // Reset cursor before copy lines.
    copyCursor.setPosition(currentCursor.position(), QTextCursor::MoveAnchor);
    setTextCursor(copyCursor);
}

void TextEdit::cutlines()
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!Utils::enableClipCopy(getFilePath())) {
        return;
    }

    if(m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (textCursor().hasSelection() || m_bIsAltMod) {
        this->cut(true);
        popupNotify(tr("Selected line(s) clipped"));
    } else {
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        if (cursor.hasSelection()) {
            QString data = cursor.selectedText();
            QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(cursor, this);
            m_pUndoStack->push(pDeleteStack);
            QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
            clipboard->setText(data);
            popupNotify(tr("Current line clipped"));
        }
    }
}

void TextEdit::joinLines()
{
    QTextCursor cursor = this->textCursor();
    //the current line isn't the last line of text.
    if (cursor.blockNumber() + 1 != this->document()->blockCount()) {
        int startpos = 0;
        int endpos = 0;
        cursor.movePosition(QTextCursor::EndOfBlock);
        startpos = cursor.position();
        cursor.movePosition(QTextCursor::NextBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        endpos = cursor.position();
        QString t = cursor.selectedText();

        cursor.setPosition(startpos);
        cursor.setPosition(endpos, QTextCursor::KeepAnchor);
        auto com = new InsertTextUndoCommand(cursor, t, this);
        m_pUndoStack->push(com);

        cursor.setPosition(startpos);
        this->setTextCursor(cursor);
    }
}

void TextEdit::killLine()
{
    if (tryUnsetMark()) {
        return;
    }

    // Remove selection content if has selection.
    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (textCursor().hasSelection()) {
        //textCursor().removeSelectedText();
        //deleteSelectTextEx(textCursor());
        deleteSelectTextEx(textCursor(), textCursor().selectedText(), false);
    } else {
        auto cursor = this->textCursor();
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        //the right of current line has no text but it is not the end line of this document
        if (cursor.selectedText().isEmpty() && cursor.blockNumber() + 1 != this->document()->blockCount()) {
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
        }

        if (!cursor.selectedText().isEmpty()) {
            DeleteBackCommand *com = new DeleteBackCommand(cursor, this);
            m_pUndoStack->push(com);
        }
    }
}

void TextEdit::killCurrentLine()
{
    if (tryUnsetMark()) {
        return;
    }

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    auto cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    if (cursor.blockNumber() + 1 != this->document()->blockCount()) {
        cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    } else {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }
    if (!cursor.selectedText().isEmpty()) {
        DeleteBackCommand *com = new DeleteBackCommand(cursor, this);
        m_pUndoStack->push(com);
    }
}

void TextEdit::killBackwardWord()
{
    tryUnsetMark();

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (textCursor().hasSelection()) {
        //textCursor().removeSelectedText();
    } else {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        deleteSelectTextEx(cursor);
    }
}

void TextEdit::killForwardWord()
{
    tryUnsetMark();

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (textCursor().hasSelection()) {
        //textCursor().removeSelectedText();
    } else {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        deleteSelectTextEx(cursor);
    }
}

void TextEdit::indentText()
{
    auto cursor = this->textCursor();
    if (cursor.hasSelection()) {
        //calculate the start postion and the end postion of current selection.
        int pos1 = cursor.position();
        int pos2 = cursor.anchor();
        if (pos1 > pos2)
            std::swap(pos1, pos2);

        //calculate the start line and end line of current selection.
        cursor.setPosition(pos1);
        int line1 = cursor.blockNumber();
        cursor.setPosition(pos2);
        int line2 = cursor.blockNumber();

        //do the indent operation
        auto com = new IndentTextCommand(this, pos1, pos2, line1, line2);
        m_pUndoStack->push(com);
    }
}

void TextEdit::unindentText()
{
    QTextCursor cursor = this->textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock);
    int pos = cursor.position();
    cursor.setPosition(cursor.position() + 1, QTextCursor::KeepAnchor);

    //the text in front of current line is '\t'.
    if ("\t" == cursor.selectedText()) {
        DeleteBackCommand *com = new DeleteBackCommand(cursor, this);
        m_pUndoStack->push(com);
    }
    //the text in front of current line is ' '.
    else if (" " == cursor.selectedText()) {
        int startpos = pos;
        int cnt = 0;
        // calculate the number of ' '.
        while (document()->characterAt(pos) == ' ' && cnt < m_tabSpaceNumber) {
            pos++;
            cnt++;
        }
        cursor.setPosition(startpos);
        cursor.setPosition(pos, QTextCursor::KeepAnchor);
        DeleteBackCommand *com = new DeleteBackCommand(cursor, this);
        m_pUndoStack->push(com);

    }

}

void TextEdit::setTabSpaceNumber(int number)
{
    m_tabSpaceNumber = number;
    updateFont();
    //updateLineNumber();
    updateLeftAreaWidget();
}

void TextEdit::upcaseWord()
{
    tryUnsetMark();
    convertWordCase(UPPER);
}

void TextEdit::downcaseWord()
{
    tryUnsetMark();
    convertWordCase(LOWER);
}

void TextEdit::capitalizeWord()
{
    tryUnsetMark();

    convertWordCase(CAPITALIZE);
}

void TextEdit::transposeChar()
{
    QTextCursor cursor = this->textCursor();
    int pos = cursor.position();
    cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
    QString r = cursor.selectedText();
    cursor.setPosition(pos - 1);
    cursor.setPosition(pos, QTextCursor::KeepAnchor);
    QString l = cursor.selectedText();

    if (!l.isEmpty() && !r.isEmpty()) {
        cursor.setPosition(pos - 1);
        cursor.setPosition(pos + 1, QTextCursor::KeepAnchor);
        auto com = new InsertTextUndoCommand(cursor, r + l, this);
        m_pUndoStack->push(com);
        ensureCursorVisible();
    }
}

void TextEdit::handleCursorMarkChanged(bool mark, QTextCursor cursor)
{
    if (mark) {
        m_markStartLine = cursor.blockNumber() + 1;
    } else {
        m_markStartLine = -1;
    }

    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();
}

void TextEdit::slotValueChanged(int iValue)
{
    Q_UNUSED(iValue);
    if (m_isSelectAll) {
        this->selectTextInView();
    }

    this->updateLeftAreaWidget();
}

void TextEdit::convertWordCase(ConvertCase convertCase)
{
#if 0
    if (textCursor().hasSelection()) {
        QString text = textCursor().selectedText();

        if (convertCase == UPPER) {
            textCursor().insertText(text.toUpper());
        } else if (convertCase == LOWER) {
            textCursor().insertText(text.toLower());
        } else {
            textCursor().insertText(capitalizeText(text));
        }
    } else {
        QTextCursor cursor;

        // Move cursor to mouse position first. if have word under mouse pointer.
        if (m_haveWordUnderCursor) {
            setTextCursor(m_wordUnderPointerCursor);
        }

        cursor = textCursor();
        cursor.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
        cursor.setPosition(getNextWordPosition(cursor, QTextCursor::KeepAnchor), QTextCursor::KeepAnchor);

        QString text = cursor.selectedText();
        if (convertCase == UPPER) {
            cursor.insertText(text.toUpper());
        } else if (convertCase == LOWER) {
            cursor.insertText(text.toLower());
        } else {
            cursor.insertText(capitalizeText(text));
        }

        setTextCursor(cursor);

        m_haveWordUnderCursor = false;
    }
#endif

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (textCursor().hasSelection()) {
        QString text = textCursor().selectedText();
        if (convertCase == UPPER) {
            text = text.toUpper();
        } else if (convertCase == LOWER) {
            text = text.toLower();
        } else {
            text = capitalizeText(text);
        }

        // 如果没有实际文本更改效果，不进行文本替换操作
        if (text != textCursor().selectedText()) {
            InsertTextUndoCommand *insertCommand = new InsertTextUndoCommand(textCursor(), text, this);
            m_pUndoStack->push(insertCommand);
        }
    } else {
        QTextCursor cursor;

        // Move cursor to mouse position first. if have word under mouse pointer.
        if (m_haveWordUnderCursor) {
            setTextCursor(m_wordUnderPointerCursor);
        }

        cursor = textCursor();
        cursor.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
        cursor.setPosition(getNextWordPosition(cursor, QTextCursor::KeepAnchor), QTextCursor::KeepAnchor);

        QString text = cursor.selectedText();
        if (!text.isEmpty()) {
            if (convertCase == UPPER) {
                text = text.toUpper();
            } else if (convertCase == LOWER) {
                text = text.toLower();
            } else {
                text = capitalizeText(text);
            }

            InsertTextUndoCommand *insertCommand = new InsertTextUndoCommand(cursor, text, this);
            m_pUndoStack->push(insertCommand);

            setTextCursor(cursor);

            m_haveWordUnderCursor = false;
        }
    }
}

QString TextEdit::capitalizeText(QString text)
{
    QString newText = text.toLower();
    QChar currentChar;
    QChar nextChar;
    if (!newText.at(0).isSpace()) {
        newText.replace(0, 1, newText.at(0).toUpper());
    }

    for (int i = 0; i < newText.size(); i++) {
        currentChar = newText.at(i);
        if (i + 1 < newText.size())
            nextChar = newText.at(i + 1);
        if (currentChar.isSpace() && !nextChar.isSpace()) {
            newText.replace(i + 1, 1, nextChar.toUpper());
        }
    }

    return newText;
}

void TextEdit::keepCurrentLineAtCenter()
{
    QScrollBar *scrollbar = verticalScrollBar();

    int currentLine = cursorRect().top() / cursorRect().height();
    int halfEditorLines = rect().height() / 2 / cursorRect().height();
    scrollbar->setValue(scrollbar->value() + currentLine - halfEditorLines);
}

void TextEdit::scrollToLine(int scrollOffset, int row, int column)
{
    // Save cursor postion.
    m_restoreRow = row;
    m_restoreColumn = column;

    // Start scroll animation.
    m_scrollAnimation->setStartValue(verticalScrollBar()->value());
    m_scrollAnimation->setEndValue(scrollOffset);
    m_scrollAnimation->start();
}

void TextEdit::setLineWrapMode(bool enable)
{
    QTextCursor cursor = textCursor();
    int nJumpLine = textCursor().blockNumber() + 1;
    this->setWordWrapMode(QTextOption::WrapAnywhere);
    QPlainTextEdit::setLineWrapMode(enable ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pFlodArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();

    jumpToLine(nJumpLine, false);
    setTextCursor(cursor);
}

void TextEdit::setFontFamily(QString name)
{
    // Update font.
    m_fontName = name;
    updateFont();
    updateLeftAreaWidget();
}

void TextEdit::setFontSize(qreal size)
{
    // Update font.
    m_fontSize = size;
    updateFont();

    // Update line number after adjust font size.
    updateLeftAreaWidget();
}

void TextEdit::updateFont()
{
    QFont font = document()->defaultFont();
    font.setFixedPitch(true);
    font.setPointSizeF(m_fontSize);
    font.setFamily(m_fontName);
    setFont(font);
    setTabStopWidth(m_tabSpaceNumber * QFontMetrics(font).width(QChar(0x2192)));

    if (m_isSelectAll) {
        selectTextInView();
    }
}

void TextEdit::replaceAll(const QString &replaceText, const QString &withText, Qt::CaseSensitivity caseFlag)
{
    if (m_readOnlyMode || m_bReadOnlyPermission) {
        return;
    }

    if (replaceText.isEmpty()) {
        return;
    }

    // 替换文本相同，返回
    if (replaceText == withText) {
        return;
    }

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::Start);
    QTextCursor startCursor = textCursor();

    QString oldText = this->toPlainText();

    // 保存旧的标记索引光标记录信息，只需要更新其坐标偏移信息即可
    QList<TextEdit::MarkReplaceInfo> backupMarkList = convertMarkToReplace(m_markOperations);
    auto replaceList = backupMarkList;
    // 计算替换颜色标记信息
    calcMarkReplaceList(replaceList, oldText, replaceText, withText, 0, caseFlag);

    QString newText = oldText;
    newText.replace(replaceText, withText, caseFlag);

    if (oldText != newText) {
        ChangeMarkCommand *pChangeMark = new ChangeMarkCommand(this, backupMarkList, replaceList);
        // 设置替换撤销项为颜色标记变更撤销项的子项
        new ReplaceAllCommand(oldText, newText, cursor, pChangeMark);
        m_pUndoStack->push(pChangeMark);
    }
}

void TextEdit::replaceNext(const QString &replaceText, const QString &withText, Qt::CaseSensitivity caseFlag)
{
    if (m_readOnlyMode || m_bReadOnlyPermission) {
        return;
    }

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (replaceText.isEmpty() || !m_findHighlightSelection.cursor.hasSelection()) {
        //无限替换的根源
        return;
    }

    QTextCursor cursor = textCursor();

    if (m_cursorStart != -1) {
        cursor.setPosition(m_cursorStart);
        m_cursorStart = -1;
    } else {
        cursor.setPosition(m_findHighlightSelection.cursor.selectionStart());
    }
    cursor.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, replaceText.size());

    // 文本替换长度变更调整量
    int adjustlen = withText.size() - replaceText.size();
    // 保存旧的标记索引光标记录信息，只需要更新其坐标偏移信息即可
    QList<TextEdit::MarkReplaceInfo> backupMarkList = convertMarkToReplace(m_markOperations);
    auto replaceList = backupMarkList;
    for (auto &info : replaceList) {
        if (MarkAll == info.opt.type
                || MarkAllMatch == info.opt.type) {
            continue;
        }

        // 获取替换文本区域和颜色标记区域的交叉关系
        Utils::RegionIntersectType type = Utils::checkRegionIntersect(
                                              cursor.selectionStart(), cursor.selectionStart() + replaceText.size(), info.start, info.end);
        // 仅进行单次处理
        switch (type) {
        case Utils::ELeft:
            // 当前无交集，颜色标记在替换文本左侧，表示当前颜色标记已经经过
            break;
        case Utils::ERight: {
            // 颜色标记位于右侧
            info.start += adjustlen;
            info.end += adjustlen;
            break;
        }
        case Utils::EIntersectLeft: {
            // 交集在替换文本左侧，拓展颜色标记右侧到替换文本右侧
            info.end = cursor.selectionStart() + withText.size();
            break;
        }
        case Utils::EIntersectRight: {
            // 交集在替换文本右侧，拓展颜色标记左侧到替换文本左侧
            info.start = cursor.selectionStart();
            info.end += adjustlen;
            break;
        }
        case Utils::EIntersectOutter: {
            // 标记内容包含替换文本
            info.end += adjustlen;
            break;
        }
        case Utils::EIntersectInner: {
            // 替换文本内容包含标记信息, 取消当前文本标记（无论单个文本还是单行文本，均移除）
            // 在 manualUpdateAllMark() 函数处理会移除此标记
            info.start = 0;
            info.end = 0;
            break;
        }
        default:
            break;
        }
    }

    QString strSelection(cursor.selectedText());
    if (!strSelection.compare(replaceText, caseFlag) || replaceText.contains("\n")) {
        ChangeMarkCommand *pChangeMark = new ChangeMarkCommand(this, backupMarkList, replaceList);
        // 设置插入撤销项为颜色标记变更撤销项的子项
        new InsertTextUndoCommand(cursor, withText, this, pChangeMark);
        m_pUndoStack->push(pChangeMark);
        ensureCursorVisible();
    }

    // Update cursor.
    setTextCursor(cursor);
    highlightKeyword(replaceText, getPosition(), caseFlag);
}

void TextEdit::replaceRest(const QString &replaceText, const QString &withText, Qt::CaseSensitivity caseFlag)
{
    if (m_readOnlyMode || m_bReadOnlyPermission) {
        return;
    }

    // If replace text is nothing, don't do replace action.
    if (replaceText.isEmpty()) {
        return;
    }

    // 替换文本相同，返回
    if (replaceText == withText) {
        return;
    }

    QTextCursor cursor = textCursor();
    QTextCursor startCursor = textCursor();
    startCursor.beginEditBlock();

    int pos = cursor.position();
    QString oldText = this->toPlainText();
    QString newText = oldText.left(pos);
    QString right = oldText.right(oldText.size() - pos);

    // 保存旧的标记索引光标记录信息，只需要更新其坐标偏移信息即可
    QList<TextEdit::MarkReplaceInfo> backupMarkList = convertMarkToReplace(m_markOperations);
    auto replaceList = backupMarkList;
    // 计算替换颜色标记信息
    calcMarkReplaceList(replaceList, right, replaceText, withText, pos, caseFlag);

    right.replace(replaceText, withText, caseFlag);
    newText += right;

    if (oldText != newText) {
        ChangeMarkCommand *pChangeMark = new ChangeMarkCommand(this, backupMarkList, replaceList);
        // 设置替换撤销项为颜色标记变更撤销项的子项
        new ReplaceAllCommand(oldText, newText, cursor, pChangeMark);
        m_pUndoStack->push(pChangeMark);
    }

    startCursor.endEditBlock();
    setTextCursor(startCursor);
}

void TextEdit::beforeReplace(const QString &strReplaceText, Qt::CaseSensitivity caseFlag)
{
    if (strReplaceText.isEmpty() || !m_findHighlightSelection.cursor.hasSelection()) {
        highlightKeyword(strReplaceText, getPosition(), caseFlag);
    }
}

bool TextEdit::findKeywordForward(const QString &keyword)
{
    if (textCursor().hasSelection()) {
        // Get selection bound.
        int startPos = textCursor().anchor();
        int endPos = textCursor().position();

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        //setTextCursor(cursor);

        QTextDocument::FindFlags options;
        if (Qt::CaseSensitive == defaultCaseSensitive) {
            options |= QTextDocument::FindCaseSensitively;
        }
        bool foundOne = find(keyword, options);

        cursor.setPosition(endPos, QTextCursor::MoveAnchor);
        cursor.setPosition(startPos, QTextCursor::KeepAnchor);
        //setTextCursor(cursor);

        return foundOne;
    } else {
        QTextCursor recordCursor = textCursor();

        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        //setTextCursor(cursor);

        QTextDocument::FindFlags options;
        if (Qt::CaseSensitive == defaultCaseSensitive) {
            options |= QTextDocument::FindCaseSensitively;
        }
        bool foundOne = find(keyword, options);

        //setTextCursor(recordCursor);

        return foundOne;
    }
}

void TextEdit::removeKeywords()
{
    m_findHighlightSelection.cursor = textCursor();
    m_findHighlightSelection.cursor.clearSelection();

    m_findMatchSelections.clear();

    updateHighlightLineSelection();

    renderAllSelections();

    //setFocus();
}

bool TextEdit::highlightKeyword(const QString &keyword, int position, Qt::CaseSensitivity caseFlag)
{
    Q_UNUSED(position)
    m_findMatchSelections.clear();
    updateHighlightLineSelection();
    updateCursorKeywordSelection(keyword, true);
    bool bRet = updateKeywordSelectionsInView(keyword, m_findMatchFormat, &m_findMatchSelections, caseFlag);
    renderAllSelections();

    return bRet;
}

bool TextEdit::highlightKeywordInView(const QString &keyword, Qt::CaseSensitivity caseFlag)
{
    m_findMatchSelections.clear();
    bool bRet = updateKeywordSelectionsInView(keyword, m_findMatchFormat, &m_findMatchSelections, caseFlag);
    // 直接设置 setExtraSelections 会导致无法显示颜色标记，调用 renderAllSelections 进行显示更新
    // setExtraSelections(m_findMatchSelections);
    renderAllSelections();

    return bRet;
}

void TextEdit::clearFindMatchSelections()
{
    m_findMatchSelections.clear();
}

void TextEdit::updateCursorKeywordSelection(QString keyword, bool findNext)
{
    bool findOne = searchKeywordSeletion(keyword, textCursor(), findNext);

    if (!findOne) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(findNext ? QTextCursor::Start : QTextCursor::End, QTextCursor::MoveAnchor);
        if (!searchKeywordSeletion(keyword, cursor, findNext)) {
            m_findHighlightSelection.cursor = textCursor();
            m_findMatchSelections.clear();
            renderAllSelections();
        }
    }
}

void TextEdit::updateHighlightLineSelection()
{
    if (m_gestureAction == GA_slide) {
        QTextCursor textCursor = QPlainTextEdit::textCursor();
        return;
    }

    QTextEdit::ExtraSelection selection;

    selection.format.setBackground(m_currentLineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    m_currentLineSelection = selection;
}

bool TextEdit::updateKeywordSelections(QString keyword, QTextCharFormat charFormat, QList<QTextEdit::ExtraSelection> &listSelection)
{
    // Clear keyword selections first.
    listSelection.clear();

    // Update selections with keyword.
    if (!keyword.isEmpty()) {
        QTextCursor cursor(document());
        QTextDocument::FindFlags flags;
        if (Qt::CaseSensitive == defaultCaseSensitive) {
            flags |= QTextDocument::FindCaseSensitively;
        }
        QTextEdit::ExtraSelection extra;
        extra.format = charFormat;
        cursor = document()->find(keyword, cursor, flags);

        if (cursor.isNull()) {
            return false;
        }

        while (!cursor.isNull()) {
            extra.cursor = cursor;
            listSelection.append(extra);
            cursor = document()->find(keyword, cursor, flags);
        }

        return true;
    }

    return false;
}

bool TextEdit::updateKeywordSelectionsInView(QString keyword, QTextCharFormat charFormat,
                                             QList<QTextEdit::ExtraSelection> *listSelection, Qt::CaseSensitivity caseFlag)
{
    // Clear keyword selections first.
    listSelection->clear();

    // Update selections with keyword.
    if (!keyword.isEmpty()) {
        QTextCursor cursor(document());
        QTextEdit::ExtraSelection extra;
        extra.format = charFormat;

        QScrollBar *pScrollBar = verticalScrollBar();
        QPoint startPoint = QPointF(0, 0).toPoint();
        QTextBlock beginBlock = cursorForPosition(startPoint).block();
        int beginPos = beginBlock.position();
        QTextBlock endBlock;

        if (pScrollBar->maximum() > 0) {
            QPoint endPoint = QPointF(0, 1.5 * height()).toPoint();
            endBlock = cursorForPosition(endPoint).block();
        } else {
            endBlock = document()->lastBlock();
        }
        int endPos = endBlock.position() + endBlock.length() - 1;

        // 内部计算时，均视为 \n 结尾
        QLatin1Char endLine('\n');
        QString multiLineText;
        QTextDocument::FindFlags flags;
        if (Qt::CaseSensitive == caseFlag) {
            flags |= QTextDocument::FindCaseSensitively;
        }
        if (keyword.contains(endLine)) {
            auto temp = this->textCursor();
            temp.setPosition(beginPos);
            while (temp.position() < endPos) {
                temp.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
                multiLineText += temp.selectedText();
                multiLineText += endLine;
                temp.setPosition(temp.position() + 1);
            }
            cursor = findCursor(keyword, multiLineText, 0, false, beginPos, caseFlag);
        } else {
            cursor = document()->find(keyword, beginPos, flags);
        }

        if (cursor.isNull()) {
            return false;
        }

        while (!cursor.isNull()) {
            extra.cursor = cursor;
            // 调整为不区分大小写
            Qt::CaseSensitivity option = defaultCaseSensitive;
            /* 查找字符时，查找到完全相等的时候才高亮，如查找小写f时，大写的F不高亮 */
            if (!extra.cursor.selectedText().compare(keyword, option) || keyword.contains(endLine, option)) {
                listSelection->append(extra);
            }

            if (keyword.contains(endLine)) {
                int pos = std::max(extra.cursor.position(), extra.cursor.anchor());
                cursor = findCursor(keyword, multiLineText, pos - beginPos, false, beginPos, caseFlag);
            } else {
                cursor = document()->find(keyword, cursor, flags);
            }

            if (cursor.position() > endPos) {
                break;
            }
        }

        return true;
    }

    return false;
}

bool TextEdit::searchKeywordSeletion(QString keyword, QTextCursor cursor, bool findNext)
{
    if (keyword.isEmpty()) {
        return false;
    }

    bool ret = false;
    int offsetLines = 3;

    if (findNext) {
        QTextDocument::FindFlags options;
        if (Qt::CaseSensitive == defaultCaseSensitive) {
            options |= QTextDocument::FindCaseSensitively;
        }

        QTextCursor next = document()->find(keyword, cursor, options);
        if (keyword.contains("\n")) {
            int pos = std::max(cursor.position(), cursor.anchor());
            next = findCursor(keyword, this->toPlainText(), pos, false, 0, defaultCaseSensitive);
        }
        if (!next.isNull()) {
            m_findHighlightSelection.cursor = next;
            jumpToLine(next.blockNumber() + offsetLines, false);
            setTextCursor(next);
            ret = true;
        }
    } else {
        QTextDocument::FindFlags options = QTextDocument::FindBackward;
        if (Qt::CaseSensitive == defaultCaseSensitive) {
            options |= QTextDocument::FindCaseSensitively;
        }

        QTextCursor prev = document()->find(keyword, cursor, options);
        if (keyword.contains("\n")) {
            int pos = std::min(cursor.position(), cursor.anchor());
            prev = findCursor(keyword, this->toPlainText().mid(0, pos), -1, true, defaultCaseSensitive);
        }
        if (!prev.isNull()) {
            m_findHighlightSelection.cursor = prev;
            jumpToLine(prev.blockNumber() + offsetLines, false);
            setTextCursor(prev);
            ret = true;
        }
    }

    return ret;
}

void TextEdit::renderAllSelections()
{
    QList<QTextEdit::ExtraSelection> finalSelections;
    QList<QPair<QTextEdit::ExtraSelection, qint64>> selectionsSortList;

    // 标记当前行的浅灰色
    if (m_HightlightYes) {
        finalSelections.append(m_currentLineSelection);
    }
    // 此处代码无作用，去除
    // else {
    //     selections.clear();
    // }

    // 按代码逻辑，推测m_markAllSelection 用于还原所有文本的背景颜色, 实际不需要
    // 考虑到各平台运行之间可能存在运行差异，暂不清除
    // finalSelections.append(m_markAllSelection);

    // 选中区域的颜色标记，先加入 selectionsSortList
    // 通过时间戳升序排序后，加入到 finalSelections 中
    selectionsSortList.append(m_wordMarkSelections);

    // Find 和 Replace 高亮选中，移动到最后放入到 finalSelections 中
    // 保证此高亮状态，若存在，一定可以被用户看到
    // selections.append(m_findMatchSelections);
    // selections.append(m_findHighlightSelection);

    // 不再使用，注释掉
    // selections.append(m_wordUnderCursorSelection);

    // 标记括号移动到处理完颜色标记后，插入到 finalSelections 中
    // selections.append(m_beginBracketSelection);
    // selections.append(m_endBracketSelection);

    // 仅对代码文件有效，标记代码中的代码段，例如函数内{}所有内容
    // 移动到处理完颜色标记后，插入到 finalSelections 中，插入晚于括号
    // selections.append(m_markFoldHighLightSelections);

    // Alt选中区域的高亮, 移动到排序后面加入
    // selections.append(m_altModSelections);

    // 将颜色标记，标记所有的 selections 加入到 selectionsSortList 中， 后边将进行排序
    QMap<QString, QList<QPair<QTextEdit::ExtraSelection, qint64>>>::Iterator it;
    for (it = m_mapKeywordMarkSelections.begin(); it != m_mapKeywordMarkSelections.end(); ++it) {
        selectionsSortList.append(it.value());
    }

    // 通过时间戳重新排序颜色标记功能的 selections
    qSort(selectionsSortList.begin(), selectionsSortList.end(), [](const QPair<QTextEdit::ExtraSelection, qint64> &A, const QPair<QTextEdit::ExtraSelection, qint64> &B) {
        return (A.second < B.second);
    });

    // 将排序后的颜色标记 selections 加入到 finalSelections 中
    for (int i = 0; i < selectionsSortList.size(); i++) {
        finalSelections.append(selectionsSortList.at(i).first);
    }

    // 标记括号
    finalSelections.append(m_beginBracketSelection);
    finalSelections.append(m_endBracketSelection);

    // 标记代码段
    finalSelections.append(m_markFoldHighLightSelections);

    // Alt选中区域的高亮
    finalSelections.append(m_altModSelections);

    // 查找替换的高亮需要放在最后
    // Find 高亮
    finalSelections.append(m_findMatchSelections);
    // Replace 高亮
    finalSelections.append(m_findHighlightSelection);

    // 设置到 QPlainText 中进行渲染
    setExtraSelections(finalSelections);
}

void TextEdit::updateMarkAllSelectColor()
{
    isMarkAllLine(m_bIsMarkAllLine, m_strMarkAllLineColorName);
    renderAllSelections();
}

DMenu *TextEdit::getHighlightMenu()
{
    return m_hlGroupMenu;
}

void TextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_pLeftAreaWidget->m_pLineNumberArea);
    QColor lineNumberAreaBackgroundColor;

    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::ColorType::DarkType) {
        lineNumberAreaBackgroundColor = palette().brightText().color();
        lineNumberAreaBackgroundColor.setAlphaF(0.06);

        m_lineNumbersColor.setAlphaF(0.2);
    } else {
        lineNumberAreaBackgroundColor = palette().brightText().color();
        lineNumberAreaBackgroundColor.setAlphaF(0.03);
        m_lineNumbersColor.setAlphaF(0.3);
    }
    //painter.fillRect(event->rect(), lineNumberAreaBackgroundColor);

    int blockNumber = getFirstVisibleBlockId();
    QTextBlock block = document()->findBlockByNumber(blockNumber);

    int top = this->viewport()->geometry().top() + verticalScrollBar()->value();
    int bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());

    Utils::setFontSize(painter, document()->defaultFont().pointSize() - 2);
    QPoint endPoint;

    if (verticalScrollBar()->maximum() > 0) {
        endPoint = QPointF(0, height() + height() / verticalScrollBar()->maximum() * verticalScrollBar()->value()).toPoint();
    }

    QTextCursor cur = cursorForPosition(endPoint);
    QTextBlock endBlock = cur.block();
    int nPageLine = endBlock.blockNumber();
    int nStartLine = block.blockNumber();

    if (verticalScrollBar()->maximum() == 0) {
        nPageLine = blockCount() - 1;
    }

    auto currentCursor = this->textCursor();
    cur = textCursor();
    for (int i = nStartLine; i <= nPageLine; i++) {
        if (i + 1 == m_markStartLine) {
            painter.setPen(m_regionMarkerColor);
        } else {
            painter.setPen(m_lineNumbersColor);
        }

        m_fontLineNumberArea.setPointSize(font().pointSize() - 1);
        painter.setFont(m_fontLineNumberArea);

        cur.setPosition(block.position(), QTextCursor::MoveAnchor);

        if (block.isVisible()) {
            int w = this->m_fontSize <= 15 ? 15 : m_fontSize;
            updateLeftWidgetWidth(w);
            int offset = 0;
            //the language currently set by the system is Tibetan.
            if ("bo_CN" == Utils::getSystemLan()) {
                offset = 2;
            }
            if (cur.blockNumber() == currentCursor.blockNumber()) {
                painter.setPen(qApp->palette().highlight().color());
            }
            painter.drawText(0, cursorRect(cur).y() + offset,
                             m_pLeftAreaWidget->m_pLineNumberArea->width(), cursorRect(cur).height(),
                             Qt::AlignVCenter | Qt::AlignHCenter, QString::number(block.blockNumber() + 1));
        }

        block = block.next();
        top = bottom/* + document()->documentMargin()*/;
        bottom = top + static_cast<int>(document()->documentLayout()->blockBoundingRect(block).height());
    }
}

void TextEdit::codeFLodAreaPaintEvent(QPaintEvent *event)
{
    m_listFlodIconPos.clear();
    QPainter painter(m_pLeftAreaWidget->m_pFlodArea);

    QColor codeFlodAreaBackgroundColor;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::ColorType::DarkType) {
        codeFlodAreaBackgroundColor = palette().brightText().color();
        codeFlodAreaBackgroundColor.setAlphaF(0.06);

        m_lineNumbersColor.setAlphaF(0.2);
    } else {
        codeFlodAreaBackgroundColor = palette().brightText().color();
        codeFlodAreaBackgroundColor.setAlphaF(0.03);
        m_lineNumbersColor.setAlphaF(0.3);
    }

    int blockNumber = getFirstVisibleBlockId();
    QTextBlock block = document()->findBlockByNumber(blockNumber);

    DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
    QString theme  = "";

    if (guiAppHelp->themeType() == DGuiApplicationHelper::ColorType::DarkType) {  //暗色主题
        theme = "d";
    } else {  //浅色主题
        theme = "l";
    }

    // QString flodImagePath = QString(":/images/d-%1.svg").arg(theme);
    //QString unflodImagePath = QString(":/images/u-%1.svg").arg(theme);

    QPoint endPoint;

    if (verticalScrollBar()->maximum() > 0) {
        endPoint = QPointF(0, height() + height() / verticalScrollBar()->maximum() * verticalScrollBar()->value()).toPoint();
    }

    QTextCursor cur = cursorForPosition(endPoint);
    QTextBlock endBlock = cur.block();
    int nPageLine = endBlock.blockNumber();

    if (verticalScrollBar()->maximum() == 0) {
        nPageLine = blockCount() - 1;
    }

    cur = textCursor();

    for (int iBlockCount = blockNumber ; iBlockCount <= nPageLine; ++iBlockCount) {
        if (block.isVisible()) {
            //判定是否包含注释代码左括号、是否整行是注释，isNeedShowFoldIcon该函数是为了做判定当前行是否包含成对的括号，如果包括，则不显示折叠标志

            //获取行数文本块 出去字符串判断　梁卫东２０２０年０９月０１日１７：１６：１７
            QString text = block.text();
            //若存在字符串行，多个字符串中间的 '{' '}' 同样被忽略
            QRegExp regExp("\".*\"");
            QString curText = text.remove(regExp);

            //不同类型文件注释符号不同 梁卫东　２０２０－０９－０３　１７：２８：４５
            bool bHasCommnent = false;
            QString multiLineCommentMark;
            QString singleLineCommentMark;

            if (m_commentDefinition.isValid()) {
                multiLineCommentMark = m_commentDefinition.multiLineStart.trimmed();
                singleLineCommentMark = m_commentDefinition.singleLine.trimmed();
                //判断是否包含单行或多行注释
                if (!multiLineCommentMark.isEmpty()) bHasCommnent = block.text().trimmed().startsWith(multiLineCommentMark);
                if (!singleLineCommentMark.isEmpty()) bHasCommnent = block.text().trimmed().startsWith(singleLineCommentMark);
            } else {
                bHasCommnent = false;
            }

            //添加注释判断 存在不显示折叠标志　不存在显示折叠标准　梁卫东　２０２０年０９月０３日１７：２８：５０
            if (curText.contains("{") && isNeedShowFoldIcon(block) && !bHasCommnent) {

                cur.setPosition(block.position(), QTextCursor::MoveAnchor);

                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setRenderHints(QPainter::SmoothPixmapTransform);

                int w = this->m_fontSize <= 15 ? 15 : m_fontSize;
                updateLeftWidgetWidth(w);
                int h = cursorRect(cur).height();
                // 绘制行纵向居中
                int offset = qMax(0, (h - w) / 2);
                //the language currently set by the system is Tibetan.
                if ("bo_CN" == Utils::getSystemLan())
                    offset = h <= 20 ? 0 : h / 10;

                QRect rect(0, cursorRect(cur).y() + offset, w, w);
                if (block.next().isVisible()) {
                    if (block.isVisible()) {
                        paintCodeFlod(&painter, rect);
                    }
                } else {
                    if (block.isVisible()) {
                        paintCodeFlod(&painter, rect, true);
                    }
                }
                m_listFlodIconPos.append(block.blockNumber());
            }
        }

        block = block.next();
    }
}

void TextEdit::setBookmarkFlagVisable(bool isVisable, bool bIsFirstOpen)
{
    int w = this->m_fontSize <= 15 ? 15 : m_fontSize;
    updateLeftWidgetWidth(w);

    m_pIsShowBookmarkArea = isVisable;
    m_pLeftAreaWidget->m_pBookMarkArea->setVisible(isVisable);
}

void TextEdit::setCodeFlodFlagVisable(bool isVisable, bool bIsFirstOpen)
{
    int w = this->m_fontSize <= 15 ? 15 : m_fontSize;
    updateLeftWidgetWidth(w);

    m_pIsShowCodeFoldArea = isVisable;
    m_pLeftAreaWidget->m_pFlodArea->setVisible(isVisable);
}

void TextEdit::setHighLineCurrentLine(bool ok)
{
    m_HightlightYes = ok;
    renderAllSelections();
}

void TextEdit::updateLeftAreaWidget()
{
#if 0
// not used anymore
    int blockSize = QString::number(blockCount()).size();
    int leftAreaWidth = 0;

    //跟新左边框宽度
    if (m_pIsShowBookmarkArea) {
        leftAreaWidth += m_pLeftAreaWidget->m_pBookMarkArea->width();
    }
    if (m_pIsShowCodeFoldArea) {
        leftAreaWidth += m_pLeftAreaWidget->m_pFlodArea->width();
    }

    if (bIsSetLineNumberWidth) {
        leftAreaWidth += blockSize * fontMetrics().width('9') + 5;
    }
    // m_pLeftAreaWidget->setFixedWidth(leftAreaWidth);
#endif
    m_pLeftAreaWidget->updateAll();
}


void TextEdit::handleScrollFinish()
{
    // Restore cursor postion.
    jumpToLine(m_restoreRow, false);

    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, m_restoreColumn);

    // Update cursor.
    setTextCursor(cursor);
}

void TextEdit::setSyntaxDefinition(KSyntaxHighlighting::Definition def)
{
    m_commentDefinition.setComments(def.singleLineCommentMarker(), def.multiLineCommentMarker().first,  def.multiLineCommentMarker().second);
}

bool TextEdit::setCursorKeywordSeletoin(int position, bool findNext)
{
    int offsetLines = 3;

    if (findNext) {
        for (int i = 0; i < m_findMatchSelections.size(); i++) {
            if (m_findMatchSelections[i].cursor.position() > position) {
                m_findHighlightSelection.cursor = m_findMatchSelections[i].cursor;

                jumpToLine(m_findMatchSelections[i].cursor.blockNumber() + offsetLines, false);

                QTextCursor cursor = textCursor();
                cursor.setPosition(m_findMatchSelections[i].cursor.position());

                // Update cursor.
                setTextCursor(cursor);

                return true;
            }
        }
    } else {
        for (int i = m_findMatchSelections.size() - 1; i >= 0; i--) {
            if (m_findMatchSelections[i].cursor.position() < position) {
                m_findHighlightSelection.cursor = m_findMatchSelections[i].cursor;

                jumpToLine(m_findMatchSelections[i].cursor.blockNumber() + offsetLines, false);

                QTextCursor cursor = textCursor();
                cursor.setPosition(m_findMatchSelections[i].cursor.position());

                // Update cursor.
                setTextCursor(cursor);
                return true;
            }
        }
    }

    return false;
}

void TextEdit::cursorPositionChanged()
{
    // 以赋值形式，清空 Bracket 括号的selection
    // m_beginBracketSelection 和 m_endBracketSelection 将在 updateHighlightBrackets 重新设置
    m_beginBracketSelection = QTextEdit::ExtraSelection();
    m_endBracketSelection = QTextEdit::ExtraSelection();

    updateHighlightLineSelection();
    updateHighlightBrackets('(', ')');
    updateHighlightBrackets('{', '}');
    updateHighlightBrackets('[', ']');
    renderAllSelections();

    QTextCursor cursor = textCursor();
    if (m_wrapper) {
        m_wrapper->bottomBar()->updatePosition(cursor.blockNumber() + 1,
                                               cursor.positionInBlock() + 1);
    }

    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();
    m_pLeftAreaWidget->m_pFlodArea->update();
}

/**
 * @brief 剪切光标选中的文本
 * @param ignoreCheck 是否忽略权限判断(外部已进行)，默认false
 */
void TextEdit::cut(bool ignoreCheck)
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!ignoreCheck && !Utils::enableClipCopy(getFilePath())) {
        return;
    }

    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    //列编辑添加撤销重做
    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
        QString data;
        for (auto it = m_altModSelections.begin(); it != m_altModSelections.end(); it++) {
            auto text = (*it).cursor.selectedText();
            data += text ;
            if (it != m_altModSelections.end() - 1)
                data += "\n";
        }
        //删除有选择
                QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(m_altModSelections, this);
                m_pUndoStack->push(pDeleteStack);
        
        //设置到剪切板
        QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
        clipboard->setText(data);
    } else {
        QTextCursor cursor = textCursor();
        //有选择内容才剪切
        if (cursor.hasSelection()) {
            QString data = this->selectedText(true);
            QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(cursor, this);
            m_pUndoStack->push(pDeleteStack);
            QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
            clipboard->setText(data);
        }
    }
    unsetMark();
}

/**
 * @brief 拷贝光标选中的文本
 * @param ignoreCheck 是否忽略权限判断(外部已进行)，默认false
 */
void TextEdit::copy(bool ignoreCheck)
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!ignoreCheck && !Utils::enableClipCopy(getFilePath())) {
        return;
    }

    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
        QString data;
        for (auto it = m_altModSelections.begin(); it != m_altModSelections.end(); it++) {
            auto text = (*it).cursor.selectedText();
            data += text ;
            if (it != m_altModSelections.end() - 1)
                data += "\n";
        }
        QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
        clipboard->setText(data);
        tryUnsetMark();
    } else {
        if (!m_isSelectAll) {
            QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
            if (textCursor().hasSelection()) {
                //clipboard->setText(textCursor().selection().toPlainText());
                clipboard->setText(this->selectedText(true));
                tryUnsetMark();
            } else {
                clipboard->setText(m_highlightWordCacheCursor.selectedText());
            }
        } else {
            QClipboard *clipboard = QApplication::clipboard();
            QString text = this->toPlainText();
            clipboard->setText(text);
        }
    }
}

void TextEdit::paste()
{
#if 0
    //2021-05-25:为解决大文本粘贴卡顿而注释重写
    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    const QClipboard *clipboard = QApplication::clipboard(); //获取剪切版内容
    auto text = clipboard->text();
    if (text.isEmpty())
        return;
    if (!m_bIsAltMod) {
        QTextCursor cursor = textCursor();
        insertSelectTextEx(cursor, text);
        unsetMark();
    } else {
        insertColumnEditTextEx(text);
    }

#endif


    //大文件粘贴-采用分块插入
    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    const QClipboard *clipboard = QApplication::clipboard(); //获取剪切版内容
    auto text = clipboard->text();

    if (text.isEmpty())
        return;
    if (!m_bIsAltMod) {
        int block = 1 * 1024 * 1024;
        int size = text.size();
        if (size > block) {
            InsertBlockByTextCommand *commond = new InsertBlockByTextCommand(text, this, m_wrapper);
            m_pUndoStack->push(commond);
        } else {
            QTextCursor cursor = textCursor();
            insertSelectTextEx(cursor, text);
            unsetMark();
        }
    } else {
        insertColumnEditTextEx(text);
    }

    m_isSelectAll = false;
}

void TextEdit::highlight()
{
    QTimer::singleShot(0, this, [&]() {
        if (nullptr != m_wrapper)
            m_wrapper->OnUpdateHighlighter();
    });
}

void TextEdit::selectTextInView()
{
    int startPos = cursorForPosition(QPoint(0, 0)).position();
    QPoint endPoint = QPoint(this->viewport()->width(), this->viewport()->height());
    int endPos = cursorForPosition(endPoint).position();

    QTextCursor cursor = this->textCursor();
    cursor.setPosition(endPos);
    cursor.setPosition(startPos, QTextCursor::KeepAnchor);
    this->setTextCursor(cursor);
    this->horizontalScrollBar()->setValue(0);
}

void TextEdit::setSelectAll()
{
    if (m_wrapper->getFileLoading())
        return;

    m_bIsAltMod = false;
    m_isSelectAll = true;
    selectTextInView();
}

void TextEdit::slotSigColorSelected(bool bSelected, QColor color)
{
    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }
    isMarkCurrentLine(bSelected, color.name());
    renderAllSelections();
    m_colorMarkMenu->close();
    m_rightMenu->close(); //选择颜色关闭菜单　梁卫东　２０２０－０８－２１　０９：３４：５３
}

void TextEdit::slotSigColorAllSelected(bool bSelected, QColor color)
{
    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }
    isMarkAllLine(bSelected, color.name());
    renderAllSelections();
    m_colorMarkMenu->close();
    m_rightMenu->close(); //选择颜色关闭菜单　梁卫东　２０２０－０８－２１　０９：３４：５３
}

void TextEdit::slotCutAction(bool checked)
{
    Q_UNUSED(checked);
    this->cut();
}

void TextEdit::slotCopyAction(bool checked)
{
    Q_UNUSED(checked);

    if (isAbleOperation(OperationType::CopyOperation)) {
        copy();
    } else {
#ifdef DTKWIDGET_CLASS_DSizeMode
        Utils::sendFloatMessageFixedFont(this, QIcon(":/images/warning.svg"), tr("Copy failed: not enough memory"));
#else
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/warning.svg"), tr("Copy failed: not enough memory"));
#endif
    }
}

void TextEdit::slotPasteAction(bool checked)
{
    Q_UNUSED(checked);
    if (isAbleOperation(OperationType::PasteOperation)) {
        paste();
    } else {
#ifdef DTKWIDGET_CLASS_DSizeMode
        Utils::sendFloatMessageFixedFont(this, QIcon(":/images/warning.svg"), tr("Paste failed: not enough memory"));
#else
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/warning.svg"), tr("Paste failed: not enough memory"));
#endif
    }
}

void TextEdit::slotDeleteAction(bool checked)
{
    Q_UNUSED(checked);
    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
        for (int i = 0; i < m_altModSelections.size(); i++) {
            if (m_altModSelections[i].cursor.hasSelection()) {
                QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(m_altModSelections[i].cursor, this);
                m_pUndoStack->push(pDeleteStack);
            }
        }
    } else {
        if (textCursor().hasSelection()) {
            QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(textCursor(), this);
            m_pUndoStack->push(pDeleteStack);
        } else {
            setTextCursor(m_highlightWordCacheCursor);
        }
    }
}

void TextEdit::slotSelectAllAction(bool checked)
{
    Q_UNUSED(checked);
    setSelectAll();
}

bool TextEdit::slotOpenInFileManagerAction(bool checked)
{
    Q_UNUSED(checked);
    return DDesktopServices::showFileItem(this->getTruePath());
}

void TextEdit::slotAddComment(bool checked)
{
    Q_UNUSED(checked);
    toggleComment(true);
}

void TextEdit::slotCancelComment(bool checked)
{
    Q_UNUSED(checked);
    toggleComment(false);
}

void TextEdit::slotVoiceReadingAction(bool checked)
{
    Q_UNUSED(checked);
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.TextToSpeech");
    emit signal_readingPath();
}

bool TextEdit::slotStopReadingAction(bool checked)
{
    Q_UNUSED(checked);
    return QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/tts com.iflytek.aiassistant.tts.stopTTSDirectly");
}

void TextEdit::slotdictationAction(bool checked)
{
    Q_UNUSED(checked);
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.SpeechToText");
}

void TextEdit::slotColumnEditAction(bool checked)
{
    Q_UNUSED(checked);
#ifdef DTKWIDGET_CLASS_DSizeMode
    Utils::sendFloatMessageFixedFont(this, QIcon(":/images/ok.svg"), tr("Press ALT and click lines to edit in column mode"));
#else
    DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), tr("Press ALT and click lines to edit in column mode"));
#endif
}

void TextEdit::slotPreBookMarkAction(bool checked)
{
    Q_UNUSED(checked);
    int line = getLineFromPoint(m_mouseClickPos);
    int index = m_listBookmark.indexOf(line);

    if (index == 0) {
        jumpToLine(m_listBookmark.last(), true);
    } else {
        jumpToLine(m_listBookmark.value(index - 1), true);
    }
}

void TextEdit::slotNextBookMarkAction(bool checked)
{
    Q_UNUSED(checked);
    int line = getLineFromPoint(m_mouseClickPos);
    int index = m_listBookmark.indexOf(line);

    if (index == -1 && !m_listBookmark.isEmpty()) {
        jumpToLine(m_listBookmark.last(), false);
    }

    if (index == m_listBookmark.count() - 1) {
        jumpToLine(m_listBookmark.first(), false);
    } else {
        jumpToLine(m_listBookmark.value(index + 1), false);
    }
}

void TextEdit::slotClearBookMarkAction(bool checked)
{
    Q_UNUSED(checked);
    m_listBookmark.clear();
    qDebug() << "ClearBookMark:" << m_listBookmark;
    m_pLeftAreaWidget->m_pBookMarkArea->update();
}

void TextEdit::slotFlodAllLevel(bool checked)
{
    Q_UNUSED(checked);
    flodOrUnflodAllLevel(true);
}

void TextEdit::slotUnflodAllLevel(bool checked)
{
    Q_UNUSED(checked);
    flodOrUnflodAllLevel(false);
}

void TextEdit::slotFlodCurrentLevel(bool checked)
{
    Q_UNUSED(checked);
    flodOrUnflodCurrentLevel(true);
}

void TextEdit::slotUnflodCurrentLevel(bool checked)
{
    Q_UNUSED(checked);
    flodOrUnflodCurrentLevel(false);
}

void TextEdit::slotCancleMarkAllLine(bool checked)
{
    Q_UNUSED(checked);
    isMarkAllLine(false);
}

void TextEdit::slotCancleLastMark(bool checked)
{
    Q_UNUSED(checked);
    cancelLastMark();
}

void TextEdit::slotUndoAvailable(bool undoIsAvailable)
{
    m_canUndo = undoIsAvailable;
}

void TextEdit::slotRedoAvailable(bool redoIsAvailable)
{
    m_canRedo = redoIsAvailable;
}

void TextEdit::redo_()
{
    if (!m_pUndoStack->canRedo()) {
        return;
    }

    m_pUndoStack->redo();
    if (m_pUndoStack->index() == m_lastSaveIndex) {
        this->m_wrapper->window()->updateModifyStatus(m_sFilePath, false);
        this->m_wrapper->setTemFile(false);
        this->document()->setModified(false);
    }

}
void TextEdit::undo_()
{
    if (!m_pUndoStack->canUndo()) {
        return;
    }

    m_pUndoStack->undo();

    // 对撤销栈清空的情况下，有两种文件仍需保留*号(重做无需如下判定)
    // 1. 备份文件，上次修改之后直接关闭时备份的文件，仍需要提示保存
    // 2. 临时文件，上次修改后关闭，撤销操作后文件内容不为空
    if (m_pUndoStack->index() == m_lastSaveIndex
            && !m_wrapper->isBackupFile()
            && !(m_wrapper->isDraftFile() && !m_wrapper->isPlainTextEmpty())) {
        this->m_wrapper->window()->updateModifyStatus(m_sFilePath, false);
        this->m_wrapper->setTemFile(false);
        this->document()->setModified(false);
    }
}

/*!
   \brief 从索引位置 \a from 移动文本内容 \a text 到 \a to, 当仅拷贝文本时，
        \a copy 置为true.
   \param from 移动/拷贝文本位置
   \param to 目的文本位置
   \param text 文本内容
   \param copy 是否为拷贝文本，默认为移动
 */
void TextEdit::moveText(int from, int to, const QString &text, bool copy)
{
    auto cursor = this->textCursor();
    auto list = new UndoList;
    cursor.setPosition(from);
    QUndoCommand *delCommand = nullptr;
    // 拷贝模式下无需删除文本
    if (!copy) {
        cursor.setPosition(from + text.size(), QTextCursor::KeepAnchor);
        delCommand = new DeleteBackCommand(cursor, this);
    }

    cursor.setPosition(to);
    auto insertCommand = new DragInsertTextUndoCommand(cursor, text, this);

    //the positon of 'from' is on the left of the position of 'to',
    //therefore,firstly do the insert operation.
    if (from < to) {
        list->appendCom(insertCommand);
        if (!copy) {
            list->appendCom(delCommand);
        }
        m_pUndoStack->push(list);
    } else if (from > to) {
        if (!copy) {
            list->appendCom(delCommand);
        }
        list->appendCom(insertCommand);
        m_pUndoStack->push(list);
    }
}

/**
 * @brief 在text中查找substr,并返回QTextCursor
 * @param substr:需要查找的串
 *        text:源字符串
 *        from:从text的from位置开始查找
 *        backword:是否反向查找
 *        cursorPos: text起始位置在全文本中的位置
 * @return
 */
QTextCursor TextEdit::findCursor(const QString &substr, const QString &text, int from, bool backward,
                                 int cursorPos, Qt::CaseSensitivity caseFlag)
{
    // 处理换行符为 \r\n (光标计算时被视为单个字符)的情况，移除多计算的字符数
    // text 均为 \n 结尾
    QString findSubStr = substr;
    if (BottomBar::Windows == m_wrapper->bottomBar()->getEndlineFormat()) {
        findSubStr.replace("\r\n", "\n");
    }

    int index = -1;
    if (backward) {
        index = text.lastIndexOf(findSubStr, from, caseFlag);
    } else {
        index = text.indexOf(findSubStr, from, caseFlag);
    }
    if (-1 != index) {
        auto cursor = this->textCursor();
        cursor.setPosition(index + cursorPos);
        cursor.setPosition(cursor.position() + findSubStr.size(), QTextCursor::KeepAnchor);
        return cursor;
    } else {
        return QTextCursor();
    }
}

/**
 * @brief 点击行号处理：选中当前行，光标置于下一行行首
   @param point 当前鼠标点击的位置
   @note 在 "bo_CN" 语言环境下，光标向上偏移2像素进行比较。
 */
void TextEdit::onPressedLineNumber(const QPoint &point)
{
    // 在执行大文件加载过程中，由于 cursorForPosition() 计算耗时，不响应点击处理
    if (TextEdit::FileOpenBegin == m_LeftAreaUpdateState) {
        return;
    }

    if (point.x() < 0 || point.x() > m_pLeftAreaWidget->m_pLineNumberArea->width()) {
        return;
    }

    int offset = 0;
    //the language currently set by the system is Tibetan.
    if ("bo_CN" == Utils::getSystemLan()) {
        offset = 2;
    }

    QPoint checkPoint(point.x(), qMax(0, point.y() - offset));
    QTextBlock possibleBlock = cursorForPosition(checkPoint).block();

    if (possibleBlock.isValid()) {
        QTextCursor cursor = textCursor();
        cursor.setPosition(possibleBlock.position(), QTextCursor::MoveAnchor);

        // 尾行光标不调整到下一行
        if (possibleBlock == document()->lastBlock()) {
            cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        } else {
            cursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
        }

        setTextCursor(cursor);
    }
}

/**
 * @return 返回当前光标选中的内容
 * @note 如果从编辑器获得的选中文本跨越换行符，则文本将包含 Unicode U+2029 段落分隔符而不是换行符 \n 字符。
 *      可使用 QString::replace() 将这些字符替换为换行符，为避免文本原有 Unicode U+2029 分割符影响，
 *      手动调整换行符插入位置。
 */
QString TextEdit::selectedText(bool checkCRLF)
{
    auto cursor = this->textCursor();
    auto temp = cursor;
    int startpos = std::min(cursor.anchor(), cursor.position());
    int endpos = std::max(cursor.anchor(), cursor.position());
    int startblock = 0, endblock = 0;
    cursor.setPosition(startpos);
    startblock = cursor.blockNumber();
    cursor.setPosition(endpos);
    endblock = cursor.blockNumber();
    // 仅单个文本块，直接提取文本数据
    if (startblock == endblock) {
        return temp.selectedText();
    }

    QString text;
    cursor.setPosition(startpos);
    // 取得首个文本块内容
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    text += cursor.selectedText();
    cursor.setPosition(cursor.position() + 1);

    // 不同风格使用不同换行符，默认不替换，仅在剪贴板、保存文件等时设置
    QString endLine = "\n";
    if (checkCRLF && BottomBar::Windows == m_wrapper->bottomBar()->getEndlineFormat()) {
        endLine = "\r\n";
    }

    // 获取中间完整文本块内容
    while ((cursor.position() + cursor.block().length()) < endpos) {
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        text += endLine;
        text += cursor.selectedText();
        cursor.setPosition(cursor.position() + 1);
    }

    // 取得尾部文本块内容
    if (cursor.position() < endpos) {
        // 判断是否尾部文本块达到当前文本块末尾，到达末尾需要将 U+2029 替换为 \n
        bool needAdjustNewline = bool(cursor.position() + cursor.block().length() == endpos);
        cursor.setPosition(endpos, QTextCursor::KeepAnchor);
        text += endLine;
        text += cursor.selectedText();

        if (needAdjustNewline && text.endsWith("\u2029")) {
            text.replace("\u2029", endLine);
        }
    }

    return text;
}

/**
 * @brief 行尾符号改变后处理函数；
 * 1.这里并没有做"\n"和"\r\n"之间的替换，实际的替换动作在保存的时候才发生；
 * @param
 * @return
 */

void TextEdit::onEndlineFormatChanged(BottomBar::EndlineFormat from, BottomBar::EndlineFormat to)
{
    //auto endlineCom = new EndlineFormartCommand(this,m_wrapper->bottomBar(),from,to);
    //m_pUndoStack->push(endlineCom);
    m_wrapper->bottomBar()->setEndlineMenuText(to);
}

/**
 * @brief 系统调色板更新时重绘部分组件，例如：列选取项
 */
void TextEdit::onAppPaletteChanged()
{
    // 判断是否处于列选取状态
    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
        QColor highlightBackground = DGuiApplicationHelper::instance()->applicationPalette().color(QPalette::Highlight);
        for (auto &selection : m_altModSelections) {
            selection.format.setBackground(highlightBackground);
        }
        // 更新高亮状态
        renderAllSelections();
    }
}

void TextEdit::updateHighlightBrackets(const QChar &openChar, const QChar &closeChar)
{
    QTextDocument *doc = document();
    QTextCursor cursor = textCursor();
    int position = cursor.position();

    QTextCursor bracketBeginCursor;
    QTextCursor bracketEndCursor;
    cursor.clearSelection();

    if (!bracketBeginCursor.isNull() || !bracketEndCursor.isNull()) {
        bracketBeginCursor.setCharFormat(QTextCharFormat());
        bracketEndCursor.setCharFormat(QTextCharFormat());
        bracketBeginCursor = bracketEndCursor = QTextCursor();
    }

    QChar begin, end;

    if (doc->characterAt(position) == openChar ||
            doc->characterAt(position) == closeChar ||
            doc->characterAt(position - 1) == openChar ||
            doc->characterAt(position - 1) == closeChar) {
        bool forward = doc->characterAt(position) == openChar ||
                       doc->characterAt(position - 1) == openChar;

        if (forward) {
            if (doc->characterAt(position) == openChar) {
                position++;
            } else {
                cursor.setPosition(position - 1);
            }

            begin = openChar;
            end = closeChar;
        } else {
            if (doc->characterAt(position) == closeChar) {
                cursor.setPosition(position + 1);
                position -= 1;
            } else {
                position -= 2;
            }

            begin = closeChar;
            end = openChar;
        }

        bracketBeginCursor = cursor;
        bracketBeginCursor.movePosition(forward ? QTextCursor::NextCharacter : QTextCursor::PreviousCharacter,
                                        QTextCursor::KeepAnchor);

        int braceDepth = 1;
        // 判断当前是否处于字符串内，字符串内的 openChar closeChar  {} () [] 等不进行统计
        bool inCodeString = false;
        QChar c;
        while (!(c = doc->characterAt(position)).isNull()) {
            if (inCodeString) {
                // 判断 " 是否存在转义字符，若不存在，则退出字符串模式
                // 注意回退(!forward)模式下，position = 0，characterAt(position - 1) 将返回空字符，同样符合判断，不会触发越界访问
                if ('"' == c
                        && '\\' != doc->characterAt(position - 1)) {
                    inCodeString = false;
                }
            } else {
                if (c == begin) {
                    braceDepth++;
                } else if (c == end) {
                    braceDepth--;

                    if (!braceDepth) {
                        bracketEndCursor = QTextCursor(doc);
                        bracketEndCursor.setPosition(position);
                        bracketEndCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                        break;
                    }
                } else if ('"' == c) {
                    inCodeString = true;
                }
            }

            forward ? position++ : position--;
        }

        // cannot find the end bracket to not need to highlight.
        if (!bracketEndCursor.isNull()) {
            m_beginBracketSelection.cursor = bracketBeginCursor;
            m_beginBracketSelection.format = m_bracketMatchFormat;

            m_endBracketSelection.cursor = bracketEndCursor;
            m_endBracketSelection.format = m_bracketMatchFormat;
        }
    }
}

int TextEdit::getFirstVisibleBlockId() const
{
    QTextCursor cur = QTextCursor(this->document());
    if (cur.isNull()) {
        return 0;
    }
    cur.movePosition(QTextCursor::Start);

    QPoint startPoint;
    QTextBlock startBlock, endBlock;

    if (verticalScrollBar()->maximum() > height()) {
        startPoint = QPointF(0, height() / verticalScrollBar()->maximum() * verticalScrollBar()->value()).toPoint();
        //endPoint = QPointF(0,height() + height()/verticalScrollBar()->maximum()*verticalScrollBar()->value()).toPoint();
    } else if (verticalScrollBar()->maximum() > 0 && verticalScrollBar()->maximum() <= height()) {

        startPoint = QPointF(0, verticalScrollBar()->value() / verticalScrollBar()->maximum()).toPoint();
    }

    cur = cursorForPosition(startPoint);
    startBlock = document()->findBlock(cur.position());
    cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    if (startBlock.text() != cur.selection().toPlainText()) {
        return startBlock.blockNumber() + 1;
    }

    return startBlock.blockNumber();
}

void TextEdit::setLeftAreaUpdateState(TextEdit::UpdateOperationType statevalue)
{
    if (statevalue != m_LeftAreaUpdateState) {
        m_LeftAreaUpdateState = statevalue;

        // 当文件读取完成时，手动触发更新界面
        if (TextEdit::FileOpenEnd == m_LeftAreaUpdateState) {
            m_pLeftAreaWidget->updateAll();
        }
    }
}

TextEdit::UpdateOperationType TextEdit::getLeftAreaUpdateState()
{
    return m_LeftAreaUpdateState;
}
//line 开始处理的行号  isvisable是否折叠  iInitnum左括号默认开始计算的数量  isFirstLine是否是第一行，因为第一行默认不折叠
bool TextEdit::getNeedControlLine(int line, bool isVisable)
{
    // 查询折叠区域文本块范围
    QTextBlock beginBlock, endBlock, curBlock;
    bool bFoundBrace = findFoldBlock(line, beginBlock, endBlock, curBlock);

    //没有找到右括弧折叠左括弧后面所有行
    if (!bFoundBrace) {
        //遍历最后右括弧文本块 设置块隐藏或显示
        while (beginBlock.isValid()) {
            beginBlock.setVisible(isVisable);
            viewport()->adjustSize();
            beginBlock = beginBlock.next();
        }
        return true;
        //没有找到匹配左右括弧 //如果左右"{" "}"在同一行不折叠
    } else if (!bFoundBrace || endBlock == curBlock) {
        return false;
    } else {
        //遍历最后右括弧文本块 设置块隐藏或显示
        while (beginBlock != endBlock && beginBlock.isValid()) {
            if (beginBlock.isValid()) {
                beginBlock.setVisible(isVisable);
            }
            viewport()->adjustSize();
            beginBlock = beginBlock.next();
        }

        //最后一行显示或隐藏,或者下行就包含"}"
        if (beginBlock.isValid() && beginBlock == endBlock && endBlock.text().simplified() == "}") {
            endBlock.setVisible(isVisable);
            viewport()->adjustSize();
        }

        return true;
    }
}

bool TextEdit::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Gesture:
            gestureEvent(static_cast<QGestureEvent *>(event));
            break;
        case QEvent::PaletteChange:
            // 调色板更新时更新选取高亮颜色
            onAppPaletteChanged();
            break;
        default:
            break;
    }

    return DPlainTextEdit::event(event);
}

bool TextEdit::gestureEvent(QGestureEvent *event)
{
    if (QGesture *tap = event->gesture(Qt::TapGesture))
        tapGestureTriggered(static_cast<QTapGesture *>(tap));
    if (QGesture *tapAndHold = event->gesture(Qt::TapAndHoldGesture))
        tapAndHoldGestureTriggered(static_cast<QTapAndHoldGesture *>(tapAndHold));
    if (QGesture *pan = event->gesture(Qt::PanGesture))
        panTriggered(static_cast<QPanGesture *>(pan));
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
    if (QGesture *swipe = event->gesture(Qt::SwipeGesture))
        swipeTriggered(static_cast<QSwipeGesture *>(swipe));

    return true;
}

void TextEdit::tapGestureTriggered(QTapGesture *tap)
{
    //单指点击函数
    switch (tap->state()) {
    case Qt::GestureStarted: {
        m_gestureAction = GA_tap;
        m_tapBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        break;
    }
    case Qt::GestureUpdated: {
        m_gestureAction = GA_slide;
        break;
    }
    case Qt::GestureCanceled: {
        //根据时间长短区分轻触滑动
        qint64 timeSpace = QDateTime::currentDateTime().toMSecsSinceEpoch() - m_tapBeginTime;
        if (timeSpace < TAP_MOVE_DELAY || m_slideContinueX || m_slideContinueY) {
            m_slideContinueX = false;
            m_slideContinueY = false;
            m_gestureAction = GA_slide;
            qDebug() << "slide start" << timeSpace;
        } else {
            qDebug() << "null start" << timeSpace;
            m_gestureAction = GA_null;
        }
        break;
    }
    case Qt::GestureFinished: {
        m_gestureAction = GA_null;
        break;
    }
    default: {
        Q_ASSERT(false);
        break;
    }
    }
}

void TextEdit::tapAndHoldGestureTriggered(QTapAndHoldGesture *tapAndHold)
{
    //单指长按
    switch (tapAndHold->state()) {
    case Qt::GestureStarted:
        m_gestureAction = GA_hold;
        break;
    case Qt::GestureUpdated:
        break;
    case Qt::GestureCanceled:
        break;
    case Qt::GestureFinished:
        m_gestureAction = GA_null;
        break;
    default:
        break;
    }
}

void TextEdit::panTriggered(QPanGesture *pan)
{
    //两指平移
    switch (pan->state()) {
    case Qt::GestureStarted:
        m_gestureAction = GA_pan;
        break;
    case Qt::GestureUpdated:
        break;
    case Qt::GestureCanceled:
        break;
    case Qt::GestureFinished:
        m_gestureAction = GA_null;
        break;
    default:
        break;
    }
}

void TextEdit::pinchTriggered(QPinchGesture *pinch)
{
    //两指拉伸   -----缩放or放大
    switch (pinch->state()) {
    case Qt::GestureStarted: {
        m_gestureAction = GA_pinch;
        if (static_cast<int>(m_scaleFactor) != m_fontSize) {
            m_scaleFactor = m_fontSize;
        }
        break;
    }
    case Qt::GestureUpdated: {
        QPinchGesture::ChangeFlags changeFlags = pinch->changeFlags();
        if (changeFlags & QPinchGesture::ScaleFactorChanged) {
            m_currentStepScaleFactor = pinch->totalScaleFactor();
        }
        break;
    }
    case Qt::GestureCanceled: {
        break;
    }
    case Qt::GestureFinished: {
        m_gestureAction = GA_null;
        m_scaleFactor *= m_currentStepScaleFactor;
        m_currentStepScaleFactor = 1;
        break;
    }
    default: {
        break;
    }
    }//switch

    //QFont font = getVTFont();
    int size = static_cast<int>(m_scaleFactor * m_currentStepScaleFactor);
    if (size < 8)
        size = 8;
    if (size > 50)
        size = 50;
    //根据移动距离设置字体大小
    setFontSize(size);
    //同步设置界面字体栏数值
    qobject_cast<Window *>(this->window())->changeSettingDialogComboxFontNumber(size);
}

void TextEdit::swipeTriggered(QSwipeGesture *swipe)
{
    //三指滑动
//    switch (swipe->state()) {
//    case Qt::GestureStarted:
//        m_gestureAction = GA_swipe;
//        break;
//    case Qt::GestureUpdated:
//        break;
//    case Qt::GestureCanceled:
//        m_gestureAction = GA_null;
//        break;
//    case Qt::GestureFinished:
//        Q_ASSERT(false);
//        break;
//    default:
//        Q_ASSERT(false);
//        break;
//    }

}

void TextEdit::slideGestureY(qreal diff)
{
    static qreal delta = 0.0;
    int step = static_cast<int>(diff + delta);
    delta = diff + delta - step;

    verticalScrollBar()->setValue(verticalScrollBar()->value() + step);
}

void TextEdit::slideGestureX(qreal diff)
{
    static qreal delta = 0.0;
    int step = static_cast<int>(diff + delta);
    delta = diff + delta - step;

    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + step * 30);
}

void TextEdit::setTheme(const QString &path)
{
    QVariantMap jsonMap = Utils::getThemeMapFromPath(path);
    QVariantMap textStylesMap = jsonMap["text-styles"].toMap();
    QString themeCurrentLineColor = jsonMap["editor-colors"].toMap()["current-line"].toString();
    QString textColor = textStylesMap["Normal"].toMap()["text-color"].toString();

    m_backgroundColor = QColor(jsonMap["editor-colors"].toMap()["background-color"].toString());
    m_currentLineColor = QColor(themeCurrentLineColor);
    m_currentLineNumberColor = QColor(jsonMap["editor-colors"].toMap()["current-line-number"].toString());
    m_lineNumbersColor = QColor(jsonMap["editor-colors"].toMap()["line-numbers"].toString());
    m_regionMarkerColor = QColor(textStylesMap["RegionMarker"].toMap()["selected-text-color"].toString());
    m_selectionColor = QColor(textStylesMap["Normal"].toMap()["selected-text-color"].toString());
    m_selectionBgColor = QColor(textStylesMap["Normal"].toMap()["selected-bg-color"].toString());
    m_bracketMatchFormat = currentCharFormat();
    m_bracketMatchFormat.setForeground(QColor(jsonMap["editor-colors"].toMap()["bracket-match-fg"].toString()));
    m_bracketMatchFormat.setBackground(QColor(jsonMap["editor-colors"].toMap()["bracket-match-bg"].toString()));

    QString findMatchBgColor = jsonMap["editor-colors"].toMap()["find-match-background"].toString();
    QString findMatchFgColor = jsonMap["editor-colors"].toMap()["find-match-foreground"].toString();
    m_findMatchFormat = currentCharFormat();
    m_findMatchFormat.setBackground(QColor(findMatchBgColor));
    m_findMatchFormat.setForeground(QColor(findMatchFgColor));

    QString findHighlightBgColor = jsonMap["editor-colors"].toMap()["find-highlight-background"].toString();
    QString findHighlightFgColor = jsonMap["editor-colors"].toMap()["find-highlight-foreground"].toString();
    m_findHighlightSelection.format.setProperty(QTextFormat::BackgroundBrush, QBrush(QColor(findHighlightBgColor)));
    m_findHighlightSelection.format.setProperty(QTextFormat::ForegroundBrush, QBrush(QColor(findHighlightFgColor)));

    m_beginBracketSelection.format = m_bracketMatchFormat;
    m_endBracketSelection.format = m_bracketMatchFormat;

    int iVerticalScrollValue = getScrollOffset();
    int iHorizontalScrollVaule = horizontalScrollBar()->value();
    getScrollOffset();
    verticalScrollBar()->setSliderPosition(iVerticalScrollValue);
    horizontalScrollBar()->setSliderPosition(iHorizontalScrollVaule);

    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();

    updateHighlightLineSelection();
    renderAllSelections();
}

void TextEdit::removeHighlightWordUnderCursor()
{
    //m_highlightWordCacheCursor = m_wordUnderCursorSelection.cursor;
    QTextEdit::ExtraSelection selection;
    //m_wordUnderCursorSelection = selection;

    renderAllSelections();
    m_nBookMarkHoverLine = -1;
    m_pLeftAreaWidget->m_pBookMarkArea->update();
}

void TextEdit::setSettings(Settings *keySettings)
{
    m_settings = keySettings;
}

/**
 * @brief 拷贝选中的文本
 * @param ignoreCheck 是否忽略权限判断(外部已进行)，默认false
 */
void TextEdit::copySelectedText(bool ignoreCheck)
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!ignoreCheck && !Utils::enableClipCopy(getFilePath())) {
        return;
    }

    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
        QString data;
        for (auto sel : m_altModSelections) {
            data.append(sel.cursor.selectedText());
        }
        QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
        clipboard->setText(data);
    } else {
        QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
        if (textCursor().hasSelection()) {
            clipboard->setText(textCursor().selection().toPlainText());
            tryUnsetMark();
        } else {
            clipboard->setText(m_highlightWordCacheCursor.selectedText());
        }
    }
    tryUnsetMark();
}

/**
 * @brief 剪切选中的文本
 * @param ignoreCheck 是否忽略权限判断(外部已进行)，默认false
 */
void TextEdit::cutSelectedText(bool ignoreCheck)
{
    // 添加权限判断是否允许拷贝，剪切；防止后续可能调用接口，冗余处理
    if (!ignoreCheck && !Utils::enableClipCopy(getFilePath())) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(textCursor().selection().toPlainText());

    QTextCursor cursor = textCursor();
    cursor.removeSelectedText();
    setTextCursor(cursor);

    unsetMark();
}

void TextEdit::pasteText()
{
    QPlainTextEdit::paste();

    unsetMark();
}

void TextEdit::setMark()
{
    bool currentMark = m_cursorMark;
    bool markCursorChanged = false;
    if (m_cursorMark) {
        if (textCursor().hasSelection()) {
            markCursorChanged = true;

            QTextCursor cursor = textCursor();
            cursor.clearSelection();
            setTextCursor(cursor);
        } else {
            m_cursorMark = false;
        }
    } else {
        m_cursorMark = true;
    }

    if (m_cursorMark != currentMark || markCursorChanged) {
        cursorMarkChanged(m_cursorMark, textCursor());
    }
}

void TextEdit::unsetMark()
{
    bool currentMark = m_cursorMark;

    m_cursorMark = false;

    if (m_cursorMark != currentMark) {
        cursorMarkChanged(m_cursorMark, textCursor());
    }
}

bool TextEdit::tryUnsetMark()
{
    if (m_cursorMark) {
        QTextCursor cursor = textCursor();
        cursor.clearSelection();
        setTextCursor(cursor);

        unsetMark();

        return true;
    } else {
        return false;
    }
}

void TextEdit::exchangeMark()
{
    unsetMark();

    if (textCursor().hasSelection()) {
        // Record cursor and seleciton position before move cursor.
        int actionStartPos = textCursor().position();
        int selectionStartPos = textCursor().selectionStart();
        int selectionEndPos = textCursor().selectionEnd();

        QTextCursor cursor = textCursor();
        if (actionStartPos == selectionStartPos) {
            cursor.setPosition(selectionStartPos, QTextCursor::MoveAnchor);
            cursor.setPosition(selectionEndPos, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(selectionEndPos, QTextCursor::MoveAnchor);
            cursor.setPosition(selectionStartPos, QTextCursor::KeepAnchor);
        }

        setTextCursor(cursor);
    }
}

void TextEdit::saveMarkStatus()
{
    m_cursorMarkStatus = m_cursorMark;
    m_cursorMarkPosition = textCursor().anchor();
}

void TextEdit::restoreMarkStatus()
{
    if (m_cursorMarkStatus) {
        QTextCursor currentCursor = textCursor();

        QTextCursor cursor = textCursor();
        cursor.setPosition(m_cursorMarkPosition, QTextCursor::MoveAnchor);
        cursor.setPosition(currentCursor.position(), QTextCursor::KeepAnchor);

        setTextCursor(cursor);
    }
}


void TextEdit::slot_translate()
{
    QProcess::startDetached("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/deepinmain com.iflytek.aiassistant.mainWindow.TextToTranslate");
}

QString TextEdit::getWordAtCursor()
{
    if (!characterCount()) {
        return "";
    } else {
        QTextCursor cursor = textCursor();
        QChar currentChar = toPlainText().at(std::max(cursor.position() - 1, 0));

        cursor.movePosition(QTextCursor::NoMove, QTextCursor::MoveAnchor);
        while (!currentChar.isSpace() && cursor.position() != 0) {
            // while (!currentChar.isSpace() && cursor.position() != 0) {
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
            currentChar = toPlainText().at(std::max(cursor.position() - 1, 0));

            if (currentChar == '-') {
                break;
            }
        }

        return cursor.selectedText();
    }
}

QString TextEdit::getWordAtMouse()
{
    if (!characterCount()) {
        return "";
    } else {
        auto pos = mapFromGlobal(QCursor::pos());
        QTextCursor cursor(cursorForPosition(pos));

        // Get cursor rectangle.
        auto rect = cursorRect(cursor);
        int widthOffset = 10;
        rect.setX(std::max(rect.x() - widthOffset / 2, 0));
        rect.setWidth(rect.width() + widthOffset);

        // Just highlight word under pointer when cursor rectangle contain mouse pointer coordinate.
        if ((rect.x() <= pos.x()) &&
                (pos.x() <= rect.x() + rect.width()) &&
                (rect.y() <= pos.y()) &&
                (pos.y() <= rect.y() + rect.height())) {
            cursor.select(QTextCursor::WordUnderCursor);

            return cursor.selectedText();
        } else {
            return "";
        }
    }
}

void TextEdit::toggleReadOnlyMode(bool notNotify)
{
    if (m_readOnlyMode) {
        if (m_cursorMode == Overwrite) {
            emit cursorModeChanged(Overwrite);
        } else {
            emit cursorModeChanged(Insert);
        }
        setReadOnly(false);
        m_readOnlyMode = false;
        setCursorWidth(1);
        updateHighlightLineSelection();

        if (!notNotify) {
            popupNotify(tr("Read-Only mode is off"));
        }
    } else {
        m_readOnlyMode = true;
        setReadOnly(true);
        setCursorWidth(0); //隐藏光标
        document()->clearUndoRedoStacks();
        updateHighlightLineSelection();

        if (!notNotify) {
            popupNotify(tr("Read-Only mode is on"));
        }
        emit cursorModeChanged(Readonly);
    }
}

void TextEdit::toggleComment(bool bValue)
{
    const auto def = m_repository.definitionForFileName(QFileInfo(m_sFilePath).fileName());
    QTextCursor selectionCursor = textCursor();
    selectionCursor.movePosition(QTextCursor::StartOfBlock);
    selectionCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    QString text = selectionCursor.selectedText();
    // init base.
    bool isBlankLine = text.trimmed().isEmpty();
    if (characterCount() == 0 || isBlankLine || def.filePath().isEmpty()) {
        return;
    }

    if (m_readOnlyMode) {
        popupNotify(tr("Read-Only mode is on"));
        return;
    }

    //不同类型文件注释符号不同 梁卫东　２０２０－０９－０３　１７：２８：４５
    bool bHasCommnent = false;
    if (m_commentDefinition.isValid()) {
        QString  multiLineCommentMark = m_commentDefinition.multiLineStart.simplified();
        QString  singleLineCommentMark = m_commentDefinition.singleLine.simplified();
        if (multiLineCommentMark.isEmpty() && singleLineCommentMark.isEmpty()) {
            bHasCommnent = false;
        } else {
            bHasCommnent = true;
        }
    }
    if (!bHasCommnent) {
        return;
    }

    QString name = def.name();
    if (name == "Markdown") {
        return;
    }

    if (!def.filePath().isEmpty()) {
        if (bValue) {
            setComment();
        } else {
            removeComment();
            //unCommentSelection();
        }
    } else {
        // do not need to prompt the user.
        // popupNotify(tr("File does not support syntax comments"));
    }
}

int TextEdit::getNextWordPosition(QTextCursor &cursor, QTextCursor::MoveMode moveMode)
{
    // FIXME(rekols): if is empty text, it will crash.
    if (!characterCount()) {
        return 0;
    }

    // Move next char first.
    QTextCursor copyCursor = cursor;
    copyCursor.movePosition(QTextCursor::NextCharacter, moveMode);
    QString currentChar = copyCursor.selection().toPlainText();

    // Just to next non-space char if current char is space.
    if (currentChar.data()->isSpace()) {
        while (copyCursor.position() < characterCount() - 1 && currentChar.data()->isSpace()) {
            copyCursor.movePosition(QTextCursor::NextCharacter, moveMode);
            currentChar = copyCursor.selection().toPlainText();
        }
        while (copyCursor.position() < characterCount() - 1 && !atWordSeparator(copyCursor.position())) {
            copyCursor.movePosition(QTextCursor::NextCharacter, moveMode);

            //cursor.movePosition(QTextCursor::NextCharacter, moveMode);
        }
    }
    // Just to next word-separator char.
    else {
        while (copyCursor.position() < characterCount() - 1 && !atWordSeparator(copyCursor.position())) {
            copyCursor.movePosition(QTextCursor::NextCharacter, moveMode);
        }
    }

    return copyCursor.position();
}

int TextEdit::getPrevWordPosition(QTextCursor cursor, QTextCursor::MoveMode moveMode)
{
    if (!characterCount()) {
        return 0;
    }

    // Move prev char first.
    QTextCursor copyCursor = cursor;
    copyCursor.movePosition(QTextCursor::PreviousCharacter, moveMode);
    QString currentChar = copyCursor.selection().toPlainText();

    // Just to next non-space char if current char is space.
    if (currentChar.data()->isSpace()) {
        while (copyCursor.position() > 0 && currentChar.data()->isSpace()) {
            copyCursor.movePosition(QTextCursor::PreviousCharacter, moveMode);
            currentChar = copyCursor.selection().toPlainText();
        }
    }
    // Just to next word-separator char.
    else {
        while (copyCursor.position() > 0 && !atWordSeparator(copyCursor.position())) {
            copyCursor.movePosition(QTextCursor::PreviousCharacter, moveMode);
        }
    }

    return copyCursor.position();
}

bool TextEdit::atWordSeparator(int position)
{
    QTextCursor copyCursor = textCursor();
    copyCursor.setPosition(position, QTextCursor::MoveAnchor);
    copyCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    QString currentChar = copyCursor.selection().toPlainText();
    return m_wordSepartors.contains(currentChar);
}

void TextEdit::showCursorBlink()
{
    // -1 表示恢复Qt的默认值
    QApplication::setCursorFlashTime(-1);
}

void TextEdit::hideCursorBlink()
{
    QApplication::setCursorFlashTime(0);
}

void TextEdit::setReadOnlyPermission(bool permission)
{
    m_bReadOnlyPermission = permission; //true为不可读
    if (permission) {
        m_Permission2 = true;
        setReadOnly(true);
        emit cursorModeChanged(Readonly);
    } else {
        m_Permission = false;
        if (!m_readOnlyMode) {
            setReadOnly(false);
            //emit cursorModeChanged(Insert);
        } else {
            setReadOnly(true);
            emit cursorModeChanged(Readonly);
        }
    }
    SendtoggleReadOnlyMode();
}

void TextEdit::SendtoggleReadmessage()
{
    if (!m_bReadOnlyPermission) {
        if (m_cursorMode == Overwrite) {
            emit cursorModeChanged(Overwrite);
        } else {
            emit cursorModeChanged(Insert);
        }
        setReadOnly(false);
        setCursorWidth(1);
        updateHighlightLineSelection();
    } else {
        setReadOnly(true);
        setCursorWidth(0); //隐藏光标
        document()->clearUndoRedoStacks();
        updateHighlightLineSelection();
        emit cursorModeChanged(Readonly);
    }
}

bool TextEdit::isAbleOperation(int iOperationType)
{
    /*
     * 读取并计算系统剩余内存大小，根据系统剩余内存大小决定本次复制/粘贴操作是否可继续执行
     * 解决的问题：复制/粘贴大文本字符内容(50MB/100MB/500MB)时会占用大量内存，系统内存不足会导致应用闪退
     */
    bool bRet = true;
    qlonglong memory = 0;
    qlonglong memoryAll = 0;
    bool bVaule = false;
    QFile file(PROC_MEMINFO_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qInfo() << "Open " << PROC_MEMINFO_PATH << " failed.";
        return bRet;
    }

    QTextStream stream(&file);
    qlonglong buff[16] = {0};
    for (int i = 0; i <= 15; ++i) {
        QString line = stream.readLine();
        QStringList list = line.split(QRegExp("\\s{1,}"));
        if (list.size() >= 2) {
            buff[i] = list.at(1).toLongLong(&bVaule);
        }
    }

    memoryAll = buff[0];
    memory = buff[0] - buff[2];
    /* 系统当前可用内存大小 */
    qlonglong iSystemAvailableMemory = memoryAll - memory;
    file.close();

    if (iOperationType == OperationType::CopyOperation) {
        if (m_isSelectAll) {
            if (characterCount() / DATA_SIZE_1024 * COPY_CONSUME_MEMORY_MULTIPLE > iSystemAvailableMemory) {
                bRet = false;
            }
        } else if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
            QString strSelectText;
            for (auto it = m_altModSelections.begin(); it != m_altModSelections.end(); it++) {
                auto text = (*it).cursor.selectedText();
                strSelectText += text;
                if (it != m_altModSelections.end() - 1)
                    strSelectText += "\n";
            }
            if (strSelectText.size() / DATA_SIZE_1024 * COPY_CONSUME_MEMORY_MULTIPLE > iSystemAvailableMemory) {
                bRet = false;
            }
        } else if (textCursor().hasSelection()) {
            if (textCursor().selection().toPlainText().size() / DATA_SIZE_1024 * COPY_CONSUME_MEMORY_MULTIPLE > iSystemAvailableMemory) {
                bRet = false;
            }
        }
    } else if (iOperationType == OperationType::PasteOperation) {
        const QClipboard *clipboard = QApplication::clipboard();
        QString strClipboardText = clipboard->text();
        //文本内容大于系统总内存,不允许粘贴
        if ((document()->characterCount() + strClipboardText.size()) / DATA_SIZE_1024 * PASTE_CONSUME_MEMORY_MULTIPLE > memoryAll) {
            bRet = false;
        }
        if (strClipboardText.size() / DATA_SIZE_1024 * PASTE_CONSUME_MEMORY_MULTIPLE > iSystemAvailableMemory) {
            bRet = false;
        }

        /* 当文本框里的文本内容达到800MB后，再次持续粘贴，则只允许粘贴<=500KB大小的文本内容 */
        if (bRet == true) {
            if (document()->characterCount() > (800 * DATA_SIZE_1024 * DATA_SIZE_1024) && strClipboardText.size() / DATA_SIZE_1024 > 500) {
                bRet = false;
            }
        }
    }

    return bRet;
}

void TextEdit::SendtoggleReadOnlyMode()
{
    if (m_bReadOnlyPermission && !m_Permission) {
        m_Permission = m_bReadOnlyPermission;
        SendtoggleReadmessage();
    } else if (m_Permission2 && !m_bReadOnlyPermission) {
        m_Permission2 = m_bReadOnlyPermission;
        SendtoggleReadmessage();
    }
}

bool TextEdit::getReadOnlyPermission()
{
    return m_bReadOnlyPermission;
}

bool TextEdit::getReadOnlyMode()
{
    return m_readOnlyMode;
}

void TextEdit::hideRightMenu()
{
    //arm平台全屏然后恢复窗口，右键菜单不会消失，所以加了这个函数
    if (m_rightMenu) {
        m_rightMenu->hide();
    }
}

void TextEdit::bookMarkAreaPaintEvent(QPaintEvent *event)
{
    BookMarkWidget *bookMarkArea = m_pLeftAreaWidget->m_pBookMarkArea;
    QPainter painter(bookMarkArea);
    QColor lineNumberAreaBackgroundColor;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::ColorType::DarkType) {
        lineNumberAreaBackgroundColor = palette().brightText().color();
        lineNumberAreaBackgroundColor.setAlphaF(0.06);

        m_lineNumbersColor.setAlphaF(0.2);
    } else {
        lineNumberAreaBackgroundColor = palette().brightText().color();
        lineNumberAreaBackgroundColor.setAlphaF(0.03);
        m_lineNumbersColor.setAlphaF(0.3);
    }

    QTextBlock lineBlock;//第几行文本块
    QImage image;
    QString pixmapPath;
    QList<int> list = m_listBookmark;
    bool bIsContains = false;

    if (!m_listBookmark.contains(m_nBookMarkHoverLine) && m_nBookMarkHoverLine != -1 && m_nBookMarkHoverLine <= blockCount()) {
        list << m_nBookMarkHoverLine;
    } else {
        bIsContains = true;
    }

    foreach (auto line, list) {
        lineBlock = document()->findBlockByNumber(line - 1);
        QTextCursor cur = textCursor();
        cur.setPosition(lineBlock.position(), QTextCursor::MoveAnchor);
        if (line == m_nBookMarkHoverLine && !bIsContains) {
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::ColorType::DarkType) {
                pixmapPath = ":/images/like_hover_dark.svg";
                image = QImage(":/images/like_hover_dark.svg");
            } else {
                image = QImage(":/images/like_hover_light.svg");
                pixmapPath = ":/images/like_hover_light.svg";
            }
        } else {
            image = QImage(":/images/bookmark.svg");
            pixmapPath = ":/images/bookmark.svg";
        }

        if (line > 0) {
            lineBlock = document()->findBlockByNumber(line - 1);
            if (!lineBlock.isVisible()) {
                continue;
            }

            int w = this->m_fontSize <= 15 ? 15 : m_fontSize;
            updateLeftWidgetWidth(w);
            int h = cursorRect(cur).height();
            // 绘制行纵向居中
            int offset = qMax(0, (h - w) / 2);
            //the language currently set by the system is Tibetan.
            if ("bo_CN" == Utils::getSystemLan())
                offset = h <= 20 ? 0 : h / 10;

            QRect rect(0, cursorRect(cur).y() + offset, w, w);
            QSvgRenderer render;
            render.load(pixmapPath);
            render.render(&painter, rect);
        }
    }
}

int TextEdit::getLineFromPoint(const QPoint &point)
{
    QTextCursor cursor = cursorForPosition(point);
    QTextBlock block = cursor.block();
//    qDebug() << "block.blockNumber()" << block.blockNumber();
    return block.blockNumber() + 1;
}

void TextEdit::addOrDeleteBookMark()
{
    int line = 0;
    if (m_bIsShortCut) {
        line = getCurrentLine();
        m_bIsShortCut = false;
    } else {
        line = getLineFromPoint(m_mouseClickPos);
    }

    if (line > blockCount()) {
        return;
    }

    if (m_listBookmark.contains(line)) {
        m_listBookmark.removeOne(line);
        m_nBookMarkHoverLine = -1;
        qDebug() << "DeleteBookMark:" << line << m_listBookmark;
    } else {
        m_listBookmark.push_back(line);
        qDebug() << "AddBookMark:" << line << m_listBookmark;
    }

    m_pLeftAreaWidget->m_pBookMarkArea->update();
}

void TextEdit::moveToPreviousBookMark()
{
    int line = getCurrentLine();
    int index = m_listBookmark.indexOf(line);

    if (index == -1 && !m_listBookmark.isEmpty()) {
        jumpToLine(m_listBookmark.last(), false);
        return;
    }

    if (index == 0) {
        jumpToLine(m_listBookmark.last(), false);
    } else {
        jumpToLine(m_listBookmark.value(index - 1), false);
    }
}

void TextEdit::moveToNextBookMark()
{
    int line = getCurrentLine();
    int index = m_listBookmark.indexOf(line);

    if (index == -1 && !m_listBookmark.isEmpty()) {
        jumpToLine(m_listBookmark.first(), false);
        return;
    }

    if (index == m_listBookmark.count() - 1) {
        jumpToLine(m_listBookmark.first(), false);
    } else {
        jumpToLine(m_listBookmark.value(index + 1), false);
    }
}

void TextEdit::checkBookmarkLineMove(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);

    if (m_bIsFileOpen) {
        return;
    }

    if (m_nLines != blockCount()) {
        QTextCursor cursor = textCursor();
        int nAddorDeleteLine = document()->findBlock(from).blockNumber() + 1;
        int currLine = textCursor().blockNumber() + 1;

        if (m_nLines > blockCount()) {
            foreach (const auto line, m_listBookmark) {
                if (m_nSelectEndLine != -1) {
                    if (nAddorDeleteLine < line && line <= m_nSelectEndLine) {
                        m_listBookmark.removeOne(line);
                    } else if (line > m_nSelectEndLine) {
                        m_listBookmark.replace(m_listBookmark.indexOf(line), line - m_nSelectEndLine + nAddorDeleteLine);
                    }
                } else {
                    if (line == currLine + 1) {
                        m_listBookmark.removeOne(currLine + 1);
                    } else if (line > currLine + 1) {
                        m_listBookmark.replace(m_listBookmark.indexOf(line), line  - m_nLines + blockCount());
                    }
                }
            }
        } else {
            foreach (const auto line, m_listBookmark) {
                if (nAddorDeleteLine < line) {
                    m_listBookmark.replace(m_listBookmark.indexOf(line), line + blockCount() - m_nLines);
                }
            }
        }
    }
    m_nLines = blockCount();
}

void TextEdit::flodOrUnflodAllLevel(bool isFlod)
{
    m_listMainFlodAllPos.clear();
    //折叠
    if (isFlod) {
        for (int line = 0; line < document()->blockCount(); line++) {
            if (blockContainStrBrackets(line)
                    && document()->findBlockByNumber(line).isVisible()
                    && !document()->findBlockByNumber(line).text().trimmed().startsWith("//")) {
                if (getNeedControlLine(line, false)) {
                    m_listMainFlodAllPos.append(line);
                }
            }
        }
        //展开
    } else {
        for (int line = 0; line < document()->blockCount(); line++) {
            if (blockContainStrBrackets(line)
                    && !document()->findBlockByNumber(line + 1).isVisible()
                    && !document()->findBlockByNumber(line).text().trimmed().startsWith("//")) {
                if (getNeedControlLine(line, true)) {
                    m_listMainFlodAllPos.append(line);
                }
            }
        }
    }

    //折叠时出现点击光标选择行变短
    QPlainTextEdit::LineWrapMode curMode = this->lineWrapMode();
    QPlainTextEdit::LineWrapMode WrapMode = curMode ==  QPlainTextEdit::NoWrap ?  QPlainTextEdit::WidgetWidth :  QPlainTextEdit::NoWrap;
    this->setWordWrapMode(QTextOption::WrapAnywhere);
    this->setLineWrapMode(WrapMode);

    m_pLeftAreaWidget->m_pFlodArea->update();
    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();

    viewport()->update();
    document()->adjustSize();

    this->setLineWrapMode(curMode);
    viewport()->update();
}

void TextEdit::flodOrUnflodCurrentLevel(bool isFlod)
{
    int line = getLineFromPoint(m_mouseClickPos);
    getNeedControlLine(line - 1, !isFlod);
    m_pLeftAreaWidget->m_pFlodArea->update();
    m_pLeftAreaWidget->m_pLineNumberArea->update();
    m_pLeftAreaWidget->m_pBookMarkArea->update();
    viewport()->update();
    document()->adjustSize();

}

void TextEdit::getHideRowContent(int iLine)
{
    // 预览文本块没有必要读取所有文本数据，调整为仅读取部分
    // Note:需要注意单个文本块(一般为一行数据)过长的情况
    // 最大显示的预览文本块数量，不超过1000
    static int s_MaxDisplayBlockCount = 1000;

    // 查询折叠区域文本块范围
    QTextBlock beginBlock, endBlock, curBlock;
    bool bFoundBrace = findFoldBlock(iLine, beginBlock, endBlock, curBlock);

    if (QColor(m_backgroundColor).lightness() < 128) {
        m_foldCodeShow->initHighLight(m_sFilePath, false);
    } else {
        m_foldCodeShow->initHighLight(m_sFilePath, true);
    }

    m_foldCodeShow->appendText("{", width());
    //左右括弧没有匹配到
    if (!bFoundBrace) {
        // 读取文本块索引
        int curIndex = 0;
        //遍历最后右括弧文本块 设置块隐藏或显示,显示文本块不超过1000
        while (beginBlock.isValid()
                && (curIndex++ < s_MaxDisplayBlockCount)) {
            m_foldCodeShow->appendText(beginBlock.text(), width());
            beginBlock = beginBlock.next();
        }

        //如果左右"{" "}"在同一行不折叠
    } else if (endBlock == curBlock) {
        return;
    } else {
        // 读取文本块索引
        int curIndex = 0;
        //遍历最后右括弧文本块 设置块隐藏或显示,显示文本块不超过1000
        while (beginBlock != endBlock
                && beginBlock.isValid()
                && (curIndex++ < s_MaxDisplayBlockCount)) {
            if (beginBlock.isValid()) {
                m_foldCodeShow->appendText(beginBlock.text(), width());
            }

            beginBlock = beginBlock.next();
        }

        if (endBlock.text().simplified() == "}") {
            m_foldCodeShow->appendText("}", width());
        }

        m_foldCodeShow->appendText("}", width());
    }
}

bool TextEdit::isNeedShowFoldIcon(QTextBlock block)
{
    QString blockText = block.text();
    bool hasFindLeft = false; // 是否已经找到当前行第一个左括号
    int rightNum = 0, leftNum = 0;//右括号数目、左括号数目
    for (int i = 0 ; i < blockText.count(); ++i) {
        if (blockText.at(i) == "}" && hasFindLeft) {
            rightNum++;
        } else if (blockText.at(i) == "{") {
            if (!hasFindLeft)
                hasFindLeft = true;
            leftNum++;
        }
    }

    if (rightNum == leftNum) {
        return  false;
    } else {
        return  true;
    }
}

int TextEdit::getHighLightRowContentLineNum(int iLine)
{
    // 查询折叠区域文本块范围
    QTextBlock beginBlock, endBlock, curBlock;
    bool bFoundBrace = findFoldBlock(iLine, beginBlock, endBlock, curBlock);

    //左右括弧没有匹配到
    if (!bFoundBrace) {
        //遍历最后右括弧文本块 设置块隐藏或显示
        while (beginBlock.isValid()) {
            iLine++;
            beginBlock = beginBlock.next();
        }
        return iLine;
        //如果左右"{" "}"在同一行不折叠
    } else if (!bFoundBrace || endBlock == curBlock) {
        return iLine;
    } else {
        while (beginBlock != endBlock && beginBlock.isValid()) {
            iLine++;
            beginBlock = beginBlock.next();
        }

        iLine++;
        return iLine;
    }
}

void TextEdit::paintCodeFlod(QPainter *painter, QRect rect, bool flod)
{
    painter->save();

    painter->translate(rect.width() / 2.0, rect.height() / 2.0);
    painter->translate(0, rect.y());
    QPainterPath path;
    //QPointF p1(rect.x()+rect.width() * (1.0/3),rect.y()+rect.height()*(1.0/3));
    //QPointF p2(rect.x()+rect.width() * (2.0/3),rect.y()+rect.height()*(1.0/3));
    //QPointF p3(rect.x()+rect.width() * (1.0/2),rect.y()+rect.height()*(1.5/3));
    QPointF p1(-1 * rect.width() * (1.0 / 6), -1 * rect.height() * (1.0 / 6));
    QPointF p2(rect.width() * (1.0 / 6), -1 * rect.height() * (1.0 / 6));
    QPointF p3(0, 0);
    path.moveTo(p1);
    path.lineTo(p3);
    path.lineTo(p2);

    if (flod) {
        painter->rotate(-90);
    }

    QPen pen(this->palette().foreground(), 2);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPath(path);

    painter->restore();
}

QColor TextEdit::getBackColor()
{
    return m_backgroundColor;
}

int TextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, this->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    // 行号使用单独字体
    QFontMetrics fm(m_fontLineNumberArea);
    int w = fm.horizontalAdvance(QLatin1Char('9')) * digits;

    return w > 15 ? w : 15;
}

void TextEdit::updateLeftWidgetWidth(int width)
{
    if (m_LeftAreaUpdateState != TextEdit::FileOpenBegin) {
        m_pLeftAreaWidget->m_pFlodArea->setFixedWidth(width);
        m_pLeftAreaWidget->m_pLineNumberArea->setFixedWidth(lineNumberAreaWidth());
        m_pLeftAreaWidget->m_pBookMarkArea->setFixedWidth(width);
        setLeftAreaUpdateState(TextEdit::Normal);
    }
}

int TextEdit::getLinePosYByLineNum(int iLine)
{
    QTextBlock block = document()->findBlockByNumber(iLine);
    QTextCursor cur = textCursor();

    while (!block.isVisible()) {
        block = block.next();
    }

    cur.setPosition(block.position(), QTextCursor::MoveAnchor);
    return cursorRect(cur).y();
}

bool TextEdit::ifHasHighlight()
{
    if (!m_findHighlightSelection.cursor.isNull())
        return m_findHighlightSelection.cursor.hasSelection();
    else {
        return  false;
    }
}

void TextEdit::setIsFileOpen()
{
    m_bIsFileOpen = true;
}

void TextEdit::setTextFinished()
{
    m_bIsFileOpen = false;
    m_nLines = blockCount();

    if (!m_listBookmark.isEmpty()) {
        return;
    }

    QStringList bookmarkList = readHistoryRecordofBookmark();
    QStringList filePathList = readHistoryRecordofFilePath("advance.editor.browsing_history_file");
    QList<int> linesList;

    QString qstrPath = m_sFilePath;

    if (filePathList.contains(qstrPath)) {
        int index = 2;
        QString qstrLines = bookmarkList.value(filePathList.indexOf(qstrPath));
        QString sign;

        for (int i = 0; i < qstrLines.count() - 1; i++) {
            sign = qstrLines.at(i);
            sign.append(qstrLines.at(i + 1));

            if (sign == ",*" || sign == ")*") {
                linesList << qstrLines.mid(index, i - index).toInt();
                index = i + 2;
            }
        }
    }

    foreach (const auto line, linesList) {
        if (line <= document()->blockCount()) {
            if (!m_listBookmark.contains(line)) {
                m_listBookmark << line;
            }
        }
    }
//    qDebug() << m_listBookmark << document()->blockCount();
}

QStringList TextEdit::readHistoryRecord(QString key)
{
    QString history = m_settings->settings->option(key)->value().toString();
    QStringList historyList;
    int nLeftPosition = history.indexOf("*{");
    int nRightPosition = history.indexOf("}*");

    while (nLeftPosition != -1) {
        historyList << history.mid(nLeftPosition, nRightPosition + 2 - nLeftPosition);
        nLeftPosition = history.indexOf("*{", nLeftPosition + 2);
        nRightPosition = history.indexOf("}*", nRightPosition + 2);
    }

    return historyList;
}

QStringList TextEdit::readHistoryRecordofBookmark()
{
    QString history = m_settings->settings->option("advance.editor.browsing_history_file")->value().toString();
    QStringList bookmarkList;
    int nLeftPosition = history.indexOf("*(");
    int nRightPosition = history.indexOf(")*");

    while (nLeftPosition != -1) {
        bookmarkList << history.mid(nLeftPosition, nRightPosition + 2 - nLeftPosition);
        nLeftPosition = history.indexOf("*(", nLeftPosition + 2);
        nRightPosition = history.indexOf(")*", nRightPosition + 2);
    }

    return bookmarkList;
}

QStringList TextEdit::readHistoryRecordofFilePath(QString key)
{
    QString history = m_settings->settings->option(key)->value().toString();
    QStringList filePathList;
    int nLeftPosition = history.indexOf("*[");
    int nRightPosition = history.indexOf("]*");

    while (nLeftPosition != -1) {
        filePathList << history.mid(nLeftPosition + 2, nRightPosition - 2 - nLeftPosition);
        nLeftPosition = history.indexOf("*[", nLeftPosition + 2);
        nRightPosition = history.indexOf("]*", nRightPosition + 2);
    }

    return filePathList;
}

void TextEdit::writeEncodeHistoryRecord()
{
    QString history = m_settings->settings->option("advance.editor.browsing_encode_history")->value().toString();

    QStringList pathList = readHistoryRecordofFilePath("advance.editor.browsing_encode_history");

    foreach (auto path, pathList) {
        QFileInfo f(path);
        if (!f.isFile()) {
            int nLeftPosition = history.indexOf(path);
            int nRightPosition = history.indexOf("}*", nLeftPosition);
            history.remove(nLeftPosition - 4, nRightPosition + 6 - nLeftPosition);
        }
    }

    int nLeftPosition = history.indexOf(m_sFilePath);
    int nRightPosition = history.indexOf("}*", nLeftPosition);

    if (history.contains(m_sFilePath)) {
        history.remove(nLeftPosition - 4, nRightPosition + 6 - nLeftPosition);
    }

    QString encodeHistory = history + "*{*[" + m_sFilePath + "]*" + m_textEncode + "}*";
    m_settings->settings->option("advance.editor.browsing_encode_history")->setValue(encodeHistory);
}

QStringList TextEdit::readEncodeHistoryRecord()
{
    QString history = m_settings->settings->option("advance.editor.browsing_encode_history")->value().toString();
    QStringList filePathList;
    int nLeftPosition = history.indexOf("]*");
    int nRightPosition = history.indexOf("}*");

    while (nLeftPosition != -1) {
        filePathList << history.mid(nLeftPosition + 2, nRightPosition - 2 - nLeftPosition);
        nLeftPosition = history.indexOf("]*", nLeftPosition + 2);
        nRightPosition = history.indexOf("}*", nRightPosition + 2);
    }

    return filePathList;
}

void TextEdit::tellFindBarClose()
{
    m_bIsFindClose = true;
}

void TextEdit::setEditPalette(const QString &activeColor, const QString &inactiveColor)
{
    QPalette pa = this->palette();
    pa.setColor(QPalette::Inactive, QPalette::Text, QColor(inactiveColor));
    pa.setColor(QPalette::Active, QPalette::Text, QColor(activeColor));
    setPalette(pa);
}

void TextEdit::setCodeFoldWidgetHide(bool isHidden)
{
    if (m_foldCodeShow) {
        m_foldCodeShow->setHidden(isHidden);
    }
}


void TextEdit::setTruePath(QString qstrTruePath)
{
    m_qstrTruePath = qstrTruePath;
}

QString TextEdit::getTruePath()
{
    if (m_qstrTruePath.isEmpty()) {
        return m_sFilePath;
    }

    return  m_qstrTruePath;
}

QList<int> TextEdit::getBookmarkInfo()
{
    return m_listBookmark;
}

void TextEdit::setBookMarkList(QList<int> bookMarkList)
{
    m_listBookmark = bookMarkList;
}

void TextEdit::updateSaveIndex()
{
    m_lastSaveIndex = m_pUndoStack->index();
}

void TextEdit::isMarkCurrentLine(bool isMark, QString strColor,  qint64 timeStamp)
{
    qint64 operationTimeStamp = timeStamp;
    if (operationTimeStamp < 0) {
        operationTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    if (isMark) {
        QTextEdit::ExtraSelection selection;
        selection.cursor = textCursor();
        selection.format.setBackground(QColor(strColor));

        TextEdit::MarkOperation markOperation;
        markOperation.color = strColor;

        if (textCursor().hasSelection()) {
            markOperation.type = MarkOnce;
        } else {
            markOperation.type = MarkLine;
            int beginPos = textCursor().block().position();
            int endPos = beginPos + textCursor().block().length() - 1;
            selection.cursor.setPosition(beginPos, QTextCursor::MoveAnchor);
            selection.cursor.setPosition(endPos, QTextCursor::KeepAnchor);
        }
        //alt选中光标单独处理
        if (m_bIsAltMod) {
            for (int i = 0; i < m_altModSelections.size(); ++i) {
                markOperation.cursor = m_altModSelections[i].cursor;
                selection.cursor = m_altModSelections[i].cursor;
                m_markOperations.append(QPair<TextEdit::MarkOperation, qint64>(markOperation, operationTimeStamp));
                m_wordMarkSelections.append(
                    QPair<QTextEdit::ExtraSelection, qint64>
                    (selection, operationTimeStamp));
            }
        } else {
            markOperation.cursor = selection.cursor;
            m_markOperations.append(QPair<TextEdit::MarkOperation, qint64>(markOperation, operationTimeStamp));
            m_wordMarkSelections.append(
                QPair<QTextEdit::ExtraSelection, qint64>
                (selection, operationTimeStamp));
        }
    } else {
        clearMarksForTextCursor();
    }
}

void TextEdit::markAllKeywordInView()
{
    if (m_markOperations.isEmpty()) {
        return;
    }

    QList<QPair<TextEdit::MarkOperation, qint64>>::iterator it;

    for (it = m_markOperations.begin(); it != m_markOperations.end(); ++it) {
        if (MarkAllMatch == it->first.type) {
            // 标记当前视图根据匹配文本查找的所有文本
            markKeywordInView(it->first.matchText, it->first.color, it->second);
        } else if (MarkAll == it->first.type) {
            markAllInView(it->first.color, it->second);
        }
    }

    renderAllSelections();
}

bool TextEdit::markKeywordInView(QString keyword, QString color, qint64 timeStamp)
{
    qint64 operationTimeStamp = timeStamp;
    if (operationTimeStamp < 0) {
        operationTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    if (keyword.isEmpty()) {
        return false;
    }

    QList<QTextEdit::ExtraSelection> listExtraSelection;
    QTextCharFormat format;
    bool ret = false;

    format.setBackground(QColor(color));
    ret = updateKeywordSelectionsInView(keyword, format, &listExtraSelection);

    // 构建带有时间戳的 listExtraSelectionWithTimeStamp
    QList<QPair<QTextEdit::ExtraSelection, qint64>> listExtraSelectionWithTimeStamp;
    for (int i = 0; i < listExtraSelection.size(); i++) {
        listExtraSelectionWithTimeStamp.append(QPair<QTextEdit::ExtraSelection, qint64>
                                               (listExtraSelection.at(i), operationTimeStamp));
    }

    if (ret) {
        m_mapKeywordMarkSelections[keyword] = listExtraSelectionWithTimeStamp;
    }

    return ret;
}

void TextEdit::markAllInView(QString color, qint64 timeStamp)
{
    // 增加时间戳
    qint64 operationTimeStamp = timeStamp;
    if (operationTimeStamp < 0) {
        operationTimeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    QTextEdit::ExtraSelection selection;
    QList<QPair<QTextEdit::ExtraSelection, qint64>> listSelections;

    // 此处选中操作存在错误
    // QScrollBar *pScrollBar = verticalScrollBar();
    // QPoint startPoint = QPointF(0, 0).toPoint();
    // QTextBlock beginBlock = cursorForPosition(startPoint).block();
    // QTextBlock endBlock;
    //
    // if (pScrollBar->maximum() > 0) {
    //     QPoint endPoint = QPointF(0, 1.5 * height()).toPoint();
    //     endBlock = cursorForPosition(endPoint).block();
    // } else {
    //     endBlock = document()->lastBlock();
    // }

    // selection.cursor = textCursor();
    // selection.cursor.setPosition(beginBlock.position(), QTextCursor::MoveAnchor);
    // selection.cursor.setPosition(endBlock.position() + endBlock.length() - 1, QTextCursor::KeepAnchor);

    // 标准 QTextCursor 选中操作
    QTextCursor textCursor(document());
    textCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    textCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    selection.format.setBackground(QColor(color));
    selection.cursor = textCursor;

    listSelections.append(QPair<QTextEdit::ExtraSelection, qint64>
                          (selection, operationTimeStamp));

    m_mapKeywordMarkSelections[TEXT_EIDT_MARK_ALL] = listSelections;
}

void TextEdit::isMarkAllLine(bool isMark, QString strColor)
{
    // 增加时间戳
    qint64 timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch();

    if (isMark) {
        QString selectionText = textCursor().selectedText();
        if (selectionText.length() != 0 && selectionText.length() < (document()->characterCount() - 1)) {
            // 没有使用的变量，去除
            // QList<QTextEdit::ExtraSelection> wordMarkSelections = m_wordMarkSelections;
            QList<QTextEdit::ExtraSelection> listExtraSelection;
            // 没有使用的变量，去除
            // QList<QTextEdit::ExtraSelection> listSelections;
            QTextEdit::ExtraSelection  extraSelection;
            QTextCharFormat format;
            format.setBackground(QColor(strColor));
            extraSelection.cursor = textCursor();
            extraSelection.format = format;

            TextEdit::MarkOperation markOperation;
            markOperation.type = MarkAllMatch;
            markOperation.cursor = textCursor();
            markOperation.color = strColor;
            markOperation.matchText = selectionText;
            m_markOperations.append(QPair<TextEdit::MarkOperation, qint64>(markOperation, timeStamp));

            if (updateKeywordSelectionsInView(selectionText, format, &listExtraSelection)) {

                QList<QPair<QTextEdit::ExtraSelection, qint64>> listExtraSelectionWithTimeStamp;
                for (int i = 0; i < listExtraSelection.size(); i++) {
                    listExtraSelectionWithTimeStamp.append(QPair<QTextEdit::ExtraSelection, qint64>
                                                           (listExtraSelection.at(i), timeStamp));
                }

                m_mapKeywordMarkSelections[selectionText] = listExtraSelectionWithTimeStamp;
            } else {
                // 选中部分在文档中仅此一个，查找标记失败了，进行补充处理
                QTextEdit::ExtraSelection extraSelect;
                listExtraSelection.append(extraSelection);
                QList<QPair<QTextEdit::ExtraSelection, qint64>> listExtraSelectionWithTimeStamp;
                for (int i = 0; i < listExtraSelection.size(); i++) {
                    listExtraSelectionWithTimeStamp.append(QPair<QTextEdit::ExtraSelection, qint64>
                                                           (listExtraSelection.at(i), timeStamp));
                }

                m_mapKeywordMarkSelections.insert(selectionText, listExtraSelectionWithTimeStamp);
            }

        } else if (!textCursor().hasSelection() || selectionText.length() == (document()->characterCount() - 1)) {
            TextEdit::MarkOperation markOperation;
            markOperation.type = MarkAll;
            markOperation.color = strColor;
            m_markOperations.append(QPair<TextEdit::MarkOperation, qint64>(markOperation, timeStamp));
            markAllInView(strColor, timeStamp);
        }
    } else {
        m_markOperations.clear();
        m_wordMarkSelections.clear();
        m_mapKeywordMarkSelections.clear();

        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(strColor));
        selection.cursor = textCursor();
        selection.cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        selection.cursor.clearSelection();
        m_markAllSelection = selection;
    }

    renderAllSelections();
}

void TextEdit::cancelLastMark()
{
    if (m_markOperations.isEmpty()) {
        return;
    }

    switch (m_markOperations.last().first.type) {
    case MarkOnce:
    case MarkLine: {
        if (!m_wordMarkSelections.isEmpty()) {
            // m_wordMarkSelections.removeLast();
            // 考虑到可能的插入操作，需要同步清理时间戳一样的selection
            const qint64 operationTimeStamp = m_markOperations.last().second;
            for (int i = 0; i < m_wordMarkSelections.size(); i++) {
                if (operationTimeStamp == m_wordMarkSelections.at(i).second) {
                    m_wordMarkSelections.removeAt(i);
                    i--;
                }
            }
        }
        break;
    }

    case MarkAllMatch: {
        // QString keyword = m_markOperations.last().first.cursor.selectedText();
        QString keyword;
        qint64 timeStamp = m_markOperations.last().second;
        // 使用时间戳查找 keyword
        QMap<QString, QList<QPair<QTextEdit::ExtraSelection, qint64>>>::Iterator it;
        for (it = m_mapKeywordMarkSelections.begin(); it != m_mapKeywordMarkSelections.end(); ++it) {
            if (it.value().size() > 0) {
                qint64 itsTimeStamp = it.value().first().second;
                if (itsTimeStamp == timeStamp) {
                    keyword = it.key();
                    break;
                }
            }
        }

        if (m_mapKeywordMarkSelections.contains(keyword)) {
            m_mapKeywordMarkSelections.remove(keyword);
        }
        break;
    }

    case MarkAll: {
        if (m_mapKeywordMarkSelections.contains(TEXT_EIDT_MARK_ALL)) {
            m_mapKeywordMarkSelections.remove(TEXT_EIDT_MARK_ALL);
        }
        break;
    }

    }

    m_markOperations.removeLast();

    // 如果在标记颜色操作后，更改文本内容，如果存在残留，补充一个清除处理
    if (m_markOperations.isEmpty() &&
            (m_wordMarkSelections.size() > 0 || m_mapKeywordMarkSelections.size() > 0)) {
        qWarning() << __FUNCTION__ << __LINE__ << " cancle mark color operation,"
                   << "find exist remain selections, will clear!";
        m_wordMarkSelections.clear();
        m_mapKeywordMarkSelections.clear();
    }

    renderAllSelections();
}

bool TextEdit::clearMarkOperationForCursor(QTextCursor cursor)
{
    bool bRet = false;
    for (int i = m_markOperations.size() - 1; i >= 0; --i) {
        if (m_markOperations.at(i).first.cursor == cursor) {
            m_markOperations.removeAt(i);
            bRet = true;
            break;
        }
    }

    return bRet;
}

bool TextEdit::clearMarksForTextCursor()
{
    bool bFind = false;
    QTextCursor cursor;
    QTextCursor textcursor = textCursor();

    for (int i = m_wordMarkSelections.size() - 1; i >= 0; --i) {
        cursor = m_wordMarkSelections.at(i).first.cursor;
        if (textcursor.hasSelection()) {
            if (textcursor == cursor) {
                bFind = true;
                clearMarkOperationForCursor(cursor);
                m_wordMarkSelections.removeAt(i);
                break;
            }

        } else {
            if (textcursor.position() >= cursor.selectionStart() && textcursor.position() <= cursor.selectionEnd()) {
                bFind = true;
                clearMarkOperationForCursor(cursor);
                m_wordMarkSelections.removeAt(i);
            }
        }
    }

    return bFind;
}

void TextEdit::toggleMarkSelections()
{
    if (!clearMarksForTextCursor()) {
        ColorSelectWdg *pColorSelectWdg = static_cast<ColorSelectWdg *>(m_actionColorStyles->defaultWidget());
        isMarkCurrentLine(true, pColorSelectWdg->getDefaultColor().name());
    }

    renderAllSelections();
}

/**
 * @brief 转换标记项替换信息 \a replaceInfo 为标记项信息，标记项替换信息包含了光标的绝对位置，
 *      在转换过程中，光标会更新当前选中区域为绝对位置
 * @param replaceInfo 标记替换信息
 * @return 转换后的标记操作项列表
 */
QList<QPair<TextEdit::MarkOperation, qint64> > TextEdit::convertReplaceToMark(const QList<TextEdit::MarkReplaceInfo> &replaceInfo)
{
    QList<QPair<TextEdit::MarkOperation, qint64> > markList;
    for (auto info : replaceInfo) {
        MarkOperation markOpt = info.opt;

        // 更新当前光标选中信息
        markOpt.cursor.setPosition(info.start);
        markOpt.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, info.end - info.start);

        markList.append(qMakePair(markOpt, info.time));
    }

    return markList;
}

/**
 * @brief 转换标记操作项 \a markInfo 为标记项替换信息，标记项替换信息包含了光标的绝对位置，
 *      在转换过程中，会记录当前选中区域为绝对位置
 * @param markInfo 标记信息
 * @return 转换后的标记替换信息列表
 */
QList<TextEdit::MarkReplaceInfo> TextEdit::convertMarkToReplace(const QList<QPair<TextEdit::MarkOperation, qint64> > &markInfo)
{
    QList<TextEdit::MarkReplaceInfo> replaceMarkList;
    for (auto info : markInfo) {
        MarkReplaceInfo replace;
        replace.opt = info.first;
        replace.time = info.second;

        // 记录当前光标位置，QTextDocument后续操作文本到QTextCursor光标范围时，会导致QTextCursor记录的光标位置失效
        replace.start = info.first.cursor.selectionStart();
        replace.end = info.first.cursor.selectionEnd();

        replaceMarkList.append(replace);
    }

    return replaceMarkList;
}

/**
 * @brief 更新所有的标记信息为 \a markInfo , 用于撤销项处理颜色标记变更
 * @param markInfo 颜色标记信息列表
 */
void TextEdit::manualUpdateAllMark(const QList<QPair<MarkOperation, qint64> > &markInfo)
{
    m_markOperations = markInfo;

    // 全部标记更新后，修改手动标记文本部分
    m_wordMarkSelections.clear();
    // 用于将跨行的单行颜色标记拓展为多行处理
    QList<QPair<TextEdit::MarkOperation, qint64>> multiLineSelections;

    for (auto itr = m_markOperations.begin(); itr != m_markOperations.end();) {
        if (MarkAll == (*itr).first.type
                || MarkAllMatch == (*itr).first.type) {
            ++itr;
            continue;
        }

        // 若无选中项，则过滤此颜色标记
        if (!(*itr).first.cursor.hasSelection()) {
            itr = m_markOperations.erase(itr);
            continue;
        }

        auto &info = *itr;
        if (MarkOnce == info.first.type) {
            QTextEdit::ExtraSelection selection;
            selection.format.setBackground(QColor(info.first.color));
            selection.cursor = info.first.cursor;

            // 更新单独文本标记
            m_wordMarkSelections.append(qMakePair(selection, info.second));

        } else if (MarkLine == info.first.type) {
            int selectStart = info.first.cursor.selectionStart();
            int selectEnd = info.first.cursor.selectionEnd();
            QTextBlock startBlock = document()->findBlock(selectStart);
            QTextBlock endBlock = document()->findBlock(selectEnd);

            // 判断是否为多行颜色标记
            bool isMultiLine = startBlock < endBlock;

            while (startBlock.isValid()
                    && endBlock.isValid()
                    && !(endBlock < startBlock)) {
                QTextEdit::ExtraSelection selection;
                selection.format.setBackground(QColor(info.first.color));
                selection.cursor = info.first.cursor;
                // 更新单行标记的索引区间, 注意标记单行颜色不包括换行符
                selection.cursor.setPosition(startBlock.position());
                selection.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, startBlock.length() - 1);

                // 更新单行文本标记，允许使用一样的时间戳
                m_wordMarkSelections.append(qMakePair(selection, info.second));

                // 追加拆解的多行颜色标记信息，允许使用一样的时间戳
                if (isMultiLine) {
                    auto copyInfo = info;
                    copyInfo.first.cursor = selection.cursor;
                    multiLineSelections.append(copyInfo);
                }

                // 继续下一文本块处理
                startBlock = startBlock.next();
            }

            // 若为多行颜色标记，会将当前文本颜色标记移除，后续添加为多行颜色标记
            if (isMultiLine) {
                itr = m_markOperations.erase(itr);
                continue;
            }
        }

        // 继续下一遍历
        ++itr;
    }

    // 追加多行颜色标记处理记录
    m_markOperations.append(multiLineSelections);

    // 对修改后的颜色标记进行排序
    std::sort(m_markOperations.begin(), m_markOperations.end(),
    [](const QPair<TextEdit::MarkOperation, qint64> &a, const QPair<TextEdit::MarkOperation, qint64> &b) {
        return a.second < b.second;
    });
    std::sort(m_wordMarkSelections.begin(), m_wordMarkSelections.end(),
    [](const QPair<QTextEdit::ExtraSelection, qint64> &a, const QPair<QTextEdit::ExtraSelection, qint64> &b) {
        return a.second < b.second;
    });

    // 计算全文标记部分并刷新界面颜色标记
    markAllKeywordInView();
}

/**
 * @brief 查询颜色标记范围内存在多少组替换文本的函数，对 \a markStart ~ \a markEnd 区间进行搜索，
 *      判断 \a posList 中是否存在已查询到的替换文本位置索引，并将这些位置索引转换为索引区间列表返回。
 * @param posList       已查询的替换文本位置索引
 * @param markStart     颜色标记开始位置索引
 * @param markEnd       颜色标记结束位置索引
 * @param replaceText   替换的文本
 * @return 查找到和颜色标记位置有交叉的文本位置索引区间
 */
static QPair<int, int> findMatchRange(const QList<int> &posList, int markStart, int markEnd, const QString &replaceText)
{
    // 判断颜色标记 info 范围内是否包含替换文本索引信息
    QPair<int, int> foundPosRange {-1, -1};
    if (posList.isEmpty()) {
        return foundPosRange;
    }

    // 将颜色标记搜索范围向左延伸 replaceText.size() - 1 位置()，若此区间出现replaceText, 那么必定和 markStart ~ markEnd 相交
    int adjustMarkStart = markStart - replaceText.size();

    // 获取最近的左侧查找文本位置索引，使用 qUpperBound, 索引必须大于 adjustMarkStart
    auto leftfindItr = qUpperBound(posList.begin(), posList.end(), adjustMarkStart);
    if (leftfindItr != posList.end()
            && (*leftfindItr) < markEnd) {
        // 设置查询的左边界
        foundPosRange.first = static_cast<int>(std::distance(posList.begin(), leftfindItr));
    }

    // 获取最近的右侧查找文本位置索引（小于右边界 markEnd）
    auto rightFindItr = qLowerBound(posList.rbegin(), posList.rend(), markEnd - 1, std::greater<int>());
    if (rightFindItr != posList.rend()
            && markStart < (*rightFindItr)) {
        // 设置右边界
        foundPosRange.second = static_cast<int>(std::distance(rightFindItr, posList.rend() - 1));
    } else if (-1 != foundPosRange.first) {
        foundPosRange.second = foundPosRange.first;
    }

    return foundPosRange;
}

/**
 * @brief 根据当前已查找到的替换文本偏移位置和颜色标记的交叉范围，调整颜色标记的位置
 * @param foundPosList  已查询的替换文本位置索引
 * @param info          需要计算的颜色标记
 * @param replaceText   被替换的文本
 * @param withText      替换后的文本
 */
static void updateMarkReplaceRange(const QList<int> &foundPosList, TextEdit::MarkReplaceInfo &info, const QString &replaceText, const QString &withText)
{
    // 文本替换长度变更调整量
    int adjustlen = withText.size() - replaceText.size();
    // 获取替换文本位置索引列表
    QPair<int, int> posIndexRange = findMatchRange(foundPosList, info.start, info.end, replaceText);

    if (-1 == posIndexRange.first
            && -1 == posIndexRange.second) {
        // 颜色标记内不包含替换文本，和替换文本无交集
        // 根据当前查询位置判断 foundPosList[findCount - 1] 方向
        if (!foundPosList.isEmpty()
                && info.end <= foundPosList.last()) {
            // 处于颜色标记右侧，减去右侧替换文本的偏移量
            info.start += (foundPosList.size() - 1) * adjustlen;
            info.end += (foundPosList.size() - 1) * adjustlen;
        } else {
            // 处于颜色标记左侧
            info.start += foundPosList.size() * adjustlen;
            info.end += foundPosList.size() * adjustlen;
        }
    } else {
        // 判断左侧是否有交集，拓展颜色标记到替换文本
        if (-1 != posIndexRange.first
                && foundPosList[posIndexRange.first] <= info.start) {
            // 存在交叉，调整位置到左侧替换位置起始边界
            info.start = foundPosList[posIndexRange.first] + (posIndexRange.first * adjustlen);
        } else {
            info.start += posIndexRange.first * adjustlen;
        }

        // 判断右侧是否有交集，拓展颜色标记到替换文本
        if (-1 != posIndexRange.second
                && info.end < (foundPosList[posIndexRange.second] + replaceText.size())) {
            // 存在交叉，调整位置到右侧替换后文本的边界
            info.end = foundPosList[posIndexRange.second] + (posIndexRange.second * adjustlen) + withText.size();
        } else {
            info.end += (posIndexRange.second + 1) * adjustlen;
        }
    }
}

/**
 * @brief 根据传入的颜色标记信息 \a replaceList 计算文本 \a oldText 替换后颜色标记的位置变化
 *      在文本替换过程前，记录当前标记光标位置，遍历需要替换的文本位置，记录需要修改的颜色标记位置变更。
 * @param replaceList   颜色标记替换信息
 * @param oldText       需要替换的文本内容
 * @param replaceText   被替换的文本
 * @param withText      替换后的文本
 * @param offset        非全文替换时使用，用于补充文本缺失部分偏移量
 *
 * @note 文本替换颜色标记处理
 * @li MarkOnce 单个文本颜色标记
 *      1. 替换文本覆盖了颜色标记内容，移除颜色标记
 *      2. 替换文本和单个颜色标记内容存在交叉，包括替换文本处于颜色标记内，将颜色标记拓展到替换文本，替换文本的背景色显示为颜色标记背景色
 *      3. 若替换文本内包含多个颜色标记，每个颜色标记均拓展到替换文本，颜色标记显示层级按设置时间排列
 * @li MarkLine 单行文本颜色标记
 *      1. 同单个颜色标记处理
 * @li MarkAllMatch 单个文本全文查找颜色标记
 *      1. 替换前后不进行特殊处理
 * @li MarkAll 全文颜色标记
 *      1. 替换前后不进行特殊处理
 */
void TextEdit::calcMarkReplaceList(QList<TextEdit::MarkReplaceInfo> &replaceList, const QString &oldText,
                                   const QString &replaceText, const QString &withText, int offset, Qt::CaseSensitivity caseFlag) const
{
    // 当前替换项为空或相同，退出
    if (replaceList.isEmpty()
            || replaceText == withText) {
        return;
    }

    // 将传入的替换列表排序，按光标位置先后顺序进行排列
    std::sort(replaceList.begin(), replaceList.end(), [](const MarkReplaceInfo & a, const MarkReplaceInfo & b) {
        return a.start < b.start;
    });

    // 当前标记的索引，后续使用 updateMarkRange() 初始化，设置为-1
    int currentMarkIndex = -1;
    // 临时缓存的当前标记的索引范围
    QPair<int, int> curMarkRange {0, 0};
    // 更新当前标记替换项索引和其对应的标记范围
    auto updateMarkIndexAndRange = [&]() {
        currentMarkIndex++;
        while (currentMarkIndex >= 0 && currentMarkIndex < replaceList.size()) {
            auto &markInfo = replaceList.at(currentMarkIndex);
            // 标记类型为标记全文或文本全文标记(使用文本查找而非光标位置)，不进行替换处理
            if (MarkAllMatch == markInfo.opt.type
                    || MarkAll == markInfo.opt.type) {
                currentMarkIndex++;
                continue;
            }

            curMarkRange = qMakePair(markInfo.start, markInfo.end);
            break;
        }
    };
    updateMarkIndexAndRange();

    // 查找统计及已查找偏移量
    int findOffset = 0;
    QList<int> foundPosList;

    // 查找替换位置，遍历查找替换文本出现位置
    int findPos = oldText.indexOf(replaceText, findOffset, caseFlag);
    // 需要取得左侧所有的变更相对偏移，从文本左侧开始循环遍历
    while (-1 != findPos
            && currentMarkIndex < replaceList.size()) {
        // 转换为相对全文的偏移
        int realPos = findPos + offset;
        // 记录已查询的索引位置
        foundPosList.append(realPos);

        // 获取替换文本区域和颜色标记区域的交叉关系
        Utils::RegionIntersectType type = Utils::checkRegionIntersect(realPos, realPos + replaceText.size(),
                                                                      curMarkRange.first, curMarkRange.second);

        // 判断是否处于查询范围内，处于右侧区间则后续处理
        while (Utils::ERight != type) {
            auto &info = replaceList[currentMarkIndex];

            bool checkNext = false;
            switch (type) {
            case Utils::EIntersectInner: {
                // 替换文本内容包含标记信息, 取消当前文本标记（无论单个文本还是单行文本，均移除）
                // 在 manualUpdateAllMark() 函数处理会移除此标记
                info.start = 0;
                info.end = 0;
                checkNext = true;
                break;
            }
            case Utils::ELeft: {
                // 当前无交集，颜色标记在替换文本左侧，表示当前颜色标记已经经过
                updateMarkReplaceRange(foundPosList, info, replaceText, withText);

                checkNext = true;
                break;
            }
            // 其它交叉类型需要等待颜色标记区间内所有替换文本均查找后处理，单个颜色标记中可能含多个替换文本
            default:
                break;
            }

            if (checkNext) {
                // 更新当前计算的颜色标记和范围
                updateMarkIndexAndRange();
                // 判断颜色标记是否计算完成
                if (currentMarkIndex == replaceList.size()) {
                    break;
                }

                // 继续查找下一颜色标记和当前查询位置的交叉范围
                type = Utils::checkRegionIntersect(realPos, realPos + replaceText.size(), curMarkRange.first, curMarkRange.second);
            } else {
                break;
            }
        }

        if (currentMarkIndex == replaceList.size()) {
            break;
        }

        // 继续查找替换文本位置
        findOffset = findPos + replaceText.size();
        findPos = oldText.indexOf(replaceText, findOffset, caseFlag);
    }

    // 继续处理剩余颜色标记偏移
    while (currentMarkIndex != replaceList.size()) {
        // 将后续未处理到的颜色标记调整偏移量
        auto &info = replaceList[currentMarkIndex];
        updateMarkReplaceRange(foundPosList, info, replaceText, withText);

        updateMarkIndexAndRange();
    }
}

void TextEdit::markSelectWord()
{
    bool isFind  = false;
    for (int i = 0 ; i < m_wordMarkSelections.size(); ++i) {
        QTextCursor curson = m_wordMarkSelections.at(i).first.cursor;
        curson.movePosition(QTextCursor::EndOfLine);
        QTextCursor currentCurson = textCursor();
        currentCurson.movePosition(QTextCursor::EndOfLine);
        //if (m_wordMarkSelections.at(i).cursor == textCursor()) {
        if (curson == currentCurson) {
            isFind = true;
            m_wordMarkSelections.removeAt(i);
            renderAllSelections();
            break;
        }
    }
    if (!isFind) {
        //添加快捷键标记颜色
        ColorSelectWdg *pColorSelectWdg = static_cast<ColorSelectWdg *>(m_actionColorStyles->defaultWidget());
        isMarkCurrentLine(true, pColorSelectWdg->getDefaultColor().name());
        renderAllSelections();
    }
}

void TextEdit::updateMark(int from, int charsRemoved, int charsAdded)
{
    //只读模式下实现禁止语音输入的效果
    if (m_readOnlyMode) {
        //undo();
        return;
    }

    //如果是读取文件导致的文本改变
    if (m_bIsFileOpen) {
        return;
    }

    //更新标记
    int nStartPos = 0,///< 标记的起始位置
        nEndPos = 0,///< 标记的结束位置
        nCurrentPos = 0;///< 当前光标位置
    QTextEdit::ExtraSelection selection;///< 指定文本格式
    QList<QTextEdit::ExtraSelection> listSelections;///< 指定文本格式列表
    QList<QPair<QTextEdit::ExtraSelection, qint64>> wordMarkSelections = m_wordMarkSelections;///< 标记列表
    QColor strColor;///< 指定文本颜色格式
    nCurrentPos = textCursor().position();

    //如果是删除字符
    if (charsRemoved > 0) {
        QList<int> listRemoveItem;///< 要移除标记的indexs

        //寻找要移除标记的index
        for (int i = 0; i < wordMarkSelections.count(); i++) {

            nEndPos = wordMarkSelections.value(i).first.cursor.selectionEnd();
            nStartPos = wordMarkSelections.value(i).first.cursor.selectionStart();
            strColor = wordMarkSelections.value(i).first.format.background().color();

            //如果有文字被选择
            if (m_nSelectEndLine != -1) {

                //如果删除的内容，完全包含标记内容
                if (m_nSelectStart <= nStartPos && m_nSelectEnd >= nEndPos) {
                    listRemoveItem.append(i);
                }
            } else {

                //如果标记内容全部被删除
                if (wordMarkSelections.value(i).first.cursor.selection().toPlainText().isEmpty()) {
                    // 此时 remove m_wordMarkSelections中的元素
                    // 会造成 m_wordMarkSelections 和 wordMarkSelections的长度不一致
                    // m_wordMarkSelections.removeAt(i);
                    listRemoveItem.append(i);
                }
            }
        }

        //从标记列表中移除标记
        for (int j = 0; j < listRemoveItem.count(); j++) {
            // 不应该-j
            // m_wordMarkSelections.removeAt(listRemoveItem.value(j) - j);
            m_wordMarkSelections.removeAt(listRemoveItem.value(j));
        }
    }

    //如果是添加字符
    if (charsAdded > 0) {
        for (int i = 0; i < wordMarkSelections.count(); i++) {
            nEndPos = wordMarkSelections.value(i).first.cursor.selectionEnd();
            nStartPos = wordMarkSelections.value(i).first.cursor.selectionStart();
            strColor = wordMarkSelections.value(i).first.format.background().color();
            qint64 timeStamp = wordMarkSelections.value(i).second;

            //如果字符添加在标记中
            if (nCurrentPos > nStartPos && nCurrentPos < nEndPos) {

                m_wordMarkSelections.removeAt(i);
                selection.format.setBackground(strColor);
                selection.cursor = textCursor();

                QTextEdit::ExtraSelection preSelection;

                //如果是输入法输入
                if (m_bIsInputMethod) {

                    //添加第一段标记
                    selection.cursor.setPosition(nStartPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nCurrentPos - m_qstrCommitString.count(), QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(i, QPair<QTextEdit::ExtraSelection, qint64>
                                                (selection, timeStamp));

                    preSelection.cursor = selection.cursor;
                    preSelection.format = selection.format;

                    //添加第二段标记
                    selection.cursor.setPosition(nCurrentPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nEndPos, QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(i + 1,  QPair<QTextEdit::ExtraSelection, qint64>
                                                (selection, timeStamp));

                    m_bIsInputMethod = false;
                } else {

                    //添加第一段标记
                    selection.cursor.setPosition(nStartPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(from, QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(i, QPair<QTextEdit::ExtraSelection, qint64>
                                                (selection, timeStamp));

                    preSelection.cursor = selection.cursor;
                    preSelection.format = selection.format;

                    //添加第二段标记
                    selection.cursor.setPosition(nCurrentPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nEndPos, QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(i + 1, QPair<QTextEdit::ExtraSelection, qint64>
                                                (selection, timeStamp));
                }

                bool bIsFind = false;

                //在记录标记的表中替换（按标记动作记录）
                for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                    auto list = m_mapWordMarkSelections.value(j);
                    for (int k = 0; k < list.count(); k++) {
                        if (list.value(k).cursor == wordMarkSelections.value(i).first.cursor
                                && list.value(k).format == wordMarkSelections.value(i).first.format) {
                            list.removeAt(k);
                            listSelections = list;
                            listSelections.insert(k, preSelection);
                            listSelections.insert(k + 1, selection);
                            bIsFind = true;
                            break;
                        }
                    }

                    if (bIsFind) {
                        m_mapWordMarkSelections.remove(j);
                        m_mapWordMarkSelections.insert(j, listSelections);
                        break;
                    }
                }
                break;

            } else if (nCurrentPos == nEndPos) { //如果字符添加在标记后
                m_wordMarkSelections.removeAt(i);
                selection.format.setBackground(strColor);
                selection.cursor = textCursor();

                if (m_bIsInputMethod) {
                    selection.cursor.setPosition(nStartPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nEndPos - m_qstrCommitString.count(), QTextCursor::KeepAnchor);
                    m_bIsInputMethod = false;
                } else {
                    selection.cursor.setPosition(nStartPos, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(from + charsAdded, QTextCursor::KeepAnchor);
                }

                m_wordMarkSelections.insert(i, QPair<QTextEdit::ExtraSelection, qint64>
                                            (selection, timeStamp));

                bool bIsFind = false;
                for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                    auto list = m_mapWordMarkSelections.value(j);
                    for (int k = 0; k < list.count(); k++) {
                        if (list.value(k).cursor == wordMarkSelections.value(i).first.cursor
                                && list.value(k).format == wordMarkSelections.value(i).first.format) {
                            list.removeAt(k);
                            listSelections = list;
                            listSelections.insert(k, selection);
                            bIsFind = true;
                            break;
                        }
                    }

                    if (bIsFind) {
                        m_mapWordMarkSelections.remove(j);
                        m_mapWordMarkSelections.insert(j, listSelections);
                        break;
                    }
                }
                break;
            }
        }
    }

    //渲染所有的指定字符格式
    renderAllSelections();

    highlight();
}

void TextEdit::setCursorStart(int pos)
{
    m_cursorStart = pos;
}


void TextEdit::completionWord(QString word)
{
    QString wordAtCursor = getWordAtCursor();
    QTextCursor cursor = textCursor();

    QString completionString = word.remove(0, wordAtCursor.size());
    if (completionString.size() > 0) {
        cursor = textCursor();
        cursor.insertText(completionString);
        setTextCursor(cursor);
    }
}

bool TextEdit::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
        QMouseEvent *event2 = static_cast<QMouseEvent *>(event);
        DPlainTextEdit::mousePressEvent(event2);
        if (touchPoints.count() == 3) {
            slot_translate();
        }
        break;
    }
    default: break;
    }
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        m_mouseClickPos = mouseEvent->pos();

        if (object == m_pLeftAreaWidget->m_pBookMarkArea) {
            m_mouseClickPos = mouseEvent->pos();
            if (mouseEvent->button() == Qt::RightButton) {
                m_rightMenu->clear();
                m_rightMenu->deleteLater();
                m_rightMenu = nullptr;
                m_rightMenu = new DMenu(this);
                connect(m_rightMenu, &DMenu::destroyed, this, &TextEdit::removeHighlightWordUnderCursor);
                int line = getLineFromPoint(mouseEvent->pos());
                if (m_listBookmark.contains(line)) {
                    m_rightMenu->addAction(m_cancelBookMarkAction);
                    if (m_listBookmark.count() > 1) {
                        m_rightMenu->addAction(m_preBookMarkAction);
                        m_rightMenu->addAction(m_nextBookMarkAction);
                    }
                } else {
                    m_rightMenu->addAction(m_addBookMarkAction);
                }

                if (!m_listBookmark.isEmpty()) {
                    m_rightMenu->addAction(m_clearBookMarkAction);
                }

                m_rightMenu->exec(mouseEvent->globalPos());
            } else {
                addOrDeleteBookMark();
            }
            return true;
        } else if (object == m_pLeftAreaWidget->m_pFlodArea) {
            m_foldCodeShow->hide();
            if (mouseEvent->button() == Qt::LeftButton) {
                int line = getLineFromPoint(mouseEvent->pos());
                m_markFoldHighLightSelections.clear();
                renderAllSelections();

                //不同类型文件注释符号不同 梁卫东　２０２０－０９－０３　１７：２８：４５
                bool bHasCommnent = false;
                QString multiLineCommentMark;
                QString singleLineCommentMark;

                if (m_commentDefinition.isValid()) {
                    multiLineCommentMark = m_commentDefinition.multiLineStart.trimmed();
                    singleLineCommentMark = m_commentDefinition.singleLine.trimmed();
                    //qDebug()<<"CommentMark:"<<multiLineCommentMark<<singleLineCommentMark;
                    //判断是否包含单行或多行注释
                    if (!multiLineCommentMark.isEmpty()) bHasCommnent = document()->findBlockByNumber(line - 1).text().trimmed().startsWith(multiLineCommentMark);
                    if (!singleLineCommentMark.isEmpty()) bHasCommnent = document()->findBlockByNumber(line - 1).text().trimmed().startsWith(singleLineCommentMark);
                } else {
                    bHasCommnent = false;
                }

                // 当前行line-1 判断下行line是否隐藏
                if (document()->findBlockByNumber(line).isVisible() && document()->findBlockByNumber(line - 1).text().contains("{") && !bHasCommnent) {
                    getNeedControlLine(line - 1, false);
                    document()->adjustSize();

                    //折叠时出现点击光标选择行变短
                    QPlainTextEdit::LineWrapMode curMode = this->lineWrapMode();
                    QPlainTextEdit::LineWrapMode WrapMode = curMode ==  QPlainTextEdit::NoWrap ?  QPlainTextEdit::WidgetWidth :  QPlainTextEdit::NoWrap;          this->setWordWrapMode(QTextOption::WrapAnywhere);
                    this->setLineWrapMode(WrapMode);
                    viewport()->update();
                    m_pLeftAreaWidget->updateBookMark();
                    m_pLeftAreaWidget->updateCodeFlod();
                    m_pLeftAreaWidget->updateLineNumber();
                    this->setLineWrapMode(curMode);
                    viewport()->update();

                } else if (!document()->findBlockByNumber(line).isVisible() && document()->findBlockByNumber(line - 1).text().contains("{") && !bHasCommnent) {
                    getNeedControlLine(line - 1, true);
                    document()->adjustSize();

                    //折叠时出现点击光标选择行变短
                    QPlainTextEdit::LineWrapMode curMode = this->lineWrapMode();
                    QPlainTextEdit::LineWrapMode WrapMode = curMode ==  QPlainTextEdit::NoWrap ?  QPlainTextEdit::WidgetWidth :  QPlainTextEdit::NoWrap;          this->setWordWrapMode(QTextOption::WrapAnywhere);
                    this->setLineWrapMode(WrapMode);
                    viewport()->update();
                    m_pLeftAreaWidget->updateBookMark();
                    m_pLeftAreaWidget->updateCodeFlod();
                    m_pLeftAreaWidget->updateLineNumber();
                    this->setLineWrapMode(curMode);
                    viewport()->update();
                } else {
                    //其他不做处理
                }

            } else {
                m_mouseClickPos = mouseEvent->pos();
                m_rightMenu->clear();
                m_rightMenu->deleteLater();
                m_rightMenu = nullptr;
                m_rightMenu = new DMenu(this);
                connect(m_rightMenu, &DMenu::destroyed, this, &TextEdit::removeHighlightWordUnderCursor);
                int line = getLineFromPoint(mouseEvent->pos());

                if (m_listFlodIconPos.contains(line - 1)) {
                    m_rightMenu->addAction(m_flodAllLevel);
                    m_rightMenu->addAction(m_unflodAllLevel);
                    m_rightMenu->addAction(m_flodCurrentLevel);
                    m_rightMenu->addAction(m_unflodCurrentLevel);
                }
                if (document()->findBlockByNumber(line).isVisible()) {
                    m_unflodCurrentLevel->setEnabled(false);
                    m_flodCurrentLevel->setEnabled(true);
                } else {
                    m_unflodCurrentLevel->setEnabled(true);
                    m_flodCurrentLevel->setEnabled(false);
                }

                //展开/折叠所有层次后，对应的菜单项置灰
                if (m_listMainFlodAllPos.count() == 0) {
                    m_unflodAllLevel->setEnabled(false);
                } else {
                    bool bExistVisible = false;
                    bool bExistInVisble = false;
                    for (int iLine : m_listMainFlodAllPos) {
                        if (document()->findBlockByNumber(iLine + 1).isVisible()) {
                            m_flodAllLevel->setEnabled(true);
                            bExistVisible = true;
                        } else {
                            m_unflodAllLevel->setEnabled(true);
                            bExistInVisble = true;
                        }

                        if (bExistVisible && bExistInVisble) {
                            break;
                        }
                    }
                    if (!bExistVisible) {
                        m_flodAllLevel->setEnabled(false);
                        m_unflodAllLevel->setEnabled(true);
                    }
                    if (!bExistInVisble) {
                        m_unflodAllLevel->setEnabled(false);
                        m_flodAllLevel->setEnabled(true);
                    }
                }

                m_rightMenu->exec(mouseEvent->globalPos());
            }

            return true;
        }

    } else if (event->type() == QEvent::HoverMove) {
        QHoverEvent *hoverEvent = static_cast<QHoverEvent *>(event);
        int line = getLineFromPoint(hoverEvent->pos());

        if (object == m_pLeftAreaWidget->m_pBookMarkArea) {
            m_nBookMarkHoverLine = line;
            m_pLeftAreaWidget->m_pBookMarkArea->update();
        } else if (object == m_pLeftAreaWidget->m_pFlodArea) {
            m_markFoldHighLightSelections.clear();
            renderAllSelections();

            if (m_listFlodIconPos.contains(line - 1)) {
                if (!document()->findBlockByNumber(line).isVisible()) {
                    m_foldCodeShow->clear();
                    m_foldCodeShow->setStyle(lineWrapMode());//enum LineWrapMode {NoWrap,WidgetWidth};
                    getHideRowContent(line - 1);
                    m_foldCodeShow->show();
                    m_foldCodeShow->hideFirstBlock();
                    m_foldCodeShow->move(5, getLinePosYByLineNum(line));
                } else {
                    QTextCursor previousCursor = textCursor();
                    int ivalue = this->verticalScrollBar()->value();
                    int iLine = getHighLightRowContentLineNum(line - 1);

                    if (line - 1 == iLine)   return false;

                    QTextEdit::ExtraSelection selection;
                    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

                    QTextBlock startblock;
                    QTextBlock endblock = document()->findBlockByNumber(iLine);
                    startblock = document()->findBlockByNumber(line - 1);

                    QTextCursor beginCursor(startblock);
                    beginCursor.setPosition(startblock.position() + startblock.text().indexOf("{"), QTextCursor::MoveAnchor);


                    beginCursor.setPosition(endblock.position() + endblock.text().indexOf("}") + 1, QTextCursor::KeepAnchor);

                    if (iLine == document()->blockCount() - 1)
                        beginCursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

                    setTextCursor(beginCursor);
                    selection.cursor = textCursor();

                    QColor highlightBackground = DGuiApplicationHelper::instance()->applicationPalette().color(QPalette::Highlight);
                    highlightBackground.setAlphaF(0.4);
                    selection.format.setBackground(highlightBackground);
                    m_markFoldHighLightSelections.push_back(selection);

                    renderAllSelections();
                    setTextCursor(QTextCursor(document()->findBlockByNumber(line - 1)));
                    this->verticalScrollBar()->setValue(ivalue);
                }
            } else {
                m_foldCodeShow->hide();
            }

            return true;
        }

    } else if (event->type() == QEvent::HoverLeave) {
        if (object == m_pLeftAreaWidget->m_pBookMarkArea) {
            m_nBookMarkHoverLine = -1;
            m_pLeftAreaWidget->m_pBookMarkArea->update();
            return true;
        } else if (object == m_pLeftAreaWidget->m_pFlodArea) {
            m_markFoldHighLightSelections.clear();
            m_foldCodeShow->hide();
            renderAllSelections();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        m_bIsDoubleClick = true;
        m_bBeforeIsDoubleClick = true;
    } else if (object == m_colorMarkMenu) {
        // 进行对于 color mark menu 的特殊按键处理
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            // 当前仅处理 Tab 键 ， 保留后续其他按键的处理
            if (keyEvent->key() == Qt::Key_Tab) {
                // handleKey 作用是保留后续增加方向键处理
                int handleKey = keyEvent->key();

                // 使用Timer进行后续处理，避免在此处处理后，和基类的处理造成混淆
                QTimer::singleShot(0, this, [ = ]() {
                    // 仅当 menu 可见时进行处理，当前仅处理Tab键
                    if (m_colorMarkMenu->isVisible() && handleKey == Qt::Key_Tab) {
                        QAction *currentAction = m_colorMarkMenu->activeAction();
                        QAction *nextAction = nullptr;
                        int currentIndex = -1;

                        // 如果没有当前 focus item ， 不需要进行处理
                        if (currentAction == nullptr) {
                            return;
                        }

                        // 遍历 Order List， 如果不符合 order list 规则，变更focus item
                        for (int i = 0; i < m_MarkColorMenuTabOrder.size(); i++) {
                            QPair<QAction *, bool> orderItem = m_MarkColorMenuTabOrder.at(i);

                            if (orderItem.first == currentAction) {
                                // 设置当前 item 位置
                                currentIndex = i;

                                if (orderItem.second == true) {
                                    // 允许 focus， 不进行处理，退出
                                    return;
                                } else {
                                    // 退出当前查找循环，进行后续处理
                                    break;
                                }
                            }
                        }

                        if (currentIndex == -1) {
                            qWarning() << " can not find current item in tab order list , need check 'Mark Color Menu' tab order setting!";
                            return;
                        }

                        // 在 order list中，寻找下一个item
                        for (int j = currentIndex + 1; j < m_MarkColorMenuTabOrder.size(); j++) {
                            QPair<QAction *, bool> newOrderItem = m_MarkColorMenuTabOrder.at(j);

                            if (newOrderItem.second == true) {
                                nextAction = newOrderItem.first;
                                break;
                            }
                        }

                        // 原 order list 中不存在后一个可用item， 从列表头开始头查找
                        if (nextAction == nullptr) {
                            // 从头进行查找，因为i为当前位置，查找到i就足够
                            for (int j = 0; j < currentIndex; j++) {
                                QPair<QAction *, bool> newOrderItem = m_MarkColorMenuTabOrder.at(j);

                                if (newOrderItem.second == true) {
                                    nextAction = newOrderItem.first;
                                    break;
                                }
                            }
                        }

                        // 为找到的新item设置focus
                        if (nextAction != nullptr) {
                            m_colorMarkMenu->setActiveAction(nextAction);
                        } else {
                            qWarning() << " can not find valid item , need check 'Mark Color Menu' tab order setting!";
                        }
                    }
                });
            }
        }
    }

    return QPlainTextEdit::eventFilter(object, event);
}

void TextEdit::adjustScrollbarMargins()
{
    QEvent event(QEvent::LayoutRequest);
    QApplication::sendEvent(this, &event);

    QMargins margins = viewportMargins();
    setViewportMargins(0, 0, 5, 0);
    setViewportMargins(margins);
    if (!verticalScrollBar()->visibleRegion().isEmpty()) {
        setViewportMargins(0, 0, 5, 0);        //-verticalScrollBar()->sizeHint().width()  原本的第三个参数
        //setViewportMargins(0, 0, -verticalScrollBar()->sizeHint().width(), 0);
    } else {
        setViewportMargins(0, 0, 5, 0);
    }
}

void TextEdit::slotSelectionChanged()
{
    if (textCursor().hasSelection()) {
        hideCursorBlink();
    } else {
        showCursorBlink();
    }
}

void TextEdit::slotCanRedoChanged(bool bCanRedo)
{
    Q_UNUSED(bCanRedo)
    bool isModified = this->m_wrapper->isTemFile() | (m_pUndoStack->canUndo() || m_pUndoStack->index() != m_lastSaveIndex);
    this->m_wrapper->window()->updateModifyStatus(m_sFilePath, isModified);
    this->m_wrapper->OnUpdateHighlighter();
}

void TextEdit::slotCanUndoChanged(bool bCanUndo)
{
    bool isModified = this->m_wrapper->isTemFile() | (bCanUndo || m_pUndoStack->index() != m_lastSaveIndex);
    this->m_wrapper->window()->updateModifyStatus(m_sFilePath, isModified);
    this->m_wrapper->OnUpdateHighlighter();
}

bool TextEdit::containsExtraSelection(QList<QTextEdit::ExtraSelection> listSelections, QTextEdit::ExtraSelection selection)
{
    for (int i = 0; i < listSelections.count(); i++) {
        if (listSelections.value(i).cursor == selection.cursor
                && listSelections.value(i).format == selection.format) {
            return true;
        }
    }
    return false;
}

void TextEdit::appendExtraSelection(QList<QTextEdit::ExtraSelection> wordMarkSelections
                                    , QTextEdit::ExtraSelection selection, QString strColor
                                    , QList<QTextEdit::ExtraSelection> *listSelections)
{
// 没有使用的方法，应该去除，降低维护成本
// 由于需要处理对应的单元测试，暂时不完全移除此函数，后期将统一进行处理
#if 1
    // 去除参数未使用警告
    Q_UNUSED(wordMarkSelections)
    Q_UNUSED(selection)
    Q_UNUSED(strColor)
    Q_UNUSED(listSelections)
#else
    //如果文档中有标记
    if (wordMarkSelections.count() > 0) {
        bool bIsContains = false;///< 是否占用已有标记
        int nWordMarkSelectionStart = 0,///< 已有标记起始位置
            nSelectionStart = 0,///< 标记起始位置
            nWordMarkSelectionEnd = 0,///< 已有标记结束位置
            nSelectionEnd = 0;///< 标记结束位置

        //按大小确定标记的起始和结束位置
        if (selection.cursor.selectionStart() > selection.cursor.selectionEnd()) {
            nSelectionStart = selection.cursor.selectionEnd();
            nSelectionEnd = selection.cursor.selectionStart();
        } else {
            nSelectionStart = selection.cursor.selectionStart();
            nSelectionEnd = selection.cursor.selectionEnd();
        }

        for (int i = 0; i < wordMarkSelections.count(); i++) {

            //按大小确定已有标记的起始和结束位置
            if (wordMarkSelections.value(i).cursor.selectionStart() > wordMarkSelections.value(i).cursor.selectionEnd()) {
                nWordMarkSelectionStart = wordMarkSelections.value(i).cursor.selectionEnd();
                nWordMarkSelectionEnd = wordMarkSelections.value(i).cursor.selectionStart();
            } else {
                nWordMarkSelectionStart = wordMarkSelections.value(i).cursor.selectionStart();
                nWordMarkSelectionEnd = wordMarkSelections.value(i).cursor.selectionEnd();
            }

            //如果被已有标记包含
            if ((nWordMarkSelectionStart <= nSelectionStart && nWordMarkSelectionEnd > nSelectionEnd)
                    || (nWordMarkSelectionStart < nSelectionStart && nWordMarkSelectionEnd >= nSelectionEnd)) {

                bIsContains = true;
                selection.format.setBackground(QColor(strColor));

                //如果标记格式不相同
                if (wordMarkSelections.value(i).format != selection.format) {
                    int nRemPos = 0;///< 标记插入位置

                    //移除已有标记
                    for (int j = 0; j < wordMarkSelections.count(); j++) {
                        if (m_wordMarkSelections.value(j).cursor == wordMarkSelections.value(i).cursor
                                && m_wordMarkSelections.value(j).format == wordMarkSelections.value(i).format) {

                            m_wordMarkSelections.removeAt(j);
                            nRemPos = j;
                            break;
                        }
                    }

                    //重新记录标记
                    selection.cursor.setPosition(nWordMarkSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::KeepAnchor);
                    selection.format.setBackground(wordMarkSelections.value(i).format.background());

                    bool bIsInsert = false;///< 标记是否将原有标记分成两段

                    //如果第一段存在
                    if (selection.cursor.selectedText() != "") {
                        bIsInsert = true;
                        m_wordMarkSelections.insert(nRemPos, selection);
                    }

                    QTextEdit::ExtraSelection preSelection;
                    preSelection.format = selection.format;
                    preSelection.cursor = selection.cursor;

                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nWordMarkSelectionEnd, QTextCursor::KeepAnchor);

                    //如果第二段存在
                    if (selection.cursor.selectedText() != "") {
                        if (bIsInsert) {
                            m_wordMarkSelections.insert(nRemPos + 1, selection);
                        } else {
                            m_wordMarkSelections.insert(nRemPos, selection);
                        }
                    }

                    //从记录标记的表中替换原有标记（按标记动作记录）
                    QList<QTextEdit::ExtraSelection> selecList;
                    bool bIsFind = false;///< 是否有包含该标记的标记动作

                    for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                        auto list = m_mapWordMarkSelections.value(j);

                        for (int k = 0; k < list.count(); k++) {
                            if (list.value(k).cursor == wordMarkSelections.value(i).cursor
                                    && list.value(k).format == wordMarkSelections.value(i).format) {

                                list.removeAt(k);
                                selecList = list;
                                bIsInsert = false;

                                if (preSelection.cursor.selectedText() != "") {
                                    bIsInsert = true;
                                    selecList.insert(k, preSelection);
                                }

                                if (selection.cursor.selectedText() != "") {
                                    if (bIsInsert) {
                                        selecList.insert(k + 1, selection);
                                    } else {
                                        selecList.insert(k, selection);
                                    }
                                }

                                bIsFind = true;
                                break;
                            }
                        }

                        if (bIsFind) {
                            m_mapWordMarkSelections.remove(j);
                            m_mapWordMarkSelections.insert(j, selecList);
                            break;
                        }
                    }

                    //记录新添加的标记
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::KeepAnchor);
                    selection.format.setBackground(QColor(strColor));
                    m_wordMarkSelections.append(selection);
                    listSelections->append(selection);
                }
            } else if (nWordMarkSelectionStart >= nSelectionStart && nWordMarkSelectionEnd <= nSelectionEnd) { //如果标记包含已有标记
                bIsContains = true;
                selection.format.setBackground(QColor(strColor));

                //移除已有标记
                for (int j = 0; j < wordMarkSelections.count(); j++) {
                    if (m_wordMarkSelections.value(j).cursor == wordMarkSelections.value(i).cursor
                            && m_wordMarkSelections.value(j).format == wordMarkSelections.value(i).format) {
                        m_wordMarkSelections.removeAt(j);
                        break;
                    }
                }

                //记录新添加的标记
                m_wordMarkSelections.append(selection);

                //如果标记格式不相同
                if (wordMarkSelections.value(i).format != selection.format) {

                    QList<QTextEdit::ExtraSelection> selecList;
                    bool bIsFind = false;///< 是否有包含该标记的标记动作

                    //从记录标记的表中替换原有标记（按标记动作记录）
                    for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                        auto list = m_mapWordMarkSelections.value(j);
                        for (int k = 0; k < list.count(); k++) {
                            if (list.value(k).cursor == wordMarkSelections.value(i).cursor
                                    && list.value(k).format == wordMarkSelections.value(i).format) {

                                list.removeAt(k);
                                selecList = list;
                                bIsFind = true;
                                break;
                            }
                        }

                        if (bIsFind) {
                            m_mapWordMarkSelections.remove(j);
                            m_mapWordMarkSelections.insert(j, selecList);
                            break;
                        }
                    }
                }

                listSelections->append(selection);
            } else if (nWordMarkSelectionEnd < nSelectionEnd && nWordMarkSelectionStart < nSelectionStart
                       && nWordMarkSelectionEnd > nSelectionStart) { //如果添加的标记占有原有标记的后段部分

                selection.format.setBackground(QColor(strColor));
                int nRemPos = 0;///< 标记插入位置

                //移除已有标记
                for (int j = 0; j < wordMarkSelections.count(); j++) {
                    if (m_wordMarkSelections.value(j).cursor == wordMarkSelections.value(i).cursor
                            && m_wordMarkSelections.value(j).format == wordMarkSelections.value(i).format) {
                        m_wordMarkSelections.removeAt(j);
                        nRemPos = j;
                        break;
                    }
                }

                //如果标记格式不相同
                if (wordMarkSelections.value(i).format != selection.format) {

                    //从记录标记的表中替换原有标记（分行记录）
                    selection.cursor.setPosition(nWordMarkSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::KeepAnchor);
                    selection.format.setBackground(wordMarkSelections.value(i).format.background());
                    m_wordMarkSelections.insert(nRemPos, selection);

                    QList<QTextEdit::ExtraSelection> selecList;
                    bool bIsFind = false;///< 是否有包含该标记的标记动作

                    //从记录标记的表中替换原有标记（按标记动作记录）
                    for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                        auto list = m_mapWordMarkSelections.value(j);
                        for (int k = 0; k < list.count(); k++) {
                            if (list.value(k).cursor == wordMarkSelections.value(i).cursor
                                    && list.value(k).format == wordMarkSelections.value(i).format) {
                                list.removeAt(k);
                                selecList = list;
                                selecList.insert(k, selection);
                                bIsFind = true;
                                break;
                            }
                        }

                        if (bIsFind) {
                            m_mapWordMarkSelections.remove(j);
                            m_mapWordMarkSelections.insert(j, selecList);
                            break;
                        }
                    }

                    //记录新添加的标记
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::KeepAnchor);
                    selection.format.setBackground(QColor(strColor));
                    m_wordMarkSelections.append(selection);
                } else { //如果标记格式相同
                    selection.cursor.setPosition(nWordMarkSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(nRemPos, selection);
                }

                if (!bIsContains) {
                    listSelections->append(selection);
                }

                bIsContains = true;
            } else if (nWordMarkSelectionEnd > nSelectionEnd && nWordMarkSelectionStart > nSelectionStart
                       && nWordMarkSelectionStart < nSelectionEnd) { //如果添加的标记占有原有标记的前段部分

                selection.format.setBackground(QColor(strColor));
                int nRemPos = 0;///< 标记插入位置

                //移除已有标记
                for (int j = 0; j < wordMarkSelections.count(); j++) {
                    if (m_wordMarkSelections.value(j).cursor == wordMarkSelections.value(i).cursor
                            && m_wordMarkSelections.value(j).format == wordMarkSelections.value(i).format) {
                        m_wordMarkSelections.removeAt(j);
                        nRemPos = j;
                        break;
                    }
                }

                //如果标记格式不相同
                if (wordMarkSelections.value(i).format != selection.format) {

                    //从记录标记的表中替换原有标记（分行记录）
                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nWordMarkSelectionEnd, QTextCursor::KeepAnchor);
                    selection.format.setBackground(wordMarkSelections.value(i).format.background());
                    m_wordMarkSelections.insert(nRemPos, selection);

                    QList<QTextEdit::ExtraSelection> selecList;
                    bool bIsFind = false;

                    //从记录标记的表中替换原有标记（按标记动作记录）
                    for (int j = 0; j < m_mapWordMarkSelections.count(); j++) {
                        auto list = m_mapWordMarkSelections.value(j);
                        for (int k = 0; k < list.count(); k++) {
                            if (list.value(k).cursor == wordMarkSelections.value(i).cursor
                                    && list.value(k).format == wordMarkSelections.value(i).format) {
                                list.removeAt(k);
                                selecList = list;
                                selecList.insert(k, selection);
                                bIsFind = true;
                                break;
                            }
                        }

                        if (bIsFind) {
                            m_mapWordMarkSelections.remove(j);
                            m_mapWordMarkSelections.insert(j, selecList);
                            break;
                        }
                    }

                    //记录新添加的标记
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nSelectionEnd, QTextCursor::KeepAnchor);
                    selection.format.setBackground(QColor(strColor));
                    m_wordMarkSelections.append(selection);
                } else { //如果标记格式相同
                    selection.cursor.setPosition(nSelectionStart, QTextCursor::MoveAnchor);
                    selection.cursor.setPosition(nWordMarkSelectionEnd, QTextCursor::KeepAnchor);
                    m_wordMarkSelections.insert(nRemPos, selection);
                }

                if (!bIsContains) {
                    listSelections->append(selection);
                }
                bIsContains = true;
            }
        }

        if (!bIsContains) {
            selection.format.setBackground(QColor(strColor));
            m_wordMarkSelections.append(selection);
            listSelections->append(selection);
        }

    } else { //如果文档中没有标记
        selection.format.setBackground(QColor(strColor));
        m_wordMarkSelections.append(selection);
        listSelections->append(selection);
    }
#endif
}

void TextEdit::onSelectionArea()
{
    if (textCursor().hasSelection()) {
        m_nSelectStart = textCursor().selectionStart();
        m_nSelectEnd = textCursor().selectionEnd();
        m_nSelectEndLine = document()->findBlock(textCursor().selectionEnd()).blockNumber() + 1;
    } else {
        m_nSelectEndLine = -1;
    }

    //防止三次点击选中一整行的功能失效
    if (m_bIsDoubleClick == true) {
        m_bIsDoubleClick = false;
        return;
    } else if (m_bBeforeIsDoubleClick == true && textCursor().selectedText() != "") {
        m_bBeforeIsDoubleClick = false;
        return;
    }

    if (m_gestureAction != GA_null) {
        QTextCursor cursor = textCursor();
        if (cursor.selectedText() != "") {
            cursor.clearSelection();
            setTextCursor(cursor);
        }
    }
}

void TextEdit::fingerZoom(QString name, QString direction, int fingers)
{
    if (name == "tap" && fingers == 3) {
        slot_translate();
    }
    // 当前窗口被激活,且有焦点
    if (hasFocus()) {
        if (name == "pinch" && fingers == 2) {
            if (direction == "in") {
                // 捏合 in是手指捏合的方向 向内缩小
                qobject_cast<Window *>(this->window())->decrementFontSize();
            } else if (direction == "out") {
                // 捏合 out是手指捏合的方向 向外放大
                qobject_cast<Window *>(this->window())->incrementFontSize();
            }
        }
    }
}

void TextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    QPlainTextEdit::dragEnterEvent(event);
    qobject_cast<Window *>(this->window())->requestDragEnterEvent(event);
}

void TextEdit::dragMoveEvent(QDragMoveEvent *event)
{
    if (m_readOnlyMode) {
        return;
    }

    const QMimeData *data = event->mimeData();

    if (data->hasUrls()) {
        event->acceptProposedAction();
    } else {
        QPlainTextEdit::dragMoveEvent(event);
    }
}

void TextEdit::dropEvent(QDropEvent *event)
{
    const QMimeData *data = event->mimeData();

    // 判断是否存在url信息，需要注意即使hasUrls()为true, urls()仍可能返回空，使用urls().first()可能越界
    QList<QUrl> dataUrls;
    if (data->hasUrls()) {
        dataUrls = data->urls();
    }

    if (!dataUrls.isEmpty() && dataUrls.first().isLocalFile()) {
        qobject_cast<Window *>(this->window())->requestDropEvent(event);
    } else if (data->hasText() && !m_readOnlyMode) {
        //drag text in the same editor
        if (event->source() && event->source()->parent() == this) {
            auto cursor = this->textCursor();
            int srcpos = std::min(cursor.position(), cursor.anchor());
            //use default behavior to make the cursor blink normally
            QPlainTextEdit::dropEvent(event);
            cursor = this->textCursor();
            int dstpos = std::min(cursor.position(), cursor.anchor()) - data->text().size();

            // 检测是否为拷贝文件而不是移动文件，使用Ctrl仅拷贝
            bool copyText = event->keyboardModifiers().testFlag(Qt::ControlModifier);

            //fall back to the original state
            if (srcpos != dstpos) {
                cursor.setPosition(dstpos);
                cursor.setPosition(dstpos + data->text().size(), QTextCursor::KeepAnchor);
                cursor.deleteChar();

                cursor.setPosition(srcpos);
                // 拷贝模式下不对原文本操作
                if (!copyText) {
                    cursor.insertText(data->text());
                }
            }
            if (srcpos < dstpos && !copyText) {
                dstpos += data->text().size();
            }

            //perform moveText operation
            moveText(srcpos, dstpos, data->text(), copyText);

            //drag text to another editor
        } else if (event->source() && event->source()->parent() != this) {

            //use default behavior to make the cursor blink normally
            QPlainTextEdit::dropEvent(event);

            //operations in the destination editor.
            //firstly do the restore operation,
            //then, insert the text
            auto cursor = this->textCursor();
            int dstpos = std::min(cursor.position(), cursor.anchor()) - data->text().size();
            cursor.setPosition(dstpos);
            cursor.setPosition(dstpos + data->text().size(), QTextCursor::KeepAnchor);
            cursor.deleteChar();
            auto com = new DragInsertTextUndoCommand(cursor, data->text(), this);
            m_pUndoStack->push(com);

            //operations in the source editor.
            //firstly do the restore operation,
            //then, delete the text
            auto another = qobject_cast<TextEdit *>(event->source()->parent());
            auto cursor2 = another->textCursor();
            cursor2.insertText(data->text());
            cursor2.setPosition(cursor2.position() - data->text().size(), QTextCursor::KeepAnchor);
            auto com2 = new DeleteBackCommand(cursor2, another);
            another->m_pUndoStack->push(com2);
        } else if (!data->text().isEmpty()) {
            if (m_bReadOnlyPermission || m_readOnlyMode) {
                return;
            }

            QPlainTextEdit::dropEvent(event);
            auto cursor = this->textCursor();
            int dstpos = std::min(cursor.position(), cursor.anchor()) - data->text().size();
            cursor.setPosition(dstpos);
            cursor.setPosition(dstpos + data->text().size(), QTextCursor::KeepAnchor);
            cursor.deleteChar();
            auto com = new DragInsertTextUndoCommand(cursor, data->text(), this);
            m_pUndoStack->push(com);
        }
    } else {
        QPlainTextEdit::dropEvent(event);
    }
}

void TextEdit::inputMethodEvent(QInputMethodEvent *e)
{
    m_bIsInputMethod = true;

    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    if (m_readOnlyMode || m_bReadOnlyPermission) {
        return;
    }

    if (m_isPreeditBefore) {
        // 每次 preedit 都是完整的字符串，所以撤销上次的 preedit
        undo_();

        // 将光标移回原位
        if (Overwrite == m_cursorMode) {
            auto cursor = textCursor();
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, m_preeditLengthBefore);
            setTextCursor(cursor);
        }
    }
    bool isPreedit = !e->preeditString().isEmpty();
    if (isPreedit || !e->commitString().isEmpty()) {
        const QString &text = isPreedit ? e->preeditString() : e->commitString();

        //列编辑添加撤销重做
        if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
            insertColumnEditTextEx(text);
        } else {
            // 覆盖模式下输入法输入时，单独处理，模拟选中替换处理
            if (Overwrite == m_cursorMode) {
                auto cursor = this->textCursor();
                // 设置光标选中后续与当前输入法输入的文本相同长度
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, text.size());
                // 使用此光标信息插入将输入的文本替换选中的长度
                insertSelectTextEx(cursor, text);
            } else {
                insertSelectTextEx(textCursor(), text);
            }
        }

        m_isSelectAll = false;
    }

    m_isPreeditBefore = isPreedit;
    m_preeditLengthBefore = e->preeditString().length();

}

void TextEdit::mousePressEvent(QMouseEvent *e)
{
    if (m_bIsFindClose)
    {
        m_bIsFindClose = false;
        removeKeywords();
    }
    if (e->button() != Qt::RightButton)
        m_isSelectAll = false;

    if (Qt::MouseEventSynthesizedByQt == e->source())
    {
        m_startY = e->y();
        m_startX = e->x();
    }
    if (e->source() == Qt::MouseEventSynthesizedByQt)
    {
        m_lastTouchBeginPos = e->pos();

        if (QScroller::hasScroller(this))
        {
            QScroller::scroller(this)->deleteLater();
        }

        if (m_updateEnableSelectionByMouseTimer)
        {
            m_updateEnableSelectionByMouseTimer->stop();
        }
        else
        {
            m_updateEnableSelectionByMouseTimer = new QTimer(this);
            m_updateEnableSelectionByMouseTimer->setSingleShot(true);

            static QObject *theme_settings = reinterpret_cast<QObject *>(qvariant_cast<quintptr>(qApp->property("_d_theme_settings_object")));
            QVariant touchFlickBeginMoveDelay;

            if (theme_settings)
            {
                touchFlickBeginMoveDelay = theme_settings->property("touchFlickBeginMoveDelay");
            }

            m_updateEnableSelectionByMouseTimer->setInterval(touchFlickBeginMoveDelay.isValid() ? touchFlickBeginMoveDelay.toInt() : 300);
            connect(m_updateEnableSelectionByMouseTimer, &QTimer::timeout, m_updateEnableSelectionByMouseTimer, &QTimer::deleteLater);
        }

        m_updateEnableSelectionByMouseTimer->start();
    }

    // add for single refers to the sliding
    if (e->type() == QEvent::MouseButtonPress && e->source() == Qt::MouseEventSynthesizedByQt)
    {
        m_lastMouseTimeX = e->timestamp();
        m_lastMouseTimeY = e->timestamp();
        m_lastMouseYpos = e->pos().y();
        m_lastMouseXpos = e->pos().x();

        if (tweenY.activeY())
        {
            m_slideContinueY = true;
            tweenY.stopY();
        }

        if (tweenX.activeX())
        {
            m_slideContinueX = true;
            tweenX.stopX();
        }
    }

    if (e->modifiers() == Qt::AltModifier)
    {
        m_bIsAltMod = true;
        // 鼠标点击位置为光标位置 　获取光标行列位置
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);
        m_altStartTextCursor = this->cursorForPosition(mouseEvent->pos());
        m_altStartTextCursor.clearSelection();
        this->setTextCursor(m_altStartTextCursor);
        m_altModSelections.clear();
    }
    else
    {
        if (e->button() != 2)
        { // 右键,调用右键菜单时候不能清空
            m_bIsAltMod = false;
            m_altModSelections.clear();
        }
    }

    QPlainTextEdit::mousePressEvent(e);
}

void TextEdit::mouseMoveEvent(QMouseEvent *e)
{
    if (Qt::MouseEventSynthesizedByQt == e->source())
    {
        m_endY = e->y();
        m_endX = e->x();
    }

    // add for single refers to the sliding
    if (e->type() == QEvent::MouseMove && e->source() == Qt::MouseEventSynthesizedByQt)
    {
        const ulong diffTimeX = e->timestamp() - m_lastMouseTimeX;
        const ulong diffTimeY = e->timestamp() - m_lastMouseTimeY;
        const int diffYpos = e->pos().y() - m_lastMouseYpos;
        const int diffXpos = e->pos().x() - m_lastMouseXpos;
        m_lastMouseTimeX = e->timestamp();
        m_lastMouseTimeY = e->timestamp();
        m_lastMouseYpos = e->pos().y();
        m_lastMouseXpos = e->pos().x();

        if (m_gestureAction == GA_slide)
        {
            QFont font = this->font();

            /*开根号时数值越大衰减比例越大*/
            qreal direction = diffYpos > 0 ? 1.0 : -1.0;
            slideGestureY(-direction * sqrt(abs(diffYpos)) / font.pointSize());
            qreal directionX = diffXpos > 0 ? 1.0 : -1.0;
            slideGestureX(-directionX * sqrt(abs(diffXpos)) / font.pointSize());

            /*预算惯性滑动时间*/
            m_stepSpeedY = static_cast<qreal>(diffYpos) / static_cast<qreal>(diffTimeY + 0.000001);
            durationY = sqrt(abs(m_stepSpeedY)) * 1000;
            m_stepSpeedX = static_cast<qreal>(diffXpos) / static_cast<qreal>(diffTimeX + 0.000001);
            durationX = sqrt(abs(m_stepSpeedX)) * 1000;

            /*预算惯性滑动距离,4.0为调优数值*/
            m_stepSpeedY /= sqrt(font.pointSize() * 4.0);
            changeY = m_stepSpeedY * sqrt(abs(m_stepSpeedY)) * 100;
            m_stepSpeedX /= sqrt(font.pointSize() * 4.0);
            changeX = m_stepSpeedX * sqrt(abs(m_stepSpeedX)) * 100;

            // return true;
        }

        if (m_gestureAction != GA_null)
        {
            // return true;
        }
    }

    // other apps will override their own cursor when opened
    // so they need to be restored.
    QApplication::restoreOverrideCursor();

    if (viewport()->cursor().shape() != Qt::IBeamCursor)
    {
        viewport()->setCursor(Qt::IBeamCursor);
    }

    QPlainTextEdit::mouseMoveEvent(e);
    if (e->modifiers() == Qt::AltModifier && m_bIsAltMod)
    {
        m_altModSelections.clear();
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(e);

        QPoint curPos = mouseEvent->pos();
        m_altEndTextCursor = this->cursorForPosition(curPos);
        int row = m_altEndTextCursor.blockNumber();
        int startRow = m_altStartTextCursor.blockNumber();
        int minRow = startRow < row ? startRow : row;
        int maxRow = startRow > row ? startRow : row;
        QTextCharFormat format;
        QPalette palette;
        QColor highlightBackground = DGuiApplicationHelper::instance()->applicationPalette().color(QPalette::Highlight);
        format.setBackground(highlightBackground);
        format.setForeground(palette.highlightedText());
        int judgeAncherPosX;
        int judgeStartLength;
        int startLineIdx;
        {
            QTextBlock block = document()->findBlockByNumber(startRow);
            int startCurPos = m_altStartTextCursor.positionInBlock();
            QTextLine startLine = block.layout()->lineForTextPosition(startCurPos);
            judgeAncherPosX = startLine.cursorToX(startCurPos);
            judgeStartLength = startCurPos - startLine.textStart();
            startLineIdx = startLine.lineNumber();
        }
        int judgeCursorPosX;
        int judgeEndLength;
        int endLineIdx;
        {
            QTextBlock block = document()->findBlockByNumber(row);
            int endCurPos = m_altEndTextCursor.positionInBlock();
            QTextLine endLine = block.layout()->lineForTextPosition(endCurPos);
            judgeCursorPosX = curPos.x();
            judgeEndLength = endCurPos - endLine.textStart();
            endLineIdx = endLine.lineNumber();
        }
        bool isDown = false;
        if (row > startRow)
        {
            isDown = true;
        }
        else if (row == startRow && endLineIdx >= startLineIdx)
        {
            isDown = true;
        }
        for (int iRow = minRow; iRow <= maxRow; iRow++)
        {
            QTextBlock block = document()->findBlockByNumber(iRow);
            int lineAt = 0;
            int lineCount = block.lineCount();
            // 对开始块和结束块的行数做判断
            if (iRow == minRow)
            {
                lineAt = isDown ? startLineIdx : endLineIdx;
            }
            if (iRow == maxRow)
            {
                lineCount = isDown ? endLineIdx + 1 : startLineIdx + 1;
            }
            for (; lineAt < lineCount; lineAt++)
            {
                // 引入行的判断
                QTextLine lineInBlock = block.layout()->lineAt(lineAt);
                int lineLength = lineInBlock.textLength();
                if (lineLength < judgeStartLength && lineLength < judgeEndLength)
                {
                    continue;
                }
                QTextCursor cursor = this->textCursor();
                cursor.clearSelection();
                setTextCursor(cursor);
                int properColumn;
                int blockPos = block.position();
                properColumn = lineInBlock.xToCursor(judgeAncherPosX);
                cursor.setPosition(blockPos + properColumn, QTextCursor::MoveAnchor);
                // 由于窗口大小小于块的最大长度的外部UI问题，这里要进行对鼠标pos进行若达到最大值无法对每行最后一个字符的覆盖的问题处理
                // 扩大了judgeCursorPosX的最大限度。
                int lineEndPosInBlock = lineInBlock.textStart() + lineInBlock.textLength();
                int lineEndPosToX = lineInBlock.cursorToX(lineEndPosInBlock);
                if (lineInBlock.width() <= lineEndPosToX)
                {
                    if (judgeCursorPosX > lineInBlock.cursorToX(lineEndPosInBlock - 1))
                    {
                        judgeCursorPosX = lineEndPosToX;
                    }
                }
                properColumn = lineInBlock.xToCursor(judgeCursorPosX);
                cursor.setPosition(blockPos + properColumn, QTextCursor::KeepAnchor);
                QTextEdit::ExtraSelection selection;
                selection.cursor = cursor;
                selection.format = format;
                m_altModSelections << selection;
            }
        }
        renderAllSelections();
        update();
    }
}

void TextEdit::mouseReleaseEvent(QMouseEvent *e)
{
    //add for single refers to the sliding
    if (e->type() == QEvent::MouseButtonRelease && e->source() == Qt::MouseEventSynthesizedByQt) {
        if (m_gestureAction == GA_slide) {

            tweenX.startX(0, 0, changeX, durationX, std::bind(&TextEdit::slideGestureX, this, std::placeholders::_1));
            tweenY.startY(0, 0, changeY, durationY, std::bind(&TextEdit::slideGestureY, this, std::placeholders::_1));
        }

        m_gestureAction = GA_null;
    }

    int i = m_endY - m_startY;
    int k = m_endX - m_startX;
    if (Qt::MouseEventSynthesizedByQt == e->source()
            && (i > 10 && this->verticalScrollBar()->value() != 0)
            && (k > 10 && this->horizontalScrollBar()->value() != 0)) {
        e->accept();
        return;
    }

    if (e->button() == Qt::MidButton) {
        bool midButtonPaste = m_settings->settings->option("advance.editor.allow_midbutton_paste")->value().toBool();
        if (midButtonPaste) {
            // 只读模式过滤鼠标中间黏贴
            if (m_readOnlyMode || m_bReadOnlyPermission) {
                popupNotify(tr("Read-Only mode is on"));
                return;
            }

            slotPasteAction();
        }
        return;
    }

    return QPlainTextEdit::mouseReleaseEvent(e);
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    Qt::KeyboardModifiers modifiers = e->modifiers();
    QString key = Utils::getKeyshortcut(e);
    //没有修改键　插入文件
    //按下esc的时候,光标退出编辑区，切换至标题栏
    if (modifiers == Qt::NoModifier && e->key() == Qt::Key_Escape) {
        emit signal_setTitleFocus();
        return;
    }

    if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "copy")) {
        slotCopyAction();
        return;
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "selectall")) {
#if 0
        //2021-2-25:setSelectAll()替换
        if (m_wrapper->getFileLoading()) {
            return;
        }
        m_bIsAltMod = false;
        selectAll();
#endif
        setSelectAll();
        return;
    }

    if (m_readOnlyMode || m_bReadOnlyPermission) {
        if (key == "J") {
            nextLine();
            return;
        } else if (key == "K") {
            prevLine();
            return;
        } else if (key == ",") {
            moveToEnd();
            return;
        } else if (key == ".") {
            moveToStart();
            return;
        } else if (key == "H") {
            backwardChar();
            return;
        } else if (key == "L") {
            forwardChar();
            return;
        } else if (key == "Space") {
            scrollUp();
            return;
        } else if (key == "V") {
            scrollDown();
            return;
        } else if (key == "F") {
            forwardWord();
            return;
        } else if (key == "B") {
            backwardWord();
            return;
        } else if (key == "A") {
            moveToStartOfLine();
            return;
        } else if (key == "E") {
            moveToEndOfLine();
            return;
        } else if (key == "M") {
            moveToLineIndentation();
            return;
        } else if (key == "Q" && m_bReadOnlyPermission == false) {
//            setReadOnly(false);
            toggleReadOnlyMode();
            return;
        } else if (key == "Shfit+J") {
            scrollLineUp();
            return;
        } else if (key == "Shift+K") {
            scrollLineDown();
            return;
        } else if (key == "P") {
            forwardPair();
            return;
        } else if (key == "N") {
            backwardPair();
            return;
        } else if (key == "Shift+:") {
            copyLines();
            return;
        } else if ((key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "togglereadonlymode")/* || key=="Alt+Meta+L"*/)
                   && m_bReadOnlyPermission == false) {
//            setReadOnly(false);
            toggleReadOnlyMode();
            return;
        } else if (key == "Shift+/" && e->modifiers() == Qt::ControlModifier) {
            e->ignore();
        } else if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
            if (e->key() == Qt::Key_Return || e->key() == Qt::Key_D || e->key() == Qt::Key_K
                    || e->key() == Qt::Key_Up || e->key() == Qt::Key_Down) {
                popupNotify(tr("Read-Only mode is on"));
                return;
            } else {
                e->ignore();
            }
        } else if (e->modifiers() == Qt::ControlModifier) {
            if (e->key() == Qt::Key_Return || e->key() == Qt::Key_K || e->key() == Qt::Key_X ||
                    e->key() == Qt::Key_V || e->key() == Qt::Key_J || e->key() == Qt::Key_Z || e->key() == Qt::Key_Y) {
                popupNotify(tr("Read-Only mode is on"));
                return;
            } else {
                e->ignore();
            }
        } else if (e->key() == Qt::Key_Control || e->key() == Qt::Key_Shift) {
            e->ignore();
        } else if (e->key() == Qt::Key_F11 || e->key() == Qt::Key_F5) {
            e->ignore();
            return;
        } else if (e->modifiers() == Qt::NoModifier || e->modifiers() == Qt::KeypadModifier) {
            popupNotify(tr("Read-Only mode is on"));
            return;
        } else {
            // If press another key
            // the main window does not receive
            e->ignore();
            return;
        }
    } else {
        if (isReadOnly()) { //原生接口setReadOnly不生效,在这里拦截模拟ReadOnly
            return;
        }

        // 左右移动光标后退出全选状态
        if (Qt::Key_Left == e->key()
                || Qt::Key_Right == e->key()) {
            m_isSelectAll = false;
            // 继续后续判断
        }

        //插入键盘可现实字符
        if (modifiers == Qt::NoModifier && (e->key() <= Qt::Key_ydiaeresis && e->key() >= Qt::Key_Space) && !e->text().isEmpty()) {

            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            //非修改键盘按键加撤销重做栈
            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                insertColumnEditTextEx(e->text());
            } else {
                if (m_cursorMode == Overwrite) {
                    auto cursor = this->textCursor();
                    cursor.clearSelection();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    this->setTextCursor(cursor);
                }
                insertSelectTextEx(textCursor(), e->text());
            }

            m_isSelectAll = false;
            return;
        }

        //键盘右边数字键
        if (modifiers == Qt::KeypadModifier && (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_Asterisk) && !e->text().isEmpty()) {

            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            //非修改键盘按键加撤销重做栈
            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                insertColumnEditTextEx(e->text());
            } else {
                if (m_cursorMode == Overwrite) {
                    auto cursor = this->textCursor();
                    cursor.clearSelection();
                    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
                    this->setTextCursor(cursor);
                }
                insertSelectTextEx(textCursor(), e->text());
            }
            m_isSelectAll = false;
            return;
        }

        //插入空白字符
        if (modifiers == Qt::NoModifier && (e->key() == Qt::Key_Tab || e->key() == Qt::Key_Return) ||
                modifiers == Qt::KeypadModifier && (e->key() == Qt::Key_Enter)) {

            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            //列编辑添加撤销重做
            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                insertColumnEditTextEx(e->text());
            } else {
                auto cursor = this->textCursor();
                bool hassel = cursor.hasSelection();
                auto selectedtext = cursor.selectedText();

                //calculate the startline and endline.
                auto anchor = cursor.anchor();
                auto pos = cursor.position();
                cursor.setPosition(anchor);
                auto line1 = cursor.blockNumber();
                cursor.setPosition(pos);
                auto line2 = cursor.blockNumber();

                //get the text of current line.
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                auto currentline = cursor.selectedText();

                if (hassel && e->key() == Qt::Key_Tab && (line1 != line2 || selectedtext == currentline)) {
                    indentText();
                } else {
                    insertSelectTextEx(textCursor(), e->text());
                }
            }
            m_isSelectAll = false;
            return;
        }

        //列编辑 删除撤销重做
        if (modifiers == Qt::NoModifier && (e->key() == Qt::Key_Backspace)) {
            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(m_altModSelections, this);
                m_pUndoStack->push(pDeleteStack);
            } else {
                //修改backspace删除，在文档最末尾点击backspace,引起标签栏*出现问题
                QTextCursor cursor = textCursor();
                if (!cursor.hasSelection()) {
                    cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
                }
                QString m_delText = cursor.selectedText();
                if (m_delText.size() <= 0) return;
                QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(cursor, this);
                m_pUndoStack->push(pDeleteStack);
            }
            m_isSelectAll = false;
            return;
        }

        //列编辑 向后删除撤销重做
        if (modifiers == Qt::NoModifier && (e->key() == Qt::Key_Delete)) {
            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                DeleteBackAltCommand *commond = new DeleteBackAltCommand(m_altModSelections, this);
                m_pUndoStack->push(commond);
            } else {
                //修改delete删除，在文档最末尾点击delete,引起标签栏*出现问题
                QTextCursor cursor = textCursor();
                if (!cursor.hasSelection()) {
                    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                }
                QString m_delText = cursor.selectedText();
                if (m_delText.size() <= 0) return;

                DeleteBackCommand *commond = new DeleteBackCommand(cursor, this);
                m_pUndoStack->push(commond);
            }
            m_isSelectAll = false;
            return;
        }

        //fix 66710 输入的内容为英文符号时，文本编辑器未识别为临时文件
        //fix 75313  lxp 2021.4.22
        if ((modifiers == Qt::ShiftModifier || e->key() == Qt::Key_Shift) && !e->text().isEmpty()) {

            if (m_isSelectAll)
                QPlainTextEdit::selectAll();

            if (m_bIsAltMod) {
                insertColumnEditTextEx(e->text());
            } else {
                insertSelectTextEx(textCursor(), e->text());
            }
            m_isSelectAll = false;
            return;
        }

        // alt+m 弹出编辑器右键菜单
        if (e->modifiers() == Qt::AltModifier && !e->text().compare(QString("m"), Qt::CaseInsensitive)) {
            popRightMenu();
            return;
        }

        //快捷建处理
        if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "undo")) {
            this->undo_();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "redo")) {
            this->redo_();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "cut")) {
#if 0
            //列编辑添加撤销重做
            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                QString data;
                for (auto sel : m_altModSelections) data.append(sel.cursor.selectedText());
                //删除有选择
                for (int i = 0; i < m_altModSelections.size(); i++) {
                    if (m_altModSelections[i].cursor.hasSelection()) {
                        QUndoCommand *pDeleteStack = new DeleteTextUndoCommand(m_altModSelections[i].cursor);
                        m_pUndoStack->push(pDeleteStack);
                    }
                }
                //设置到剪切板
                QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
                clipboard->setText(data);
            } else {
                QTextCursor cursor = textCursor();
                //有选择内容才剪切
                if (cursor.hasSelection()) {
                    QString data = cursor.selectedText();
                    deleteTextEx(cursor);
                    QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
                    clipboard->setText(data);
                }
            }
#endif
            this->cut();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "paste")) {
#if 0
            //添加剪切板内容到撤销重做栈
            const QClipboard *clipboard = QApplication::clipboard(); //获取剪切版内容

            if (m_bIsAltMod && !m_altModSelections.isEmpty()) {
                insertColumnEditTextEx(clipboard->text());
            } else {
                insertSelectTextEx(textCursor(), clipboard->text());
            }
#endif
            slotPasteAction();
            return;
        }  else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrollup")) {
            //向上翻页
            scrollUp();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrolldown")) {
            //向下翻页
            scrollDown();
            return;
        }  else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "copylines")) {
            copyLines();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "cutlines")) {
            cutlines();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "indentline")) {
            QPlainTextEdit::keyPressEvent(e);
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "backindentline")) {
            unindentText();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "forwardchar")) {
            forwardChar();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "backwardchar")) {
            backwardChar();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "forwardword")) {
            forwardWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "backwardword")) {
            backwardWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "nextline")) {
            nextLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "prevline")) {
            prevLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "newline")/* || key == "Return"*/) {
            newline();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "opennewlineabove")) {
            openNewlineAbove();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "opennewlinebelow")) {
            openNewlineBelow();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "duplicateline")) {
            duplicateLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "killline")) {
            killLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "killcurrentline")) {
            killCurrentLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "swaplineup")) {
            moveLineDownUp(true);
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "swaplinedown")) {
            moveLineDownUp(false);
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrolllineup")) {
            scrollLineUp();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrolllinedown")) {
            scrollLineDown();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrollup")) {
            scrollUp();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "scrolldown")) {
            scrollDown();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetoendofline")) {
            moveToEndOfLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetostartofline")) {
            moveToStartOfLine();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetostart")) {
            moveToStart();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetoend")) {
            moveToEnd();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetolineindentation")) {
            moveToLineIndentation();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "upcaseword")) {
            upcaseWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "downcaseword")) {
            downcaseWord();
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "capitalizeword")) {
            capitalizeWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "killbackwardword")) {
            killBackwardWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "killforwardword")) {
            killForwardWord();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "forwardpair")) {
            forwardPair();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "backwardpair")) {
            backwardPair();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "transposechar")) {
            transposeChar();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "setmark")) {
            setMark();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "exchangemark")) {
            exchangeMark();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "joinlines")) {
            joinLines();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "togglereadonlymode")/*|| key=="Alt+Meta+L"*/) {
            //setReadOnly(false);
            toggleReadOnlyMode();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "togglecomment")) {
            toggleComment(true);
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "removecomment")) {
            toggleComment(false);
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "switchbookmark")) {
            m_bIsShortCut = true;
            addOrDeleteBookMark();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetoprebookmark")) {
            moveToPreviousBookMark();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "movetonextbookmark")) {
            moveToNextBookMark();
            return;
        } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "editor", "mark")) {
            toggleMarkSelections();
            return;
        } else if (e->key() == Qt::Key_Insert && key != "Shift+Ins" && e->modifiers() == Qt::NoModifier) {
            setOverwriteMode(!overwriteMode());
            //update();
            if (!overwriteMode()) {
                auto cursor = this->textCursor();
                cursor.clearSelection();
                cursor.movePosition(QTextCursor::Right);
                this->setTextCursor(cursor);

                cursor = this->textCursor();
                cursor.movePosition(QTextCursor::Left);
                this->setTextCursor(cursor);
            }

            m_cursorMode = overwriteMode() ? Overwrite : Insert;
            emit cursorModeChanged(m_cursorMode);
            e->accept();
        } else {
            // Post event to window widget if key match window key list.
            for (auto option : m_settings->settings->group("shortcuts.window")->options()) {
                if (key == m_settings->settings->option(option->key())->value().toString()) {
                    e->ignore();
                    return;
                }
            }

            // Post event to window widget if match Alt+0 ~ Alt+9
            QRegularExpression re("^Alt\\+\\d");
            QRegularExpressionMatch match = re.match(key);
            if (match.hasMatch()) {
                e->ignore();
                return;
            }

            /* qt原生控件QPlainTextEdit对Alt+Tab快捷键有接收响应，需求里无定义Alt+Tab快捷键响应功能，遇到该快捷键直接return即可 */
            if (key.contains(QString("Alt+Tab"))) {
                return;
            }

            // Text editor handle key self.
            QPlainTextEdit::keyPressEvent(e);
        }

        //return QPlainTextEdit::keyPressEvent(e);
    }
}

void TextEdit::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int deltaY = e->angleDelta().y();

        if (deltaY < 0) {
            qobject_cast<Window *>(this->window())->decrementFontSize();
        } else {
            qobject_cast<Window *>(this->window())->incrementFontSize();
        }

        return;
    }

    QPlainTextEdit::wheelEvent(e);
}

void TextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    popRightMenu(event->globalPos());
}

void TextEdit::paintEvent(QPaintEvent *e)
{
    DPlainTextEdit::paintEvent(e);

    if (m_altModSelections.length() > 0) {
        for (auto sel : m_altModSelections) {
            if (sel.cursor.hasSelection()) {
                m_hasColumnSelection = true;
                break;
            } else {
                m_hasColumnSelection = false;
            }
        }
    }
    QColor lineColor = palette().text().color();

    if (m_bIsAltMod && !m_altModSelections.isEmpty()) {

        QTextCursor textCursor = this->textCursor();
        int cursorWidth = this->cursorWidth();
        QPainter painter(viewport());
        QPen pen;
        pen.setColor(lineColor);
        pen.setWidth(cursorWidth);
        painter.setPen(pen);

        for (int i = 0; i < m_altModSelections.size(); i++)
        {
            QRect textCursorRect = this->cursorRect(m_altModSelections[i].cursor);
            painter.drawRect(textCursorRect);
        }
    }
}

void TextEdit::resizeEvent(QResizeEvent *e)
{
    if (m_isSelectAll)
        selectTextInView();

    // 显示区域变化时同时更新视图
    markAllKeywordInView();

    // 当前处于文档页面尾部时，缩放后保持焦点在文档页面尾部
    if (e->oldSize().width() < e->size().width() && verticalScrollBar()->maximum() == verticalScrollBar()->value()) {
        QTimer::singleShot(0, [this]() {
            // 宽度变大时文档布局大小变更信号未触发，手动通知
            auto docLayout = this->document()->documentLayout();
            Q_EMIT docLayout->documentSizeChanged(docLayout->documentSize());

            verticalScrollBar()->setValue(verticalScrollBar()->maximum());
        });
    }

    QPlainTextEdit::resizeEvent(e);
}

bool TextEdit::isComment(const QString &text, int index, const QString &commentType)
{
    int length = commentType.length();

    Q_ASSERT(text.length() - index >= length);

    int i = 0;
    while (i < length) {
        if (text.at(index + i) != commentType.at(i))
            return false;
        ++i;
    }
    return true;
}


void TextEdit::unCommentSelection()
{
    if (!m_commentDefinition.isValid())
        return;

    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    QTextCursor cursor = textCursor();
    QTextDocument *doc = cursor.document();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);
    bool anchorIsStart = (anchor == start);

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    if (end > start && endBlock.position() == end) {
        --end;
        endBlock = endBlock.previous();
    }

    bool doMultiLineStyleUncomment = false;
    bool doMultiLineStyleComment = false;
    bool doSingleLineStyleUncomment = false;

    bool hasSelection = cursor.hasSelection();

    if (hasSelection && m_commentDefinition.hasMultiLineStyle()) {

        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
        const int multiLineStartLength = m_commentDefinition.multiLineStart.length();
        bool hasLeadingCharacters = !startText.left(startPos).trimmed().isEmpty();

        if (startPos >= multiLineStartLength
                && isComment(startText,
                             startPos - multiLineStartLength,
                             m_commentDefinition.multiLineStart)) {
            startPos -= multiLineStartLength;
            start -= multiLineStartLength;
        }

        bool hasSelStart = startPos <= startText.length() - multiLineStartLength
                           && isComment(startText, startPos, m_commentDefinition.multiLineStart);

        QString endText = endBlock.text();
        int endPos = end - endBlock.position();
        const int multiLineEndLength = m_commentDefinition.multiLineEnd.length();
        bool hasTrailingCharacters =
            !endText.left(endPos).remove(m_commentDefinition.singleLine).trimmed().isEmpty()
            && !endText.mid(endPos).trimmed().isEmpty();

        if (endPos <= endText.length() - multiLineEndLength
                && isComment(endText, endPos, m_commentDefinition.multiLineEnd)) {
            endPos += multiLineEndLength;
            end += multiLineEndLength;
        }

        bool hasSelEnd = endPos >= multiLineEndLength
                         && isComment(endText, endPos - multiLineEndLength, m_commentDefinition.multiLineEnd);

        doMultiLineStyleUncomment = hasSelStart && hasSelEnd;
        doMultiLineStyleComment = !doMultiLineStyleUncomment
                                  && (hasLeadingCharacters
                                      || hasTrailingCharacters
                                      || !m_commentDefinition.hasSingleLineStyle());
    } else if (!hasSelection && !m_commentDefinition.hasSingleLineStyle()) {

        QString text = startBlock.text().trimmed();
        doMultiLineStyleUncomment = text.startsWith(m_commentDefinition.multiLineStart)
                                    && text.endsWith(m_commentDefinition.multiLineEnd);
        doMultiLineStyleComment = !doMultiLineStyleUncomment && !text.isEmpty();

        start = startBlock.position();
        end = endBlock.position() + endBlock.length() - 1;

        if (doMultiLineStyleUncomment) {
            int offset = 0;
            text = startBlock.text();
            const int length = text.length();
            while (offset < length && text.at(offset).isSpace())
                ++offset;
            start += offset;
        }
    }

    if (doMultiLineStyleUncomment) {
        cursor.setPosition(end);
        cursor.movePosition(QTextCursor::PreviousCharacter,
                            QTextCursor::KeepAnchor,
                            m_commentDefinition.multiLineEnd.length());
        //cursor.removeSelectedText();
        deleteTextEx(cursor);
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::NextCharacter,
                            QTextCursor::KeepAnchor,
                            m_commentDefinition.multiLineStart.length());
        //cursor.removeSelectedText();
        deleteTextEx(cursor);
    } else if (doMultiLineStyleComment) {
        cursor.setPosition(end);
        insertTextEx(cursor, m_commentDefinition.multiLineEnd);
        //cursor.insertText(m_commentDefinition.multiLineEnd);
        cursor.setPosition(start);
        insertTextEx(cursor, m_commentDefinition.multiLineStart);
        //cursor.insertText(m_commentDefinition.multiLineStart);
    } else {
        endBlock = endBlock.next();
        doSingleLineStyleUncomment = true;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text().trimmed();
            if (!text.isEmpty() && !text.startsWith(m_commentDefinition.singleLine)) {
                doSingleLineStyleUncomment = false;
                break;
            }
        }

        const int singleLineLength = m_commentDefinition.singleLine.length();
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            if (doSingleLineStyleUncomment) {
                QString text = block.text();
                int i = 0;
                while (i <= text.size() - singleLineLength) {
                    if (isComment(text, i, m_commentDefinition.singleLine)) {
                        cursor.setPosition(block.position() + i);
                        cursor.movePosition(QTextCursor::NextCharacter,
                                            QTextCursor::KeepAnchor,
                                            singleLineLength);
                        //cursor.removeSelectedText();
                        deleteTextEx(cursor);
                        break;
                    }
                    if (i < text.size()) {
                        if (!text.at(i).isSpace())
                            break;
                    }

                    ++i;
                }
            } else {
                const QString text = block.text();
                foreach (QChar c, text) {
                    if (!c.isSpace()) {
                        if (m_commentDefinition.isAfterWhiteSpaces)
                            cursor.setPosition(block.position() + text.indexOf(c));
                        else
                            cursor.setPosition(block.position());
                        insertTextEx(cursor, m_commentDefinition.singleLine);
                        break;
                    }
                }
            }
        }
    }

    // adjust selection when commenting out
    if (hasSelection && !doMultiLineStyleUncomment && !doSingleLineStyleUncomment) {
        cursor = textCursor();
        if (!doMultiLineStyleComment)
            start = startBlock.position(); // move the comment into the selection
        int lastSelPos = anchorIsStart ? cursor.position() : cursor.anchor();
        if (anchorIsStart) {
            cursor.setPosition(start);
            cursor.setPosition(lastSelPos, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(lastSelPos);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
        }
        setTextCursor(cursor);
    }
}

void TextEdit::setComment()
{
    //此函数是删除了unCommentSelection()的if-else的uncomment分支得来的
    if (!m_commentDefinition.isValid())
        return;
    if (m_isSelectAll)
        QPlainTextEdit::selectAll();

    QTextCursor cursor = textCursor();
    if (cursor.isNull()) return;

    QTextDocument *doc = cursor.document();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);
    bool anchorIsStart = (anchor == start);

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);
    if (end > start && endBlock.position() == end) {
        --end;
        endBlock = endBlock.previous();
    }
    bool doMultiLineStyleUncomment = false;
    bool doMultiLineStyleComment = false;
    bool doSingleLineStyleUncomment = false;

    bool hasSelection = cursor.hasSelection();

    if (hasSelection && m_commentDefinition.hasMultiLineStyle()) {

        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
        const int multiLineStartLength = m_commentDefinition.multiLineStart.length();
        bool hasLeadingCharacters = !startText.left(startPos).trimmed().isEmpty();

        if (startPos >= multiLineStartLength
                && isComment(startText,
                             startPos - multiLineStartLength,
                             m_commentDefinition.multiLineStart)) {
            startPos -= multiLineStartLength;
            start -= multiLineStartLength;
        }

        bool hasSelStart = startPos <= startText.length() - multiLineStartLength
                           && isComment(startText, startPos, m_commentDefinition.multiLineStart);

        QString endText = endBlock.text();
        int endPos = end - endBlock.position();
        const int multiLineEndLength = m_commentDefinition.multiLineEnd.length();
        bool hasTrailingCharacters =
            !endText.left(endPos).remove(m_commentDefinition.singleLine).trimmed().isEmpty()
            && !endText.mid(endPos).trimmed().isEmpty();

        if (endPos <= endText.length() - multiLineEndLength
                && isComment(endText, endPos, m_commentDefinition.multiLineEnd)) {
            endPos += multiLineEndLength;
            end += multiLineEndLength;
        }

        bool hasSelEnd = endPos >= multiLineEndLength
                         && isComment(endText, endPos - multiLineEndLength, m_commentDefinition.multiLineEnd);

        doMultiLineStyleUncomment = hasSelStart && hasSelEnd;
        doMultiLineStyleComment = !doMultiLineStyleUncomment
                                  && (hasLeadingCharacters
                                      || hasTrailingCharacters
                                      || m_commentDefinition.hasMultiLineStyle());
    } else if (!hasSelection && !m_commentDefinition.hasSingleLineStyle()) {
        QString text = startBlock.text().trimmed();
        doMultiLineStyleUncomment = text.startsWith(m_commentDefinition.multiLineStart)
                                    && text.endsWith(m_commentDefinition.multiLineEnd);
        doMultiLineStyleComment = !doMultiLineStyleUncomment && !text.isEmpty();

        start = startBlock.position();
        end = endBlock.position() + endBlock.length() - 1;

        if (doMultiLineStyleUncomment) {
            int offset = 0;
            text = startBlock.text();
            const int length = text.length();
            while (offset < length && text.at(offset).isSpace())
                ++offset;
            start += offset;
        }
    }

    if (doMultiLineStyleComment) {
        // 压入添加信息，注意光标索引顺序不能调换
        QList<QPair<QTextCursor, QString> > multiText;
        cursor.setPosition(end);
        multiText.append(qMakePair(cursor, m_commentDefinition.multiLineEnd));
        cursor.setPosition(start);
        multiText.append(qMakePair(cursor, m_commentDefinition.multiLineStart));

        // 同时将两组插入信息压入撤销栈
        insertMultiTextEx(multiText);
    } else {
        endBlock = endBlock.next();
        doSingleLineStyleUncomment = true;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text().trimmed();
            if (!text.isEmpty() && !text.startsWith(m_commentDefinition.singleLine)) {
                doSingleLineStyleUncomment = false;
                break;
            }
        }

        QList<QPair<QTextCursor, QString> > multiText;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            cursor.setPosition(block.position());
            multiText.append(qMakePair(cursor, m_commentDefinition.singleLine));
        }
        // 同时将多组组插入信息压入撤销栈
        insertMultiTextEx(multiText);
    }

    // adjust selection when commenting out
    if (hasSelection && !doMultiLineStyleUncomment && !doSingleLineStyleUncomment) {
        cursor = textCursor();
        if (!doMultiLineStyleComment)
            start = startBlock.position(); // move the comment into the selection
        int lastSelPos = anchorIsStart ? cursor.position() : cursor.anchor();
        if (anchorIsStart) {
            cursor.setPosition(start);
            cursor.setPosition(lastSelPos, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(lastSelPos);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
        }
        setTextCursor(cursor);
    }
}

void TextEdit::removeComment()
{
    //此函数是删除了unCommentSelection()的if-else的comment分支得来的
    if (!m_commentDefinition.isValid()) {
        return;
    }

    QString tep = m_commentDefinition.singleLine;
    if (tep.isEmpty()) {
        tep = m_commentDefinition.multiLineStart;
    }
    QString abb = tep.remove(QRegExp("\\s"));

    if (m_isSelectAll) {
        QPlainTextEdit::selectAll();
    }

    QTextCursor cursor = textCursor();
    QTextDocument *doc = cursor.document();

    int pos = cursor.position();
    int anchor = cursor.anchor();
    int start = qMin(anchor, pos);
    int end = qMax(anchor, pos);
    bool anchorIsStart = (anchor == start);

    QTextBlock startBlock = doc->findBlock(start);
    QTextBlock endBlock = doc->findBlock(end);

    if (end > start && endBlock.position() == end) {
        --end;
        endBlock = endBlock.previous();
    }

    bool doMultiLineStyleUncomment = false;
    bool doMultiLineStyleComment = false;
    bool doSingleLineStyleUncomment = false;

    bool hasSelection = cursor.hasSelection();

    if (hasSelection && m_commentDefinition.hasMultiLineStyle()) {
        QString startText = startBlock.text();
        int startPos = start - startBlock.position();
        const int multiLineStartLength = m_commentDefinition.multiLineStart.length();
        bool hasLeadingCharacters = !startText.left(startPos).trimmed().isEmpty();

        if (startPos >= multiLineStartLength
                && isComment(startText,
                             startPos - multiLineStartLength,
                             m_commentDefinition.multiLineStart)) {
            startPos -= multiLineStartLength;
            start -= multiLineStartLength;
        }

        bool hasSelStart = startPos <= startText.length() - multiLineStartLength
                           && isComment(startText, startPos, m_commentDefinition.multiLineStart);

        QString endText = endBlock.text();
        int endPos = end - endBlock.position();
        const int multiLineEndLength = m_commentDefinition.multiLineEnd.length();
        bool hasTrailingCharacters =
            !endText.left(endPos).remove(m_commentDefinition.singleLine).trimmed().isEmpty()
            && !endText.mid(endPos).trimmed().isEmpty();

        if (endPos <= endText.length() - multiLineEndLength
                && isComment(endText, endPos, m_commentDefinition.multiLineEnd)) {
            endPos += multiLineEndLength;
            end += multiLineEndLength;
        }

        bool hasSelEnd = endPos >= multiLineEndLength
                         && isComment(endText, endPos - multiLineEndLength, m_commentDefinition.multiLineEnd);
        doMultiLineStyleUncomment = hasSelStart && hasSelEnd;
        doMultiLineStyleComment = !doMultiLineStyleUncomment
                                  && (hasLeadingCharacters
                                      || hasTrailingCharacters
                                      || !m_commentDefinition.hasSingleLineStyle());
    } else if (!hasSelection && !m_commentDefinition.hasSingleLineStyle()) {

        QString text = startBlock.text().trimmed();
        doMultiLineStyleUncomment = text.startsWith(m_commentDefinition.multiLineStart)
                                    && text.endsWith(m_commentDefinition.multiLineEnd);
        doMultiLineStyleComment = !doMultiLineStyleUncomment && !text.isEmpty();
        start = startBlock.position();
        end = endBlock.position() + endBlock.length() - 1;

        if (doMultiLineStyleUncomment) {
            int offset = 0;
            text = startBlock.text();
            const int length = text.length();
            while (offset < length && text.at(offset).isSpace())
                ++offset;
            start += offset;
        }
    }

    if (doMultiLineStyleUncomment) {
        // 注意删除先后顺序不能调换
        QList<QTextCursor> multiText;
        cursor.setPosition(end);
        cursor.movePosition(QTextCursor::PreviousCharacter,
                            QTextCursor::KeepAnchor,
                            m_commentDefinition.multiLineEnd.length());
        multiText.append(cursor);
        cursor.setPosition(start);
        cursor.movePosition(QTextCursor::NextCharacter,
                            QTextCursor::KeepAnchor,
                            m_commentDefinition.multiLineStart.length());
        multiText.append(cursor);

        // 同时删除多组注释文本
        deleteMultiTextEx(multiText);
    } else if (textCursor().hasSelection() && m_commentDefinition.singleLine.isEmpty()) {
        doSingleLineStyleUncomment = false;
    } else {
        int tmp = 0;//备注偏移量，判断备注标记后面有没有空格
        endBlock = endBlock.next();
        doSingleLineStyleUncomment = true;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            QString text = block.text().trimmed();
            if (!text.isEmpty() && m_commentDefinition.singleLine.isEmpty()) {
                if (text.startsWith(abb)) {
                    doSingleLineStyleUncomment = false;
                    break;
                }
            }

            if (!text.isEmpty() && (!text.startsWith(m_commentDefinition.singleLine) || (!text.startsWith(m_commentDefinition.multiLineStart)))) {
                if (!text.startsWith(abb)) {
                    doSingleLineStyleUncomment = false;
                    break;
                }
            }
        }

        int singleLineLength = m_commentDefinition.singleLine.length();
        QString text = startBlock.text().trimmed();

        if (text.startsWith(m_commentDefinition.singleLine)) {
            tmp = 0;
        } else if (text.startsWith(abb)) {
            tmp = 1;
        }
        QString check = "";

        QList<QTextCursor> multiText;
        for (QTextBlock block = startBlock; block != endBlock; block = block.next()) {
            if (doSingleLineStyleUncomment) {
                text = block.text();
                int i = 0;
                if (tmp == 1)
                    check = abb;
                else {
                    check = m_commentDefinition.singleLine;
                }
                while (i <= text.size() - singleLineLength) {
                    if (isComment(text, i, check)) {
                        cursor.setPosition(block.position() + i);
                        cursor.movePosition(QTextCursor::NextCharacter,
                                            QTextCursor::KeepAnchor,
                                            singleLineLength - tmp);
                        // 添加待删除的文本
                        multiText.append(cursor);
                        break;
                    }
                    if (i < text.size()) {
                        if (!text.at(i).isSpace())
                            break;
                    }

                    ++i;
                }
            }
        }

        if (!multiText.isEmpty()) {
            // 同时删除多组注释文本
            deleteMultiTextEx(multiText);
        }
    }

    // adjust selection when commenting out
    if (hasSelection && !doMultiLineStyleUncomment && !doSingleLineStyleUncomment) {
        cursor = textCursor();
        if (!doMultiLineStyleComment)
            start = startBlock.position(); // move the comment into the selection
        int lastSelPos = anchorIsStart ? cursor.position() : cursor.anchor();
        if (anchorIsStart) {
            cursor.setPosition(start);
            cursor.setPosition(lastSelPos, QTextCursor::KeepAnchor);
        } else {
            cursor.setPosition(lastSelPos);
            cursor.setPosition(start, QTextCursor::KeepAnchor);
        }
        setTextCursor(cursor);
    }
}

bool TextEdit::blockContainStrBrackets(int line)
{
    //获取行数文本块
    QTextBlock curBlock = document()->findBlockByNumber(line);
    QString text = curBlock.text();
    //若存在字符串行，多个字符串中间的 '{' '}' 同样被忽略
    QRegExp regExp("\".*\"");
    if (text.contains(regExp)) {
        QString curText = text.remove(regExp);
        return curText.contains("{");
    } else {
        return text.contains("{");
    }
}

/**
 * @brief 根据传入的起始行号 \a line ，查找在此行号下的折叠区域，查找后将返回查询过程中的折叠区域起始文本块
 *      \a beginBlock 和结束文本块 \a endBlock 。
 * @param line          查找起始行号
 * @param beginBlock    起始文本块
 * @param endBlock      结束文本框
 * @param curBlock      行号对应文本块
 * @return 是否查找到折叠区域
 */
bool TextEdit::findFoldBlock(int line, QTextBlock &beginBlock, QTextBlock &endBlock, QTextBlock &curBlock)
{
    //使用统一 折叠判断算法 根据左右"{""}"高亮算法
    QTextDocument *doc = document();
    //获取行号对应文本块
    curBlock = doc->findBlockByNumber(line);

    //开始本文块 结束文本块
    beginBlock = curBlock.next();
    endBlock = curBlock.next();

    //如果是第一行不包括左括弧"{"
    if (line == 0 && !curBlock.text().contains("{")) {
        curBlock = curBlock.next();
    }

    //获取当前块文本左括弧所在光标
    int position = curBlock.position();
    int offset = curBlock.text().lastIndexOf('{');
    position += offset;

    //获取当前文本块第一个字符光标
    QChar begin = '{', end = '}';
    QTextCursor cursor = textCursor();
    cursor.setPosition(position, QTextCursor::MoveAnchor);
    cursor.clearSelection();

    //左括弧光标 右括弧光标
    QTextCursor bracketBeginCursor = cursor;
    QTextCursor bracketEndCursor = cursor;
    bracketBeginCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    //获取最后右括弧光标
    int braceDepth = 0;
    // 判断当前是否处于字符串内，字符串内的 { } 不进行统计
    bool inCodeString = false;
    QChar c;
    while (!(c = doc->characterAt(position)).isNull()) {
        // 判断是否处于字符串内，已在字符串内将不处理其它字符，只判断结束处理
        if (inCodeString) {
            // 判断 " 前是否存在转义字符，若不存在，则退出字符串
            if ('"' == c
                    && '\\' != doc->characterAt(position - 1)) {
                inCodeString = false;
            }
        } else {
            if (c == begin) {
                braceDepth++;
            } else if (c == end) {
                braceDepth--;

                if (0 == braceDepth) {
                    bracketEndCursor = QTextCursor(doc);
                    bracketEndCursor.setPosition(position);
                    bracketEndCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                    endBlock = bracketEndCursor.block();
                    break;
                }
            } else if ('"' == c) {
                inCodeString = true;
            }
        }

        position++;
    }

    return 0 == braceDepth;
}

/**
 * @brief 文档内容变更时触发
 *      当前用于记录中键更新前后的动作并压入撤销栈，需要注意鼠标中键不会移除字符仅会插入选中的字符。
 * @param from          更新的光标位置
 * @param charsRemoved  移除的字符数
 * @param charsAdded    插入的字符数
 */
void TextEdit::onTextContentChanged(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(charsRemoved)

    // 判断是否正在执行中键黏贴动作
    if (m_MidButtonPatse) {
        QUndoCommand *undo = new QUndoCommand;
        m_pUndoStack->push(undo);

        // 记录鼠标中键变更，滞后插入撤销栈，在push()时不执行redo()
        QTextCursor cursor = textCursor();
        cursor.setPosition(from);
        cursor.setPosition(from + charsAdded, QTextCursor::KeepAnchor);
        // 取得已插入的文本信息
        QString insertText = cursor.selectedText();
        cursor.setPosition(from);
        (void)new MidButtonInsertTextUndoCommand(cursor, insertText, this, undo);

        m_MidButtonPatse = false;
    }
}
