// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/vnotemainwindow.h"
#include "views/leftview.h"
#include "views/middleview.h"
#include "views/homepage.h"
#include "views/splashview.h"
#include "views/middleviewsortfilter.h"
#include "views/webrichtexteditor.h"

#include "common/vtextspeechandtrmanager.h"
#include "common/standarditemcommon.h"
#include "common/vnotedatamanager.h"
#include "common/vnotea2tmanager.h"
#include "common/vnoteitem.h"
#include "common/vnoteforlder.h"
#include "common/actionmanager.h"
#include "common/metadataparser.h"

#include "widgets/vnotemultiplechoiceoptionwidget.h"

#include "common/utils.h"
#include "common/actionmanager.h"
#include "common/setting.h"
#include "common/performancemonitor.h"
#include "common/jscontent.h"

#include "db/vnotefolderoper.h"
#include "db/vnoteitemoper.h"
#include "db/vnotedbmanager.h"

#include "dbus/dbuslogin1manager.h"

#include "dialog/vnotemessagedialog.h"
#include "views/vnoterecordbar.h"
#include "widgets/vnoteiconbutton.h"
#include "widgets/vnotepushbutton.h"
#include "task/vnmainwnddelayinittask.h"
#include "task/filecleanupworker.h"

#ifdef IMPORT_OLD_VERSION_DATA
#include "importolddata/upgradeview.h"
#include "importolddata/upgradedbutil.h"
#endif

#include "vnoteapplication.h"

#include <DApplication>
#include <DToolButton>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DLog>
#include <DStatusBar>
#include <DTitlebar>
#include <DSettingsDialog>
#include <DSysInfo>
#include <DFileDialog>

#include <QScrollBar>
#include <QLocale>
#include <QDesktopServices>

static OpsStateInterface *stateOperation = nullptr;

/**
 * @brief VNoteMainWindow::VNoteMainWindow
 * @param parent
 */
VNoteMainWindow::VNoteMainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    //Init app data here
    initAppSetting();
    initUI();
    initConnections();
    // Request DataManager load  note folders
    initData();
    initShortcuts();
    initA2TManager();
    //Init the login manager
    initLogin1Manager();
    //Init delay task
    delayInitTasks();
}

/**
 * @brief VNoteMainWindow::~VNoteMainWindow
 */
VNoteMainWindow::~VNoteMainWindow()
{
    release();
}

/**
 * @brief VNoteMainWindow::initUI
 */
void VNoteMainWindow::initUI()
{
    initTitleBar();
    initMainView();
}

/**
 * @brief VNoteMainWindow::initData
 */
void VNoteMainWindow::initData()
{
    VNoteDataManager::instance()->reqNoteDefIcons();
    VNoteDataManager::instance()->reqNoteFolders();
    VNoteDataManager::instance()->reqNoteItems();
}

/**
 * @brief VNoteMainWindow::initAppSetting
 */
void VNoteMainWindow::initAppSetting()
{
    stateOperation = OpsStateInterface::instance();
}

/**
 * @brief VNoteMainWindow::initConnections
 */
void VNoteMainWindow::initConnections()
{
    connect(VNoteDataManager::instance(), &VNoteDataManager::onAllDatasReady,
            this, &VNoteMainWindow::onVNoteFoldersLoaded);

    connect(m_noteSearchEdit, &DSearchEdit::editingFinished,
            this, &VNoteMainWindow::onVNoteSearch);

    connect(m_noteSearchEdit, &DSearchEdit::textChanged,
            this, &VNoteMainWindow::onVNoteSearchTextChange);

    connect(m_leftView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &VNoteMainWindow::onVNoteFolderChange);

    connect(m_middleView, SIGNAL(currentChanged(const QModelIndex &)),
            this, SLOT(onVNoteChange(const QModelIndex &)));

    connect(m_addNotepadBtn, &DPushButton::clicked,
            this, &VNoteMainWindow::onNewNotebook);

    connect(m_addNoteBtn, &VNoteIconButton::clicked,
            this, &VNoteMainWindow::onNewNote);

    connect(m_wndHomePage, &HomePage::sigAddFolderByInitPage,
            this, &VNoteMainWindow::addNotepad);

    connect(m_recordBar, &VNoteRecordBar::sigStartRecord,
            this, &VNoteMainWindow::onStartRecord);
    connect(m_recordBar, &VNoteRecordBar::sigFinshRecord,
            this, &VNoteMainWindow::onFinshRecord);
    connect(m_recordBar, &VNoteRecordBar::sigPlayVoice,
            this, &VNoteMainWindow::onPlayPlugVoicePlay);
    connect(m_recordBar, &VNoteRecordBar::sigPauseVoice,
            this, &VNoteMainWindow::onPlayPlugVoicePause);
    connect(m_recordBar, &VNoteRecordBar::sigWidgetClose,
            this, &VNoteMainWindow::onPlayPlugVoiceStop);
    connect(m_recordBar, &VNoteRecordBar::sigDeviceExceptionMsgShow,
            this, &VNoteMainWindow::showDeviceExceptionErrMessage);
    connect(m_recordBar, &VNoteRecordBar::sigDeviceExceptionMsgClose,
            this, &VNoteMainWindow::closeDeviceExceptionErrMessage);

    //Bind all context menu states handler
    connect(ActionManager::Instance()->notebookContextMenu(), &DMenu::aboutToShow,
            this, &VNoteMainWindow::onMenuAbout2Show);
    connect(ActionManager::Instance()->notebookContextMenu(), &DMenu::triggered,
            this, &VNoteMainWindow::onMenuAction);
    connect(ActionManager::Instance()->noteContextMenu(), &DMenu::aboutToShow,
            this, &VNoteMainWindow::onMenuAbout2Show);
    connect(ActionManager::Instance()->noteContextMenu(), &DMenu::triggered,
            this, &VNoteMainWindow::onMenuAction);
    connect(ActionManager::Instance()->saveNoteContextMenu(), &DMenu::aboutToShow,
            this, &VNoteMainWindow::onMenuAbout2Show);
    //处理笔记拖拽释放
    connect(m_leftView, &LeftView::dropNotesEnd, this, &VNoteMainWindow::onDropNote);
    connect(m_leftView, &LeftView::renameVNoteFolder, this, &VNoteMainWindow::onVNoteFolderRename);
    //处理详情页刷新请求
    connect(m_middleView, &MiddleView::requestChangeRightView, this, &VNoteMainWindow::changeRightView);
    //处理详情页多选操作
    connect(m_multipleSelectWidget, &VnoteMultipleChoiceOptionWidget::requestMultipleOption, this, &VNoteMainWindow::handleMultipleOption);

    connect(m_viewChange, &DIconButton::clicked, this, &VNoteMainWindow::onViewChangeClicked);
    //关联图片插入按钮点击事件
    connect(m_imgInsert, &VNoteIconButton::clicked, this, &VNoteMainWindow::onInsertImageToWebEditor);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &VNoteMainWindow::onThemeChanged);
    connect(JsContent::instance(), &JsContent::playVoice, this, &VNoteMainWindow::onWebVoicePlay);
    connect(m_richTextEdit, &WebRichTextEditor::currentSearchEmpty, this, &VNoteMainWindow::onWebSearchEmpty);
    connect(m_richTextEdit, &WebRichTextEditor::contentChanged, m_middleView, &MiddleView::onNoteChanged, Qt::QueuedConnection);
    //创建笔记
    connect(JsContent::instance(), &JsContent::createNote, this, &VNoteMainWindow::onAddNoteShortcut, Qt::QueuedConnection);
}

/**
 * @brief VNoteMainWindow::changeRightView
 */
//刷新详情页显示
void VNoteMainWindow::changeRightView(bool isMultiple)
{
    m_multipleSelectWidget->setNoteNumber(m_middleView->getSelectedCount());
    if (isMultiple) {
        if (m_stackedRightMainWidget->currentWidget() == m_rightViewHolder) {
            m_stackedRightMainWidget->setCurrentWidget(m_multipleSelectWidget);
        }
        //多选插入图片按钮禁用
        m_imgInsert->setDisabled(true);
        bool moveButtonEnable = true;
        if (stateOperation->isVoice2Text()
                || stateOperation->isSearching()
                || 1 == m_leftView->folderCount()) {
            moveButtonEnable = false;
        }
        m_multipleSelectWidget->enableButtons(m_middleView->haveText(), m_middleView->haveVoice(), moveButtonEnable);
    } else {
        if (m_stackedRightMainWidget->currentWidget() == m_multipleSelectWidget) {
            m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
            //多选切换为详情页时，刷新数据
            m_richTextEdit->initData(m_middleView->getCurrVNotedata(), m_searchKey);
        }
        //恢复单选如果有笔记图片按钮可用
        if (nullptr != m_middleView->getCurrVNotedata()) {
            m_imgInsert->setDisabled(false);
        }
    }
}

/**
 * @brief VNoteMainWindow::initShortcuts
 */
void VNoteMainWindow::initShortcuts()
{
    m_stEsc.reset(new QShortcut(this));
    m_stEsc->setKey(Qt::Key_Escape);
    m_stEsc->setContext(Qt::WidgetWithChildrenShortcut);
    m_stEsc->setAutoRepeat(false);
    connect(m_stEsc.get(), &QShortcut::activated, this, &VNoteMainWindow::onEscShortcut);

    m_stPopupMenu.reset(new QShortcut(this));
    m_stPopupMenu->setKey(Qt::ALT + Qt::Key_M);
    m_stPopupMenu->setContext(Qt::ApplicationShortcut);
    m_stPopupMenu->setAutoRepeat(false);
    connect(m_stPopupMenu.get(), &QShortcut::activated, this, &VNoteMainWindow::onPoppuMenuShortcut);

    //*******************LeftView Shortcuts****************************
    //Add notebook
    m_stNewNotebook.reset(new QShortcut(this));
    m_stNewNotebook->setKey(Qt::CTRL + Qt::Key_N);
    m_stNewNotebook->setContext(Qt::ApplicationShortcut);
    m_stNewNotebook->setAutoRepeat(false);
    connect(m_stNewNotebook.get(), &QShortcut::activated, this, &VNoteMainWindow::onAddNotepadShortcut);

    //Rename notebook
    m_stRemNotebook.reset(new QShortcut(this));
    m_stRemNotebook->setKey(Qt::Key_F2);
    m_stRemNotebook->setContext(Qt::ApplicationShortcut);
    m_stRemNotebook->setAutoRepeat(false);
    connect(m_stRemNotebook.get(), &QShortcut::activated, this, &VNoteMainWindow::onReNameNotepadShortcut);

    //*******************MiddleView Shortcuts***************************
    //Add note
    m_stNewNote.reset(new QShortcut(this));
    m_stNewNote->setKey(Qt::CTRL + Qt::Key_B);
    m_stNewNote->setContext(Qt::ApplicationShortcut);
    m_stNewNote->setAutoRepeat(false);
    connect(m_stNewNote.get(), &QShortcut::activated, this, &VNoteMainWindow::onAddNoteShortcut);

    //Rename note
    m_stRemNote.reset(new QShortcut(this));
    m_stRemNote->setKey(Qt::Key_F3);
    m_stRemNote->setContext(Qt::ApplicationShortcut);
    m_stRemNote->setAutoRepeat(false);
    connect(m_stRemNote.get(), &QShortcut::activated, this, &VNoteMainWindow::onRenameNoteShortcut);

    //*******************RightView Shortcuts*****************************
    //Play/Pause
    m_stPlayorPause.reset(new QShortcut(this));
    m_stPlayorPause->setKey(Qt::Key_Space);
    m_stPlayorPause->setContext(Qt::ApplicationShortcut);
    m_stPlayorPause->setAutoRepeat(false);
    connect(m_stPlayorPause.get(), &QShortcut::activated, this, &VNoteMainWindow::onPlayPauseShortcut);

    //Add new recording
    m_stRecording.reset(new QShortcut(this));
    m_stRecording->setKey(Qt::CTRL + Qt::Key_R);
    m_stRecording->setContext(Qt::ApplicationShortcut);
    m_stRecording->setAutoRepeat(false);
    connect(m_stRecording.get(), &QShortcut::activated, this, &VNoteMainWindow::onRecordShorcut);

    //Save as Text
    m_stSaveAsText.reset(new QShortcut(this));
    m_stSaveAsText->setKey(Qt::CTRL + Qt::Key_S);
    m_stSaveAsText->setContext(Qt::ApplicationShortcut);
    m_stSaveAsText->setAutoRepeat(false);
    connect(m_stSaveAsText.get(), &QShortcut::activated, this, &VNoteMainWindow::onSaveNoteShortcut);

    //Save recordings
    m_stSaveVoices.reset(new QShortcut(this));
    m_stSaveVoices->setKey(Qt::CTRL + Qt::Key_D);
    m_stSaveVoices->setContext(Qt::ApplicationShortcut);
    m_stSaveVoices->setAutoRepeat(false);
    connect(m_stSaveVoices.get(), &QShortcut::activated, this, &VNoteMainWindow::onSaveVoicesShortcut);

    //Notebook/Note/Detial delete key
    m_stDelete.reset(new QShortcut(this));
    m_stDelete->setKey(Qt::Key_Delete);
    m_stDelete->setContext(Qt::ApplicationShortcut);
    m_stDelete->setAutoRepeat(false);
    connect(m_stDelete.get(), &QShortcut::activated, this, &VNoteMainWindow::onDeleteShortcut);

    m_stPreviewShortcuts.reset(new QShortcut(this));
    m_stPreviewShortcuts->setKey(QString("Ctrl+Shift+/"));
    m_stPreviewShortcuts->setContext(Qt::ApplicationShortcut);
    m_stPreviewShortcuts->setAutoRepeat(false);
    connect(m_stPreviewShortcuts.get(), &QShortcut::activated,
            this, &VNoteMainWindow::onPreviewShortcut);
}

/**
 * @brief VNoteMainWindow::initTitleBar
 */
void VNoteMainWindow::initTitleBar()
{
    titlebar()->setFixedHeight(VNOTE_TITLEBAR_HEIGHT);
    titlebar()->setTitle("");
    // Add logo
    titlebar()->setIcon(QIcon::fromTheme(DEEPIN_VOICE_NOTE));

    initMenuExtension();
    titlebar()->setMenu(m_menuExtension);

    initTitleIconButton(); //初始化标题栏图标控件
    //自定义标题栏中部控件的基控件
    QWidget *titleWidget = new QWidget();
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget); //标题栏中部控件以横向列表展示
    titleLayout->setSpacing(10);
    titleLayout->addWidget(m_viewChange); //添加记事本列表收起控件按钮

    // Search note
    m_noteSearchEdit = new DSearchEdit(this);
    DFontSizeManager::instance()->bind(m_noteSearchEdit, DFontSizeManager::T6);
    m_noteSearchEdit->setMaximumSize(QSize(VNOTE_SEARCHBAR_W, VNOTE_SEARCHBAR_H));
    m_noteSearchEdit->setMinimumSize(QSize(VNOTE_SEARCHBAR_MIN_W, VNOTE_SEARCHBAR_H));
    m_noteSearchEdit->setPlaceHolder(DApplication::translate("TitleBar", "Search"));
    m_noteSearchEdit->lineEdit()->installEventFilter(this);

    titlebar()->addWidget(titleWidget, Qt::AlignLeft); //图标控件居左显示
    titlebar()->addWidget(m_noteSearchEdit/*, Qt::AlignCenter*/); //将搜索框添加到标题栏居中显示
    titlebar()->addWidget(m_imgInsert, Qt::AlignRight);

    QWidget::setTabOrder(titlebar(), m_viewChange); //设置title焦点传递的下一个控件
    QWidget::setTabOrder(m_viewChange, m_imgInsert); //设置焦点传递顺序

    //捕捉标题栏最右边控件事件
    QObject *btn = titlebar()->findChild<QObject *>("DTitlebarDWindowCloseButton");

    if (nullptr != btn) {
        btn->installEventFilter(this);
    }
    //改变标题栏焦点策略，解决键盘交互第一次显示"关于"窗口后ESC关闭窗口菜单按钮无焦点
    titlebar()->setFocusPolicy(Qt::ClickFocus);
}

/**
 * @brief VNoteMainWindow::initMainView
 */
void VNoteMainWindow::initMainView()
{
    initSpliterView();
    initSplashView();
    initEmptyFoldersView();
    m_centerWidget = new QWidget(this);

    m_stackedWidget = new DStackedWidget(m_centerWidget);
    m_stackedWidget->setContentsMargins(0, 0, 0, 0);
    m_stackedWidget->insertWidget(WndSplashAnim, m_splashView);
    m_stackedWidget->insertWidget(WndHomePage, m_wndHomePage);
    m_stackedWidget->insertWidget(WndNoteShow, m_mainWndSpliter);

    WindowType defaultWnd = WndSplashAnim;

#ifdef IMPORT_OLD_VERSION_DATA
    //*******Upgrade old Db code here only********
    initUpgradeView();
    m_stackedWidget->insertWidget(WndUpgrade, m_upgradeView);

    int upgradeState = UpgradeDbUtil::readUpgradeState();

    UpgradeDbUtil::checkUpdateState(upgradeState);

    m_fNeedUpgradeOldDb = UpgradeDbUtil::needUpdateOldDb(upgradeState);

    qInfo() << "Check if need upgrade old version:" << upgradeState
            << " isNeed:" << m_fNeedUpgradeOldDb;

    if (m_fNeedUpgradeOldDb) {
        defaultWnd = WndUpgrade;
    }
#endif
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_stackedWidget);
    m_centerWidget->setLayout(layout);

    switchWidget(defaultWnd);
    setCentralWidget(m_centerWidget);
    setTitlebarShadowEnabled(true);

    showNotepadList();
}

/**
 * @brief VNoteMainWindow::initLeftView
 */
void VNoteMainWindow::initLeftView()
{
    m_leftViewHolder = new QWidget(m_mainWndSpliter);
    m_leftViewHolder->setObjectName("leftMainLayoutHolder");
    m_leftViewHolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_leftViewHolder->setMaximumWidth(VNOTE_LEFTVIEW_W);
    m_leftViewHolder->setMinimumWidth(VNOTE_LEFTVIEW_MIN_W);
    m_leftViewHolder->setBackgroundRole(DPalette::Base);
    m_leftViewHolder->setAutoFillBackground(true);

    QVBoxLayout *leftHolderLayout = new QVBoxLayout();
    leftHolderLayout->setSpacing(0);
    leftHolderLayout->setContentsMargins(0, 0, 0, 0);
    m_leftView = new LeftView(m_leftViewHolder);

    //背景色透明，隐藏disable时的背景色
    DPalette pb = DApplicationHelper::instance()->palette(m_leftView->viewport());
    pb.setBrush(DPalette::Base, QColor(0, 0, 0, 0));
    m_leftView->viewport()->setPalette(pb);

    m_leftView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_leftView->setContentsMargins(0, 0, 0, 0);
    m_leftView->setHeaderHidden(true);
    m_leftView->setFrameShape(QFrame::NoFrame);
    m_leftView->setItemsExpandable(false);
    m_leftView->setIndentation(0);
    QModelIndex notepadRootIndex = m_leftView->getNotepadRootIndex();
    m_leftView->expand(notepadRootIndex);
    leftHolderLayout->addWidget(m_leftView);

    m_addNotepadBtn = new DPushButton(DApplication::translate("VNoteMainWindow", "Create Notebook"),
                                      m_leftViewHolder);
    m_addNotepadBtn->installEventFilter(this);

    QVBoxLayout *btnLayout = new QVBoxLayout();
    btnLayout->addWidget(m_addNotepadBtn);
    btnLayout->setContentsMargins(10, 0, 10, 10);
    leftHolderLayout->addLayout(btnLayout, Qt::AlignHCenter);
    m_leftViewHolder->setLayout(leftHolderLayout);
}

/**
 * @brief VNoteMainWindow::initMiddleView
 */
void VNoteMainWindow::initMiddleView()
{
    m_middleViewHolder = new QWidget(m_mainWndSpliter);
    m_middleViewHolder->setObjectName("middleMainLayoutHolder");
    m_middleViewHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_middleViewHolder->setMaximumWidth(VNOTE_MIDDLEVIEW_W);
    m_middleViewHolder->setMinimumWidth(VNOTE_MIDDLEVIEW_MIN_W);
    m_middleViewHolder->setBackgroundRole(DPalette::Base);
    m_middleViewHolder->setAutoFillBackground(true);

    QVBoxLayout *middleHolderLayout = new QVBoxLayout();
    middleHolderLayout->setSpacing(0);
    middleHolderLayout->setContentsMargins(0, 5, 0, 5);

    m_middleView = new MiddleView(m_middleViewHolder);
    m_addNoteBtn = new DFloatingButton(DStyle::StandardPixmap::SP_IncreaseElement, m_middleViewHolder);
    m_addNoteBtn->setFixedSize(QSize(54, 54));
    m_addNoteBtn->raise();

    m_buttonAnchor = new DAnchorsBase(m_addNoteBtn);
    m_buttonAnchor->setAnchor(Qt::AnchorLeft, m_middleView, Qt::AnchorLeft);
    m_buttonAnchor->setAnchor(Qt::AnchorBottom, m_middleView, Qt::AnchorBottom);
    m_buttonAnchor->setBottomMargin(17);
    m_buttonAnchor->setLeftMargin(97);

    m_middleView->installEventFilter(this);
    m_addNoteBtn->installEventFilter(this);
    m_middleViewHolder->installEventFilter(this);
    // ToDo:
    //    Add Left view widget here
    m_noteBookName = new DLabel(this);
    DFontSizeManager::instance()->bind(m_noteBookName, DFontSizeManager::T4, QFont::Medium);
    QVBoxLayout *labelLayout = new QVBoxLayout();
    labelLayout->setMargin(10);
    labelLayout->addWidget(m_noteBookName);

    middleHolderLayout->addLayout(labelLayout);
    middleHolderLayout->addWidget(m_middleView);

    m_middleViewHolder->setLayout(middleHolderLayout);
}

/**
 * @brief VNoteMainWindow::initRightView
 */
void VNoteMainWindow::initRightView()
{
    m_stackedRightMainWidget = new QStackedWidget(m_mainWndSpliter);
    m_rightViewHolder = new QWidget(this);

    //初始化多选详情页面
    m_multipleSelectWidget = new VnoteMultipleChoiceOptionWidget(this);
    m_multipleSelectWidget->setBackgroundRole(DPalette::Base);
    m_multipleSelectWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_multipleSelectWidget->setAutoFillBackground(true);

    //添加多选与单选详情页
    m_stackedRightMainWidget->addWidget(m_rightViewHolder);
    m_stackedRightMainWidget->addWidget(m_multipleSelectWidget);

    m_rightViewHolder->setObjectName("rightMainLayoutHolder");
    m_rightViewHolder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_rightViewHolder->setBackgroundRole(DPalette::Base);
    m_rightViewHolder->setAutoFillBackground(true);

    QVBoxLayout *rightHolderLayout = new QVBoxLayout(m_rightViewHolder);
    rightHolderLayout->setSpacing(0);
    //设置框布局的左右下边界为4
    //右边界为防止QWebEngineView控件与应用边界为0时拉伸界面是出现黑边
    //大小4为ui要求录音块左右下边界为10，录音块控件自带外边界6，组合为10
    rightHolderLayout->setContentsMargins(4, 0, 4, 4);

    //TODO:
    //    Add record area code here
    m_recordBarHolder = new QWidget(m_rightViewHolder);
    //78: 语音块显示区域高度为64px，语音控件阴影宽6px，上下阴影12px，合计78px
    m_recordBarHolder->setFixedHeight(78);
    QVBoxLayout *recordBarHolderLayout = new QVBoxLayout(m_recordBarHolder);
    recordBarHolderLayout->setSpacing(0);
    //取消框布局自带的边界
    recordBarHolderLayout->setContentsMargins(0, 0, 0, 0);

    m_recordBar = new VNoteRecordBar(m_recordBarHolder);
    m_recordBar->setBackgroundRole(DPalette::Base);
    m_recordBar->setAutoFillBackground(true);
    m_recordBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_richTextEdit = new WebRichTextEditor(m_rightViewHolder);
    rightHolderLayout->addWidget(m_richTextEdit);
    m_richTextEdit->installEventFilter(this);

    recordBarHolderLayout->addWidget(m_recordBar);

    rightHolderLayout->addWidget(m_recordBarHolder);

    m_rightViewHolder->setLayout(rightHolderLayout);

    m_recordBar->setVisible(false);
    //初始化详情页面为当前笔记
    m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
}

/**
 * @brief VNoteMainWindow::initA2TManager
 */
void VNoteMainWindow::initA2TManager()
{
    //audio to text manager
    m_a2tManager = new VNoteA2TManager(this);

    connect(m_richTextEdit, &WebRichTextEditor::asrStart, this, &VNoteMainWindow::onA2TStart);
    connect(m_a2tManager, &VNoteA2TManager::asrError, this, &VNoteMainWindow::onA2TError);
    connect(m_a2tManager, &VNoteA2TManager::asrSuccess, this, &VNoteMainWindow::onA2TSuccess);
}

/**
 * @brief VNoteMainWindow::initLogin1Manager
 */
void VNoteMainWindow::initLogin1Manager()
{
    m_pLogin1Manager = new DBusLogin1Manager(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        QDBusConnection::systemBus(), this);

    connect(m_pLogin1Manager, &DBusLogin1Manager::PrepareForSleep,
            this, &VNoteMainWindow::onSystemDown);
    connect(m_pLogin1Manager, &DBusLogin1Manager::PrepareForShutdown,
            this, &VNoteMainWindow::onSystemDown);
}

/**
 * @brief VNoteMainWindow::holdHaltLock
 */
void VNoteMainWindow::holdHaltLock()
{
    m_lockFd = m_pLogin1Manager->Inhibit(
                   "shutdown:sleep",
                   DApplication::translate("AppMain", "Voice Notes"),
                   DApplication::translate("AppMain", "Recordings not saved"),
                   "block");

    if (m_lockFd.isError()) {
        qCritical() << "Init login manager error:" << m_lockFd.error();
    }
}

/**
 * @brief VNoteMainWindow::releaseHaltLock
 */
void VNoteMainWindow::releaseHaltLock()
{
    QDBusPendingReply<QDBusUnixFileDescriptor> releaseLock = m_lockFd;
    m_lockFd = QDBusPendingReply<QDBusUnixFileDescriptor>();
}

/**
 * @brief VNoteMainWindow::initDelayWork
 */
void VNoteMainWindow::initDelayWork()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    QDBusConnectionInterface *bus = connection.interface();
    bool isVailid = bus->isServiceRegistered("com.iflytek.aiassistant");
    //没有语音助手时，不显示语音助手相关功能
    if (!isVailid) {
        ActionManager::Instance()->visibleAiActions(isVailid);
    }
    stateOperation->operState(OpsStateInterface::StateAISrvAvailable, isVailid);
}

/**
 * @brief VNoteMainWindow::delayInitTasks
 */
void VNoteMainWindow::delayInitTasks()
{
    VNMainWndDelayInitTask *pMainWndDelayTask = new VNMainWndDelayInitTask(this);
    pMainWndDelayTask->setAutoDelete(true);
    pMainWndDelayTask->setObjectName("MainWindowDelayTask");

    QThreadPool::globalInstance()->start(pMainWndDelayTask);
}

/**
 * @brief VNoteMainWindow::onVNoteFoldersLoaded
 */
void VNoteMainWindow::onVNoteFoldersLoaded()
{
#ifdef IMPORT_OLD_VERSION_DATA
    if (m_fNeedUpgradeOldDb) {
        //Start upgrade if all components are ready.
        m_upgradeView->startUpgrade();
        return;
    }
#endif

    //If have folders show note view,else show
    //default home page
    if (loadNotepads() > 0) {
        m_leftView->setDefaultNotepadItem();
        switchWidget(WndNoteShow);
    } else {
        switchWidget(WndHomePage);
    }

    PerformanceMonitor::initializeAppFinish();

    //注册文件清理工作
    FileCleanupWorker *pFileCleanupWorker =
        new FileCleanupWorker(VNoteDataManager::instance()->getAllNotesInFolder(), this);
    pFileCleanupWorker->setAutoDelete(true);
    pFileCleanupWorker->setObjectName("FileCleanupWorker");
    QThreadPool::globalInstance()->start(pFileCleanupWorker);
}

/**
 * @brief VNoteMainWindow::onVNoteSearch
 */
void VNoteMainWindow::onVNoteSearch()
{
    if (m_noteSearchEdit->lineEdit()->hasFocus()) {
        QString text = m_noteSearchEdit->text();
        if (!text.isEmpty()) {
            //搜索内容不为空，切换为单选详情页面
            changeRightView(false);
            setSpecialStatus(SearchStart);
            if (m_searchKey == text && m_stackedRightMainWidget->currentWidget() == m_richTextEdit) { //搜索关键字不变只更新当前笔记搜索
                m_richTextEdit->searchText(m_searchKey);
            } else {
                m_searchKey = text;
                //重新搜索之前先更新笔记内容
                m_richTextEdit->updateNote();
                //重新搜索
                loadSearchNotes(m_searchKey);
            }
        } else {
            setSpecialStatus(SearchEnd);
        }
    }
}

/**
 * @brief VNoteMainWindow::onVNoteSearchTextChange
 * @param text
 */
void VNoteMainWindow::onVNoteSearchTextChange(const QString &text)
{
    if (text.isEmpty()) {
        setSpecialStatus(SearchEnd);
    }
}

/**
 * @brief VNoteMainWindow::onVNoteFolderChange
 * @param current
 * @param previous
 */
void VNoteMainWindow::onVNoteFolderChange(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    //记事本切换后刷新详情页
    changeRightView(false);
    VNoteFolder *data = static_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(current));
    if (!loadNotes(data)) {
        m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
        m_richTextEdit->initData(nullptr, m_searchKey, false);
        m_imgInsert->setDisabled(true);
        m_recordBar->setVisible(false);
    }
    m_buttonAnchor->setBottomMargin(7);
}

/**
 * @brief VNoteMainWindow::initSpliterView
 */
void VNoteMainWindow::initSpliterView()
{
    m_mainWndSpliter = new DSplitter(Qt::Horizontal, this);
    m_mainWndSpliter->setHandleWidth(2);
    m_mainWndSpliter->setChildrenCollapsible(false);

    initLeftView();
    initMiddleView();
    initRightView();

    // Disable spliter drag & resize
    /*
    QList<QWidget *> Children {m_middleViewHolder, m_stackedRightMainWidget};

    for (auto it : Children) {
        QSplitterHandle *handle = m_mainWndSpliter->handle(m_mainWndSpliter->indexOf(it));
        if (handle) {
            handle->setFixedWidth(2);
            handle->setDisabled(true);
        }
    }
    */
}

#ifdef IMPORT_OLD_VERSION_DATA
//*******Upgrade old Db code here only********

/**
 * @brief VNoteMainWindow::initUpgradeView
 */
void VNoteMainWindow::initUpgradeView()
{
    m_upgradeView = new UpgradeView(this);

    connect(m_upgradeView, &UpgradeView::upgradeDone,
    this, [this]() {
        //Clear the flag after upgrade & and call
        //the data loaded slot to refresh.
        m_fNeedUpgradeOldDb = false;

        onVNoteFoldersLoaded();

        qInfo() << "upgrade success.";
    },
    Qt::QueuedConnection);
}

#endif

/**
 * @brief VNoteMainWindow::initSplashView
 */
void VNoteMainWindow::initSplashView()
{
    m_splashView = new SplashView(this);
}

/**
 * @brief VNoteMainWindow::initEmptyFoldersView
 */
void VNoteMainWindow::initEmptyFoldersView()
{
    m_wndHomePage = new HomePage(this);
}

/**
 * @brief VNoteMainWindow::onStartRecord
 * @param path
 */
void VNoteMainWindow::onStartRecord(const QString &path)
{
    Q_UNUSED(path)
    setSpecialStatus(RecordStart);
    //Hold shutdown locker
    holdHaltLock();
}

/**
 * @brief VNoteMainWindow::onFinshRecord
 * @param voicePath
 * @param voiceSize
 */
void VNoteMainWindow::onFinshRecord(const QString &voicePath, qint64 voiceSize)
{
    if (voiceSize >= 1000) {
        m_richTextEdit->insertVoiceItem(voicePath, voiceSize);
    }
    setSpecialStatus(RecordEnd);

    //Release shutdonw locker
    releaseHaltLock();

    if (stateOperation->isAppQuit()) {
        //退出程序
        qApp->quit();
    }
}

/**
 * @brief VNoteMainWindow::onA2TStart
 * @param voiceBlock 语音数据
 */
void VNoteMainWindow::onA2TStart(const VNVoiceBlock *voiceBlock)
{
    m_voiceBlock = voiceBlock;

    if (nullptr == m_voiceBlock) {
        return;
    }
    //超过20分钟的语音不支持转文字
    if (m_voiceBlock->voiceSize > MAX_A2T_AUDIO_LEN_MS) {
        VNoteMessageDialog audioOutLimit(
            VNoteMessageDialog::AsrTimeLimit, this);
        audioOutLimit.exec();
    } else {
        setSpecialStatus(VoiceToTextStart); //更新状态
        QTimer::singleShot(0, this, [this]() {
            m_a2tManager->startAsr(m_voiceBlock->voicePath, m_voiceBlock->voiceSize); //开始转文字
        });
    }
}

/**
 * @brief VNoteMainWindow::onA2TError
 * 语音转文字失败
 * @param error
 */
void VNoteMainWindow::onA2TError(int error)
{
    emit JsContent::instance()->callJsSetVoiceText("", JsContent::AsrFlag::End);
    QString message = ""; //错误信息提示语
    if (error == VNoteA2TManager::NetworkError) {
        message = DApplication::translate(
                      "VNoteErrorMessage",
                      "The voice conversion failed due to the poor network connection, please have a check");
    } else {
        message = DApplication::translate(
                      "VNoteErrorMessage",
                      "Voice to text conversion failed");
    }
    showAsrErrMessage(message); //显示错误信息
    setSpecialStatus(VoiceToTextEnd); //更新状态
}

/**
 * @brief VNoteMainWindow::onA2TSuccess
 * 语音转文字成功
 * @param text 转写后的文本
 */
void VNoteMainWindow::onA2TSuccess(const QString &text)
{
    emit JsContent::instance()->callJsSetVoiceText(text, JsContent::AsrFlag::End); //将转写后的文本发送到web端
    setSpecialStatus(VoiceToTextEnd); //更新状态
}

/**
 * @brief VNoteMainWindow::onPreviewShortcut
 */
void VNoteMainWindow::onPreviewShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    //******************************Notebooks**************************************************
    QMap<QString, QString> shortcutNotebookKeymap = {
        //Notebook
        {DApplication::translate("Shortcuts", "New notebook"), "Ctrl+N"},
        {DApplication::translate("Shortcuts", "Rename notebook"), "F2"},
        {DApplication::translate("Shortcuts", "Delete notebook"), "Delete"},
    };

    QJsonObject notebookJsonGroup;
    notebookJsonGroup.insert("groupName", DApplication::translate("ShortcutsGroups", "Notebooks"));
    QJsonArray notebookJsonItems;

    for (QMap<QString, QString>::iterator it = shortcutNotebookKeymap.begin();
            it != shortcutNotebookKeymap.end(); ++it) {
        QJsonObject jsonItem;
        jsonItem.insert("name", it.key());
        jsonItem.insert("value", it.value().replace("Meta", "Super"));
        notebookJsonItems.append(jsonItem);
    }

    notebookJsonGroup.insert("groupItems", notebookJsonItems);
    jsonGroups.append(notebookJsonGroup);

    //******************************Notes**************************************************

    QMap<QString, QString> shortcutNoteKeymap = {
        //Note
        {DApplication::translate("Shortcuts", "New note"), "Ctrl+B"},
        {DApplication::translate("Shortcuts", "Rename note"), "F3"},
        {DApplication::translate("Shortcuts", "Delete note"), "Delete"},
        {DApplication::translate("Shortcuts", "Play/Pause"), "Space"},
        {DApplication::translate("Shortcuts", "Record voice"), "Ctrl+R"},
        {DApplication::translate("Shortcuts", "Save note"), "Ctrl+S"},
        {DApplication::translate("Shortcuts", "Save recordings"), "Ctrl+D"},
    };

    QJsonObject noteJsonGroup;
    noteJsonGroup.insert("groupName", DApplication::translate("ShortcutsGroups", "Notes"));
    QJsonArray noteJsonItems;

    for (QMap<QString, QString>::iterator it = shortcutNoteKeymap.begin();
            it != shortcutNoteKeymap.end(); ++it) {
        QJsonObject jsonItem;
        jsonItem.insert("name", it.key());
        jsonItem.insert("value", it.value().replace("Meta", "Super"));
        noteJsonItems.append(jsonItem);
    }

    noteJsonGroup.insert("groupItems", noteJsonItems);
    jsonGroups.append(noteJsonGroup);
    //******************************Edit***************************************************
    QList<QPair<QString, QString>> shortcutEditKeymap = {
        //Edit
        {DApplication::translate("Shortcuts", "Select all"), "Ctrl+A"},
        {DApplication::translate("Shortcuts", "Copy"), "Ctrl+C"},
        {DApplication::translate("Shortcuts", "Cut"), "Ctrl+X"},
        {DApplication::translate("Shortcuts", "Paste"), "Ctrl+V"},
        {DApplication::translate("Shortcuts", "Undo"), "Ctrl+Z"},
        {DApplication::translate("Shortcuts", "Redo"), "Ctrl+Y"},
        {DApplication::translate("Shortcuts", "Delete"), "Delete"},
    };

    QJsonObject editJsonGroup;
    editJsonGroup.insert("groupName", DApplication::translate("ShortcutsGroups", "Edit"));
    QJsonArray editJsonItems;

    for (int i = 0; i < shortcutEditKeymap.count(); i++) {
        QJsonObject jsonItem;
        jsonItem.insert("name", shortcutEditKeymap[i].first);
        QString value = shortcutEditKeymap[i].second;
        jsonItem.insert("value", value.replace("Meta", "Super"));
        editJsonItems.append(jsonItem);
    }

    editJsonGroup.insert("groupItems", editJsonItems);
    jsonGroups.append(editJsonGroup);
    //******************************Setting************************************************
    QMap<QString, QString> shortcutSettingKeymap = {
        //Setting
        //        {DApplication::translate("Shortcuts","Close window"),         "Alt+F4"},
        //        {DApplication::translate("Shortcuts","Resize window"),        "Ctrl+Alt+F"},
        //        {DApplication::translate("Shortcuts","Find"),                 "Ctrl+F"},
        {DApplication::translate("Shortcuts", "Help"), "F1"},
        {DApplication::translate("Shortcuts", "Display shortcuts"), "Ctrl+Shift+?"},
    };

    QJsonObject settingJsonGroup;
    settingJsonGroup.insert("groupName", DApplication::translate("ShortcutsGroups", "Settings"));
    QJsonArray settingJsonItems;

    for (QMap<QString, QString>::iterator it = shortcutSettingKeymap.begin();
            it != shortcutSettingKeymap.end(); ++it) {
        QJsonObject jsonItem;
        jsonItem.insert("name", it.key());
        jsonItem.insert("value", it.value().replace("Meta", "Super"));
        settingJsonItems.append(jsonItem);
    }

    settingJsonGroup.insert("groupItems", settingJsonItems);
    jsonGroups.append(settingJsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess(this);
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

/**
 * @brief VNoteMainWindow::closeEvent
 * @param event
 */
void VNoteMainWindow::closeEvent(QCloseEvent *event)
{
    if (checkIfNeedExit()) {
        if (stateOperation->isRecording()) {
            stateOperation->operState(OpsStateInterface::StateAppQuit, true);
            m_recordBar->stopRecord();
            event->ignore();
        } else {
            //退出程序
            qApp->quit();
        }
    } else {
        event->ignore();
    }
}

/**
 * @brief VNoteMainWindow::resizeEvent
 * @param event
 */
void VNoteMainWindow::resizeEvent(QResizeEvent *event)
{
    // 更新搜索框的大小
    if (this->geometry().width() >= DEFAULT_WINDOWS_WIDTH) {
        m_noteSearchEdit->resize(VNOTE_SEARCHBAR_W, VNOTE_SEARCHBAR_H);
    } else if (this->geometry().width() <= MIN_WINDOWS_WIDTH) {
        m_noteSearchEdit->resize(VNOTE_SEARCHBAR_MIN_W, VNOTE_SEARCHBAR_H);
    } else {
        double wMainWin = ((this->geometry().width() - MIN_WINDOWS_WIDTH) * 1.0) / (DEFAULT_WINDOWS_WIDTH - MIN_WINDOWS_WIDTH);
        int wsearch = static_cast<int>((VNOTE_SEARCHBAR_W - VNOTE_SEARCHBAR_MIN_W) * wMainWin) + VNOTE_SEARCHBAR_MIN_W;
        m_noteSearchEdit->resize(wsearch , VNOTE_SEARCHBAR_H);
    }

    if (m_asrErrMeassage && m_asrErrMeassage->isVisible()) {
        int xPos = (m_centerWidget->width() - m_asrErrMeassage->width()) / 2;
        int yPos = m_centerWidget->height() - m_asrErrMeassage->height() - 5;
        m_asrErrMeassage->move(xPos, yPos);
    }

    if (m_pDeviceExceptionMsg && m_pDeviceExceptionMsg->isVisible()) {
        int xPos = (m_centerWidget->width() - m_pDeviceExceptionMsg->width()) / 2;
        int yPos = m_centerWidget->height() - m_pDeviceExceptionMsg->height() - 5;
        m_pDeviceExceptionMsg->move(xPos, yPos);
    }

    Qt::WindowStates states = windowState();
    m_needShowMax = states.testFlag(Qt::WindowMaximized);

    //最大化状态同时拥有Qt::WindowActive状态会导致最大化图标错误,去除该状态
    if (m_needShowMax && states.testFlag(Qt::WindowActive)) {
        states.setFlag(Qt::WindowActive, false);
        setWindowState(states);
    }

    DMainWindow::resizeEvent(event);
}

/**
 * @brief VNoteMainWindow::checkIfNeedExit
 * @return true 关闭应用
 */
bool VNoteMainWindow::checkIfNeedExit()
{
    QScopedPointer<VNoteMessageDialog> pspMessageDialg;

    bool bNeedExit = true;

    //Is audio converting to text
    if (stateOperation->isVoice2Text()) {
        pspMessageDialg.reset(new VNoteMessageDialog(
                                  VNoteMessageDialog::AborteAsr,
                                  this));
    } else if (stateOperation->isRecording()) { //Is recording
        pspMessageDialg.reset(new VNoteMessageDialog(
                                  VNoteMessageDialog::AbortRecord,
                                  this));
    }

    if (!pspMessageDialg.isNull()) {
        //Use cancel exit.
        if (QDialog::Rejected == pspMessageDialg->exec()) {
            bNeedExit = false;
        }
    }

    return bNeedExit;
}

/**
 * @brief VNoteMainWindow::onVNoteChange
 * @param previous
 */
void VNoteMainWindow::onVNoteChange(const QModelIndex &previous)
{
    Q_UNUSED(previous);

    QModelIndex index = m_middleView->currentIndex();
    VNoteItem *data = static_cast<VNoteItem *>(StandardItemCommon::getStandardItemData(index));
    if (data == nullptr || stateOperation->isSearching()) {
        m_recordBar->setVisible(false);
    } else {
        m_recordBar->setVisible(true);
    }
    //多选笔记时不更新详情页内容
    if (!m_middleView->isMultipleSelected()) {
        m_richTextEdit->initData(data, m_searchKey, m_rightViewHasFouse);
    }
    //没有数据，插入图片按钮禁用
    m_imgInsert->setDisabled(nullptr == data);
    m_rightViewHasFouse = false;
}

/**
 * @brief VNoteMainWindow::onMenuAction
 * @param action
 */
void VNoteMainWindow::onMenuAction(QAction *action)
{
    ActionManager::ActionKind kind = ActionManager::Instance()->getActionKind(action);
    switch (kind) {
    case ActionManager::NotebookRename:
        editNotepad();
        break;
    case ActionManager::NotebookDelete: {
        VNoteMessageDialog confirmDialog(VNoteMessageDialog::DeleteFolder, this);
        if (VNoteBaseDialog::Accepted == confirmDialog.exec()) {
            delNotepad();
        }
    } break;
    case ActionManager::NotebookAddNew:
        addNote();
        break;
    case ActionManager::NoteDelete: {
        VNoteMessageDialog confirmDialog(VNoteMessageDialog::DeleteNote, this, m_middleView->getSelectedCount());
        if (VNoteBaseDialog::Accepted == confirmDialog.exec()) {
            delNote();
        }
    } break;
    case ActionManager::NoteAddNew:
        addNote();
        break;
    case ActionManager::NoteRename:
        editNote();
        break;
    case ActionManager::SaveNoteAsText:
        m_middleView->saveAs(MiddleView::SaveAsType::Text);
        break;
    case ActionManager::SaveNoteAsHtml:
        m_middleView->saveAs(MiddleView::SaveAsType::Html);
        break;
    case ActionManager::NoteSaveVoice:
        m_middleView->saveAs(MiddleView::SaveAsType::Voice);
        break;
    case ActionManager::NoteTop:
        m_middleView->noteStickOnTop();
        break;
    case ActionManager::NoteMove: {
        //删除前记录选中位置
        m_middleView->setNextSelection();

        QModelIndexList notesdataList = m_middleView->getAllSelectNote();
        if (notesdataList.size()) {
            QModelIndex selectFolder = m_leftView->selectMoveFolder(notesdataList);
            m_leftView->setNumberOfNotes(m_middleView->count());
            if (selectFolder.isValid() && m_leftView->doNoteMove(notesdataList, selectFolder)) {
                m_middleView->deleteModelIndexs(notesdataList);
                //删除后设置选中
                m_middleView->selectAfterRemoved();
            }
        }
    } break;
    default:
        break;
    }
}

/**
 * @brief VNoteMainWindow::onMenuAbout2Show
 */
void VNoteMainWindow::onMenuAbout2Show()
{
    //TODO:
    //    Add context menu item disable/enable code here
    //Eg:
    //ActionManager::Instance()->enableAction(ActionManager::NoteAddNew, false);
    DMenu *menu = static_cast<DMenu *>(sender());
    QAction *topAction = ActionManager::Instance()->getActionById(ActionManager::NoteTop);

    if (menu == ActionManager::Instance()->noteContextMenu()) {
        //右键弹出先更新数据，避免数据不同步
        m_richTextEdit->updateNote();

        bool notMultipleSelected = !m_middleView->isMultipleSelected();
        ActionManager::Instance()->visibleAction(ActionManager::NoteTop, notMultipleSelected);
        ActionManager::Instance()->visibleAction(ActionManager::NoteRename, notMultipleSelected);

        ActionManager::Instance()->resetCtxMenu(ActionManager::MenuType::NoteCtxMenu);

        if (stateOperation->isPlaying()
                || stateOperation->isRecording()
                || stateOperation->isVoice2Text()
                || stateOperation->isSearching()) {
            ActionManager::Instance()->enableAction(ActionManager::NoteAddNew, false);
            if (!stateOperation->isSearching()) {
                ActionManager::Instance()->enableAction(ActionManager::NoteDelete, false);
            } else {
                topAction->setEnabled(false);
            }
            if (stateOperation->isRecording()) {
                ActionManager::Instance()->enableAction(ActionManager::NoteSaveVoice, false);
            }
            ActionManager::Instance()->enableAction(ActionManager::NoteMove, false);
        }

        //Disable SaveText if note have no text
        //Disable SaveVoice if note have no voice.
        //只有当前一个记事本，右键移动置灰
        if (1 == m_leftView->folderCount()) {
            ActionManager::Instance()->enableAction(ActionManager::NoteMove, false);
        }
        if (m_middleView->isMultipleSelected()) {
            if (!m_middleView->haveVoice()) {
                ActionManager::Instance()->enableAction(ActionManager::NoteSaveVoice, false);
            }
            //根据选中笔记是否有文本设置保存笔记二级菜单置灰状态
            ActionManager::Instance()->saveNoteContextMenu()->setEnabled(m_middleView->haveText());
        } else {
            VNoteItem *currNoteData = m_middleView->getCurrVNotedata();
            if (nullptr != currNoteData) {
                //根据当前笔记是否有文本设置保存笔记二级菜单置灰状态
                ActionManager::Instance()->saveNoteContextMenu()->setEnabled(currNoteData->haveText());
                if (!currNoteData->haveVoice()) {
                    ActionManager::Instance()->enableAction(ActionManager::NoteSaveVoice, false);
                }
                if (currNoteData->isTop) {
                    topAction->setText(DApplication::translate("NotesContextMenu", "Unstick"));
                } else {
                    topAction->setText(DApplication::translate("NotesContextMenu", "Sticky on Top"));
                }
            }
        }
    } else if (menu == ActionManager::Instance()->notebookContextMenu()) {
        ActionManager::Instance()->resetCtxMenu(ActionManager::MenuType::NotebookCtxMenu);

        if (stateOperation->isPlaying()
                || stateOperation->isRecording()
                || stateOperation->isVoice2Text()) {
            ActionManager::Instance()->enableAction(ActionManager::NotebookAddNew, false);
            ActionManager::Instance()->enableAction(ActionManager::NotebookDelete, false);
        }
    }
}

/**
 * @brief VNoteMainWindow::loadNotepads
 * @return 记事本数量
 */
int VNoteMainWindow::loadNotepads()
{
    VNOTE_FOLDERS_MAP *folders = VNoteDataManager::instance()->getNoteFolders();
    QVariant value = setting::instance()->getOption(VNOTE_FOLDER_SORT);
    QStringList tmpsortFolders = value.toString().split(",");

    int folderCount = 0;

    if (folders) {
        folders->lock.lockForRead();

        folderCount = folders->folders.size();

        for (auto it : folders->folders) {
            int tmpIndexCount = tmpsortFolders.indexOf(QString::number(it->id));
            if (-1 != tmpIndexCount) {
                it->sortNumber = folderCount - tmpIndexCount;
            }
            m_leftView->appendFolder(it);
        }

        folders->lock.unlock();

        m_leftView->sort();
    }

    return folderCount;
}

/**
 * @brief VNoteMainWindow::addNotepad
 */
void VNoteMainWindow::addNotepad()
{
    VNoteFolder itemData;
    VNoteFolderOper folderOper;
    itemData.name = folderOper.getDefaultFolderName();

    VNoteFolder *newFolder = folderOper.addFolder(itemData);

    if (newFolder) {
        //Switch to note view
        switchWidget(WndNoteShow);

        bool sortEnable = false;
        VNoteFolder *tmpFolder = m_leftView->getFirstFolder();
        if (tmpFolder && -1 != tmpFolder->sortNumber) {
            newFolder->sortNumber = tmpFolder->sortNumber + 1;
            sortEnable = true;
        }
        //添加记事本，解绑详情页数据
        m_richTextEdit->unboundCurrentNoteData();
        m_leftView->addFolder(newFolder);
        m_leftView->sort();

        if (sortEnable) {
            QString folderSortData = m_leftView->getFolderSort();
            setting::instance()->setOption(VNOTE_FOLDER_SORT, folderSortData);
        }

        addNote();
    }
}

/**
 * @brief VNoteMainWindow::delNotepad
 */
void VNoteMainWindow::delNotepad()
{
    VNoteFolder *data = m_leftView->removeFolder();

    if (nullptr == data) {
        return;
    }

    // 删除之前清空JS详情页内容
    m_richTextEdit->clearJSContent();

    //删除记事本之前先解除详情页绑定的笔记数据
    m_richTextEdit->unboundCurrentNoteData();

    // 判断当前删除的记事本的排序编号，如果不是-1，则将当前所有记事本的排序编号写入配置文件中
    if (-1 != data->sortNumber) {
        QString folderSortData = m_leftView->getFolderSort();
        setting::instance()->setOption(VNOTE_FOLDER_SORT, folderSortData);
    }

    VNoteFolderOper folderOper(data);
    folderOper.deleteVNoteFolder(data);

    if (0 == m_leftView->folderCount()) {
        switchWidget(WndHomePage);
    }
}

/**
 * @brief VNoteMainWindow::editNotepad
 */
void VNoteMainWindow::editNotepad()
{
    //只有在记事本列表可见的情况下进行记事本重命名操作
    if (m_leftViewHolder->isVisible())
        m_leftView->editFolder();
}

/**
 * @brief VNoteMainWindow::addNote
 */
void VNoteMainWindow::addNote()
{
    m_middleView->clearSelection();

    qint64 id = m_middleView->getCurrentId();
    if (id != -1) {
        m_rightViewHasFouse = true;
        VNoteItem tmpNote;
        tmpNote.folderId = id;
        tmpNote.noteType = VNoteItem::VNT_Text;
        //无内容时富文本为空白段落标签
        tmpNote.htmlCode = "<p><br></p>";
        VNoteItemOper noteOper;
        //Get default note name in the folder
        tmpNote.noteTitle = noteOper.getDefaultNoteName(tmpNote.folderId);

        VNoteItem *newNote = noteOper.addNote(tmpNote);

        //Refresh the notes count of folder
        m_leftView->update(m_leftView->currentIndex());
        //解绑详情页绑定的笔记数据
        m_richTextEdit->unboundCurrentNoteData();
        m_middleView->addRowAtHead(newNote);
    }
}

/**
 * @brief VNoteMainWindow::editNote
 */
void VNoteMainWindow::editNote()
{
    m_middleView->editNote();
}

/**
 * @brief VNoteMainWindow::delNote
 * 删除记事项
 */
void VNoteMainWindow::delNote()
{
    // 删除之前清空JS详情页内容
    m_richTextEdit->clearJSContent();

    m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
    //记录移除前位置
    m_middleView->setNextSelection();

    QList<VNoteItem *> noteDataList = m_middleView->deleteCurrentRow();

    if (noteDataList.size()) {
        //删除笔记之前先解除详情页绑定的笔记数据
        m_richTextEdit->unboundCurrentNoteData();
        for (auto noteData : noteDataList) {
            VNoteItemOper noteOper(noteData);
            noteOper.deleteNote();
        }
        //Refresh the middle view
        if (m_middleView->rowCount() <= 0 && stateOperation->isSearching()) {
            m_middleView->setVisibleEmptySearch(true);
        }

        //Refresh the notes count of folder
        m_leftView->update(m_leftView->currentIndex());
    }
    //设置移除后选中
    m_middleView->selectAfterRemoved();
}

/**
 * @brief VNoteMainWindow::loadNotes
 * @param folder
 * @return 记事项数量
 */
int VNoteMainWindow::loadNotes(VNoteFolder *folder)
{
    m_middleView->clearAll();
    m_middleView->setVisibleEmptySearch(false);
    if (folder) {
        m_middleView->setCurrentId(folder->id);
        updateFolderName(folder->name);
        VNoteItemOper noteOper;
        VNOTE_ITEMS_MAP *notes = noteOper.getFolderNotes(folder->id);
        if (notes) {
            notes->lock.lockForRead();
            for (auto it : notes->folderNotes) {
                m_middleView->appendRow(it);
            }
            notes->lock.unlock();

            //Sort the view & set focus on first item
            m_middleView->onNoteChanged();
            m_middleView->setCurrentIndex(0);
        }
    } else {
        m_middleView->setCurrentId(-1);
    }
    return m_middleView->rowCount();
}

/**
 * @brief VNoteMainWindow::loadSearchNotes
 * @param key
 * @return 记事项数量
 */
int VNoteMainWindow::loadSearchNotes(const QString &key)
{
    m_middleView->clearAll();
    m_middleView->setSearchKey(key);
    VNOTE_ALL_NOTES_MAP *noteAll = VNoteDataManager::instance()->getAllNotesInFolder();
    if (noteAll) {
        noteAll->lock.lockForRead();
        for (auto &foldeNotes : noteAll->notes) {
            for (auto note : foldeNotes->folderNotes) {
                if (note->search(key)) {
                    m_middleView->appendRow(note);
                }
            }
        }
        noteAll->lock.unlock();
        if (m_middleView->rowCount() == 0) {
            m_middleView->setVisibleEmptySearch(true);
            m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
            m_richTextEdit->initData(nullptr, m_searchKey);
            m_imgInsert->setDisabled(true);
            m_recordBar->setVisible(false);
        } else {
            m_middleView->sortView(false);
            m_middleView->setVisibleEmptySearch(false);
            m_middleView->setCurrentIndex(0);
        }
        //刷新详情页-切换至当前笔记
        m_stackedRightMainWidget->setCurrentWidget(m_rightViewHolder);
    }
    return m_middleView->rowCount();
}

/**
 * @brief VNoteMainWindow::onPlayPlugVoicePlay
 * @param voiceData
 */
void VNoteMainWindow::onPlayPlugVoicePlay(VNVoiceBlock *voiceData)
{
    Q_UNUSED(voiceData)
    //设置播放状态
    if (!stateOperation->isPlaying()) {
        setSpecialStatus(PlayVoiceStart);
    }
    //更新web前端语音播放状态
    emit JsContent::instance()->callJsSetPlayStatus(0);
}

/**
 * @brief VNoteMainWindow::onPlayPlugVoicePause
 * @param voiceData
 */
void VNoteMainWindow::onPlayPlugVoicePause(VNVoiceBlock *voiceData)
{
    Q_UNUSED(voiceData)
    //更新web前端语音暂停状态
    emit JsContent::instance()->callJsSetPlayStatus(1);
}

/**
 * @brief VNoteMainWindow::onPlayPlugVoiceStop
 * @param voiceData
 */
void VNoteMainWindow::onPlayPlugVoiceStop(VNVoiceBlock *voiceData)
{
    Q_UNUSED(voiceData)
    setSpecialStatus(PlayVoiceEnd);
}

/**
 * @brief VNoteMainWindow::onNewNotebook
 */
void VNoteMainWindow::onNewNotebook()
{
    static struct timeval curret = {0, 0};
    static struct timeval lastPress = {0, 0};

    gettimeofday(&curret, nullptr);

    if (TM(lastPress, curret) > MIN_STKEY_RESP_TIME) {
        //新建记事本，设置焦点为主窗口，防止焦点在标题栏造成标题栏按钮控件闪烁
        m_stackedWidget->setFocus();

        addNotepad();

        UPT(lastPress, curret);
    }
}

/**
 * @brief VNoteMainWindow::onNewNote
 */
void VNoteMainWindow::onNewNote()
{
    qDebug() << "==========" << m_addNoteBtn->geometry().topLeft();
    static struct timeval curret = {0, 0};
    static struct timeval lastPress = {0, 0};

    gettimeofday(&curret, nullptr);

    if (TM(lastPress, curret) > MIN_STKEY_RESP_TIME) {
        addNote();

        UPT(lastPress, curret);
    }
}

/**
 * @brief VNoteMainWindow::canDoShortcutAction
 * @return true 可以使用快捷键
 */
bool VNoteMainWindow::canDoShortcutAction() const
{
    return (m_stackedWidget->currentIndex() == WndNoteShow);
}

/**
 * @brief VNoteMainWindow::setSpecialStatus
 * @param status
 */
void VNoteMainWindow::setSpecialStatus(SpecialStatus status)
{
    switch (status) {
    case SearchStart:
        if (!stateOperation->isSearching()) {
            stateOperation->operState(OpsStateInterface::StateSearching, true);
            m_leftView->clearSelection();
            m_leftView->setEnabled(false);
            m_addNotepadBtn->setVisible(false);
            m_addNoteBtn->setVisible(false);
            m_noteBookName->setVisible(false);
        }
        break;
    case SearchEnd:
        if (stateOperation->isSearching()) {
            m_searchKey = "";
            m_middleView->setSearchKey(m_searchKey);
            m_leftView->setEnabled(true);
            m_addNotepadBtn->setVisible(true);
            m_addNoteBtn->setVisible(true);
            m_noteSearchEdit->lineEdit()->setFocus();
            m_noteBookName->setVisible(true);
            stateOperation->operState(OpsStateInterface::StateSearching, false);
            onVNoteFolderChange(m_leftView->restoreNotepadItem(), QModelIndex());
        }
        break;
    case PlayVoiceStart:
        if (stateOperation->isSearching()) {
            m_recordBar->setVisible(true);
        }
        stateOperation->operState(OpsStateInterface::StatePlaying, true);
        m_noteSearchEdit->setEnabled(false);
        m_leftView->setOnlyCurItemMenuEnable(true);
        m_addNotepadBtn->setEnabled(false);
        m_middleView->setOnlyCurItemMenuEnable(true);
        m_addNoteBtn->setDisabled(true);
        m_noteBookName->setDisabled(true);
        break;
    case PlayVoiceEnd:
        if (stateOperation->isSearching()) {
            m_recordBar->setVisible(false);
        }
        if (!stateOperation->isVoice2Text()) {
            m_noteSearchEdit->setEnabled(true);
            m_leftView->setOnlyCurItemMenuEnable(false);
            m_addNotepadBtn->setEnabled(true);
            m_middleView->setOnlyCurItemMenuEnable(false);
            m_addNoteBtn->setDisabled(false);
            m_noteBookName->setDisabled(false);
        }
        stateOperation->operState(OpsStateInterface::StatePlaying, false);
        //停止播放更新web前端语音停止状态
        emit JsContent::instance()->callJsSetPlayStatus(2);
        break;
    case RecordStart:
        stateOperation->operState(OpsStateInterface::StateRecording, true);
        //录音时设置播放按钮不可用
        emit JsContent::instance()->callJsSetVoicePlayBtnEnable(false);
        m_noteSearchEdit->setEnabled(false);
        m_leftView->setOnlyCurItemMenuEnable(true);
        m_leftView->closeMenu();
        m_middleView->closeMenu();
        m_addNotepadBtn->setEnabled(false);
        m_middleView->setOnlyCurItemMenuEnable(true);
        //m_rightView->setEnablePlayBtn(false);
        m_addNoteBtn->setDisabled(true);
        m_noteBookName->setDisabled(true);
        break;
    case RecordEnd:
        //结束录音时设置播放按钮可用
        emit JsContent::instance()->callJsSetVoicePlayBtnEnable(true);
        if (!stateOperation->isVoice2Text()) {
            m_noteSearchEdit->setEnabled(true);
            m_leftView->setOnlyCurItemMenuEnable(false);
            m_addNotepadBtn->setEnabled(true);
            m_middleView->setOnlyCurItemMenuEnable(false);
            m_addNoteBtn->setDisabled(false);
            m_noteBookName->setDisabled(false);
        }
        //m_rightView->setEnablePlayBtn(true);
        stateOperation->operState(OpsStateInterface::StateRecording, false);
        break;
    case VoiceToTextStart:
        emit JsContent::instance()->callJsSetVoiceText(DApplication::translate("VoiceNoteItem", "Converting voice to text"), JsContent::AsrFlag::Start);
        stateOperation->operState(OpsStateInterface::StateVoice2Text, true);
        m_noteSearchEdit->setEnabled(false);
        m_leftView->setOnlyCurItemMenuEnable(true);
        m_addNotepadBtn->setEnabled(false);
        m_middleView->setOnlyCurItemMenuEnable(true);
        m_addNoteBtn->setDisabled(true);
        m_noteBookName->setDisabled(true);
        m_leftView->closeMenu();
        m_middleView->closeMenu();
        break;
    case VoiceToTextEnd:
        if (!stateOperation->isRecording() && !stateOperation->isPlaying()) {
            m_noteSearchEdit->setEnabled(true);
            m_leftView->setOnlyCurItemMenuEnable(false);
            m_addNotepadBtn->setEnabled(true);
            m_middleView->setOnlyCurItemMenuEnable(false);
            m_addNoteBtn->setDisabled(false);
            m_noteBookName->setDisabled(false);
        }
        stateOperation->operState(OpsStateInterface::StateVoice2Text, false);
        break;
    default:
        break;
    }
}

/**
 * @brief VNoteMainWindow::initAsrErrMessage
 * 初始化语音转文字失败提示框
 */
void VNoteMainWindow::initAsrErrMessage()
{
    //实例化为临时消息弹出
    m_asrErrMeassage = new DFloatingMessage(DFloatingMessage::TransientType,
                                            m_centerWidget);
    QString iconPath = STAND_ICON_PAHT;
    iconPath.append("warning.svg");
    m_asrErrMeassage->setIcon(QIcon(iconPath));
    DWidget *m_widget = new DWidget(m_asrErrMeassage);
    QHBoxLayout *m_layout = new QHBoxLayout();
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addStretch();
    m_widget->setLayout(m_layout);
    m_asrErrMeassage->setWidget(m_widget);
    m_asrErrMeassage->setVisible(false);
}

/**
 * @brief VNoteMainWindow::initDeviceExceptionErrMessage
 */
void VNoteMainWindow::initDeviceExceptionErrMessage()
{
    m_pDeviceExceptionMsg = new DFloatingMessage(DFloatingMessage::ResidentType,
                                                 m_centerWidget);
    QString iconPath = STAND_ICON_PAHT;
    iconPath.append("warning.svg");
    m_pDeviceExceptionMsg->setIcon(QIcon(iconPath));
    m_pDeviceExceptionMsg->setMessage(
        DApplication::translate(
            "VNoteRecordBar",
            "Your audio recording device does not work."));
    m_pDeviceExceptionMsg->setVisible(false);
}

/**
 * @brief VNoteMainWindow::showAsrErrMessage
 * @param strMessage
 */
void VNoteMainWindow::showAsrErrMessage(const QString &strMessage)
{
    if (m_asrErrMeassage == nullptr) {
        initAsrErrMessage();
    }

    m_asrErrMeassage->setMessage(strMessage);
    m_asrErrMeassage->setVisible(true);
    m_asrErrMeassage->setMinimumHeight(60);
    m_asrErrMeassage->setMaximumWidth(m_centerWidget->width());
    m_asrErrMeassage->adjustSize();

    int xPos = (m_centerWidget->width() - m_asrErrMeassage->width()) / 2;
    int yPos = m_centerWidget->height() - m_asrErrMeassage->height() - 5;

    m_asrErrMeassage->move(xPos, yPos);
}

/**
 * @brief VNoteMainWindow::showDeviceExceptionErrMessage
 */
void VNoteMainWindow::showDeviceExceptionErrMessage()
{
    if (m_pDeviceExceptionMsg == nullptr) {
        initDeviceExceptionErrMessage();
    }

    m_pDeviceExceptionMsg->setVisible(true);
    m_pDeviceExceptionMsg->setMinimumWidth(520);
    m_pDeviceExceptionMsg->setMinimumHeight(65);
    m_pDeviceExceptionMsg->setMaximumWidth(m_centerWidget->width());
    m_pDeviceExceptionMsg->adjustSize();

    int xPos = (m_centerWidget->width() - m_pDeviceExceptionMsg->width()) / 2;
    int yPos = m_centerWidget->height() - m_pDeviceExceptionMsg->height() - 5;

    m_pDeviceExceptionMsg->move(xPos, yPos);
}

/**
 * @brief VNoteMainWindow::closeDeviceExceptionErrMessage
 */
void VNoteMainWindow::closeDeviceExceptionErrMessage()
{
    if (m_pDeviceExceptionMsg) {
        m_pDeviceExceptionMsg->setVisible(false);
    }
}

/**
 * @brief VNoteMainWindow::onSystemDown
 * @param active
 */
void VNoteMainWindow::onSystemDown(bool active)
{
    qInfo() << "System going down...";

    if (active) {
        if (stateOperation->isRecording()) {
            m_recordBar->stopRecord();

            qInfo() << "System going down when recording, cancel it.";
        }
        releaseHaltLock();
    }
}

/**
 * @brief VNoteMainWindow::switchWidget
 * @param type
 */
void VNoteMainWindow::switchWidget(WindowType type)
{
    bool searchEnable = type == WndNoteShow ? true : false;
    if (!searchEnable) {
        setTitleBarTabFocus();
    }
    m_noteSearchEdit->setEnabled(searchEnable);
    m_viewChange->setEnabled(searchEnable); //设置记事本收起按钮禁用状态
    m_stackedWidget->setCurrentIndex(type);
}

/**
 * @brief VNoteMainWindow::release
 */
void VNoteMainWindow::release()
{
    //Save main window size
    if (!isMaximized()) {
        setting::instance()->setOption(VNOTE_MAINWND_SZ_KEY, saveGeometry());
    }

    VTextSpeechAndTrManager::onStopTextToSpeech();
    m_richTextEdit->updateNote();

    if (stateOperation->isVoice2Text()) {
        QScopedPointer<VNoteA2TManager> releaseA2TManger(m_a2tManager);
        releaseA2TManger->stopAsr();
    }
}

/**
 * @brief VNoteMainWindow::onShowPrivacy
 */
void VNoteMainWindow::onShowPrivacy()
{
    QString url = "";
    QLocale locale;
    QLocale::Country country = locale.country();
    bool isCommunityEdition = DSysInfo::isCommunityEdition();
    if (country == QLocale::China) {
        if (isCommunityEdition) {
            url = "https://www.deepin.org/zh/agreement/privacy/";
        } else {
            url = "https://www.uniontech.com/agreement/privacy-cn";
        }
    } else {
        if (isCommunityEdition) {
            url = "https://www.deepin.org/en/agreement/privacy/";
        } else {
            url = "https://www.uniontech.com/agreement/privacy-en";
        }
    }
    QDesktopServices::openUrl(url);
}

/**
 * @brief VNoteMainWindow::onShowSettingDialog
 */
void VNoteMainWindow::onShowSettingDialog()
{
    DSettingsDialog dialog(this);
    dialog.updateSettings("Setting", setting::instance()->getSetting());
    dialog.setResetVisible(false);
    dialog.exec();
}

/**
 * @brief VNoteMainWindow::initMenuExtension
 */
void VNoteMainWindow::initMenuExtension()
{
    m_menuExtension = new DMenu(this);
    QAction *setting = new QAction(DApplication::translate("TitleBar", "Settings"), m_menuExtension);
    QAction *privacy = new QAction(DApplication::translate("TitleBar", "Privacy Policy"), m_menuExtension);
    m_menuExtension->addAction(setting);
    m_menuExtension->addAction(privacy);
    m_menuExtension->addSeparator();
    connect(privacy, &QAction::triggered, this, &VNoteMainWindow::onShowPrivacy);
    connect(setting, &QAction::triggered, this, &VNoteMainWindow::onShowSettingDialog);
}

/**
 * @brief VNoteMainWindow::initMenuExtension
 * 初始化标题栏图标按钮
 */
void VNoteMainWindow::initTitleIconButton()
{
    m_viewChange = new DToolButton(this);
    m_viewChange->setIconSize(QSize(36, 36));
    m_viewChange->setFixedSize(QSize(36, 36));
//    m_viewChange->setIcon(Utils::loadSVG("sidebar_show.svg", false));
    m_viewChange->setIcon(QIcon::fromTheme("dvn_sidebar"));
    m_viewChange->setCheckable(true);

    m_imgInsert = new DToolButton(this);
    m_imgInsert->setIconSize(QSize(36, 36));
    m_imgInsert->setFixedSize(QSize(36, 36));
//    m_imgInsert->setIcon(Utils::loadSVG("image_show.svg", false));
    m_imgInsert->setIcon(QIcon::fromTheme("dvn_image"));
    m_imgInsert->setDisabled(true); //初始为禁用状态
    m_imgInsert->setCheckable(false); //初始为禁用状态
}

/**
 * @brief MiddleView::onDropNote
 * @param dropCancel
 * 笔记拖拽结束
 */
void VNoteMainWindow::onDropNote(bool dropCancel)
{
    //取消拖拽，return
    if (dropCancel) {
        m_middleView->setDragSuccess(false);
        return;
    }
    QModelIndexList indexList = m_middleView->getAllSelectNote();
    QPoint point = m_leftView->mapFromGlobal(QCursor::pos());
    QModelIndex selectIndex = m_leftView->indexAt(point);
    m_leftView->setNumberOfNotes(m_middleView->count());
    bool ret = m_leftView->doNoteMove(indexList, selectIndex);
    if (ret) {
        m_middleView->deleteModelIndexs(indexList);
    }
    //拖拽完成
    m_middleView->setDragSuccess(true);
}

/**
 * @brief MiddleView::handleMultipleOption
 * @param id
 * 响应多选详情页操作
 */
void VNoteMainWindow::handleMultipleOption(int id)
{
    switch (id) {
    case ButtonValue::Move: {
        //多选页面-移动
        m_middleView->setNextSelection();
        QModelIndexList notesdata = m_middleView->getAllSelectNote();
        if (notesdata.size()) {
            QModelIndex selectFolder = m_leftView->selectMoveFolder(notesdata);
            m_leftView->setNumberOfNotes(m_middleView->count());
            if (selectFolder.isValid() && m_leftView->doNoteMove(notesdata, selectFolder)) {
                m_middleView->deleteModelIndexs(notesdata);
                //移除后选中
                m_middleView->selectAfterRemoved();
            }
        }
    } break;
    case ButtonValue::SaveAsTxT:
        //多选页面-保存TxT
        m_middleView->saveAs(MiddleView::SaveAsType::Note);
        break;
    case ButtonValue::SaveAsVoice:
        //多选页面-保存语音
        m_middleView->saveAs(MiddleView::SaveAsType::Voice);
        break;
    case ButtonValue::Delete:
        //多选页面-删除
        VNoteMessageDialog confirmDialog(VNoteMessageDialog::DeleteNote, this, m_middleView->getSelectedCount());
        if (VNoteBaseDialog::Accepted == confirmDialog.exec()) {
            delNote();
        }
        break;
    }
}

/**
 * @brief VNoteMainWindow::setTitleBarTabFocus
 * @param event
 */
void VNoteMainWindow::setTitleBarTabFocus(QKeyEvent *event)
{
    titlebar()->setFocus(Qt::MouseFocusReason);
    if (event != nullptr) {
        DWidget::keyPressEvent(event);
    }
}

/**
 * @brief VNoteMainWindow::eventFilter
 * @param o
 * @param e
 * @return
 */
bool VNoteMainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_middleViewHolder && e->type() == QEvent::Resize) {
        // 更新按钮位置
        int position = (m_middleViewHolder->width() - m_addNoteBtn->width()) / 2 - 6;
        m_buttonAnchor->setLeftMargin(position);
        m_middleView->resize(m_middleViewHolder->width(), m_middleViewHolder->height() - m_noteBookName->height() - 20);
        m_buttonAnchor->setBottomMargin(17);
        //qDebug() << m_middleViewHolder->geometry() << m_noteBookName->height() << m_noteBookName->geometry();
        updateFolderName();
        return true;
    }

    if (e->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Tab) {
            return setTabFocus(o, keyEvent);
        } else if (o == m_addNotepadBtn && (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)) {
            addNotepad();
            return true;
        }
    }
    if (o == m_noteSearchEdit->lineEdit() && e->type() == QEvent::FocusIn) {
        m_showSearchEditMenu = false;
    }
    if(o == m_noteSearchEdit->lineEdit() && QEvent::Move == e->type()){
        // 触发搜索框移动事件时需要，更新搜索框的大小
        if (this->geometry().width() >= DEFAULT_WINDOWS_WIDTH) {
            m_noteSearchEdit->resize(VNOTE_SEARCHBAR_W, VNOTE_SEARCHBAR_H);
        } else if (this->geometry().width() <= MIN_WINDOWS_WIDTH) {
            m_noteSearchEdit->resize(VNOTE_SEARCHBAR_MIN_W, VNOTE_SEARCHBAR_H);
        } else {
            double wMainWin = ((this->geometry().width() - MIN_WINDOWS_WIDTH) * 1.0) / (DEFAULT_WINDOWS_WIDTH - MIN_WINDOWS_WIDTH);
            int wsearch = static_cast<int>((VNOTE_SEARCHBAR_W - VNOTE_SEARCHBAR_MIN_W) * wMainWin) + VNOTE_SEARCHBAR_MIN_W;
            m_noteSearchEdit->resize(wsearch , VNOTE_SEARCHBAR_H);
        }
    }
    return false;
}

/**
 * @brief VNoteMainWindow::setTabFocus
 * @param obj
 * @param event
 * @return
 */
bool VNoteMainWindow::setTabFocus(QObject *obj, QKeyEvent *event)
{
    if (obj == m_richTextEdit) {
        return true;
    } else if (obj == m_noteSearchEdit->lineEdit()) {
        return false;
    } else if (obj == m_addNoteBtn) {
        return setAddnoteButtonNext(event);
    } else if (obj == m_middleView) {
        return setMiddleviewNext(event);
    } else if (obj == m_addNotepadBtn) {
        return setAddnotepadButtonNext(event);
    } else {
        return setTitleCloseButtonNext(event);
    }
}

/**
 * @brief VNoteMainWindow::setAddnotepadButtonNext
 * @param event
 * @return
 */
bool VNoteMainWindow::setAddnotepadButtonNext(QKeyEvent *event)
{
    if (m_middleView->rowCount()) {
        return false;
    }
    if (m_addNoteBtn->isEnabled() && m_addNoteBtn->isVisible()) {
        m_addNoteBtn->setFocus(Qt::TabFocusReason);
        return true;
    }

    return setMiddleviewNext(event);
}

/**
 * @brief VNoteMainWindow::setAddnoteButtonNext
 * @param event
 * @return
 */
bool VNoteMainWindow::setAddnoteButtonNext(QKeyEvent *event)
{
    if (m_middleView->rowCount() == 0) {
        setTitleBarTabFocus(event);
        return true;
    }
    if (m_stackedRightMainWidget->currentWidget() == m_rightViewHolder) {
        m_recordBar->setFocus(Qt::TabFocusReason);
    } else {
        m_multipleSelectWidget->setFocus(Qt::TabFocusReason);
    }
    DWidget::keyPressEvent(event);
    return true;
}

/**
 * @brief VNoteMainWindow::setTitleCloseButtonNext
 * @param event
 * @return
 */
bool VNoteMainWindow::setTitleCloseButtonNext(QKeyEvent *event)
{
    //搜索状态
    if (stateOperation->isSearching()) {
        //搜索内容为空状态
        if (m_middleView->searchEmpty()) {
            //焦点切换由第一个控件m_viewChange开始
            m_viewChange->setFocus(Qt::TabFocusReason);
            return true;
        }
        m_middleView->setFocus(Qt::TabFocusReason);
        return true;
    }
    m_stackedWidget->currentWidget()->setFocus(Qt::TabFocusReason);
    //当前记事本列表为隐藏笔记列表为显示状态
    if (!m_leftViewHolder->isVisible()
            && m_middleViewHolder->isVisible()) {
        //进入笔记列表焦点获取优先级选择
        return setAddnotepadButtonNext(event);
    }
    DWidget::keyPressEvent(event);
    return true;
}

/**
 * @brief VNoteMainWindow::setMiddleviewNext
 * @param event
 * @return
 */
bool VNoteMainWindow::setMiddleviewNext(QKeyEvent *event)
{
    if (stateOperation->isSearching()) {
        m_richTextEdit->setFocus(Qt::TabFocusReason);
        return true;
    }
    if (stateOperation->isRecording() || stateOperation->isPlaying() || stateOperation->isVoice2Text()) {
        m_recordBar->setFocus(Qt::TabFocusReason);
        DWidget::keyPressEvent(event);
        return true;
    }
    return false;
}

/**
 * @brief VNoteMainWindow::needShowMax
 * @return
 */
bool VNoteMainWindow::needShowMax()
{
    return m_needShowMax;
}

void VNoteMainWindow::onDeleteShortcut()
{
    if (canDoShortcutAction()) {
        QAction *deleteAct = nullptr;

        /*
         * TODO:
         *     We check focus here to choice what action we will
         * take. Focus in leftview for delete notebook, in midlle
         * view for delete note, in Rightview for delete note content.
         * or do nothing.
         * */
        if (m_leftView->hasFocus()) {
            if (!stateOperation->isRecording()
                    && !stateOperation->isVoice2Text()
                    && !stateOperation->isPlaying()) {
                deleteAct = ActionManager::Instance()->getActionById(
                                ActionManager::NotebookDelete);
            }
        } else if (m_middleView->hasFocus()) {
            if (!stateOperation->isRecording()
                    && !stateOperation->isVoice2Text()
                    && !stateOperation->isPlaying()
                    && m_middleView->count() > 0) {
                deleteAct = ActionManager::Instance()->getActionById(
                                ActionManager::NoteDelete);
            }
        }
        if (nullptr != deleteAct) {
            deleteAct->triggered();
        }
    }
}

void VNoteMainWindow::onEscShortcut()
{
    setTitleBarTabFocus();
}

void VNoteMainWindow::onPoppuMenuShortcut()
{
    if (m_leftView->hasFocus()) {
        m_leftView->popupMenu();
    } else if (m_middleView->hasFocus()) {
        m_middleView->popupMenu();
    } else if (m_noteSearchEdit->lineEdit()->hasFocus()) {
        if (!m_showSearchEditMenu) {
            QPoint pos(m_noteSearchEdit->pos());
            QContextMenuEvent eve(QContextMenuEvent::Reason::Keyboard, pos, m_noteSearchEdit->mapToGlobal(pos));
            m_showSearchEditMenu = QApplication::sendEvent(m_noteSearchEdit->lineEdit(), &eve);
        }
    } else if (m_richTextEdit->hasFocus()) {
        m_richTextEdit->shortcutPopupMenu();
    }
}

void VNoteMainWindow::onAddNotepadShortcut()
{
    if (!(stateOperation->isRecording()
            || stateOperation->isPlaying()
            || stateOperation->isVoice2Text()
            || stateOperation->isSearching())) {
        onNewNotebook();

        //If do shortcut in home page,need switch to note
        //page after add new notebook.
        if (!canDoShortcutAction()) {
            switchWidget(WndNoteShow);
        }
    }
}

void VNoteMainWindow::onReNameNotepadShortcut()
{
    //当前记事本是否正在编辑
    bool isEditing = m_leftView->isPersistentEditorOpen(m_leftView->currentIndex());
    //如果已在编辑状态，取消操作，解决重复快捷键警告信息
    if (canDoShortcutAction()) {
        if (!stateOperation->isSearching() && !isEditing) {
            editNotepad();
        }
    }
}

void VNoteMainWindow::onAddNoteShortcut()
{
    // 焦点若在文本编辑区域，仅执行文本加粗操作，不新建文本笔记.
    QWidget* focusWidget = this->focusWidget();
    if (focusWidget && m_richTextEdit == focusWidget->parentWidget())
        return;

    if (canDoShortcutAction()
            && !(stateOperation->isRecording()
                 || stateOperation->isPlaying()
                 || stateOperation->isVoice2Text()
                 || stateOperation->isSearching())) {
        onNewNote();
    }
}

void VNoteMainWindow::onRenameNoteShortcut()
{
    //当前笔记是否正在编辑
    bool isEditing = m_middleView->isPersistentEditorOpen(m_middleView->currentIndex());
    //如果已在编辑状态，取消操作，解决重复快捷键警告信息
    if (canDoShortcutAction() && !isEditing) {
        editNote();
    }
}

void VNoteMainWindow::onPlayPauseShortcut()
{
    if (canDoShortcutAction()) {
        //只要播放状态下才能用快捷键控制播放/暂停
        if (stateOperation->isPlaying()) {
            m_recordBar->playVoice(m_currentPlayVoice.get(), true);
        }
    }
}

void VNoteMainWindow::onRecordShorcut()
{
    if (canDoShortcutAction()) {
        m_recordBar->onStartRecord();
    }
}

void VNoteMainWindow::onSaveNoteShortcut()
{
    //多选笔记导出文本
    if (canDoShortcutAction()) {
        if (m_middleView->haveText()) {
            m_middleView->saveAs(MiddleView::SaveAsType::Note);
        }
    }
}

void VNoteMainWindow::onSaveVoicesShortcut()
{
    if (canDoShortcutAction()) {
        //Can't save recording when do recording.
        //多选笔记导出语音
        if (!stateOperation->isRecording()) {
            if (m_middleView->haveVoice()) {
                m_middleView->saveAs(MiddleView::SaveAsType::Voice);
            }
        }
    }
}

/**
 * @brief VNoteMainWindow::onThemeChanged
 * 主题切换响应槽
 */
void VNoteMainWindow::onThemeChanged()
{
    showNotepadList();
}

/**
 * @brief VNoteMainWindow::onViewChangeClicked
 * 记事本列表显示或隐藏事件响应
 */
void VNoteMainWindow::onViewChangeClicked()
{
    setting::instance()->setOption(VNOTE_NOTEPAD_LIST_SHOW, !setting::instance()->getOption(VNOTE_NOTEPAD_LIST_SHOW).toBool());
    showNotepadList();
}

/**
 * @brief 响应web前端语音播放控制
 * @param json :语音数据
 * @param bIsSame : 此次语音是否与上一次语音相同
 */
void VNoteMainWindow::onWebVoicePlay(const QVariant &json, bool bIsSame)
{
    //录音状态下不允许播放
    if (stateOperation->isRecording()) {
        qInfo() << "The recording cannot be played";
        return;
    }

    //当前播放的语音为空时，此时bIsSame应该为false
    if (m_currentPlayVoice.isNull() && true == bIsSame) {
        qInfo() << "Parameter error";
        bIsSame = false;
    }

    //不同语音则重新解析json文件
    if (!bIsSame) {
        m_currentPlayVoice.reset(new VNVoiceBlock);
        MetaDataParser dataParser;
        dataParser.parse(json, m_currentPlayVoice.get());
    }

    //文件不存在出现错误弹出并删除该语音文本
    if (!QFile::exists(m_currentPlayVoice->voicePath)) {
        //异步提示，防止阻塞前端事件
        QTimer::singleShot(0, this, [this] {
            //正在播放时，停止播放
            if (stateOperation->isPlaying())
            {
                m_recordBar->stopPlay();
            }
            //弹出提示
            VNoteMessageDialog audioOutLimit(VNoteMessageDialog::VoicePathNoAvail);
            audioOutLimit.exec();
            //删除语音文本
            m_richTextEdit->deleteSelectText();
        });
        return;
    }

    m_recordBar->playVoice(m_currentPlayVoice.get(), bIsSame);
}

/**
 * @brief 富文本编辑器插入图片
 */
void VNoteMainWindow::onInsertImageToWebEditor()
{
    QStringList filePaths = DFileDialog::getOpenFileNames(
                this,
                "",
                QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
                "Image file(*.jpg *.png *.bmp)");

    if (JsContent::instance()->insertImages(filePaths)) {
        m_richTextEdit->setFocus(); //插入成功，将焦点转移至编辑区
    }
//    m_imgInsert->setChecked(false);
}

/**
 * @brief 当前编辑区内容搜索为空
 */
void VNoteMainWindow::onWebSearchEmpty()
{
    //没有搜索到则从笔记列表删除当前笔记
    m_middleView->setNextSelection();
    m_middleView->deleteCurrentRow();
    //没有笔记符合条件则提示搜索无结果
    if (0 == m_middleView->rowCount()) {
        m_middleView->setVisibleEmptySearch(true);
    } else {
        m_middleView->selectAfterRemoved();
    }
}
void VNoteMainWindow::onVNoteFolderRename(QString name)
{
    updateFolderName(name);
}
void VNoteMainWindow::updateFolderName(QString name)
{
    QString showText = name;
    if (showText.isEmpty()) {
        showText = m_noteBookNameStr;
    } else {
        m_noteBookNameStr = showText;
    }
    QFontMetrics fm(DFontSizeManager::instance()->t4());
    int labelWidth = m_noteBookName->geometry().width();
    int leftIndex = 1;
    if (fm.boundingRect(showText).size().width() > labelWidth) {
        for (;leftIndex < showText.size(); ++leftIndex) {
            if (fm.boundingRect((showText.left(leftIndex) + "...")).size().width() >= labelWidth) {
                break;
            }
        }
        showText = showText.left(leftIndex - 1) + "...";
    }
    m_noteBookName->setText(showText);
}

void VNoteMainWindow::showNotepadList()
{
    //切换主题色/图标
    DPalette appDp = DGuiApplicationHelper::instance()->applicationPalette();
    bool visible = setting::instance()->getOption(VNOTE_NOTEPAD_LIST_SHOW).toBool();
    if (visible) {
        appDp.setBrush(DPalette::Light, appDp.color(DPalette::Active, DPalette::Highlight));
        appDp.setBrush(DPalette::Dark, appDp.color(DPalette::Active, DPalette::Highlight));
        //m_viewChange->setIcon(Utils::loadSVG("view_change_show.svg", false));
        m_viewChange->setChecked(true);
    } else {
        m_viewChange->setChecked(false);
        //m_viewChange->setIcon(Utils::loadSVG("view_change_hide.svg", false));
    }
    //m_viewChange->setPalette(appDp);
    //显示/隐藏笔记列表
    m_leftViewHolder->setVisible(visible);
}
