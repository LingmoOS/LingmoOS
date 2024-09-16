// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"
#include "pathsettintwgt.h"
#include <DTitlebar>
#include <DAnchors>
#include <DThemeManager>
#include <DToast>
#include <DSettingsWidgetFactory>
#include <DSettingsGroup>
#include <DSettings>
#include <DSettingsOption>
#include <QApplication>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QScreen>
#include <QStyleFactory>
#include <QEvent>
#include <DDialog>
#include <QStackedWidget>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <DGuiApplicationHelper>
#include <DMessageManager>
#include <DGuiApplicationHelper>
#include <DPrintPreviewDialog>
#include <QGuiApplication>
#include <QWindow>
#include <DWidgetUtil>
#include <QTextDocumentFragment>
#include <dprintpreviewwidget.h>

#ifdef DTKWIDGET_CLASS_DFileDialog

#include <DFileDialog>
#else
#include <QFileDialog>
#endif

#define PRINT_FLAG 2
#define PRINT_ACTION 8
#define PRINT_FORMAT_MARGIN 10
#define FLOATTIP_MARGIN 95

/**
 * @brief 根据传入的源文档 \a doc 创建新的文档
 * @param doc 源文档指针
 * @return 创建的新文档，用于拷贝数据
 */
static QTextDocument *createNewDocument(QTextDocument *doc)
{
    QTextDocument *createDoc = new QTextDocument(doc);
    // 不记录撤销信息
    createDoc->setUndoRedoEnabled(false);
    createDoc->setMetaInformation(QTextDocument::DocumentTitle,
                                  doc->metaInformation(QTextDocument::DocumentTitle));
    createDoc->setMetaInformation(QTextDocument::DocumentUrl,
                                  doc->metaInformation(QTextDocument::DocumentUrl));
    createDoc->setDefaultFont(doc->defaultFont());
    createDoc->setDefaultStyleSheet(doc->defaultStyleSheet());
    createDoc->setIndentWidth(doc->indentWidth());

    // 设置固定为从左向右布局，降低后续高亮等操作导致更新计算布局方向
    auto textOption = createDoc->defaultTextOption();
    textOption.setTextDirection(Qt::LeftToRight);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    createDoc->setDefaultTextOption(textOption);

    return createDoc;
}

/*!
 * \~chinese \brief printPage 绘制每一页文本纸张到打印机
 * \~chinese \param index 纸张索引
 * \~chinese \param doc 文本文件
 * \~chinese \param body 范围大小
 * \~chinese \param pageCountBox 绘制页码的范围
 */
void Window::printPage(int index, QPainter *painter, const QTextDocument *doc,
                       const QRectF &body, const QRectF &pageCountBox)
{
    painter->save();

    painter->translate(body.left(), body.top() - (index - 1) * body.height());
    const QRectF view(0, (index - 1) * body.height(), body.width(), body.height());

    QAbstractTextDocumentLayout *layout = doc->documentLayout();
    QAbstractTextDocumentLayout::PaintContext ctx;

    painter->setFont(QFont(doc->defaultFont()));
    const QRectF box = pageCountBox.translated(0, view.top());
    const QString pageString = QString::number(index);
    painter->drawText(box, Qt::AlignRight, pageString);

    painter->setClipRect(view);
    ctx.clip = view;
    ctx.palette.setColor(QPalette::Text, Qt::black);

    layout->draw(painter, ctx);

    painter->restore();
}

/**
 * @brief 使用多组文档打印，用于超大文档的情况
 * @param index         纸张索引
 * @param painter       打印指针
 * @param printInfo     多组文档信息
 * @param body          范围大小
 * @param pageCountBox  绘制页码的范围
 */
void Window::printPageWithMultiDoc(
    int index, QPainter *painter, const QVector<Window::PrintInfo> &printInfo, const QRectF &body, const QRectF &pageCountBox)
{
    painter->save();

    int docIndex = index;
    for (auto info : printInfo) {
        if (docIndex <= info.doc->pageCount()) {
            // 调整绘制工具坐标到当前页面顶部
            painter->translate(body.left(), body.top() - (docIndex - 1) * body.height());

            // 绘制页码
            painter->setFont(QFont(info.doc->defaultFont()));
            const QRectF box = pageCountBox.translated(0, (docIndex - 1) * body.height());
            const QString pageString = QString::number(index);
            painter->drawText(box, Qt::AlignRight, pageString);

            // 设置文档裁剪区域
            const QRectF docView(0, (docIndex - 1) * body.height(), body.width(), body.height());
            QAbstractTextDocumentLayout *layout = info.doc->documentLayout();

            // 大文本的高亮单独处理
            if (info.highlighter) {
                // 提前两页拷贝数据，用于处理高亮计算时连续处理
                qreal offsetHeight = qMax<qreal>(0, (qMin(2, docIndex - 1) * body.height()));
                QPointF point = docView.topLeft() - QPointF(0, offsetHeight);
                int pos = layout->hitTest(point, Qt::FuzzyHit);
                QTextCursor cursor(info.doc);
                cursor.setPosition(pos);
                // 选取后续内容
                int endPos = layout->hitTest(docView.bottomLeft(), Qt::FuzzyHit);
                endPos = qMin(endPos + 1000, info.doc->characterCount() - 1);
                cursor.setPosition(endPos, QTextCursor::KeepAnchor);
                cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

                // 创建临时文档
                QTextDocument *tmpDoc = createNewDocument(info.doc);
                QTextCursor insertCursor(tmpDoc);
                insertCursor.beginEditBlock();
                insertCursor.insertFragment(QTextDocumentFragment(cursor));
                insertCursor.endEditBlock();

                auto margin = info.doc->rootFrame()->frameFormat().margin();
                auto fmt = tmpDoc->rootFrame()->frameFormat();
                fmt.setMargin(margin);
                tmpDoc->rootFrame()->setFrameFormat(fmt);

                tmpDoc->setPageSize(body.size());
                // 重新高亮文本
                auto newHighlighter = new CSyntaxHighlighter(tmpDoc);
                newHighlighter->setDefinition(info.highlighter->definition());
                newHighlighter->setTheme(info.highlighter->theme());
                rehighlightPrintDoc(tmpDoc, newHighlighter);

                // 确保布局完成
                tmpDoc->pageCount();

                // 调整显示位置
                painter->resetTransform();
                painter->translate(body.left(), body.top() - offsetHeight);
                QAbstractTextDocumentLayout::PaintContext ctx;
                ctx.clip = QRectF(0, offsetHeight, body.width(), body.height());
                ctx.palette.setColor(QPalette::Text, Qt::black);
                tmpDoc->documentLayout()->setPaintDevice(painter->device());
                tmpDoc->documentLayout()->draw(painter, ctx);

                delete newHighlighter;
                delete tmpDoc;
            } else {
                QAbstractTextDocumentLayout::PaintContext ctx;
                ctx.clip = docView;
                ctx.palette.setColor(QPalette::Text, Qt::black);
                // 绘制文档
                layout->draw(painter, ctx);
            }

            break;
        }
        docIndex -= info.doc->pageCount();
    }

    painter->restore();
}

Window::Window(DMainWindow *parent)
    : DMainWindow(parent),
      m_centralWidget(new QWidget),
      m_editorWidget(new QStackedWidget),
      m_centralLayout(new QVBoxLayout(m_centralWidget)),
      m_tabbar(new Tabbar),
      m_jumpLineBar(new JumpLineBar()),
      m_replaceBar(new ReplaceBar(this)),
      m_themePanel(new ThemePanel(this)),
      m_findBar(new FindBar(this)),
      m_menu(new DMenu),
      m_blankFileDir(QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("blank-files")),
      m_backupDir(QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("backup-files")),
      m_autoBackupDir(QDir(Utils::cleanPath(QStandardPaths::standardLocations(QStandardPaths::DataLocation)).first()).filePath("autoBackup-files")),
      m_titlebarStyleSheet(titlebar()->styleSheet()),
      m_themePath(Settings::instance()->settings->option("advance.editor.theme")->value().toString())
{
    qRegisterMetaType<TextEdit *>("TextEdit");
    m_rootSaveDBus = new DBusDaemon::dbus("com.deepin.editor.daemon", "/", QDBusConnection::systemBus(), this);
    m_settings = Settings::instance();

    // Init.
    setAcceptDrops(true);
    loadTheme(m_themePath);

    DGuiApplicationHelper *guiAppHelp = DGuiApplicationHelper::instance();
    slotLoadContentTheme(guiAppHelp->themeType());

    //关闭　替换　查找 跳行bar
    connect(this, &Window::pressEsc, m_replaceBar, &ReplaceBar::pressEsc, Qt::QueuedConnection);
    connect(this, &Window::pressEsc, m_findBar, &FindBar::pressEsc, Qt::QueuedConnection);
    connect(this, &Window::pressEsc, m_jumpLineBar, &JumpLineBar::pressEsc, Qt::QueuedConnection);

    // Init settings.
    connect(m_settings, &Settings::sigAdjustFont, this, &Window::slotSigAdjustFont);
    connect(m_settings, &Settings::sigAdjustFontSize, this, &Window::slotSigAdjustFontSize);
    connect(m_settings, &Settings::sigAdjustTabSpaceNumber, this, &Window::slotSigAdjustTabSpaceNumber);
    connect(m_settings, &Settings::sigThemeChanged, this, &Window::slotSigThemeChanged);
    connect(m_settings, &Settings::sigAdjustWordWrap, this, &Window::slotSigAdjustWordWrap);
    connect(m_settings, &Settings::sigSetLineNumberShow, this, &Window::slotSigSetLineNumberShow);
    connect(m_settings, &Settings::sigAdjustBookmark, this, &Window::slotSigAdjustBookmark);
    connect(m_settings, &Settings::sigShowBlankCharacter, this, &Window::slotSigShowBlankCharacter);
    connect(m_settings, &Settings::sigHightLightCurrentLine, this, &Window::slotSigHightLightCurrentLine);
    connect(m_settings, &Settings::sigShowCodeFlodFlag, this, &Window::slotSigShowCodeFlodFlag);
    /* 设置页面里窗口模式的设置是针对新创建窗口的设置，本窗口不需要实时响应，暂且屏蔽此信号的绑定 */
    //connect(m_settings, &Settings::sigChangeWindowSize, this, &Window::slotSigChangeWindowSize);

    // Init layout and editor.
    m_centralLayout->setMargin(0);
    m_centralLayout->setSpacing(0);

    m_centralLayout->addWidget(m_editorWidget);
    setWindowIcon(QIcon::fromTheme("deepin-editor"));
    setCentralWidget(m_centralWidget);

    // Init titlebar.
    if (titlebar()) {
        initTitlebar();
    }

    // window minimum size.
    setMinimumSize(680, 300);
    // resize window size.
    int window_width = Settings::instance()->settings->option("advance.window.window_width")->value().toInt();
    int window_height = Settings::instance()->settings->option("advance.window.window_height")->value().toInt();
    window_height == 1 ? window_height = 600 : window_height;
    window_width == 1 ? window_width = 1000 : window_width;
    resize(window_width, window_height);

    //设置函数最大化或者正常窗口的初始化　2021.4.26 ut002764 lxp   fix bug:74774
    showCenterWindow(true);
    //检测语音助手服务是否被注册
    detectionIflytekaiassistant();

    // Init find bar.
    connect(m_findBar, &FindBar::findNext, this, &Window::handleFindNextSearchKeyword, Qt::QueuedConnection);
    connect(m_findBar, &FindBar::findPrev, this, &Window::handleFindPrevSearchKeyword, Qt::QueuedConnection);
    connect(m_findBar, &FindBar::removeSearchKeyword, this, &Window::handleRemoveSearchKeyword, Qt::QueuedConnection);
    connect(m_findBar, &FindBar::updateSearchKeyword, this, [ = ](QString file, QString keyword) {
        handleUpdateSearchKeyword(m_findBar, file, keyword);
    });
    connect(m_findBar, &FindBar::sigFindbarClose, this, &Window::slotFindbarClose, Qt::QueuedConnection);

    // Init replace bar.
    //connect(m_replaceBar, &ReplaceBar::removeSearchKeyword, this, &Window::handleRemoveSearchKeyword, Qt::QueuedConnection);

    connect(m_replaceBar, &ReplaceBar::beforeReplace, this, &Window::slot_beforeReplace, Qt::QueuedConnection);
    connect(m_replaceBar, &ReplaceBar::replaceAll, this, &Window::handleReplaceAll, Qt::QueuedConnection);
    connect(m_replaceBar, &ReplaceBar::replaceNext, this, &Window::handleReplaceNext, Qt::QueuedConnection);
    connect(m_replaceBar, &ReplaceBar::replaceRest, this, &Window::handleReplaceRest, Qt::QueuedConnection);
    connect(m_replaceBar, &ReplaceBar::replaceSkip, this, &Window::handleReplaceSkip, Qt::QueuedConnection);
    connect(m_replaceBar, &ReplaceBar::updateSearchKeyword, this, [ = ](QString file, QString keyword) {
        handleUpdateSearchKeyword(m_replaceBar, file, keyword);
    });
    connect(m_replaceBar, &ReplaceBar::sigReplacebarClose, this, &Window::slotReplacebarClose, Qt::QueuedConnection);

    // Init jump line bar.
    //QTimer::singleShot(0, m_jumpLineBar, SLOT(hide()));
    m_jumpLineBar->hide();
    m_jumpLineBar->setParent(this);

    connect(m_jumpLineBar, &JumpLineBar::jumpToLine, this, &Window::handleJumpLineBarJumpToLine, Qt::QueuedConnection);
    connect(m_jumpLineBar, &JumpLineBar::backToPosition, this, &Window::handleBackToPosition, Qt::QueuedConnection);
    connect(m_jumpLineBar, &JumpLineBar::lostFocusExit, this, &Window::handleJumpLineBarExit, Qt::QueuedConnection);

    // Make jump line bar pop at top-right of editor.
    //DAnchorsBase::setAnchor(m_jumpLineBar, Qt::AnchorTop, m_centralWidget, Qt::AnchorTop);
    //DAnchorsBase::setAnchor(m_jumpLineBar, Qt::AnchorRight, m_centralWidget, Qt::AnchorRight);

    DAnchors<JumpLineBar> anchors_jumpLineBar(m_jumpLineBar);
    anchors_jumpLineBar.setAnchor(Qt::AnchorTop, m_centralWidget, Qt::AnchorTop);
    anchors_jumpLineBar.setAnchor(Qt::AnchorRight, m_centralWidget, Qt::AnchorRight);
    anchors_jumpLineBar.setTopMargin(5);
    anchors_jumpLineBar.setRightMargin(5);
    m_jumpLineBar->raise();

    // Init findbar panel.
    static DAnchors<FindBar> anchors_findbar(m_findBar);
    anchors_findbar.setAnchor(Qt::AnchorBottom, m_centralWidget, Qt::AnchorBottom);
    anchors_findbar.setAnchor(Qt::AnchorHorizontalCenter, m_centralWidget, Qt::AnchorHorizontalCenter);
    anchors_findbar.setBottomMargin(1);
    //m_findBar->move(QPoint(10, height() - 5));
    m_findBar->raise();

    // Init replaceBar panel.
    DAnchors<ReplaceBar> anchors_replaceBar(m_replaceBar);
    anchors_replaceBar.setAnchor(Qt::AnchorBottom, m_centralWidget, Qt::AnchorBottom);
    anchors_replaceBar.setAnchor(Qt::AnchorHorizontalCenter, m_centralWidget, Qt::AnchorHorizontalCenter);
    anchors_replaceBar.setBottomMargin(1);
    //m_replaceBar->move(QPoint(10, height() - 57));
    m_replaceBar->raise();

    // Init theme panel.
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorTop, m_centralWidget, Qt::AnchorTop);
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorBottom, m_centralWidget, Qt::AnchorBottom);
    DAnchorsBase::setAnchor(m_themePanel, Qt::AnchorRight, m_centralWidget, Qt::AnchorRight);

    // for the first time open the need be init.
    m_themePanel->setSelectionTheme(m_themePath);

    connect(m_themePanel, &ThemePanel::themeChanged, this, &Window::themeChanged);
    connect(this, &Window::requestDragEnterEvent, this, &Window::dragEnterEvent);
    connect(this, &Window::requestDropEvent, this, &Window::dropEvent);

    connect(qApp, &QGuiApplication::focusWindowChanged, this, &Window::handleFocusWindowChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &Window::slotLoadContentTheme);

    //setChildrenFocus(false);
    Utils::clearChildrenFocus(m_tabbar);//使用此函数把tabbar的组件焦点去掉(左右箭头不能focus)

#ifdef DTKWIDGET_CLASS_DSizeMode
    // 适配紧凑模式更新，注意 Qt::QueuedConnection 需要在其他子组件更新后触发
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &Window::updateSizeMode, Qt::QueuedConnection);
#endif
}

Window::~Window()
{
    // We don't need clean pointers because application has exit here.
    if (nullptr != m_shortcutViewProcess) {
        delete (m_shortcutViewProcess);
        m_shortcutViewProcess = nullptr;
    }

    clearPrintTextDocument();
}

void Window::detectionIflytekaiassistant()
{
    IflytekAiassistantThread *pIflyThread = new IflytekAiassistantThread(this);
    connect(pIflyThread, &IflytekAiassistantThread::sigIsRegisteredIflytekAiassistant,
            this, &Window::slotIsRegisteredIflytekAiassistant);
    connect(pIflyThread, &IflytekAiassistantThread::finished, pIflyThread, &IflytekAiassistantThread::deleteLater);
    pIflyThread->start();
}

bool Window::getIsRegistIflytekAiassistant()
{
    return m_bIsRegistIflytekAiassistant;
}

void Window::loadIflytekaiassistantConfig()
{
    QString configPath = QString("%1/%2")
                         .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                         .arg("iflytek");

    QDir dir(configPath);
    if (!dir.exists()) {
        return;
    }
    QString key = "base/enable";
    dir.setFilter(QDir::Files);
    QStringList nameList = dir.entryList();
    for (auto name : nameList) {
        if (name.contains("-iat") || name.contains("-tts") || name.contains("-trans")) {
            QString filename = configPath + "/" + name;
            QSettings file(filename, QSettings::IniFormat);
            m_IflytekAiassistantState[name.split(".").first()] = file.value(key).toBool();
        }
    }
}

bool Window::getIflytekaiassistantConfig(const QString &mode_name)
{
    if (m_IflytekAiassistantState.contains(mode_name)) {
        return m_IflytekAiassistantState[mode_name];
    } else {
        qWarning() << "mode_name is not valid";
        return false;
    }
}

void Window::updateModifyStatus(const QString &path, bool isModified)
{
    int tabIndex = m_tabbar->indexOf(path);
    if (tabIndex == -1) {
        return;
    }

    QString tabName;
    QString filePath = m_tabbar->truePathAt(tabIndex);
    if (filePath.isNull() || filePath.length() <= 0 || Utils::isDraftFile(filePath)) {
        tabName = m_tabbar->textAt(tabIndex);
        if (isModified) {
            if (!tabName.contains('*')) {
                tabName.prepend('*');
            }
        } else {
            QRegularExpression reg("[^*](.+)");
            QRegularExpressionMatch match = reg.match(tabName);
            if (match.hasMatch()) {
                tabName = match.captured(0);
            }
        }
    } else {
        QFileInfo fileInfo(filePath);
        tabName = fileInfo.fileName();
        if (isModified) {
            tabName.prepend('*');
        }
    }

    m_tabbar->setTabText(tabIndex, tabName);
    //判断是否需要阻塞系统关机
    emit sigJudgeBlockShutdown();
}

void Window::updateSaveAsFileName(QString strOldFilePath, QString strNewFilePath)
{
    int tabIndex = m_tabbar->indexOf(strOldFilePath);
    EditWrapper *wrapper = m_wrappers.value(strOldFilePath);
    m_tabbar->updateTab(tabIndex, strNewFilePath, QFileInfo(strNewFilePath).fileName());
    wrapper->updatePath(strNewFilePath);

    //tabbar中存在和strNewFilePath同名的tab
    if (m_wrappers.contains(strNewFilePath)) {
        closeTab(strNewFilePath);
    }

    m_wrappers.remove(strOldFilePath);
    m_wrappers.insert(strNewFilePath, wrapper);
}

void Window::updateSabeAsFileNameTemp(QString strOldFilePath, QString strNewFilePath)
{
    int tabIndex = m_tabbar->indexOf(strOldFilePath);
    EditWrapper *wrapper = m_wrappers.value(strOldFilePath);
    m_tabbar->updateTab(tabIndex, strNewFilePath, QFileInfo(strNewFilePath).fileName());
    wrapper->updatePath(strNewFilePath);
    m_wrappers.remove(strOldFilePath);
    m_wrappers.insert(strNewFilePath, wrapper);
}

void Window::showCenterWindow(bool bIsCenter)
{
    // Init window state with config.
    // Below code must before this->titlebar()->setMenu, otherwise main menu can't display pre-build-in menu items by dtk.
    QString windowState = Settings::instance()->settings->option("advance.window.windowstate")->value().toString();

    if (bIsCenter) {
        Dtk::Widget::moveToCenter(this);
    }
    // init window state.
    if (windowState == "window_maximum") {
        showMaximized();
        m_needMoveToCenter = true;
    } else if (windowState == "fullscreen") {
        showFullScreen();
        m_needMoveToCenter = true;
    } else {
        showNormal();
    }
}

void Window::initTitlebar()
{
    QAction *newWindowAction(new QAction(tr("New window"), this));
    QAction *newTabAction(new QAction(tr("New tab"), this));
    QAction *openFileAction(new QAction(tr("Open file"), this));
    QAction *saveAction(new QAction(tr("Save"), this));
    QAction *saveAsAction(new QAction(tr("Save as"), this));
    QAction *printAction(new QAction(tr("Print"), this));
    QAction *switchThemeAction(new QAction(tr("Switch theme"), this));
    QAction *settingAction(new QAction(tr("Settings"), this));
    QAction *findAction(new QAction(QApplication::translate("TextEdit", "Find"), this));
    QAction *replaceAction(new QAction(QApplication::translate("TextEdit", "Replace"), this));

    m_menu->addAction(newWindowAction);
    m_menu->addAction(newTabAction);
    m_menu->addAction(openFileAction);
    m_menu->addSeparator();
    m_menu->addAction(findAction);
    m_menu->addAction(replaceAction);
    m_menu->addAction(saveAction);
    m_menu->addAction(saveAsAction);
    m_menu->addAction(printAction);
    //此接口不可删除，预留的编辑器内部主题选择接口
    //m_menu->addAction(switchThemeAction);
    m_menu->addSeparator();
    m_menu->addAction(settingAction);

    m_menu->setMinimumWidth(150);

    titlebar()->addWidget(m_tabbar);

    titlebar()->setCustomWidget(m_tabbar, false);
    titlebar()->setSeparatorVisible(false);
    titlebar()->setMenu(m_menu);
    titlebar()->setIcon(QIcon::fromTheme("deepin-editor"));
    titlebar()->setFocusPolicy(Qt::NoFocus);         //设置titlebar无焦点，点击titlebar时光标不移动

    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    addButton->setFocusPolicy(Qt::NoFocus);
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    optionBtn->setFocusPolicy(Qt::NoFocus);
    DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    minBtn->setFocusPolicy(Qt::NoFocus);
    DIconButton *quitFullBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
    quitFullBtn->setFocusPolicy(Qt::NoFocus);
    DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    maxBtn->setFocusPolicy(Qt::NoFocus);
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    closeBtn->setFocusPolicy(Qt::NoFocus);

    connect(m_tabbar, &DTabBar::tabBarDoubleClicked, titlebar(), &DTitlebar::doubleClicked, Qt::QueuedConnection);

    connect(m_tabbar, &Tabbar::closeTabs, this, &Window::handleTabsClosed, Qt::QueuedConnection);
    connect(m_tabbar, &Tabbar::requestHistorySaved, this, [ = ](const QString & filePath) {
        // 单个Tab页关闭文件时记录文件信息
        if (StartManager::instance()->checkPath(filePath)) {
            Utils::recordCloseFile(filePath);
        }

        if (QFileInfo(filePath).dir().absolutePath() == m_blankFileDir) {
            return;
        }

        if (!m_closeFileHistory.contains(filePath)) {
            m_closeFileHistory << filePath;
        }
    });

    connect(m_tabbar, &DTabBar::tabCloseRequested, this, &Window::handleTabCloseRequested, Qt::QueuedConnection);
    connect(m_tabbar, &DTabBar::tabAddRequested, this, static_cast<void (Window::*)()>(&Window::addBlankTab), Qt::QueuedConnection);
    connect(m_tabbar, &DTabBar::currentChanged, this, &Window::handleCurrentChanged, Qt::QueuedConnection);
    // 当标签页新增、删除时触发变更信号(用于备份当前打开文件信息)
    connect(m_tabbar, &DTabBar::tabIsInserted, this, &Window::tabChanged, Qt::QueuedConnection);
    connect(m_tabbar, &DTabBar::tabIsRemoved, this, &Window::tabChanged, Qt::QueuedConnection);

    connect(newWindowAction, &QAction::triggered, this, &Window::newWindow);
    connect(newTabAction, &QAction::triggered, this, static_cast<void (Window::*)()>(&Window::addBlankTab));
    connect(openFileAction, &QAction::triggered, this, &Window::openFile);
    connect(findAction, &QAction::triggered, this, &Window::popupFindBar);
    connect(replaceAction, &QAction::triggered, this, &Window::popupReplaceBar);
    connect(saveAction, &QAction::triggered, this, &Window::saveFile);
    connect(saveAsAction, &QAction::triggered, this, &Window::saveAsFile);
    connect(printAction, &QAction::triggered, this, &Window::popupPrintDialog);
    connect(settingAction, &QAction::triggered, this, &Window::popupSettingsDialog);
    connect(switchThemeAction, &QAction::triggered, this, &Window::popupThemePanel);
}

bool Window::checkBlockShutdown()
{
    //判断是否有未保存的tab项
    for (int i = 0; i < m_tabbar->count(); i++) {
        if (m_tabbar->textAt(i).isNull()) {
            return false;
        }
        //如果有未保存的tab项，return true阻塞系统关机
        if (m_tabbar->textAt(i).at(0) == '*') {
            return true;
        }
    }

    return false;
}

int Window::getTabIndex(const QString &file)
{
    return m_tabbar->indexOf(file);
}

void Window::activeTab(int index)
{
    DMainWindow::activateWindow();
    m_tabbar->setCurrentIndex(index);
}

Tabbar *Window::getTabbar()
{
    return m_tabbar;
}

void Window::addTab(const QString &filepath, bool activeTab)
{
    // check whether it is an editable file thround mimeType.
    if (Utils::isMimeTypeSupport(filepath)) {
        const QFileInfo fileInfo(filepath);
        QString tabName = fileInfo.fileName();
        EditWrapper *curWrapper = currentWrapper();

        if (!fileInfo.isWritable() && fileInfo.isReadable()) {
            tabName += QString(" (%1)").arg(tr("Read-Only"));
        }

        if (curWrapper) {
            // if the current page is a draft file and is empty
            if (m_tabbar->indexOf(filepath) != -1) {
                m_tabbar->setCurrentIndex(m_tabbar->indexOf(filepath));
            }
        }

        // check if have permission to read the file.
        QFile file(filepath);
        QFile::Permissions permissions = file.permissions();
        bool bIsRead = (permissions & QFile::ReadUser || permissions & QFile::ReadOwner || permissions & QFile::ReadOther);
        if (fileInfo.exists() && !bIsRead) {
            DMessageManager::instance()->sendMessage(m_editorWidget->currentWidget(), QIcon(":/images/warning.svg")
                                                     , QString(tr("You do not have permission to open %1")).arg(filepath));
            return;
        }

        if (StartManager::instance()->checkPath(filepath)) {

            m_tabbar->addTab(filepath, tabName, filepath);

            if (!m_wrappers.contains(filepath)) {
                EditWrapper *wrapper = createEditor();
                showNewEditor(wrapper);
                m_wrappers[filepath] = wrapper;

                if (!fileInfo.isWritable() && fileInfo.isReadable()) {
                    wrapper->textEditor()->setReadOnlyPermission(true);
                }

                wrapper->openFile(filepath, filepath);

                // 查找文件是否存在书签
                auto bookmarkInfo = StartManager::instance()->findBookmark(filepath);
                wrapper->textEditor()->setBookMarkList(bookmarkInfo);
            }
            // Activate window.
            activateWindow();
        }

        // Active tab if activeTab is true.
        if (activeTab) {
            int tabIndex = m_tabbar->indexOf(filepath);
            if (tabIndex != -1) {
                m_tabbar->setCurrentIndex(tabIndex);
            }
        }
    } else {
        if (currentWrapper() == nullptr) {
            this->addBlankTab();
        }

        QString strFileName = QFileInfo(filepath).fileName();
        strFileName = tr("Invalid file: %1").arg(strFileName);
        strFileName = Utils::lineFeed(strFileName, m_editorWidget->currentWidget()->width() - FLOATTIP_MARGIN, m_editorWidget->currentWidget()->font(), 2);
        DMessageManager::instance()->sendMessage(m_editorWidget->currentWidget(), QIcon(":/images/warning.svg"), strFileName);
    }
}

void Window::addTabWithWrapper(EditWrapper *wrapper, const QString &filepath, const QString &qstrTruePath, const QString &tabName, int index)
{
    if (index == -1) {
        index = m_tabbar->count();
    }

    //这里会重复连接信号和槽，先全部取消
    QDBusConnection dbus = QDBusConnection::sessionBus();
    switch (Utils::getSystemVersion()) {
    case Utils::V23:
        dbus.systemBus().disconnect("org.deepin.dde.Gesture1",
                                    "/org/deepin/dde/Gesture1", "org.deepin.dde.Gesture1",
                                    "Event",
                                    wrapper->textEditor(), SLOT(fingerZoom(QString, QString, int)));
        break;
    default:
        dbus.systemBus().disconnect("com.deepin.daemon.Gesture",
                                    "/com/deepin/daemon/Gesture", "com.deepin.daemon.Gesture",
                                    "Event",
                                    wrapper->textEditor(), SLOT(fingerZoom(QString, QString, int)));
        break;
    }
    wrapper->textEditor()->disconnect();
    connect(wrapper->textEditor(), &TextEdit::cursorModeChanged, wrapper, &EditWrapper::handleCursorModeChanged);
    connect(wrapper->textEditor(), &TextEdit::clickFindAction, this, &Window::popupFindBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickReplaceAction, this, &Window::popupReplaceBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickJumpLineAction, this, &Window::popupJumpLineBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickFullscreenAction, this, &Window::toggleFullscreen, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::popupNotify, this, &Window::showNotify, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::signal_setTitleFocus, this, &Window::slot_setTitleFocus, Qt::QueuedConnection);

    switch (Utils::getSystemVersion()) {
    case Utils::V23:
        dbus.systemBus().connect("org.deepin.dde.Gesture1",
                                 "/org/deepin/dde/Gesture1", "org.deepin.dde.Gesture1",
                                 "Event",
                                 wrapper->textEditor(), SLOT(fingerZoom(QString, QString, int)));
        break;
    default:
        dbus.systemBus().connect("com.deepin.daemon.Gesture",
                                 "/com/deepin/daemon/Gesture", "com.deepin.daemon.Gesture",
                                 "Event",
                                 wrapper->textEditor(), SLOT(fingerZoom(QString, QString, int)));
        break;
    }
    connect(wrapper->textEditor(), &QPlainTextEdit::cursorPositionChanged, wrapper->textEditor(), &TextEdit::cursorPositionChanged);

    connect(wrapper->textEditor(), &QPlainTextEdit::textChanged, wrapper->textEditor(), [ = ]() {
        wrapper->UpdateBottomBarWordCnt(wrapper->textEditor()->characterCount());
    });


    // add wrapper to this window.
    m_tabbar->addTabWithIndex(index, filepath, tabName, qstrTruePath);
    m_wrappers[filepath] = wrapper;
    wrapper->updatePath(filepath, qstrTruePath);

    showNewEditor(wrapper);
    wrapper->OnThemeChangeSlot(m_themePath);
}

/**
 * @return 关闭当前焦点的标签页并返回是否成功关闭。
 */
bool Window::closeTab()
{
    const QString &filePath = m_tabbar->currentPath();
    // 避免异常情况重入时当前已无标签页的情况
    if (filePath.isEmpty()) {
        return false;
    }
    return closeTab(filePath);
}

bool Window::closeTab(const QString &filePath)
{
    EditWrapper *wrapper = m_wrappers.value(filePath);
    if (!wrapper) {
        return false;
    }

    if (!currentWrapper()) {
        return false;
    }

    if (m_reading_list.contains(currentWrapper()->textEditor())) {
        if (m_settings->settings) {
            m_settings->settings->sync();
        }

        QProcess process;
        process.start("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/tts com.iflytek.aiassistant.tts.stopTTSDirectly");
        if (!process.waitForFinished(10)) {
            process.terminate();
        }
    }

    // 当前窗口为打印文本窗口
    if (wrapper == m_printWrapper) {
        // 退出打印
        m_bPrintProcessing = false;
    }

    disconnect(wrapper, nullptr);
    disconnect(wrapper->textEditor(), &TextEdit::textChanged, nullptr, nullptr);

    // this property holds whether the document has been modified by the user
    bool isModified = wrapper->isModified();
    bool isDraftFile = wrapper->isDraftFile();
    bool bIsBackupFile = false;

    if (wrapper->isTemFile()) {
        bIsBackupFile = true;
    }

    if (wrapper->getFileLoading()) {
        isModified = false;
    }

    // 关闭标签页前，记录全局的书签信息
    QList<int> bookmarkInfo = wrapper->textEditor()->getBookmarkInfo();
    if (!bookmarkInfo.isEmpty()) {
        QString localPath = wrapper->textEditor()->getTruePath();
        StartManager::instance()->recordBookmark(localPath, bookmarkInfo);
    }

    if (isDraftFile) {
        if (isModified) {
            DDialog *dialog = createDialog(tr("Do you want to save this file?"), "");
            int res = dialog->exec();

            //取消或关闭弹窗不做任务操作
            if (res == 0 || res == -1) {
                return false;
            }

            //不保存
            if (res == 1) {
                removeWrapper(filePath, true);
                m_tabbar->closeCurrentTab(filePath);
                QFile(filePath).remove();
                return true;
            }

            //保存
            if (res == 2) {
                QFileInfo fileInfo(filePath);
                QString newFilePath;

                if (wrapper->saveDraftFile(newFilePath)) {
                    removeWrapper(filePath, true);
                    // 保存临时文件后已更新tab页的文件路径，使用新的文件路径删除窗口
                    m_tabbar->closeCurrentTab(newFilePath);
                    QFile(filePath).remove();
                } else {
                    // 保存不成功时不关闭窗口
                    return false;
                }
            }
        } else {
            removeWrapper(filePath, true);
            m_tabbar->closeCurrentTab(filePath);
            QFile(filePath).remove();
        }
    }
    // document has been modified or unsaved draft document.
    // need to prompt whether to save.
    else {
        QFileInfo fileInfo(filePath);
        isModified = false;
        if (m_tabbar->textAt(m_tabbar->currentIndex()).front() == "*") {
            isModified = true;
        }
        if (isModified) {
            DDialog *dialog = createDialog(tr("Do you want to save this file?"), "");
            int res = dialog->exec();

            //取消或关闭弹窗不做任务操作
            if (res == 0 || res == -1) {
                return false;
            }

            //不保存
            if (res == 1) {
                removeWrapper(filePath, true);
                m_tabbar->closeCurrentTab(filePath);

                //删除备份文件
                if (bIsBackupFile) {
                    QFile(filePath).remove();
                }

                //删除自动备份文件
                if (QFileInfo(m_autoBackupDir).exists()) {
                    fileInfo.setFile(wrapper->textEditor()->getTruePath());
                    QString name = fileInfo.absolutePath().replace("/", "_");
                    QDir(m_autoBackupDir).remove(fileInfo.baseName() + "." + name + "." + fileInfo.suffix());
                }

                return true;
            }

            //保存
            if (res == 2) {
                if (bIsBackupFile) {
                    if (wrapper->saveFile()) {
                        removeWrapper(filePath, true);
                        m_tabbar->closeCurrentTab(filePath);
                        QFile(filePath).remove();
                    } else {
                        saveAsFile();
                    }
                } else {
                    if (wrapper->saveFile()) {
                        removeWrapper(filePath, true);
                        m_tabbar->closeCurrentTab(filePath);
                    } else {
                        saveAsFile();
                    }
                }
            }
        } else {
            removeWrapper(filePath, true);
            m_tabbar->closeCurrentTab(filePath);
        }

        //删除自动备份文件
        if (QFileInfo(m_autoBackupDir).exists()) {
            fileInfo.setFile(wrapper->textEditor()->getTruePath());
            QString name = fileInfo.absolutePath().replace("/", "_");
            QDir(m_autoBackupDir).remove(fileInfo.baseName() + "." + name + "." + fileInfo.suffix());
        }
    }

    return true;
}

void Window::restoreTab()
{
    if (m_closeFileHistory.size() > 0) {
        addTab(m_closeFileHistory.takeLast());
    }
}

EditWrapper *Window::createEditor()
{
    EditWrapper *wrapper = new EditWrapper(this);
    connect(wrapper, &EditWrapper::sigClearDoubleCharaterEncode, this, &Window::slotClearDoubleCharaterEncode);
    connect(wrapper->textEditor(), &TextEdit::signal_readingPath, this, &Window::slot_saveReadingPath, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::signal_setTitleFocus, this, &Window::slot_setTitleFocus, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickFindAction, this, &Window::popupFindBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickReplaceAction, this, &Window::popupReplaceBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickJumpLineAction, this, &Window::popupJumpLineBar, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::clickFullscreenAction, this, &Window::toggleFullscreen, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::popupNotify, this, &Window::showNotify, Qt::QueuedConnection);
    connect(wrapper->textEditor(), &TextEdit::textChanged, this, [ = ]() {
        updateJumpLineBar(wrapper->textEditor());
    }, Qt::QueuedConnection);

    bool wordWrap = m_settings->settings->option("base.font.wordwrap")->value().toBool();
    wrapper->textEditor()->m_pIsShowCodeFoldArea = m_settings->settings->option("base.font.codeflod")->value().toBool();
    wrapper->OnThemeChangeSlot(m_themePath);
    wrapper->textEditor()->setSettings(m_settings);
    wrapper->textEditor()->setTabSpaceNumber(m_settings->settings->option("advance.editor.tabspacenumber")->value().toInt());
    wrapper->textEditor()->setFontFamily(m_settings->settings->option("base.font.family")->value().toString());
    wrapper->textEditor()->setLineWrapMode(wordWrap);
    setFontSizeWithConfig(wrapper);

    //设置显示空白符　梁卫东
    wrapper->setShowBlankCharacter(m_settings->settings->option("base.font.showblankcharacter")->value().toBool());
    //yanyuhan 设置行号显示
    wrapper->setLineNumberShow(m_settings->settings->option("base.font.showlinenumber")->value().toBool(), true);
    wrapper->textEditor()->setHighLineCurrentLine(m_settings->settings->option("base.font.hightlightcurrentline")->value().toBool());
    wrapper->textEditor()->setBookmarkFlagVisable(m_settings->settings->option("base.font.showbookmark")->value().toBool(), true);
    wrapper->textEditor()->setCodeFlodFlagVisable(m_settings->settings->option("base.font.codeflod")->value().toBool(), true);
    wrapper->textEditor()->updateLeftAreaWidget();

    return wrapper;
}

EditWrapper *Window::currentWrapper()
{
    if (m_wrappers.contains(m_tabbar->currentPath())) {
        return m_wrappers.value(m_tabbar->currentPath());
    } else {
        return nullptr;
    }
}

EditWrapper *Window::wrapper(const QString &filePath)
{
    if (m_wrappers.contains(filePath)) {
        return m_wrappers.value(filePath);
    } else {
        for (auto wrapper : m_wrappers) {
            QString truePath = wrapper->textEditor()->getTruePath();
            if (truePath == filePath) {
                return m_wrappers.value(wrapper->textEditor()->getFilePath());
            }
        }

        return nullptr;
    }
}

TextEdit *Window::getTextEditor(const QString &filepath)
{
    if (m_wrappers.contains(filepath)) {
        return m_wrappers.value(filepath)->textEditor();
    } else {
        return nullptr;
    }
}

void Window::focusActiveEditor()
{
    if (m_tabbar->count() > 0) {
        if (currentWrapper() == nullptr) {
            return;
        }
        currentWrapper()->textEditor()->setFocus();
    }
}

void Window::removeWrapper(const QString &filePath, bool isDelete)
{
    EditWrapper *wrapper = m_wrappers.value(filePath);

    if (wrapper) {
        qInfo() << "begin removeWrapper";
        if (nullptr == m_editorWidget) {
            return;
        }
        m_editorWidget->removeWidget(wrapper);
        m_wrappers.remove(filePath);
        if (isDelete) {
            disconnect(wrapper->textEditor(), nullptr);
            disconnect(wrapper, nullptr);
            wrapper->setQuitFlag();
            wrapper->deleteLater();
            qInfo() << "after delete wrapper";
        }
    }

    // Exit window after close all tabs.
    if (m_wrappers.isEmpty()) {
        close();
        qInfo() << "after close";
    }

    qInfo() << "end removeWrapper";
}

void Window::openFile()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    // read history directory.
    QString historyDirStr = m_settings->settings->option("advance.editor.file_dialog_dir")->value().toString();
    if (historyDirStr.isEmpty() || !QDir(historyDirStr).exists() || !QFileInfo(historyDirStr).isWritable() || !QDir(historyDirStr).isReadable()) {
        historyDirStr = QDir::homePath() + "/Documents";
    }
    dialog.setDirectory(historyDirStr);

    QDir historyDir(historyDirStr);

    if (historyDir.exists()) {
        dialog.setDirectory(historyDir);
    } else {
        qDebug() << "historyDir or default path not existed:" << historyDir;
    }

    QString path = m_settings->getSavePath(m_settings->getSavePathId());
    // 使用当前文件路径时，打开当前文件的目录，新建文档为系统-文档目录
    if (PathSettingWgt::CurFileBox == m_settings->getSavePathId()) {
        path = getCurrentOpenFilePath();
    }
    if (path.isEmpty() || !QDir(path).exists() || !QFileInfo(path).isWritable() || !QDir(path).isReadable()) {
        path = QDir::homePath() + "/Documents";
    }
    dialog.setDirectory(path);

    const int mode = dialog.exec();

    PerformanceMonitor::openFileStart();
    // save the directory string.
    m_settings->settings->option("advance.editor.file_dialog_dir")->setValue(dialog.directoryUrl().toLocalFile());

    if (mode != QDialog::Accepted) {
        return;
    }

    QStringList supportfileNames;
    QStringList otherfiles;
    for (const QString &file : dialog.selectedFiles()) {

        if (Utils::isMimeTypeSupport(file)) {
            supportfileNames.append(file);
        } else {
            otherfiles.append(file);
        }

        //先添加支持的文件
    }
    foreach (QString var, supportfileNames) {
        addTab(var, true);
    }

    //后添加不支持文件　在最后编辑页面显示
    foreach (QString var, otherfiles) {
        addTab(var, true);
    }
}

bool Window::saveFile()
{
    EditWrapper *wrapperEdit = currentWrapper();

    //大文本加载过程不允许保存
    if (!wrapperEdit || wrapperEdit->getFileLoading()) return false;

    bool isDraftFile = wrapperEdit->isDraftFile();
    //bool isEmpty = wrapperEdit->isPlainTextEmpty();
    QString filePath = wrapperEdit->textEditor()->getTruePath();

    // save blank file.
    if (isDraftFile) {
        return saveAsFile();
    }

    QFileInfo info(filePath);
    if (info.exists()) {
        //判断文件是否有写的权限
        QFile temporaryBuffer(filePath);
        QFile::Permissions pers = temporaryBuffer.permissions();
        bool isWrite = ((pers & QFile::WriteUser) || (pers & QFile::WriteOwner) || (pers & QFile::WriteOther));
        if (!isWrite) {
            DMessageManager::instance()->sendMessage(m_editorWidget->currentWidget(), QIcon(":/images/warning.svg")
                                                     , QString(tr("You do not have permission to save %1")).arg(info.fileName()));
            return false;
        }
    }

    // save normal file.
    QString temPath = "";

    if (wrapperEdit->isTemFile()) {
        temPath = wrapperEdit->textEditor()->getFilePath();
    } else {
        temPath = filePath;
    }
    bool success = wrapperEdit->saveFile();

    if (success) {
        updateSabeAsFileNameTemp(temPath, filePath);
        if (currentWrapper()) {
            currentWrapper()->hideWarningNotices();
        }
        showNotify(tr("Saved successfully"));

        //删除备份文件
        if (temPath != filePath) {
            QFile(temPath).remove();
        }

        //删除自动备份文件
        if (QFileInfo(m_autoBackupDir).exists()) {
            QFileInfo fileInfo(filePath);
            QString name = fileInfo.absolutePath().replace("/", "_");
            QDir(m_autoBackupDir).remove(fileInfo.baseName() + "." + name + "." + fileInfo.suffix());
        }

        return true;
    }

    return false;
}

bool Window::saveAsFile()
{
    return !saveAsFileToDisk().isEmpty();
}

QString Window::saveAsFileToDisk()
{
    EditWrapper *wrapper = currentWrapper();
    //大文本加载过程不允许保存　梁卫东
    if (!wrapper || wrapper->getFileLoading()) {
        return QString();
    }

    bool isDraft = wrapper->isDraftFile();
    QFileInfo fileInfo(wrapper->textEditor()->getTruePath());

    DFileDialog dialog(this, tr("Save File"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setDirectory(QDir::homePath());
    // 允许选取保存的编码格式
    DFileDialog::DComboBoxOptions encodingOptions;
    encodingOptions.editable = false;
    encodingOptions.defaultValue = wrapper->getTextEncode();
    encodingOptions.data = Utils::getSupportEncodingList();
    dialog.addComboBox(QObject::tr("Encoding"), encodingOptions);

    QString path = m_settings->getSavePath(m_settings->getSavePathId());
    // 使用当前文件路径时，打开当前文件的目录，新建文档为系统-文档目录
    if (PathSettingWgt::CurFileBox == m_settings->getSavePathId()) {
        path = getCurrentOpenFilePath();
    }
    if (path.isEmpty() || !QDir(path).exists() || !QFileInfo(path).isWritable() || !QDir(path).isReadable()) {
        path = QDir::homePath() + "/Documents";
    }
    dialog.setDirectory(path);

    if (isDraft) {
        QRegularExpression reg("[^*](.+)");
        QRegularExpressionMatch match = reg.match(m_tabbar->currentName());
        dialog.selectFile(match.captured(0) + ".txt");
    } else {
        dialog.setDirectory(fileInfo.dir());
        dialog.selectFile(fileInfo.fileName());
    }

    int mode = dialog.exec();
    if (mode == QDialog::Accepted) {
        const QByteArray encode = dialog.getComboBoxValue(QObject::tr("Encoding")).toUtf8();
        const QString newFilePath = dialog.selectedFiles().value(0);
        Settings::instance()->setSavePath(PathSettingWgt::LastOptBox, QFileInfo(newFilePath).absolutePath());
        Settings::instance()->setSavePath(PathSettingWgt::CurFileBox, QFileInfo(newFilePath).absolutePath());

        wrapper->updatePath(wrapper->filePath(), newFilePath);

        bool needChangeEncode = (encode != wrapper->getTextEncode().toUtf8());
        bool saveSucc = false;
        if (QFileInfo(wrapper->filePath()).absoluteFilePath()
                == QFileInfo(newFilePath).absoluteFilePath()) {
            // 相同路径，直接保存文件
            saveSucc = wrapper->saveFile(encode);
        } else {
            saveSucc = wrapper->saveAsFile(newFilePath, encode);
        }

        if (!saveSucc) {
            /* 如果保存未成功，无需记录更新新文件路径 */
            wrapper->updatePath(wrapper->filePath(), QString());
            return QString();
        } else {
            // 更新文件编码
            wrapper->bottomBar()->setEncodeName(encode);

            // 若编码变更，保存完成后，重新加载文件
            if (needChangeEncode) {
                wrapper->readFile(encode);
            }
        }

        if (wrapper->filePath().contains(m_backupDir) || wrapper->filePath().contains(m_blankFileDir)) {
            QFile(wrapper->filePath()).remove();
        }

        //删除自动备份文件
        if (QFileInfo(m_autoBackupDir).exists()) {
            QString truePath = wrapper->textEditor()->getTruePath();
            fileInfo.setFile(truePath);
            QString name = fileInfo.absolutePath().replace("/", "_");
            QDir(m_autoBackupDir).remove(fileInfo.baseName() + "." + name + "." + fileInfo.suffix());
        }

        /* 如果另存为的文件名+路径与当前tab项对应的文件名+路径是一致，则直接做保存操作即可 */
        if (!wrapper->filePath().compare(newFilePath)) {
            if (saveFile()) {
                return newFilePath;
            }
        }

        updateSaveAsFileName(wrapper->filePath(), newFilePath);
        return newFilePath;
    }

    return QString();
}

QString Window::saveBlankFileToDisk()
{
    QString filePath = m_tabbar->currentPath();
    EditWrapper *wrapper = m_wrappers.value(filePath);
    bool isDraft = Utils::isDraftFile(filePath);
    QFileInfo fileInfo(filePath);

    if (!wrapper)
        //return false;
        return QString();

    DFileDialog dialog(this, tr("Save File"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.addComboBox(tr("Encoding"), QStringList() << wrapper->getTextEncode());
    dialog.setDirectory(QDir::homePath());

    if (isDraft) {
        QRegularExpression reg("[^*](.+)");
        QRegularExpressionMatch match = reg.match(m_tabbar->currentName());
        dialog.selectFile(match.captured(0) + ".txt");
    } else {
        dialog.setDirectory(fileInfo.dir());
        dialog.selectFile(fileInfo.fileName());
    }

    int mode = dialog.exec();
    if (mode == QDialog::Accepted) {
        const QString newFilePath = dialog.selectedFiles().value(0);
        wrapper->updatePath(newFilePath);
        wrapper->saveFile();

        m_wrappers.remove(filePath);
        m_wrappers.insert(newFilePath, wrapper);

        // wrapper->textEditor()->loadHighlighter();
        return newFilePath;
    }

    return QString();
}

bool Window::saveAsOtherTabFile(EditWrapper *wrapper)
{
    QString filePath = wrapper->textEditor()->getFilePath();
    bool isDraft = Utils::isDraftFile(filePath);
    QFileInfo fileInfo(filePath);
    int index = m_tabbar->indexOf(filePath);
    QString strTabText = m_tabbar->tabText(index);

    if (!wrapper)
        return false;

    DFileDialog dialog(this, tr("Save File"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.addComboBox(QObject::tr("Encoding"), Utils::getEncodeList());
    dialog.addComboBox(QObject::tr("Line Endings"), QStringList() << "Linux" << "Windows" << "Mac OS");
    dialog.setDirectory(QDir::homePath());

    if (isDraft) {
        QRegularExpression reg("[^*](.+)");
        QRegularExpressionMatch match = reg.match(strTabText);
        dialog.selectFile(match.captured(0) + ".txt");
    } else {
        dialog.setDirectory(fileInfo.dir());
        dialog.selectFile(fileInfo.fileName());
    }

    int mode = dialog.exec();
    if (mode == QDialog::Accepted) {
        const QByteArray encode = dialog.getComboBoxValue(QObject::tr("Encoding")).toUtf8();
        const QString endOfLine = dialog.getComboBoxValue(QObject::tr("Line Endings"));
        const QString newFilePath = dialog.selectedFiles().value(0);
        const QFileInfo newFileInfo(newFilePath);
//        EditWrapper::EndOfLineMode eol = EditWrapper::eolUnix;

//        if (endOfLine == "Windows") {
//            eol = EditWrapper::eolDos;
//        } else if (endOfLine == "Mac OS") {
//            eol = EditWrapper::eolMac;
//        }

        if (isDraft) {
            QFile(filePath).remove();
        }

        //m_tabbar->updateTab(m_tabbar->currentIndex(), newFilePath, newFileInfo.fileName());

        //  wrapper->setTextCodec(encode);
        wrapper->updatePath(newFilePath);
        //wrapper->setEndOfLineMode(eol);
        wrapper->saveFile();

        // wrapper->textEditor()->loadHighlighter();
    } else {
        return false;
    }

    return true;
}

void Window::changeSettingDialogComboxFontNumber(int fontNumber)
{
    m_settings->settings->option("base.font.size")->setValue(fontNumber);
}

/**
 * @brief 根据传入的字体大小计算字体的比例，字体大小范围在 8 ~ 500，比例范围在 10% ~ 500%,
 *      默认字体大小为12。因此在 8~12 和 12~50 两组范围字体的缩放间隔不同。
 * @param fontSize 字体大小
 * @return 字体缩放比例，范围 10 ~ 500
 */
qreal Window::calcFontScale(qreal fontSize)
{
    if (qFuzzyCompare(fontSize, m_settings->m_iDefaultFontSize)) {
        return 100.0;
    } else if (fontSize > m_settings->m_iDefaultFontSize) {
        static const qreal delta = (500 - 100) * 1.0 / (m_settings->m_iMaxFontSize - m_settings->m_iDefaultFontSize);
        qreal fontScale = 100 + delta * (fontSize - m_settings->m_iDefaultFontSize);
        return qMin(fontScale, 500.0);
    } else {
        static const qreal delta = (100 - 10) * 1.0 / (m_settings->m_iDefaultFontSize - m_settings->m_iMinFontSize);
        qreal fontScale = 100 + delta * (fontSize - m_settings->m_iDefaultFontSize);
        return qMax(fontScale, 10.0);
    }
}

/**
 * @brief 根据字体缩放比例返回字体大小
 * @param fontScale 字体缩放比例
 * @return 字体大小，范围 8~50
 */
qreal Window::calcFontSizeFromScale(qreal fontScale)
{
    if (qFuzzyCompare(fontScale, 100.0)) {
        return m_settings->m_iDefaultFontSize;
    } else if (fontScale > 100.0) {
        static const qreal delta = (m_settings->m_iMaxFontSize - m_settings->m_iDefaultFontSize) * 1.0 / (500 - 100);
        qreal fontSize = m_settings->m_iDefaultFontSize + delta * ((fontScale) - 100);
        return qMin<qreal>(fontSize, m_settings->m_iMaxFontSize);
    } else {
        static const qreal delta = (m_settings->m_iDefaultFontSize - m_settings->m_iMinFontSize) * 1.0 / (100 - 10) ;
        qreal fontSize = m_settings->m_iMinFontSize + delta * (fontScale - 10);
        return qMax<qreal>(fontSize, m_settings->m_iMinFontSize);
    }
}

void Window::decrementFontSize()
{
    qreal fontScale = calcFontScale(m_fontSize);
    // 减少10%
    fontScale -= 10;
    m_fontSize = calcFontSizeFromScale(fontScale);

    qreal size = qMax<qreal>(m_fontSize, m_settings->m_iMinFontSize);
    m_settings->settings->option("base.font.size")->setValue(size);
}

void Window::incrementFontSize()
{
    qreal fontScale = calcFontScale(m_fontSize);
    // 增加10%
    fontScale += 10;
    m_fontSize = calcFontSizeFromScale(fontScale);

    qreal size = qMin<qreal>(m_fontSize, m_settings->m_iMaxFontSize);
    m_settings->settings->option("base.font.size")->setValue(size);
}

void Window::resetFontSize()
{
    m_settings->settings->option("base.font.size")->setValue(m_settings->m_iDefaultFontSize);
}

void Window::setFontSizeWithConfig(EditWrapper *wrapper)
{
    qreal size = m_settings->settings->option("base.font.size")->value().toReal();
    wrapper->textEditor()->setFontSize(size);
    wrapper->bottomBar()->setScaleLabelText(size);

    m_fontSize = size;
}

void Window::popupFindBar()
{
#if 0
    if (m_findBar->isVisible()) {
        m_findBar->move(QPoint(10, height() - 59));
        if (m_findBar->isFocus()) {
            m_wrappers.value(m_tabbar->currentPath())->textEditor()->setFocus();
        } else {
            m_findBar->focus();
        }
    } else {
        addBottomWidget(m_findBar);
    }
#endif

    m_findBar->setSearched(false);
    QString tabPath = m_tabbar->currentPath();
    EditWrapper *wrapper = currentWrapper();

    if (currentWrapper() == nullptr) {
        return;
    }

    if (wrapper->textEditor()->document()->isEmpty()) {
        return;
    }

    currentWrapper()->bottomBar()->updateSize(m_findBar->height() + 8, true);

    if (m_replaceBar->isVisible()) {
        m_replaceBar->hide();
    }
    m_findBar->raise();
    m_findBar->show();
    m_findBar->move(QPoint(4, height() - m_findBar->height() - 4));

    //QString text = wrapper->textEditor()->textCursor().selectedText();
    QString text = wrapper->textEditor()->selectedText();
    int row = wrapper->textEditor()->getCurrentLine();
    int column = wrapper->textEditor()->getCurrentColumn();
    int scrollOffset = wrapper->textEditor()->getScrollOffset();

    m_findBar->activeInput(text, tabPath, row, column, scrollOffset);
    // highlight keyword when findbar show
    wrapper->textEditor()->highlightKeywordInView(text);
    // set keywords
    m_keywordForSearchAll = m_keywordForSearch = text;

    QTimer::singleShot(10, this, [ = ] { m_findBar->focus(); });
}

void Window::popupReplaceBar()
{
    if (currentWrapper() == nullptr) {
        return;
    }

    if (currentWrapper()->textEditor()->document()->isEmpty()) {
        return;
    }

    if (currentWrapper() && currentWrapper()->getFileLoading()) {
        return;
    }

    QTextCursor cursor = currentWrapper()->textEditor()->textCursor();

    m_replaceBar->setsearched(false);
    EditWrapper *curWrapper = currentWrapper();
    bool bIsReadOnly = curWrapper->textEditor()->getReadOnlyMode();

    if (cursor.hasSelection()) {
        currentWrapper()->textEditor()->setCursorStart(cursor.selectionStart());
    }

    if (bIsReadOnly) {
        showNotify(tr("Read-Only mode is on"));
        return;
    }

    currentWrapper()->bottomBar()->updateSize(m_replaceBar->height() + 8, true);

    EditWrapper *wrapper = currentWrapper();
    if (m_findBar->isVisible()) {
        m_findBar->hide();
    }
    m_replaceBar->raise();
    m_replaceBar->show();
    m_replaceBar->move(QPoint(4, height() - m_replaceBar->height() - 4));
    //addBottomWidget(m_replaceBar);

    QString tabPath = m_tabbar->currentPath();
    // QString text = wrapper->textEditor()->textCursor().selectedText();
    QString text = wrapper->textEditor()->selectedText();
    int row = wrapper->textEditor()->getCurrentLine();
    int column = wrapper->textEditor()->getCurrentColumn();
    int scrollOffset = wrapper->textEditor()->getScrollOffset();

    m_replaceBar->activeInput(text, tabPath, row, column, scrollOffset);

    QTimer::singleShot(10, this, [ = ] { m_replaceBar->focus(); });
}

void Window::popupJumpLineBar()
{
    EditWrapper *curWrapper = currentWrapper();

    if (curWrapper == nullptr) {
        return;
    }

    if (curWrapper->textEditor()->document()->isEmpty()) {
        return;
    }

    if (m_jumpLineBar->isVisible()) {
        m_jumpLineBar->hide();
        return;
    }

    if (m_jumpLineBar->isVisible()) {
        if (m_jumpLineBar->isFocus()) {
            //QTimer::singleShot(0, m_wrappers.value(m_tabbar->currentPath())->textEditor(), SLOT(setFocus()));
        } else {
            m_jumpLineBar->focus();
        }
    } else {
        QString tabPath = m_tabbar->currentPath();
        EditWrapper *wrapper = currentWrapper();
        QString text = wrapper->textEditor()->textCursor().selectedText();
        int row = wrapper->textEditor()->getCurrentLine();
        int column = wrapper->textEditor()->getCurrentColumn();
        int count = wrapper->textEditor()->blockCount();
        int scrollOffset = wrapper->textEditor()->getScrollOffset();

        m_jumpLineBar->activeInput(tabPath, row, column, count, scrollOffset);
        m_jumpLineBar->show();
        m_jumpLineBar->focus();
    }
}

void Window::updateJumpLineBar(TextEdit *editor)
{
    // 文本块内容未新增行不更新跳转行号
    if (m_jumpLineBar->isVisible() && editor->blockCount() != m_jumpLineBar->getLineCount()) {
        QString tabPath = m_tabbar->currentPath();
        QString text = editor->textCursor().selectedText();
        int row = editor->getCurrentLine();
        int column = editor->getCurrentColumn();
        int count = editor->blockCount();
        int scrollOffset = editor->getScrollOffset();
        m_jumpLineBar->activeInput(tabPath, row, column, count, scrollOffset);
    }
    if (editor && !editor->ifHasHighlight()) {
        m_findBar->setSearched(false);
        m_replaceBar->setsearched(false);
    }
}

void Window::popupSettingsDialog()
{
    DSettingsDialog *dialog = new DSettingsDialog(this);
    dialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    dialog->widgetFactory()->registerWidget("keySequenceEdit", Settings::createKeySequenceEditHandle);
    dialog->widgetFactory()->registerWidget("savingpathwgt", Settings::createSavingPathWgt);
    dialog->resize(680, 300);
    dialog->setMinimumSize(680, 300);
    m_settings->setSettingDialog(dialog);

    dialog->updateSettings(m_settings->settings);

    dialog->exec();

    delete dialog;
    m_settings->settings->sync();
}

/**
 * @brief 清空打印文档信息，用于关闭打印对话框或中止打印时释放构造的临时文档
 */
void Window::clearPrintTextDocument()
{
    if (m_printDoc != nullptr) {
        if (!m_printDoc->isEmpty()) {
            m_printDoc->clear();
        }
        m_printDoc = nullptr;
    }

    // 清空打印对象列表
    if (!m_printDocList.isEmpty()) {
        for (auto &info : m_printDocList) {
            if (info.highlighter) {
                delete info.highlighter;
            }

            if (info.doc) {
                info.doc->clear();
                delete info.doc;
            }
        }

        m_printDocList.clear();
    }
}

void Window::setWindowTitleInfo()
{
    if (m_tabbar) {
        qDebug() << __FUNCTION__ << "--" << m_tabbar->truePathAt(m_tabbar->currentIndex());
        if (m_tabbar->truePathAt(m_tabbar->currentIndex()) != "") {
            setWindowTitle(m_tabbar->truePathAt(m_tabbar->currentIndex()));
        } else {
            setWindowTitle(m_tabbar->currentName());
        }
    }
}

/**
 * @brief 取得当前文件打开文档目录，新建文档为"系统-文档"目录(~/Documents)
 * @return 当前文件打开文档目录
 */
QString Window::getCurrentOpenFilePath()
{
    QString path;
    EditWrapper *wrapper = currentWrapper();
    if (wrapper) {
        QString curFilePath = wrapper->textEditor() ? wrapper->textEditor()->getTruePath()
                              : wrapper->filePath();
        // 临时文件或备份文件，均返回"文档"目录
        if (Utils::isDraftFile(curFilePath) || Utils::isBackupFile(curFilePath)) {
            path = QDir::homePath() + "/Documents";
        } else {
            path = QFileInfo(curFilePath).absolutePath();
        }
    }

    return path;
}

/**
 * @brief 克隆文本数据，不同于 QTextDocument::clone(), 主要用于大文本的拷贝，拷贝过程通过
 *      QApplication::processEvents() 执行其它事件
 * @param editWrapper 文本编辑处理，提供文本编辑器和高亮信息
 * @return 是否克隆数据成功
 *
 * @note Qt自带的布局算法在超长文本时存在计算越界的问题，计算长度将返回溢出值，导致显示异常，
        调整为拆分的文档结构
 */
bool Window::cloneLargeDocument(EditWrapper *editWrapper)
{
    if (!editWrapper) {
        return false;
    }

    // 获取源文本
    QTextDocument *srcDoc = editWrapper->textEditor()->document();
    // 是否需要高亮
    bool needHighlighter = editWrapper->getSyntaxHighlighter()
                           && editWrapper->getSyntaxHighlighter()->definition().isValid();
    // 创建新的打印信息
    auto createPrintInfo = [ = ]()->PrintInfo {
        PrintInfo info;
        info.doc = createNewDocument(srcDoc);
        if (needHighlighter)
        {
            // 构造高亮处理
            info.highlighter = new CSyntaxHighlighter(info.doc);
            info.highlighter->setDefinition(editWrapper->getSyntaxHighlighter()->definition());
            info.highlighter->setTheme(editWrapper->getSyntaxHighlighter()->theme());
        }
        return info;
    };

    // 每次拷贝约100kb数据
    static const int s_StepCopyCharCount = 1024 * 100;
    // 单个文档字符不超过50MB, 防止布局失败
    static const int s_MaxDocCharCount = 1024 * 1024 * 50;
    // 单个文档文本块数不超过10万行
    static const int s_MaxDocBlockCount = 100000;

    // 创建首个拷贝文本对象
    PrintInfo info = createPrintInfo();
    QTextDocument *copyDoc = info.doc;
    m_printDocList.append(info);

    try {
        // 此次拷贝到字符数统计
        int currentCopyCharCount = 0;
        // 此次拷贝勾选的文本块数
        int currentSelectBlock = 0;
        // 已读取的源文件偏移量
        int srcOffset = 0;

        // 标记开始处理
        m_bPrintProcessing = true;
        QPointer<QObject> checkPtr(this);

        QTextCursor copyCursor(copyDoc);
        QTextCursor srcCursor(srcDoc);

        for (QTextBlock srcBlock = srcDoc->firstBlock(); srcBlock.isValid(); srcBlock = srcBlock.next()) {
            currentSelectBlock++;
            currentCopyCharCount += srcBlock.length();

            if (currentCopyCharCount >= s_StepCopyCharCount) {
                // 判断是否超过单个文档允许的最大范围
                if ((copyDoc->characterCount() + currentCopyCharCount) > s_MaxDocCharCount ||
                    (copyDoc->blockCount() + currentSelectBlock) > s_MaxDocBlockCount) {
                    // 创建新的打印文档
                    m_printDocList.append(createPrintInfo());
                    copyDoc = m_printDocList.last().doc;
                    // 创建新的插入光标
                    copyCursor = QTextCursor(copyDoc);
                }

                srcCursor.setPosition(srcOffset);
                srcCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, currentSelectBlock);
                copyCursor.insertFragment(QTextDocumentFragment(srcCursor));

                srcOffset += currentCopyCharCount;
                currentSelectBlock = 0;
                currentCopyCharCount = 0;
            }

            // 处理其它事件
            qApp->processEvents();
            if (!checkPtr || !m_bPrintProcessing) {
                qWarning() << "Abort print copy!";
                clearPrintTextDocument();
                return false;
            }
        }

        // 补充保存最后的信息
        srcCursor.setPosition(srcOffset);
        srcCursor.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor, currentSelectBlock);
        // 保证拷贝到文档末尾
        srcCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor, 1);
        copyCursor.insertFragment(QTextDocumentFragment(srcCursor));

        m_bPrintProcessing = false;
    } catch (const std::exception &e) {
        qWarning() << "Print copy doc error!" << e.what();
        // 清理数据
        m_bPrintProcessing = false;
        clearPrintTextDocument();
        return false;
    }

    return true;
}

/**
   @brief 使用 \a highlighter 重新高亮传入的文本 \a doc ,此函数用于大文本打印时对临时文本的处理
 */
void Window::rehighlightPrintDoc(QTextDocument *doc, CSyntaxHighlighter *highlighter)
{
    if (!doc || !highlighter) {
        return;
    }

    QColor background = m_printWrapper->textEditor()->palette().color(QPalette::Base);
    bool backgroundIsDark = background.value() < 128;

    highlighter->setEnableHighlight(true);
    if (backgroundIsDark) {
        for (QTextBlock block = doc->firstBlock(); block.isValid(); block = block.next()) {
            highlighter->rehighlightBlock(block);

            QVector<QTextLayout::FormatRange> formatList = block.layout()->formats();
            // adjust syntax highlighting colors for better contrast
            for (int i = formatList.count() - 1; i >= 0; --i) {
                QTextCharFormat &format = formatList[i].format;
                if (format.background().color() == background) {
                    QBrush brush = format.foreground();
                    QColor color = brush.color();
                    int h, s, v, a;
                    color.getHsv(&h, &s, &v, &a);
                    color.setHsv(h, s, qMin(128, v), a);
                    brush.setColor(color);
                    format.setForeground(brush);
                }
                format.setBackground(Qt::white);
            }

            block.layout()->setFormats(formatList);
        }
    } else {
        highlighter->rehighlight();
    }
    highlighter->setEnableHighlight(false);
}

/**
   @brief 接收布局模式变更信号 DGuiApplicationHelper::sizeModeChanged() ，更新界面布局
        Window 接收布局模式变更，调整 findBar 和 replaceBar 的坐标位置。
   @note 需要在 findBar / replaceBar / bottomBar 更新后触发更新
 */
void Window::updateSizeMode()
{
    if (m_findBar && m_findBar->isVisible()) {
        currentWrapper()->bottomBar()->updateSize(m_findBar->height() + 8, true);
        m_findBar->move(QPoint(4, height() - m_findBar->height() - 4));
    }

    if (m_replaceBar && m_replaceBar->isVisible()) {
        currentWrapper()->bottomBar()->updateSize(m_replaceBar->height() + 8, true);
        m_replaceBar->move(QPoint(4, height() - m_replaceBar->height() - 4));
    }
}

void Window::popupPrintDialog()
{
    //大文本加载过程不允许打印操作
    if (currentWrapper() && currentWrapper()->getFileLoading()) return;

    // 已有处理的打印事件，不继续进入
    if (m_bPrintProcessing) {
        return;
    }

    const QString &filePath = currentWrapper()->textEditor()->getFilePath();
    const QString &fileDir = QFileInfo(filePath).dir().absolutePath();

    qInfo() << qPrintable("Start print doc");

    //适配打印接口2.0，dtk版本大于或等于5.4.10才放开最新的2.0打印预览接口
#if (DTK_VERSION_MAJOR > 5 \
    || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR > 4) \
    || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR == 4 && DTK_VERSION_PATCH >= 10))

    QTextDocument *doc = currentWrapper()->textEditor()->document();
    m_bLargePrint = false;
    m_bPrintProcessing = false;
    m_printWrapper = currentWrapper();
    m_multiDocPageCount = 0;

    if (doc != nullptr && !doc->isEmpty()) {
        static const int s_maxDirectReadLen = 1024 * 1024 * 10;
        static const int s_maxHighlighterDirectReadLen = 1024 * 1024 * 5;
        // 判断是否需要文本高亮，文本数据量大小，不同数据量使用不同分支。
        bool needHighlighter = currentWrapper()->getSyntaxHighlighter()
                               && currentWrapper()->getSyntaxHighlighter()->definition().isValid();

        if (needHighlighter
                && doc->characterCount() > s_maxHighlighterDirectReadLen) {
            // 需要高亮且数据量超过 5MB
            m_bLargePrint = true;
        } else if (!needHighlighter
                   && doc->characterCount() > s_maxDirectReadLen) {
            // 无需高亮且数据量超过 10MB
            m_bLargePrint = true;
        } else {
            currentWrapper()->updateHighlighterAll();
            m_printDoc = doc->clone(doc);
        }

        // 大文件处理
        if (m_bLargePrint) {
            qInfo() << qPrintable("Clone large print document");
            // 克隆大文本数据
            if (!cloneLargeDocument(currentWrapper())) {
                return;
            }
        }
    }

    if ((!m_bLargePrint && nullptr == m_printDoc)
            || (m_bLargePrint && m_printDocList.isEmpty())) {
        qWarning() << "The print document is not valid!";
        return;
    }

    DPrinter printer(QPrinter::HighResolution);
    m_isNewPrint = true;
    m_pPreview = new DPrintPreviewDialog(this);
    m_pPreview->setAttribute(Qt::WA_DeleteOnClose);

    // 设置 QPrinter 的文档名称，保留绝对路径和文件后缀(在cups的page_log中保留完整的job-name)
    // 注意和文件的输出文件路径进行区分
    if (fileDir == m_blankFileDir) {
        m_pPreview->setDocName(filePath);
    } else {
        QString path = currentWrapper()->textEditor()->getTruePath();
        m_pPreview->setDocName(path);
    }

    // 后续布局计算后再更新打印页数
    m_pPreview->setAsynPreview(PRINT_FLAG);

    connect(m_pPreview, &DPrintPreviewDialog::finished, this, [ = ] {
        m_bPrintProcessing = false;
        clearPrintTextDocument();
    });
    connect(m_pPreview, &DPrintPreviewDialog::rejected, this, [ = ] {
        m_bPrintProcessing = false;
        clearPrintTextDocument();
    });

    connect(m_pPreview, QOverload<DPrinter *, const QVector<int> &>::of(&DPrintPreviewDialog::paintRequested),
    this, [ = ](DPrinter * _printer, const QVector<int> &pageRange) {
        if (m_bLargePrint) {
            this->doPrintWithLargeDoc(_printer, pageRange);
        } else {
            this->doPrint(_printer, pageRange);
        }
    });

    m_pPreview->exec();

#else
    DPrintPreviewDialog preview(this);

    connect(&preview, QOverload<DPrinter *>::of(&DPrintPreviewDialog::paintRequested),
    this, [ & ](DPrinter * printer) {
        if (fileDir == m_blankFileDir) {
            printer->setDocName(QString(m_tabbar->currentName()));
        } else {
            printer->setDocName(QString(QFileInfo(filePath).baseName()));
        }
        currentWrapper()->textEditor()->print(printer);
    });

    currentWrapper()->updateHighlighterAll();

    preview.exec();
#endif
}

void Window::popupThemePanel()
{
    updateThemePanelGeomerty();
    m_themePanel->setSelectionTheme(m_themePath);
    m_themePanel->popup();
}

void Window::toggleFullscreen()
{
    if (!window()->windowState().testFlag(Qt::WindowFullScreen)) {
        window()->setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        window()->setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

void Window::remberPositionSave()
{
    EditWrapper *wrapper = currentWrapper();

    m_remberPositionFilePath = m_tabbar->currentPath();
    m_remberPositionRow = wrapper->textEditor()->getCurrentLine();
    m_remberPositionColumn = wrapper->textEditor()->getCurrentColumn();
    m_remberPositionScrollOffset = wrapper->textEditor()->getScrollOffset();

    currentWrapper()->showNotify(tr("Current location remembered"));
}

void Window::remberPositionRestore()
{
    if (m_remberPositionFilePath.isEmpty()) {
        return;
    }

    if (m_wrappers.contains(m_remberPositionFilePath)) {
        const QString &filePath = m_remberPositionFilePath;
        const int &scrollOffset = m_remberPositionScrollOffset;
        const int &row = m_remberPositionRow;
        const int &column = m_remberPositionColumn;

        activeTab(m_tabbar->indexOf(m_remberPositionFilePath));
        m_wrappers.value(filePath)->textEditor()->scrollToLine(scrollOffset, row, column);
    }
}


void Window::displayShortcuts()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);
    // 获取当前焦点位置（光标所在屏幕中心）
    QScreen *screen = nullptr;
    if (DGuiApplicationHelper::isTabletEnvironment()) {
        // bug 88079 避免屏幕旋转弹出位置错误
        screen = qApp->primaryScreen();
    } else {
        screen = QGuiApplication::screenAt(QCursor::pos());
    }

    if (screen) {
        pos = screen->geometry().center();
    }

    //窗体快捷键
    QStringList windowKeymaps;
    windowKeymaps << "addblanktab" << "newwindow" << "savefile"
                  << "saveasfile" << "selectnexttab" << "selectprevtab"
                  << "closetab" << "closeothertabs" << "restoretab"
                  << "openfile" << "incrementfontsize" << "decrementfontsize"
                  << "resetfontsize" << "togglefullscreen" << "find" << "replace"
                  << "jumptoline" << "saveposition" << "restoreposition"
                  << "escape" << "print";

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    QJsonObject windowJsonGroup;
    windowJsonGroup.insert("groupName", QObject::tr("Window"));
    QJsonArray windowJsonItems;

    for (const QString &keymap : windowKeymaps) {
        auto option = m_settings->settings->group("shortcuts.window")->option(QString("shortcuts.window.%1").arg(keymap));
        QJsonObject jsonItem;
        jsonItem.insert("name", QObject::tr(option->name().toUtf8().data()));
        if (keymap != "incrementfontsize" && keymap != "decrementfontsize") {
            jsonItem.insert("value", option->value().toString().replace("Meta", "Super"));
        } else if (keymap == "incrementfontsize") {
            QString strIncrementfontValue = QString(tr("Ctrl+'='"));
            jsonItem.insert("value", strIncrementfontValue.replace("Meta", "Super"));
        } else if (keymap == "decrementfontsize" && option->value().toString() == "Ctrl+-") {
            QString strDecrementfontValue = QString(tr("Ctrl+'-'"));
            jsonItem.insert("value", strDecrementfontValue.replace("Meta", "Super"));
        }

        windowJsonItems.append(jsonItem);
    }

    windowJsonGroup.insert("groupItems", windowJsonItems);
    jsonGroups.append(windowJsonGroup);

    //编辑快捷键
    QStringList editorKeymaps;
    editorKeymaps << "indentline" << "backindentline" << "forwardchar"
                  << "backwardchar" << "forwardword" << "backwardword"
                  << "nextline" << "prevline" << "newline" << "opennewlineabove"
                  << "opennewlinebelow" << "duplicateline" << "killline"
                  << "killcurrentline" << "swaplineup" << "swaplinedown"
                  << "scrolllineup" << "scrolllinedown" << "scrollup"
                  << "scrolldown" << "movetoendofline" << "movetostartofline"
                  << "movetoend" << "movetostart" << "movetolineindentation"
                  << "upcaseword" << "downcaseword" << "capitalizeword"
                  << "killbackwardword" << "killforwardword" << "forwardpair"
                  << "backwardpair" << "selectall" << "copy" << "cut"
                  << "paste" << "transposechar" << "setmark" << "exchangemark"
                  << "copylines" << "cutlines" << "joinlines" << "togglereadonlymode"
                  << "togglecomment" << "removecomment" << "undo" << "redo" << "switchbookmark" << "movetoprebookmark"
                  << "movetonextbookmark" << "mark";

    QJsonObject editorJsonGroup;
    editorJsonGroup.insert("groupName", tr("Editor"));
    QJsonArray editorJsonItems;

    for (const QString &keymap : editorKeymaps) {
        auto option = m_settings->settings->group("shortcuts.editor")->option(QString("shortcuts.editor.%1").arg(keymap));
        QJsonObject jsonItem;
        jsonItem.insert("name", QObject::tr(option->name().toUtf8().data()));
        jsonItem.insert("value", option->value().toString().replace("Meta", "Super"));
        editorJsonItems.append(jsonItem);
    }
    editorJsonGroup.insert("groupItems", editorJsonItems);
    jsonGroups.append(editorJsonGroup);

    //设置快捷键
    QStringList setupKeymaps;
    setupKeymaps << "help" << "displayshortcuts";

    QJsonObject setupJsonGroup;
    setupJsonGroup.insert("groupName", tr("Settings"));
    QJsonArray setupJsonItems;

    for (const QString &keymap : setupKeymaps) {
        auto option = m_settings->settings->group("shortcuts.window")->option(QString("shortcuts.window.%1").arg(keymap));
        QJsonObject jsonItem;
        jsonItem.insert("name", QObject::tr(option->name().toUtf8().data()));
        jsonItem.insert("value", option->value().toString().replace("Meta", "Super"));
        setupJsonItems.append(jsonItem);
    }
    setupJsonGroup.insert("groupItems", setupJsonItems);
    jsonGroups.append(setupJsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    m_shortcutViewProcess = new QProcess();
    m_shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(m_shortcutViewProcess, SIGNAL(finished(int)), m_shortcutViewProcess, SLOT(deleteLater()));
}

/*!
 * \~chinese \brief Window::doPrint 调用打印预览，将文本分块输出给打印机
 * \~chinese \param pageRange 打印预览请求的页码列表
 */
#if (DTK_VERSION_MAJOR > 5 \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR > 4) \
 || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR == 4 && DTK_VERSION_PATCH >= 10))
void Window::doPrint(DPrinter *printer, const QVector<int> &pageRange)
{
    if (nullptr == m_printDoc) {
        return;
    }

    //如果打印预览请求的页码为空，则直接返回
    if (pageRange.isEmpty()) {
        return;
    }

    QPainter p(printer);
    if (!p.isActive())
        return;

    if (m_lastLayout.isValid() && !m_isNewPrint) {
        if (m_lastLayout == printer->pageLayout()) {
            // 如果打印属性没发生变化，直接加载已生成的资源，提高切换速度
            asynPrint(p, printer, pageRange);
            return;
        }
    }

    //保留print的打印布局
    m_lastLayout = printer->pageLayout();
    m_isNewPrint = false;

    QTextOption opt = m_printDoc->defaultTextOption();
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_printDoc->setDefaultTextOption(opt);

    (void)m_printDoc->documentLayout(); // make sure that there is a layout

    if (currentWrapper() == nullptr) {
        return;
    }
    QColor background = currentWrapper()->textEditor()->palette().color(QPalette::Base);
    bool backgroundIsDark = background.value() < 128;
    //对文本进行分页处理
    for (QTextBlock srcBlock = currentWrapper()->textEditor()->document()->firstBlock(), dstBlock = m_printDoc->firstBlock();
            srcBlock.isValid() && dstBlock.isValid();
            srcBlock = srcBlock.next(), dstBlock = dstBlock.next()) {
        QVector<QTextLayout::FormatRange> formatList = srcBlock.layout()->formats();
        if (backgroundIsDark) {
            // adjust syntax highlighting colors for better contrast
            for (int i = formatList.count() - 1; i >= 0; --i) {
                QTextCharFormat &format = formatList[i].format;
                if (format.background().color() == background) {
                    QBrush brush = format.foreground();
                    QColor color = brush.color();
                    int h, s, v, a;
                    color.getHsv(&h, &s, &v, &a);
                    color.setHsv(h, s, qMin(128, v), a);
                    brush.setColor(color);
                    format.setForeground(brush);
                }
                format.setBackground(Qt::white);
            }
        }

        dstBlock.layout()->setFormats(formatList);
    }

    QAbstractTextDocumentLayout *layout = m_printDoc->documentLayout();
    layout->setPaintDevice(p.device());

    int dpiy = p.device()->logicalDpiY();
    int margin = static_cast<int>((2 / 2.54) * dpiy); // 2 cm margins

    auto fmt = m_printDoc->rootFrame()->frameFormat();
    fmt.setMargin(margin);
    m_printDoc->rootFrame()->setFrameFormat(fmt);

    QRectF pageRect(printer->pageRect());
    QRectF body = QRectF(0, 0, pageRect.width(), pageRect.height());
    QFontMetrics fontMetrics(m_printDoc->defaultFont(), p.device());
    QRectF titleBox(margin,
                    body.bottom() - margin
                    + fontMetrics.height()
                    - 6 * dpiy / 72.0,
                    body.width() - 2 * margin,
                    fontMetrics.height());
    m_printDoc->setPageSize(body.size());
    //输出总页码给到打印预览
    m_pPreview->setAsynPreview(m_printDoc->pageCount());

    //渲染第一页文本
    for (int i = 0; i < pageRange.count(); ++i) {
        if (pageRange[i] > m_printDoc->pageCount())
            continue;
        Window::printPage(pageRange[i], &p, m_printDoc, body, titleBox);
        if (i != pageRange.count() - 1)
            printer->newPage();
    }
}

/**
 * @brief 用于较大的文本文件进行打印计算和绘制，使用 processEvents 规避打印输入
 * @param printer       打印指针
 * @param pageRange     打印范围
 */
void Window::doPrintWithLargeDoc(DPrinter *printer, const QVector<int> &pageRange)
{
    if (m_bPrintProcessing) {
        return;
    }

    if (m_printDocList.isEmpty()) {
        return;
    }

    //如果打印预览请求的页码为空，则直接返回
    if (pageRange.isEmpty()) {
        return;
    }

    QPainter p(printer);
    if (!p.isActive())
        return;

    if (m_lastLayout.isValid() && !m_isNewPrint) {
        if (m_lastLayout == printer->pageLayout()) {
            // 如果打印属性没发生变化，直接加载已生成的资源，提高切换速度
            asynPrint(p, printer, pageRange);
            return;
        }
    }

    //保留print的打印布局
    m_lastLayout = printer->pageLayout();
    m_isNewPrint = false;

    if (currentWrapper() == nullptr) {
        return;
    }

    qInfo() << qPrintable("Calc print large doc!");

    // 执行处理时屏蔽其它输入
    DPrintPreviewWidget *prieviewWidget = m_pPreview->findChild<DPrintPreviewWidget *>();
    QStringList findWidgetList {"leftWidget", "rightWidget"};
    QWidgetList widList;
    if (prieviewWidget) {
        // 通过设置刷新标识，防止频繁的触发刷新
        prieviewWidget->refreshBegin();

        for (QString widStr : findWidgetList) {
            QWidget *wid = m_pPreview->findChild<QWidget *>(widStr);
            if (wid) {
                wid->setEnabled(false);
                widList.append(wid);
            } else {
                qWarning() << "not find widget";
            }
        }

        QApplication::processEvents();
    }

    m_bPrintProcessing = true;
    QPointer<QObject> checkPtr(this);

    int dpiy = p.device()->logicalDpiY();
    int margin = static_cast<int>((2 / 2.54) * dpiy); // 2 cm margins

    QRectF pageRect(printer->pageRect());
    QRectF body = QRectF(0, 0, pageRect.width(), pageRect.height());
    // 使用首个文档信息即可
    QFontMetrics fontMetrics(m_printDocList.first().doc->defaultFont(), p.device());
    QRectF titleBox(margin,
                    body.bottom() - margin
                    + fontMetrics.height()
                    - 6 * dpiy / 72.0,
                    body.width() - 2 * margin,
                    fontMetrics.height());

    // Note:大文本打印的高亮处理被延迟到实际打印时计算
    m_multiDocPageCount = 0;
    for (auto &info : m_printDocList) {
        QTextDocument *printDoc = info.doc;

        QAbstractTextDocumentLayout *lay = info.doc->documentLayout();
        lay->setPaintDevice(p.device());

        auto fmt = printDoc->rootFrame()->frameFormat();
        fmt.setMargin(margin);
        printDoc->rootFrame()->setFrameFormat(fmt);

        // 设置打印大小
        printDoc->setPageSize(body.size());

        for (QTextBlock block = printDoc->firstBlock(); block.isValid(); block = block.next()) {
            // 通过对单个文本块进行布局，拆分布局总时间，防止布局时间过长
            lay->blockBoundingRect(block);
            // 处理其它事件
            qApp->processEvents(QEventLoop::AllEvents | QEventLoop::DialogExec);
            // 判断当前窗口是否关闭，退出打印状态
            if (!checkPtr || !m_bPrintProcessing) {
                qWarning() << "Abort print layout!";

                // 手动清理数据
                QObject::disconnect(m_pPreview, nullptr, this, nullptr);
                clearPrintTextDocument();

                // 异常退出、中断打印时需要关闭窗口
                QMetaObject::invokeMethod(m_pPreview, "reject", Qt::QueuedConnection);
                return;
            }
        }

        // 更新文件总打印页数
        m_multiDocPageCount += printDoc->pageCount();
    }

    if (prieviewWidget) {
        prieviewWidget->refreshEnd();
    }

    m_bPrintProcessing = false;
    for (auto wid : widList) {
        wid->setEnabled(true);
    }

    //输出总页码给到打印预览
    m_pPreview->setAsynPreview(m_multiDocPageCount);

    if (!m_printDocList.isEmpty()) {
        //渲染第一页文本
        for (int i = 0; i < pageRange.count(); ++i) {
            if (pageRange[i] > m_multiDocPageCount)
                continue;
            printPageWithMultiDoc(pageRange[i], &p, m_printDocList, body, titleBox);
            if (i != pageRange.count() - 1)
                printer->newPage();
        }
    }

    qInfo() << qPrintable("Calc print large doc finised!");
}
#endif

/*!
 * \~chinese \brief Window::asynPrint 进行翻页预览打印
 * \~chinese \param pageRange 打印预览请求的页码列表
 */
void Window::asynPrint(QPainter &p, DPrinter *printer, const QVector<int> &pageRange)
{
    if ((!m_bLargePrint && nullptr == m_printDoc)
            || (m_bLargePrint && m_printDocList.isEmpty())) {
        qWarning() << "The print document is not valid!";
        return;
    }

    QRectF pageRect(printer->pageRect());
    int dpiy = p.device()->logicalDpiY();
    int margin = static_cast<int>((2 / 2.54) * dpiy); // 2 cm margins
    QRectF body = QRectF(0, 0, pageRect.width(), pageRect.height());

    QTextDocument *curDoc = m_bLargePrint ? m_printDocList.first().doc : m_printDoc;
    QFontMetrics fontMetrics(curDoc->defaultFont(), p.device());
    QRectF titleBox(margin,
                    body.bottom() - margin
                    + fontMetrics.height()
                    - 6 * dpiy / 72.0,
                    body.width() - 2 * margin,
                    fontMetrics.height());

    if (m_bLargePrint
            && !m_printDocList.isEmpty()) {
        // 大文本打印
        for (int i = 0; i < pageRange.count(); ++i) {
            if (pageRange[i] > m_multiDocPageCount)
                continue;
            printPageWithMultiDoc(pageRange[i], &p, m_printDocList, body, titleBox);
            if (i != pageRange.count() - 1)
                printer->newPage();
        }
    } else {
        for (int i = 0; i < pageRange.count(); ++i) {
            if (pageRange[i] > m_printDoc->pageCount())
                continue;
            Window::printPage(pageRange[i], &p, m_printDoc, body, titleBox);
            if (i != pageRange.count() - 1)
                printer->newPage();
        }
    }
}

void Window::backupFile()
{
    if (!QFileInfo(m_backupDir).exists()) {
        QDir().mkpath(m_backupDir);
    }

    QMap<QString, EditWrapper *> wrappers = m_wrappers;
    QStringList listBackupInfo;
    QString filePath, localPath, curPos;
    QFileInfo fileInfo;
    m_qlistTemFile.clear();
    m_qlistTemFile = wrappers.keys();

    for (EditWrapper *wrapper : wrappers) {
        if (nullptr == wrapper) {
            continue;
        }
        if (wrapper->getFileLoading()) continue;

        if (nullptr == wrapper->textEditor()) {
            continue;
        }

        filePath = wrapper->textEditor()->getFilePath();
        localPath = wrapper->textEditor()->getTruePath();

        if (localPath.isEmpty()) {
            localPath = wrapper->textEditor()->getFilePath();
        }

        qInfo() << "begin backupFile()";
        StartManager::FileTabInfo tabInfo = StartManager::instance()->getFileTabInfo(filePath);
        curPos = QString::number(wrapper->textEditor()->textCursor().position());
        fileInfo.setFile(localPath);

        QJsonObject jsonObject;
        QJsonDocument document;
        jsonObject.insert("localPath", localPath);
        jsonObject.insert("cursorPosition", QString::number(wrapper->textEditor()->textCursor().position()));
        jsonObject.insert("modify", wrapper->isModified());
        jsonObject.insert("lastModifiedTime", wrapper->getLastModifiedTime().toString());
        QList<int> bookmarkList = wrapper->textEditor()->getBookmarkInfo();
        if (!bookmarkList.isEmpty()) {
            QString bookmarkInfo;

            //记录书签
            for (int i = 0; i < bookmarkList.count(); i++) {
                if (i == bookmarkList.count() - 1) {
                    bookmarkInfo.append(QString::number(bookmarkList.value(i)));
                } else {
                    bookmarkInfo.append(QString::number(bookmarkList.value(i)) + ",");
                }
            }

            jsonObject.insert("bookMark", bookmarkInfo);
        }

        //记录活动页
        if (filePath == m_tabbar->currentPath()) {
            jsonObject.insert("focus", true);
        }

        //保存备份文件
        if (Utils::isDraftFile(filePath)) {
            wrapper->saveTemFile(filePath);
        } else {
            if (wrapper->isModified()) {
                QString name = fileInfo.absolutePath().replace("/", "_");
                QString qstrFilePath = m_backupDir + "/" + Utils::getStringMD5Hash(fileInfo.baseName()) + "." + name + "." + fileInfo.suffix();
                jsonObject.insert("temFilePath", qstrFilePath);
                wrapper->saveTemFile(qstrFilePath);
            }
        }

        qInfo() << "after call wrapper->saveTemFile()";

        //使用json串形式保存
        document.setObject(jsonObject);
        QByteArray byteArray = document.toJson(QJsonDocument::Compact);
        m_qlistTemFile.replace(tabInfo.tabIndex, byteArray);
    }

    //将json串列表写入配置文件
    m_settings->settings->option("advance.editor.browsing_history_temfile")->setValue(m_qlistTemFile);

    //删除自动备份文件
    if (QFileInfo(m_autoBackupDir).exists()) {
        QDir(m_autoBackupDir).removeRecursively();
    }

    qInfo() << "end backupFile()";
}

bool Window::closeAllFiles()
{
    qInfo() << "begin closeAllFiles()";
    QMap<QString, EditWrapper *> wrappers = m_wrappers;

    // 被删除的窗口索引已变更，需要计算其范围
    int closedTabCount = 0;
    //关闭所有文件
    for (int i = 0; i < wrappers.count(); i++) {
        // 窗口索引 - 已删除窗口索引
        m_tabbar->setCurrentIndex(i - closedTabCount);

        if (!closeTab()) {
            // 取消操作时，停留在当前标签页
            return false;
        } else {
            // 窗口已删除，调整索引
            ++closedTabCount;
        }
    }
    qInfo() << "end closeAllFiles()";
    return true;
}

/**
 * @brief addTemFileTab　恢复备份文件标签页
 * @param qstrPath　打开文件路径
 * @param qstrName　真实文件名
 * @param qstrTruePath　真实文件路径
 * @param qstrTruePath　最后一次修改时间
 * @param bIsTemFile　是否修改
 */
void Window::addTemFileTab(const QString &qstrPath, const QString &qstrName, const QString &qstrTruePath, const QString &lastModifiedTime, bool bIsTemFile)
{
    if (qstrPath.isEmpty() || !Utils::fileExists(qstrPath)) {
        return;
    }

    // 若为MIMETYPE不支持的文件，给出无效文件提示 修复bug：https://pms.uniontech.com/bug-view-132653.html
    if (!Utils::isMimeTypeSupport(qstrPath)) {
        if (currentWrapper() == nullptr) {
            this->addBlankTab();
        }

        QString strFileName = QFileInfo(qstrPath).fileName();
        strFileName = tr("Invalid file: %1").arg(strFileName);
        strFileName = Utils::lineFeed(strFileName, m_editorWidget->currentWidget()->width() - FLOATTIP_MARGIN, m_editorWidget->currentWidget()->font(), 2);
        DMessageManager::instance()->sendMessage(m_editorWidget->currentWidget(), QIcon(":/images/warning.svg"), strFileName);

        return;
    }

    EditWrapper *wrapper = createEditor();
    m_tabbar->addTab(qstrPath, qstrName, qstrTruePath);
    wrapper->openFile(qstrPath, qstrTruePath, bIsTemFile);

    // 查找文件是否存在书签，临时文件同样可标记书签
    auto bookmarkInfo = StartManager::instance()->findBookmark(qstrTruePath);
    wrapper->textEditor()->setBookMarkList(bookmarkInfo);

    //set m_tModifiedDateTime in wrapper again.
    if (bIsTemFile && !lastModifiedTime.isEmpty()) {
        wrapper->setLastModifiedTime(lastModifiedTime);
    }
    m_wrappers[qstrPath] = wrapper;
    showNewEditor(wrapper);
}

QMap<QString, EditWrapper *> Window::getWrappers()
{
    return m_wrappers;
}

void Window::setChildrenFocus(bool ok)
{
    if (ok) {
        DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
        DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
        DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
        DIconButton *quitFullBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
        DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
        DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");

        titlebar()->setFocusPolicy(Qt::TabFocus);
        if (addButton) addButton->setFocusPolicy(Qt::TabFocus);
        if (optionBtn) optionBtn->setFocusPolicy(Qt::TabFocus);
        if (minBtn) minBtn->setFocusPolicy(Qt::TabFocus);
        if (quitFullBtn) quitFullBtn->setFocusPolicy(Qt::TabFocus);
        if (maxBtn) maxBtn->setFocusPolicy(Qt::TabFocus);
        if (closeBtn) closeBtn->setFocusPolicy(Qt::TabFocus);

        setTabOrder(addButton, optionBtn);
        setTabOrder(optionBtn, minBtn);
        setTabOrder(minBtn, quitFullBtn);
        setTabOrder(quitFullBtn, maxBtn);
        setTabOrder(maxBtn, closeBtn);
    } else {
        DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
        DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
        DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
        DIconButton *quitFullBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
        DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
        DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");

        titlebar()->setFocusPolicy(Qt::NoFocus);
        if (addButton) addButton->setFocusPolicy(Qt::NoFocus);
        if (optionBtn) optionBtn->setFocusPolicy(Qt::NoFocus);
        if (minBtn) minBtn->setFocusPolicy(Qt::NoFocus);
        if (quitFullBtn) quitFullBtn->setFocusPolicy(Qt::NoFocus);
        if (maxBtn) maxBtn->setFocusPolicy(Qt::NoFocus);
        if (closeBtn) closeBtn->setFocusPolicy(Qt::NoFocus);
    }
}

void Window::addBlankTab()
{
    addBlankTab("");
}

void Window::addBlankTab(const QString &blankFile)
{
    QString blankTabPath;

    if (blankFile.isEmpty()) {
        const QString &fileName = QString("blank_file_%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss-zzz"));
        blankTabPath = QDir(m_blankFileDir).filePath(fileName);

        if (!Utils::fileExists(blankTabPath)) {
            QDir().mkpath(m_blankFileDir);
        }
    } else {
        blankTabPath = blankFile;
    }

    int blankFileIndex = getBlankFileIndex();
    m_tabbar->addTab(blankTabPath, tr("Untitled %1").arg(blankFileIndex));
    EditWrapper *wrapper = createEditor();
    wrapper->updatePath(blankTabPath);

    if (!blankFile.isEmpty() && Utils::fileExists(blankFile)) {
        wrapper->openFile(blankTabPath, blankTabPath);
    }

    m_wrappers[blankTabPath] = wrapper;
    showNewEditor(wrapper);
}

/**
 * @brief 标签页关闭请求处理
 * @param index 标签页索引
 *
 * @note 由于使用 Qt::QueuedConnection 模式，在界面被其它任务阻塞时，可能触发多次关闭事件,
 *      导致队列中存在多个请求关闭任务，阻塞结束后，重复触发，删除所有的标签页，导致窗口关闭。
 */
void Window::handleTabCloseRequested(int index)
{
    // 延迟执行关闭标签页
    if (!m_delayCloseTabTimer.isActive()) {
        // 记录关闭标签索引
        m_requestCloseTabIndex = index;

        // 延迟10ms
        static const int s_nDelayTime = 10;
        m_delayCloseTabTimer.start(s_nDelayTime, this);
    }
}

void Window::handleTabsClosed(QStringList tabList)
{
    if (tabList.isEmpty()) {
        return;
    }

    for (const QString &path : tabList) {
        int index = m_tabbar->indexOf(path);
        m_tabbar->setCurrentIndex(index);
        closeTab();
    }
}

void Window::handleCurrentChanged(const int &index)
{
    if (m_findBar->isVisible()) {
        m_findBar->hide();
    }

    if (m_replaceBar->isVisible()) {
        m_replaceBar->hide();
    }

    if (m_jumpLineBar->isVisible()) {
        m_jumpLineBar->hide();
    }

    for (auto wrapper : m_wrappers.values()) {
        wrapper->textEditor()->removeKeywords();
    }

    if (currentWrapper()) {
        currentWrapper()->checkForReload();
        checkTabbarForReload();
    }

    const QString &filepath = m_tabbar->fileAt(index);

    if (m_wrappers.contains(filepath)) {
        bool bIsContains = false;
        EditWrapper *wrapper = m_wrappers.value(filepath);
        wrapper->textEditor()->setFocus();
        for (int i = 0; i < m_editorWidget->count(); i++) {
            if (m_editorWidget->widget(i) == wrapper) {
                bIsContains = true;
            }
        }

        if (!bIsContains) {
            m_editorWidget->addWidget(wrapper);
        }

        m_editorWidget->setCurrentWidget(wrapper);
    }

    if (currentWrapper() != nullptr) {
        currentWrapper()->bottomBar()->show();
        currentWrapper()->bottomBar()->updateSize(BottomBar::defaultHeight(), false);
    }
}

void Window::handleJumpLineBarExit()
{
    if (currentWrapper() != nullptr) {
        QTimer::singleShot(0, currentWrapper()->textEditor(), SLOT(setFocus()));
    }
}

void Window::handleJumpLineBarJumpToLine(const QString &filepath, int line, bool focusEditor)
{
    if (m_wrappers.contains(filepath)) {
        getTextEditor(filepath)->jumpToLine(line, true);

        if (focusEditor) {
            QTimer::singleShot(0, getTextEditor(filepath), SLOT(setFocus()));
        }
    }
}

void Window::handleBackToPosition(const QString &file, int row, int column, int scrollOffset)
{
    if (m_wrappers.contains(file)) {
        m_wrappers.value(file)->textEditor()->scrollToLine(scrollOffset, row, column);
    }
}

void Window::handleFindNextSearchKeyword(const QString &keyword)
{
    handleFindKeyword(keyword, true);
}

void Window::handleFindPrevSearchKeyword(const QString &keyword)
{
    handleFindKeyword(keyword, false);
}

void Window::handleFindKeyword(const QString &keyword, bool state)
{
    EditWrapper *wrapper = currentWrapper();
    m_keywordForSearch = keyword;
    wrapper->textEditor()->saveMarkStatus();
    wrapper->textEditor()->updateCursorKeywordSelection(m_keywordForSearch, state);
    if (QString::compare(m_keywordForSearch, m_keywordForSearchAll, Qt::CaseInsensitive) != 0) {
        m_keywordForSearchAll.clear();
        wrapper->textEditor()->clearFindMatchSelections();
    } else {
        wrapper->textEditor()->highlightKeywordInView(m_keywordForSearchAll);
    }

    wrapper->textEditor()->markAllKeywordInView();

    wrapper->textEditor()->renderAllSelections();
    wrapper->textEditor()->restoreMarkStatus();
    wrapper->textEditor()->updateLeftAreaWidget();

    // 变更查询字符串位置后(可能滚屏)，刷新当前界面的代码高亮效果
    wrapper->OnUpdateHighlighter();
}

void Window::slotFindbarClose()
{
    EditWrapper *wrapper = currentWrapper();

    if (wrapper->bottomBar()->isHidden()) {
        wrapper->bottomBar()->show();
    }

    wrapper->bottomBar()->updateSize(BottomBar::defaultHeight(), false);
    currentWrapper()->textEditor()->setFocus();
    currentWrapper()->textEditor()->tellFindBarClose();
}

void Window::slotReplacebarClose()
{
    EditWrapper *wrapper = currentWrapper();

    if (wrapper->bottomBar()->isHidden()) {
        wrapper->bottomBar()->show();
    }

    wrapper->bottomBar()->updateSize(BottomBar::defaultHeight(), false);
    currentWrapper()->textEditor()->setFocus();
    currentWrapper()->textEditor()->tellFindBarClose();
}

void Window::handleReplaceAll(const QString &replaceText, const QString &withText)
{
    EditWrapper *wrapper = currentWrapper();
    wrapper->textEditor()->replaceAll(replaceText, withText);
}

void Window::handleReplaceNext(const QString &file, const QString &replaceText, const QString &withText)
{
    Q_UNUSED(file);
    m_keywordForSearch = replaceText;
    m_keywordForSearchAll = replaceText;
    EditWrapper *wrapper = currentWrapper();
    wrapper->textEditor()->replaceNext(replaceText, withText);
}

void Window::handleReplaceRest(const QString &replaceText, const QString &withText)
{
    EditWrapper *wrapper = currentWrapper();
    wrapper->textEditor()->replaceRest(replaceText, withText);
}

void Window::handleReplaceSkip(QString file, QString keyword)
{
    EditWrapper *wrapper = currentWrapper();
    handleUpdateSearchKeyword(m_replaceBar, file, keyword);
    if (QString::compare(m_keywordForSearch, m_keywordForSearchAll, Qt::CaseInsensitive) != 0) {
        m_keywordForSearchAll.clear();
        wrapper->textEditor()->clearFindMatchSelections();
    } else {
        wrapper->textEditor()->highlightKeywordInView(m_keywordForSearchAll);
    }
#if 0
    EditWrapper *wrapper = currentWrapper();
    wrapper->textEditor()->updateCursorKeywordSelection(m_keywordForSearchAll, true);
    wrapper->textEditor()->renderAllSelections();
#endif
}

void Window::handleRemoveSearchKeyword()
{
    if (currentWrapper() != nullptr) {
        currentWrapper()->textEditor()->removeKeywords();
    }
}

void Window::handleUpdateSearchKeyword(QWidget *widget, const QString &file, const QString &keyword)
{
    if (file == m_tabbar->currentPath() && m_wrappers.contains(file)) {

        // Update input widget warning status along with keyword match situation.
        bool findKeyword = m_wrappers.value(file)->textEditor()->highlightKeyword(keyword, m_wrappers.value(file)->textEditor()->getPosition());
        m_keywordForSearchAll = keyword;
        m_keywordForSearch = keyword;
        //    bool findKeyword = m_wrappers.value(file)->textEditor()->findKeywordForward(keyword);
        bool emptyKeyword = keyword.trimmed().isEmpty();

        auto *findBarWidget = qobject_cast<FindBar *>(widget);
        if (findBarWidget != nullptr) {
            if (emptyKeyword) {
                findBarWidget->setMismatchAlert(false);
            } else {
                findBarWidget->setMismatchAlert(!findKeyword);
            }
        } else {
            auto *replaceBarWidget = qobject_cast<ReplaceBar *>(widget);
            if (replaceBarWidget != nullptr) {
                if (emptyKeyword) {
                    replaceBarWidget->setMismatchAlert(false);
                } else {
                    replaceBarWidget->setMismatchAlert(false);
                }
            }
        }
    }
    EditWrapper *wrapper = currentWrapper();
    wrapper->textEditor()->updateLeftAreaWidget();
    // 在设置查询字符串并跳转后，及时刷新代码高亮效果
    wrapper->OnUpdateHighlighter();
}

void Window::loadTheme(const QString &path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return;
    }

    m_themePath = path;

    QVariantMap jsonMap = Utils::getThemeMapFromPath(path);
    const QString &backgroundColor = jsonMap["editor-colors"].toMap()["background-color"].toString();
    const QString &tabbarStartColor = jsonMap["app-colors"].toMap()["tab-background-start-color"].toString();
    const QString &tabbarEndColor = jsonMap["app-colors"].toMap()["tab-background-end-color"].toString();


    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->OnThemeChangeSlot(m_themePath);
    }

    m_themePanel->setBackground(backgroundColor);
    m_tabbar->setBackground(tabbarStartColor, tabbarEndColor);
    m_tabbar->setDNDColor(jsonMap["app-colors"].toMap()["tab-dnd-start"].toString(), jsonMap["app-colors"].toMap()["tab-dnd-end"].toString());

    const QString &frameSelectedColor = jsonMap["app-colors"].toMap()["themebar-frame-selected"].toString();
    const QString &frameNormalColor = jsonMap["app-colors"].toMap()["themebar-frame-normal"].toString();
    m_themePanel->setFrameColor(frameSelectedColor, frameNormalColor);
    m_settings->settings->option("advance.editor.theme")->setValue(path);
}

void Window::showNewEditor(EditWrapper *wrapper)
{
    m_editorWidget->addWidget(wrapper);
    m_editorWidget->setCurrentWidget(wrapper);
}

void Window::showNotify(const QString &message)
{
    //DMainWindow::sendMessage(QIcon(":/images/ok.svg"), message);
    //如果是第一次打开编辑器，需要创建一个空白编辑显示框窗体
    if (currentWrapper() == nullptr) {
        this->addBlankTab();
    }
    currentWrapper()->showNotify(message);
}

int Window::getBlankFileIndex()
{
    // get blank tab index list.
    QList<int> tabIndexes;

    // tabFiles.size()
    for (int i = 0; i < m_tabbar->count(); ++i) {
        // find all the blank tab index number.
        QFileInfo fileInfo(m_tabbar->fileAt(i));
        if (fileInfo.dir().absolutePath() == m_blankFileDir) {
            const QString tabText = m_tabbar->textAt(i);
            QRegularExpression reg("(\\d+)");
            QRegularExpressionMatch match = reg.match(tabText);

            tabIndexes << match.captured(1).toInt();
        }
    }
    std::sort(tabIndexes.begin(), tabIndexes.end());

    // Return 1 if no blank file exists.
    if (tabIndexes.size() == 0) {
        return 1;
    }

    // Return first mismatch index as new blank file index.
    for (int j = 0; j < tabIndexes.size(); j++) {
        if (tabIndexes[j] != j + 1) {
            return j + 1;
        }
    }

    // Last, return biggest index as blank file index.
    return tabIndexes.size() + 1;
}

DDialog *Window::createDialog(const QString &title, const QString &content)
{
    DDialog *dialog = new DDialog(title, content, this);
    //dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowStaysOnBottomHint);
    dialog->setWindowModality(Qt::ApplicationModal);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setIcon(QIcon::fromTheme("deepin-editor"));
    dialog->addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
    dialog->addButton(QString(tr("Discard")), false, DDialog::ButtonNormal);
    dialog->addButton(QString(tr("Save")), true, DDialog::ButtonRecommend);

    return dialog;
}

void Window::slotLoadContentTheme(DGuiApplicationHelper::ColorType themeType)
{
    if (themeType == DGuiApplicationHelper::ColorType::LightType) {
        loadTheme(DEEPIN_THEME);
        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::ColorType::UnknownType) {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType::UnknownType);
        }
    } else if (themeType == DGuiApplicationHelper::ColorType::DarkType) {
        loadTheme(DEEPIN_DARK_THEME);
        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::ColorType::UnknownType) {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::ColorType::UnknownType);
        }
    }

    QString qstrColor = palette().color(QPalette::Active, QPalette::Text).name();

    for (auto wrapper : m_wrappers) {
        wrapper->textEditor()->setEditPalette(qstrColor, qstrColor);
        wrapper->OnUpdateHighlighter();
    }

    qstrColor = palette().color(QPalette::Active, QPalette::ButtonText).name();
    QString qstrHighlightColor = palette().color(QPalette::Active, QPalette::HighlightedText).name();

    m_tabbar->setTabPalette(qstrColor, qstrHighlightColor);
}

void Window::slotSettingResetTheme(const QString &path)
{
    if (path == DEEPIN_THEME) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            return;
        } else {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
        }
    } else if (path == DEEPIN_DARK_THEME) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            return;
        } else {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        }
    }
}

void Window::slot_saveReadingPath()
{
    m_reading_list.clear();
    m_reading_list.append(currentWrapper()->textEditor());
}

void Window::slot_beforeReplace(QString _)
{
    currentWrapper()->textEditor()->beforeReplace(_);
}

void Window::slot_setTitleFocus()
{
    QMap<QString, EditWrapper *>::Iterator it = m_wrappers.begin();
    for (; it != m_wrappers.end(); it++) {
        it.value()->bottomBar()->setChildrenFocus(true);
    }

    titlebar()->setFocusPolicy(Qt::TabFocus);
    titlebar()->setFocus(Qt::MouseFocusReason);
    DIconButton *addButton = m_tabbar->findChild<DIconButton *>("AddButton");
    addButton->setFocusPolicy(Qt::TabFocus);
    DIconButton *optionBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    optionBtn->setFocusPolicy(Qt::TabFocus);
    DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
    minBtn->setFocusPolicy(Qt::TabFocus);

    DIconButton *quitFullBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
    quitFullBtn->setFocusPolicy(Qt::TabFocus);
    DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
    maxBtn->setFocusPolicy(Qt::TabFocus);
    DIconButton *closeBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    closeBtn->setFocusPolicy(Qt::TabFocus);
    QWidget::setTabOrder(addButton, optionBtn);
    QWidget::setTabOrder(optionBtn, minBtn);
    QWidget::setTabOrder(minBtn, quitFullBtn);
    QWidget::setTabOrder(quitFullBtn, maxBtn);
    QWidget::setTabOrder(maxBtn, closeBtn);
    currentWrapper()->bottomBar()->setChildrenFocus(true, closeBtn);
}

void Window::slotClearDoubleCharaterEncode()
{
    //赛迪方要求不能出现以下字符，但是编码库中存在，所以手动去掉
    QStringList shouldBeEmpty;
    shouldBeEmpty << "\uE768" << "\uE769" << "\uE76A" << "\uE76B" << "\uE76D" << "\uE76E" << "\uE76F" << "\uE766" << "\uE767" << "\uE770"
                  << "\uE771" << "\uE777" << "\uE778" << "\uE779" << "\uE77A" << "\uE77B" << "\uE77C" << "\uE77D" << "\uE77E" << "\uE77F" << "\uE7FE" << "\uE7FF"
                  << "\uE801" << "\uE802" << "\uE803" << "\uE804" << "\uE805" << "\uE806" << "\uE807" << "\uE808" << "\uE809" << "\uE80A" << "\uE80B" << "\uE80C" << "\uE80D" << "\uE80E"
                  << "\uE80F" << "\uE800" << "\uE7D3" << "\uE7D4" << "\uE7D5" << "\uE7D6" << "\uE7D7" << "\uE7D8" << "\uE7D9" << "\uE7DA" << "\uE7DB" << "\uE7DC" << "\uE7DD"
                  << "\uE7DE" << "\uE7DF" << "\uE7E0" << "\uE7E1" << "\uE7CD" << "\uE7CE" << "\uE7CF" << "\uE7D0" << "\uE7D1" << "\uE7D2" << "\uE7AF" << "\uE7B0" << "\uE7B1" << "\uE7B2"
                  << "\uE7B3" << "\uE7B4" << "\uE7B5" << "\uE7B6" << "\uE7B7" << "\uE7B8" << "\uE7B9" << "\uE7BA" << "\uE7BB" << "\uE7A0" << "\uE7A1" << "\uE7A2" << "\uE7A3" << "\uE7A4"
                  << "\uE7A5" << "\uE7A6" << "\uE7A7" << "\uE7A8" << "\uE7A9" << "\uE7AA" << "\uE7AB" << "\uE7AC" << "\uE7AD" << "\uE7AE" << "\uE797" << "\uE798" << "\uE799" << "\uE79A"
                  << "\uE79B" << "\uE79C" << "\uE79D" << "\uE79E" << "\uE79F" << "\uE780" << "\uE781" << "\uE782" << "\uE783" << "\uE784" << "\uE772" << "\uE773" << "\uE774" << "\uE775"
                  << "\uE776" << "\uE78D" << "\uE78E" << "\uE78F" << "\uE790" << "\uE791" << "\uE792" << "\uE793" << "\uE796"
                  << "\uE7BC" << "\uE7BD" << "\uE7BE" << "\uE7BF" << "\uE7C0" << "\uE7C1" << "\uE7C2" << "\uE7C3" << "\uE7C4"
                  << "\uE7C5" << "\uE7C6" << "\uE7E3" << "\uE7E4" << "\uE7E5" << "\uE7E6" << "〾⿰⿱⿲⿳⿴⿵" << "\uE7F4" << "\uE7F5" << "\uE7F6"
                  << "\uE7F7" << "\uE7F8" << "\uE7F9" << "\uE7FA" << "\uE7FB" << "\uE7FC" << "⿶⿷⿸⿹⿺⿻" << "\uE7FD"
                  << "\u4DB6" << "\uE26C" << "\uE28F" << "\uE290" << "\uE291" << "\uE292" << "\uE293" << "\uE294" << "\uE295" << "\uE296" << "\uE297" << "\uE298" << "\uE299"
                  << "\uE29A" << "\uE29B" << "\uE29C" << "\uE29D" << "\uE29E" << "\uE29F" << "\uE26D"
                  << "\uE644" << "\uE645" << "\uE645" << "\uE646" << "\uE647" << "\uE648" << "\uE649" << "\uE64A" << "\uE64B" << "\uE64C" << "\uE64D" << "\uE64E" << "\uE64F"
                  << "\uE680" << "\uE681" << "\uE682" << "\uE683" << "\uE686" << "\uE688" << "\uE689" << "\uE68A" << "\uE68B" << "\uE68C" << "\uE68D" << "\uE6AC" << "\uE6AD" << "\uE6AE";

    for (const QString &strTemp : shouldBeEmpty) {
        handleReplaceAll(strTemp, " ");
    }
}

void Window::slotSigThemeChanged(const QString &path)
{
    if (path == DEEPIN_THEME) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            return;
        } else {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
        }
    } else if (path == DEEPIN_DARK_THEME) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
            return;
        } else {
            DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        }
    }
}

void Window::slotSigAdjustFont(QString fontName)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->textEditor()->setFontFamily(fontName);
    }
}

void Window::slotSigAdjustFontSize(qreal fontSize)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->textEditor()->setFontSize(fontSize);
        wrapper->bottomBar()->setScaleLabelText(fontSize);
        wrapper->OnUpdateHighlighter();
    }

    m_fontSize = fontSize;
}

void Window::slotSigAdjustTabSpaceNumber(int number)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->textEditor()->setTabSpaceNumber(number);
    }
}

void Window::slotSigAdjustWordWrap(bool enable)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        TextEdit *textedit = wrapper->textEditor();
        textedit->setLineWrapMode(enable);
    }
}

void Window::slotSigSetLineNumberShow(bool bIsShow)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->setLineNumberShow(bIsShow);
    }
}

void Window::slotSigAdjustBookmark(bool bIsShow)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        TextEdit *textedit = wrapper->textEditor();
        textedit->setBookmarkFlagVisable(bIsShow);
    }
}

void Window::slotSigShowBlankCharacter(bool bIsShow)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->setShowBlankCharacter(bIsShow);
    }
}

void Window::slotSigHightLightCurrentLine(bool bIsShow)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->textEditor()->setHighLineCurrentLine(bIsShow);
    }
}

void Window::slotSigShowCodeFlodFlag(bool bIsShow)
{
    for (EditWrapper *wrapper : m_wrappers.values()) {
        TextEdit *textedit = wrapper->textEditor();
        textedit->setCodeFlodFlagVisable(bIsShow);
    }
}

void Window::slotSigChangeWindowSize(QString mode)
{
    if (mode == "fullscreen") {
        this->showFullScreen();
    } else if (mode == "window_maximum") {
        this->showNormal();
        this->showMaximized();
    } else {
        this->showNormal();
    }
}

void Window::slotIsRegisteredIflytekAiassistant(bool bIsRegistIflytekAiassistant)
{
    m_bIsRegistIflytekAiassistant = bIsRegistIflytekAiassistant;
}

void Window::handleFocusWindowChanged(QWindow *w)
{
    if (windowHandle() != w || !currentWrapper() || !isActiveWindow()) {
        return;
    }

    currentWrapper()->checkForReload();
    checkTabbarForReload();
}

void Window::updateThemePanelGeomerty()
{
    int yOffset = isFullScreen() ? 0 : titlebar()->height();
    QRect themePanelRect(0, yOffset, 250, height() - yOffset);
    themePanelRect.moveRight(rect().right());
    m_themePanel->setGeometry(themePanelRect);
}

void Window::checkTabbarForReload()
{
    /* 修复99423 bug暂且屏蔽；拖拽出只读tab文件项，只读字样消失
    int cur = m_tabbar->currentIndex();
    QFileInfo fi(m_tabbar->truePathAt(cur));
    */
    QFileInfo fi;
    if (m_tabbar->currentPath().contains("backup-files")) {
        fi.setFile(m_tabbar->truePathAt(m_tabbar->currentIndex()));
    } else {
        fi.setFile(m_tabbar->currentPath());
    }

    QString tabName = m_tabbar->currentName();
    QString readOnlyStr = QString(" (%1)").arg(tr("Read-Only"));
    tabName.remove(readOnlyStr);

    EditWrapper *wrapper = m_wrappers.value(m_tabbar->currentPath());
    if (fi.exists() && !fi.isWritable()) {
        tabName.append(readOnlyStr);
        m_tabbar->setTabText(m_tabbar->currentIndex(), tabName);
        wrapper->textEditor()->setReadOnlyPermission(true);
    } else {
        tabName.remove(readOnlyStr);
        m_tabbar->setTabText(m_tabbar->currentIndex(), tabName);
        wrapper->textEditor()->setReadOnlyPermission(false);
    }

    setWindowTitleInfo();
    //m_tabbar->setTabText(m_tabbar->currentIndex(), tabName);
    //判断是否需要阻塞系统关机
    emit sigJudgeBlockShutdown();
}

void Window::resizeEvent(QResizeEvent *e)
{
    if (m_themePanel->isVisible()) {
        updateThemePanelGeomerty();
    }

    if (!isMaximized() && !isFullScreen()) {
        m_settings->settings->option("advance.window.window_width")->setValue(rect().width());
        m_settings->settings->option("advance.window.window_height")->setValue(rect().height());
        if (m_needMoveToCenter) {
            Dtk::Widget::moveToCenter(this);
            m_needMoveToCenter = false;
        }
    }

    m_findBar->setGeometry(4, height() - m_findBar->height() - 4, width() - 8, m_findBar->height());
    m_replaceBar->setGeometry(4, height() - m_replaceBar->height() - 4, width() - 8, m_replaceBar->height());

    for (EditWrapper *wrapper : m_wrappers.values()) {
        wrapper->OnUpdateHighlighter();
    }

#if 0
    if (!(m_tabbar->currentPath() == "")) {
        EditWrapper *wrapper = m_wrappers.value(m_tabbar->currentPath());
        wrapper->textEditor()->hideRightMenu();
    }
#endif

    DMainWindow::resizeEvent(e);
}

void Window::closeEvent(QCloseEvent *e)
{
    PerformanceMonitor::closeAppStart();

    if (StartManager::instance()->isMultiWindow()) {
        if (!closeAllFiles()) {
            e->ignore();
            return;
        }
    } else {
        bool save_tab_before_close = m_settings->settings->option("advance.startup.save_tab_before_close")->value().toBool();
        if (!save_tab_before_close) {
            if (!closeAllFiles()) {
                e->ignore();
                return;
            }
        } else {
            // 单个窗口时，没有记录单独关闭窗口，需要记录窗口信息。
            for (auto itr = m_wrappers.begin(); itr != m_wrappers.end(); ++itr) {
                QString filePath = itr.value()->textEditor()->getFilePath();
                Utils::recordCloseFile(filePath);
            }

            backupFile();
        }
    }

    // WARNING: 调用 QProcess::startDetached() 前同步配置，防止多线程对 qgetenv() 中的 environmentMutex 加锁
    // 导致创建进程 fork() 时保留了 environmentMutex 锁状态，使得父子进程陷入资源竞态，阻塞状态。
    if (m_settings->settings) {
        m_settings->settings->sync();
    }

    // 退出打印状态
    m_bPrintProcessing = false;

    QProcess process;
    process.start("dbus-send  --print-reply --dest=com.iflytek.aiassistant /aiassistant/tts com.iflytek.aiassistant.tts.stopTTSDirectly");
    if (!process.waitForFinished(10)) {
        process.terminate();
    }

    QList<EditWrapper *> needSaveList;
    QMap<QString, EditWrapper *> wrappers = m_wrappers;

    for (EditWrapper *wrapper : wrappers) {
        if (nullptr == wrapper) {
            continue;
        }
        m_wrappers.remove(wrapper->filePath());
        disconnect(wrapper->textEditor());
        wrapper->setQuitFlag();
        wrapper->deleteLater();
    }

    disconnect(m_settings, nullptr, this, nullptr);
    //this->close();


    StartManager::instance()->closeAboutForWindow(this);

    emit closeWindow();

    return DMainWindow::closeEvent(e);
}

void Window::hideEvent(QHideEvent *event)
{
    if (this->isVisible()) {
        if (currentWrapper() != nullptr) {
            currentWrapper()->textEditor()->setFocus();
        }
    }
    //如果查找浮窗正显示着，则隐藏
    if (m_findBar->isVisible()) {
        // m_findBar->hide();
        if (currentWrapper() != nullptr) {
            currentWrapper()->bottomBar()->show();
        }
    }

    //如果替换浮窗正显示着，则隐藏
#if 0
    if (m_replaceBar->isVisible()) {
        //m_replaceBar->hide();
        if (currentWrapper() != nullptr) {
            currentWrapper()->m_bottomBar->show();
        }
    }
#endif
    DMainWindow::hideEvent(event);
}

void Window::keyPressEvent(QKeyEvent *e)
{
    QString key = Utils::getKeyshortcut(e);

    if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "decrementfontsize") ||
            key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "incrementfontsize") ||
            key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "togglefullscreen")) {
        currentWrapper()->textEditor()->setCodeFoldWidgetHide(true);
    }

    if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "addblanktab")) {
        addBlankTab();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "newwindow")) {
        emit newWindow();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "savefile")) {
        saveFile();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "saveasfile")) {
        saveAsFile();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "selectnexttab")) {
        m_tabbar->nextTab();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "selectprevtab")) {
        m_tabbar->previousTab();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "closetab")) {
        closeTab();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "restoretab")) {
        restoreTab();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "closeothertabs")) {
        m_tabbar->closeOtherTabs();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "openfile")) {
        openFile();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "incrementfontsize")) {
        incrementFontSize();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "decrementfontsize")) {
        decrementFontSize();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "resetfontsize")) {
        resetFontSize();
    }  else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "togglefullscreen")) {
        DIconButton *minBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMinButton");
        DIconButton *quitFullBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowQuitFullscreenButton");
        quitFullBtn->setFocusPolicy(Qt::TabFocus);
        DIconButton *maxBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowMaxButton");
        if (minBtn->hasFocus() || maxBtn->hasFocus()) {
            toggleFullscreen();
            quitFullBtn->setFocus();
        } else {
            toggleFullscreen();
        }
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "find")) {
        popupFindBar();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "findNext")) {
        handleFindNextSearchKeyword(m_keywordForSearch);
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "findPrev")) {
        handleFindPrevSearchKeyword(m_keywordForSearch);
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "replace")) {
        popupReplaceBar();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "jumptoline")) {
        popupJumpLineBar();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "saveposition")) {
        remberPositionSave();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "restoreposition")) {
        remberPositionRestore();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "escape")) {
        emit pressEsc();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "displayshortcuts")) {
        displayShortcuts();
    } else if (key == Utils::getKeyshortcutFromKeymap(m_settings, "window", "print")) {
        popupPrintDialog();
    } else {
        // Post event to window widget if match Alt+0 ~ Alt+9
        QRegularExpression re("^Alt\\+\\d");
        QRegularExpressionMatch match = re.match(key);
        if (match.hasMatch()) {
            auto tabIndex = key.replace("Alt+", "").toInt();
            if (tabIndex == 9) {
                if (m_tabbar->count() > 1) {
                    activeTab(m_tabbar->count() - 1);
                }
            } else {
                if (tabIndex <= m_tabbar->count()) {
                    activeTab(tabIndex - 1);
                }
            }
        }
    }
}

void Window::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if ((keyEvent->modifiers() | Qt::ShiftModifier) || (keyEvent->modifiers() | Qt::ControlModifier)) {
//        if (nullptr != m_shortcutViewProcess) {
//            int count = Utils::getProcessCountByName("deepin-shortcut-viewer");
//            if (count > 0) {
//                Utils::killProcessByName("deepin-shortcut-viewer");
//            }
//            delete (m_shortcutViewProcess);
//            m_shortcutViewProcess = nullptr;
//        }
    }
}


void Window::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept drag event if mime type is url.
    event->accept();
}

void Window::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QStringList supportfileNames;
        QStringList otherfiles;
        for (auto url : mimeData->urls()) {
            QString file = url.toLocalFile();
            if (Utils::isMimeTypeSupport(file)) {
                supportfileNames.append(file);
            } else {
                otherfiles.append(file);
            }
        }

        //先添加支持的文件
        foreach (QString var, supportfileNames) {
            addTab(var, true);
        }

        //后添加不支持文件　在最后编辑页面显示
        foreach (QString var, otherfiles) {
            addTab(var, true);
        }
    }
}

/**
 * @brief 处理定时事件 \a e , 主要处理延迟关闭标签事件，会在定时器结束后关闭标签页
 */
void Window::timerEvent(QTimerEvent *e)
{
    // 处理延迟关闭标签事件
    if (e->timerId() == m_delayCloseTabTimer.timerId()) {
        m_delayCloseTabTimer.stop();

        // 判断当前处理的标签页是否合法
        if (0 <= m_requestCloseTabIndex && m_requestCloseTabIndex < m_tabbar->count()) {
            activeTab(m_requestCloseTabIndex);
            closeTab();
        }
    }
}

bool Window::findBarIsVisiable()
{
    if (m_findBar->isVisible()) {
        return true;
    } else {
        return false;
    }
}

bool Window::replaceBarIsVisiable()
{
    return m_replaceBar == nullptr ? false : m_replaceBar->isVisible();
}

QString Window::getKeywordForSearchAll()
{
    return m_keywordForSearchAll;
}

QString Window::getKeywordForSearch()
{
    return m_keywordForSearch;
}

void Window::setPrintEnabled(bool enabled)
{
    for (int i = 0; i < m_menu->actions().count(); i++) {
        if (!m_menu->actions().at(i)->text().compare(QString("Print"))) {
            m_menu->actions().at(i)->setEnabled(enabled);
            return;
        }
    }
}

QStackedWidget *Window::getStackedWgt()
{
    return m_editorWidget;
}
