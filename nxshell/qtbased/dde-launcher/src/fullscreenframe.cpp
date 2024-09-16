// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fullscreenframe.h"
#include "constants.h"
#include "xcb_misc.h"
#include "sharedeventfilter.h"

#include <QApplication>
#include <QClipboard>
#include <QScreen>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QKeyEvent>
#include <QProcess>
#include <QScroller>
#include <QDebug>

#include <DWindowManagerHelper>
#include <DDBusSender>
#include <DDialog>
#include <DGuiApplicationHelper>
#include <DPlatformWindowHandle>

DGUI_USE_NAMESPACE

const QPoint widgetRelativeOffset(const QWidget *const self, const QWidget *w)
{
    QPoint offset;
    while (w && w != self) {
        offset += w->pos();
        w = qobject_cast<QWidget *>(w->parent());
    }

    return offset;
}

FullScreenFrame::FullScreenFrame(QWidget *parent)
    : BoxFrame(parent)
    , m_displayMode(AppsListModel::FullscreenAll)
    , m_focusIndex(0)
    , m_mousePressSeconds(0)
    , m_mousePressState(false)
    , m_menuWorker(new MenuWorker(this))
    , m_eventFilter(new SharedEventFilter(this))
    , m_calcUtil(CalculateUtil::instance())
    , m_appsManager(AppsManager::instance())
    , m_delayHideTimer(new QTimer(this))
    , m_searchWidget(new SearchWidget(this))
    , m_contentFrame(new QFrame(this))
    , m_appsIconBox(new DHBoxWidget(m_contentFrame))
    , m_tipsLabel(new QLabel(this))
    , m_appItemDelegate(new AppItemDelegate(this))
    , m_multiPagesView(new MultiPagesView(AppsListModel::FullscreenAll, this))
    , m_searchModeWidget(new SearchModeWidget(this))
    , m_allAppsModel(new AppsListModel(AppsListModel::Search, this))
    , m_filterModel(new SortFilterProxyModel(this))
    , m_topSpacing(new QFrame(this))
    , m_bottomSpacing(new QFrame(this))
    , m_drawerWidget(new AppDrawerWidget(this))
    , m_curScreen(m_appsManager->currentScreen())
    , m_bMousePress(false)
    , m_nMousePos(0)
    , m_scrollValue(0)
    , m_scrollStart(0)
{
    initAccessibleName();
    setFocusPolicy(Qt::NoFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_InputMethodEnabled, true);

    create();
    if (windowHandle()) {
        windowHandle()->setProperty("_d_dwayland_window-type", "launcher");
    }

#if (DTK_VERSION <= DTK_VERSION_CHECK(2, 0, 9, 9))
    setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
#else
    auto compositeChanged = [ = ] {
        if (DWindowManagerHelper::instance()->windowManagerName() == DWindowManagerHelper::WMName::KWinWM)
            setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
        else
            setWindowFlags(Qt::FramelessWindowHint | Qt::SplashScreen);
    };

    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, compositeChanged);
    compositeChanged();
#endif

    // 全局键盘按键事件处理、搜索框字符输入处理
    installEventFilter(m_eventFilter);
    initConnection();
    initUI();
}

FullScreenFrame::~FullScreenFrame()
{
}

void FullScreenFrame::exit()
{
    qApp->quit();
}

int FullScreenFrame::dockPosition()
{
    return m_appsManager->dockPosition();
}

/**
 * @brief FullScreenFrame::addViewEvent
 * 处理全屏模式下视图中的鼠标事件
  * @param pView 当前列表视图
 */
void FullScreenFrame::addViewEvent(AppGridView *pView)
{
    connect(pView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(pView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(pView, &AppGridView::clicked, this, &FullScreenFrame::onAppClick);
    connect(pView, &AppGridView::requestMouseRelease, this, &FullScreenFrame::onRequestMouseRelease);
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pView, qOverload<>(&AppGridView::update));
}

void FullScreenFrame::onHideMenu()
{
    if (m_menuWorker.get() && !isVisible())
        m_menuWorker->onHideMenu();
}

void FullScreenFrame::onDrawerClick(AppGridView *pView)
{
    connect(pView, &AppGridView::popupMenuRequested, this, &FullScreenFrame::showPopupMenu);
    connect(pView, &AppGridView::entered, m_appItemDelegate, &AppItemDelegate::setCurrentIndex);
    connect(pView, &AppGridView::clicked, this, &FullScreenFrame::onDrawerAppClick);
    connect(pView, &AppGridView::requestMouseRelease, this, &FullScreenFrame::onRequestMouseRelease);
    connect(m_appItemDelegate, &AppItemDelegate::requestUpdate, pView, qOverload<>(&AppGridView::update));
}

void FullScreenFrame::showTips(const QString &tips)
{
    if (m_displayMode != AppsListModel::Search)
        return;

    m_tipsLabel->setText(tips);
    QFont font(m_tipsLabel->font());
    font.setPointSize(30);
    m_tipsLabel->setFont(font);

    QColor color(Qt::white);
    color.setAlpha(0.5 * 255);
    QPalette palette(m_tipsLabel->palette());
    palette.setColor(QPalette::WindowText, color);
    m_tipsLabel->setPalette(palette);

    // 根据文字内容的边界矩形设置label大小, 避免在葡萄牙语等语言下内容被截断而显示不全
    QFontMetricsF fontMetric(m_tipsLabel->font());
    int width = qCeil(fontMetric.boundingRect(m_tipsLabel->text()).width());
    int height = qCeil(fontMetric.boundingRect(m_tipsLabel->text()).height());

    m_tipsLabel->setFixedSize(width, height);

    const QPoint center = rect().center() - m_tipsLabel->rect().center();
    m_tipsLabel->move(center);
    m_tipsLabel->setVisible(true);
    m_tipsLabel->raise();
}

void FullScreenFrame::hideTips()
{
    m_tipsLabel->setVisible(false);
}

void FullScreenFrame::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Equal) {
        if (!e->modifiers().testFlag(Qt::ControlModifier))
            return;
        e->accept();
    } else if (e->key() == Qt::Key_V &&
               e->modifiers().testFlag(Qt::ControlModifier)) {
        const QString &clipboardText = QApplication::clipboard()->text();

        // support Ctrl+V shortcuts.
        if (!clipboardText.isEmpty()) {
            m_searchWidget->edit()->lineEdit()->setText(clipboardText);
            m_searchWidget->edit()->lineEdit()->setFocus();
            m_focusIndex = SearchEdit;
        }
    }
}

void FullScreenFrame::showEvent(QShowEvent *e)
{
    m_delayHideTimer->stop();
    m_searchWidget->clearSearchContent();

    if (QApplication::platformName() != "wayland")
        XcbMisc::instance()->set_deepin_override(winId());

    if (!m_appsManager->isVaild())
        m_appsManager->refreshAllList();

    updateDockPosition();

    QFrame::showEvent(e);

    QTimer::singleShot(0, this, [ this ]() {
        raise();
        activateWindow();
        m_searchWidget->raise();
        emit visibleChanged(true);
    });

    m_canResizeDockPosition = true;
}

void FullScreenFrame::hideEvent(QHideEvent *e)
{
    BoxFrame::hideEvent(e);

    QTimer::singleShot(1, this, [ = ] { emit visibleChanged(false); });
}

void FullScreenFrame::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;

    m_searchWidget->clearSearchContent();
    m_mousePressState = true;
    m_mousePressSeconds =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    m_mouseMovePos = e->pos();
    m_mousePressPos = e->pos();

    m_startPoint = e->globalPos();

    // 全屏模式下支持全屏范围滑动翻页
    mousePressDrag(e);
}

void FullScreenFrame::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_mousePressState || e->button() == Qt::RightButton)
        return;

    int categoryCount = m_appsManager->getVisibleCategoryCount();

    if (categoryCount <= 2)
        return;

    qint64 mouseReleaseSeconds =  QDateTime::currentDateTime().toMSecsSinceEpoch();
    int horizontalXOffset = e->pos().x() - m_mousePressPos.x();

// TODO: 优化点
//    if (mouseReleaseSeconds - m_mousePressSeconds > DLauncher::MOUSE_PRESS_TIME_DIFF) {
//        if (horizontalXOffset < 0)
//            m_animationGroup->setScrollType(Scroll_Next);
//        else
//            m_animationGroup->setScrollType(Scroll_Prev);
//    }

    m_mouseMovePos = e->pos();
    // 全屏模式下支持全屏范围滑动翻页
    mouseMoveDrag(e);
}

void FullScreenFrame::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton || !m_mousePressState)
        return;

    int diff_x = qAbs(e->pos().x() - m_mousePressPos.x());
    int diff_y = qAbs(e->pos().y() - m_mousePressPos.y());
    // 小范围位置变化，当作没有变化，针对触摸屏
    if ((e->source() == Qt::MouseEventSynthesizedByQt && diff_x < DLauncher::TOUCH_DIFF_THRESH && diff_y < DLauncher::TOUCH_DIFF_THRESH)
            || (e->source() != Qt::MouseEventSynthesizedByQt && e->pos() == m_mousePressPos )) {

        if (m_drawerWidget->isVisible()) {
            m_drawerWidget->hide();
            return;
        }

        hide();
    }
    m_mousePressState = false;

    if (!m_drawerWidget->isVisible()) {
        // 全屏模式下支持全屏范围滑动翻页
        mouseReleaseDrag(e);
    }
}

bool FullScreenFrame::eventFilter(QObject *o, QEvent *e)
{
    // we filter some key events from LineEdit, to implements cursor move.
    if (o == m_searchWidget->edit()->lineEdit() && e->type() == QEvent::KeyPress) {
        QKeyEvent *keyPress = static_cast<QKeyEvent *>(e);

        // 应用搜索时, 隐藏展开窗口
        if (m_drawerWidget->isVisible())
            m_drawerWidget->hide();

        if (keyPress->key() == Qt::Key_Left || keyPress->key() == Qt::Key_Right) {
            QKeyEvent *event = new QKeyEvent(keyPress->type(), keyPress->key(), keyPress->modifiers());
            qApp->postEvent(this, event);
            return true;
        } else if (keyPress->key() == Qt::Key_Tab) {
            moveCurrentSelectApp(Qt::Key_Tab);
            return true;
        }
    } else if (o == m_contentFrame && e->type() == QEvent::Resize && m_canResizeDockPosition) {
        updateDockPosition();
    }

    return false;
}

void FullScreenFrame::inputMethodEvent(QInputMethodEvent *e)
{
    if (!e->commitString().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->setText(e->commitString());
        m_searchWidget->edit()->lineEdit()->setFocus();
        m_focusIndex =  SearchEdit;
    }

    QWidget::inputMethodEvent(e);
}

QVariant FullScreenFrame::inputMethodQuery(Qt::InputMethodQuery prop) const
{
    switch (prop) {
    case Qt::ImEnabled:
        return true;
    case Qt::ImCursorRectangle:
        return widgetRelativeOffset(this, m_searchWidget->edit()->lineEdit());
    default:
        break;
    }

    return QWidget::inputMethodQuery(prop);
}

void FullScreenFrame::initUI()
{
    m_drawerWidget->setVisible(false);

    m_tipsLabel->setAlignment(Qt::AlignCenter);
    m_tipsLabel->setFixedSize(500, 50);
    m_tipsLabel->setVisible(false);

    m_delayHideTimer->setInterval(500);
    m_delayHideTimer->setSingleShot(true);

    QPalette palette = m_searchWidget->palette();
    QColor colorButton(Qt::white);
    colorButton.setAlpha(static_cast<int>(255 * 0.15));

    QColor colorText(Qt::white);
    palette.setColor(QPalette::Button, colorButton);
    palette.setColor(QPalette::Text, colorText);
    palette.setColor(QPalette::ButtonText, colorText);

    m_searchWidget->edit()->lineEdit()->setPalette(palette);
    m_searchWidget->toggleModeBtn()->setPalette(palette);

    m_searchWidget->edit()->lineEdit()->installEventFilter(this);
    m_searchWidget->installEventFilter(m_eventFilter);
    m_searchWidget->showToggle();

    m_searchModeWidget->hide();
    m_appItemDelegate->installEventFilter(m_eventFilter);
    initAppView();

    // 自由排序模式，设置大小调整方式为固定方式
    // 启动时默认按屏幕大小设置自由排序widget的大小
    // 启动时全屏自由模式设置控件大小，解决模式切换界面抖动问题
    const int appsContentWidth = m_calcUtil->getScreenSize().width();
    const int appsContentHeight = m_calcUtil->getScreenSize().height() - DLauncher::APPS_AREA_TOP_MARGIN;

    m_appsIconBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_appsIconBox->layout()->setSpacing(0);
    m_appsIconBox->layout()->addWidget(m_multiPagesView, 0, Qt::AlignCenter);
    m_appsIconBox->setFixedSize(appsContentWidth, appsContentHeight);
    m_multiPagesView->setFixedSize(appsContentWidth, appsContentHeight);

    // 设置搜索控件大小
    QVBoxLayout *scrollVLayout = new QVBoxLayout;
    scrollVLayout->setContentsMargins(0, DLauncher::APPS_AREA_TOP_MARGIN, 0, 0);
    scrollVLayout->setMargin(0);
    scrollVLayout->setSpacing(0);
    scrollVLayout->addWidget(m_appsIconBox, 0, Qt::AlignCenter);
    scrollVLayout->addWidget(m_searchModeWidget, 0, Qt::AlignCenter);

    m_contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_contentFrame->setFrameStyle(QFrame::NoFrame);
    m_contentFrame->setLayout(scrollVLayout);
    m_contentFrame->installEventFilter(this);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_topSpacing);
    mainLayout->addWidget(m_searchWidget);
    mainLayout->addWidget(m_contentFrame);
    mainLayout->addWidget(m_bottomSpacing);

    setLayout(mainLayout);
}

void FullScreenFrame::initAppView()
{
    // 自由
    m_multiPagesView->setDataDelegate(m_appItemDelegate);
    m_multiPagesView->updatePageCount(AppsListModel::FullscreenAll);
    m_multiPagesView->installEventFilter(this);

    m_filterModel->setSourceModel(m_allAppsModel);
    m_filterModel->setFilterRole(AppsListModel::AppRawItemInfoRole);
    m_filterModel->setFilterKeyColumn(0);
    m_filterModel->setSortCaseSensitivity(Qt::CaseInsensitive);
}

void FullScreenFrame::initConnection()
{
    connect(this, &BoxFrame::backgroundImageChanged, m_drawerWidget, &AppDrawerWidget::updateBackgroundImage);
    connect(this, &FullScreenFrame::visibleChanged, this, &FullScreenFrame::onHideMenu);

    connect(m_multiPagesView, &MultiPagesView::connectViewEvent, this, &FullScreenFrame::addViewEvent);
    connect(m_searchModeWidget, &SearchModeWidget::connectViewEvent, this , &FullScreenFrame::addViewEvent);
    connect(m_drawerWidget, &AppDrawerWidget::drawerClicked, this, &FullScreenFrame::onDrawerClick);

    connect(m_calcUtil, &CalculateUtil::layoutChanged, this, &FullScreenFrame::layoutChanged, Qt::QueuedConnection);
    connect(m_searchWidget, &SearchWidget::searchTextChanged, this, &FullScreenFrame::searchTextChanged);
    connect(m_searchWidget, &SearchWidget::toggleMode, m_drawerWidget, &AppDrawerWidget::hide);
    connect(m_searchWidget, &SearchWidget::toggleMode, m_multiPagesView, &MultiPagesView::resetCurPageIndex);
    connect(m_delayHideTimer, &QTimer::timeout, this, &FullScreenFrame::onWindowHide, Qt::QueuedConnection);

    connect(m_menuWorker.get(), &MenuWorker::appLaunched, this, &FullScreenFrame::onAppLaunch);
    connect(m_menuWorker.get(), &MenuWorker::unInstallApp, m_appsManager, QOverload<const QModelIndex &>::of(&AppsManager::uninstallApp));

    connect(m_appsManager, &AppsManager::requestTips, this, &FullScreenFrame::showTips);
    connect(m_appsManager, &AppsManager::requestHideTips, this, &FullScreenFrame::hideTips);
    connect(m_appsManager, &AppsManager::IconSizeChanged, this, &FullScreenFrame::updateDockPosition);
    connect(m_appsManager, &AppsManager::dataChanged, this, &FullScreenFrame::refreshPageView);
    connect(m_appsManager, &AppsManager::requestHidePopup, this, &FullScreenFrame::onRequestHidePopUp);

    connect(m_curScreen, &QScreen::geometryChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(qApp, &QApplication::primaryScreenChanged, this, &FullScreenFrame::onScreenInfoChange);
}

void FullScreenFrame::initAccessibleName()
{
    setObjectName("LauncherFrame");
    setAccessibleName("FullScrreenFrame");
    m_topSpacing->setAccessibleName("topspacing");
    m_bottomSpacing->setAccessibleName("BottomSpacing");
    m_searchWidget->setAccessibleName("searchWidget");
    m_contentFrame->setAccessibleName("ContentFrame");
    m_tipsLabel->setAccessibleName("tipsLabel");
    m_appItemDelegate->setObjectName("appItemDelegate");

    m_multiPagesView->setAccessibleName("allAppPagesView");
    m_searchWidget->edit()->setAccessibleName("FullScreenSearchEdit");
    m_drawerWidget->setAccessibleName("dirWidget");
}

void FullScreenFrame::showLauncher()
{
    m_focusIndex = 1;
    m_appItemDelegate->setCurrentIndex(QModelIndex());

    // 启动器跟随任务栏位置
    updateGeometry();

    m_searchWidget->edit()->clearEdit();
    m_searchWidget->edit()->clear();

    updateDisplayMode(m_displayMode);

    m_searchWidget->edit()->lineEdit()->clearFocus();

    setFixedSize(m_appsManager->currentScreen()->geometry().size());
    show();
}

void FullScreenFrame::hideLauncher()
{
    if (!isVisible())
        return;

    m_searchWidget->clearSearchContent();
    hide();
}

bool FullScreenFrame::visible()
{
    return isVisible();
}

void FullScreenFrame::updateGeometry()
{
    QRect rect = m_appsManager->currentScreen()->geometry();

    // 设置启动器显示位置，触发BoxFrame::moveEvent事件．
    setGeometry(rect);

    QFrame::updateGeometry();
}

void FullScreenFrame::moveCurrentSelectApp(const int key)
{
    m_searchWidget->edit()->lineEdit()->clearFocus();
    if (Qt::Key_Tab == key || Qt::Key_Backtab == key) {
        nextTabWidget(key);
        return;
    }

    if (Qt::Key_Undo == key) {
        auto oldStr = m_searchWidget->edit()->lineEdit()->text();
        m_searchWidget->edit()->lineEdit()->undo();

        if (!oldStr.isEmpty() && oldStr == m_searchWidget->edit()->lineEdit()->text())
            m_searchWidget->edit()->lineEdit()->clear();

        return;
    }

    const QModelIndex curModelIndex = m_appItemDelegate->currentIndex();
    // move operation should be start from a valid location, if not, just init it.
    if (!curModelIndex.isValid()) {
        if (m_displayMode == AppsListModel::FullscreenAll) {
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));
            update();
            return;
        }
    }

    const int column = m_calcUtil->appColumnCount();
    QModelIndex index;

    // calculate destination sibling by keys, it may cause an invalid position.
    switch (key) {
    case Qt::Key_Backtab:
    case Qt::Key_Left:
        index = curModelIndex.sibling(curModelIndex.row() - 1, 0);
        break;
    case Qt::Key_Right:
        index = curModelIndex.sibling(curModelIndex.row() + 1, 0);
        break;
    case Qt::Key_Up:
        index = curModelIndex.sibling(curModelIndex.row() - column, 0);
        break;
    case Qt::Key_Down:
        index = curModelIndex.sibling(curModelIndex.row() + column, 0);
        break;
    default:
        break;
    }

    int curPage = m_multiPagesView->currentPage();
    if ((key == Qt::Key_Right || key == Qt::Key_Down) && !index.isValid()) {
        if (curPage >= m_multiPagesView->pageCount() - 1) {
            m_multiPagesView->showCurrentPage(0);
        } else {
            m_multiPagesView->showCurrentPage(curPage + 1);
        }
        m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));
    } else if ((key == Qt::Key_Left || key == Qt::Key_Up) && !index.isValid()) {
        if (curPage <= 0) {
            m_multiPagesView->showCurrentPage(m_multiPagesView->pageCount() - 1);
        } else {
            m_multiPagesView->showCurrentPage(curPage - 1);
        }
        auto model = m_multiPagesView->pageModel(m_multiPagesView->currentPage());
        m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(model->rowCount() - 1));
    } else if (index.isValid()) {
        // valid verify and UI adjustment.
        const QModelIndex selectedIndex = index.isValid() ? index : curModelIndex;
        m_appItemDelegate->setCurrentIndex(selectedIndex);
        update();
    }
}

void FullScreenFrame::appendToSearchEdit(const char ch)
{
    m_searchWidget->edit()->lineEdit()->setFocus();

    // -1 means backspace key pressed
    if (ch == static_cast<const char>(-1)) {
        m_searchWidget->edit()->lineEdit()->backspace();
        return;
    }

    if (!m_searchWidget->edit()->lineEdit()->selectedText().isEmpty()) {
        m_searchWidget->edit()->lineEdit()->backspace();
    }
    m_focusIndex =  SearchEdit;

    m_searchWidget->edit()->lineEdit()->setText(m_searchWidget->edit()->lineEdit()->text() + ch);
}

void FullScreenFrame::launchCurrentApp()
{
    const QModelIndex &index = m_appItemDelegate->currentIndex();

    if (index.isValid() && !index.data(AppsListModel::AppDesktopRole).toString().isEmpty()) {
        m_appsManager->launchApp(index);
        hide();
    }
}

void FullScreenFrame::regionMonitorPoint(const QPoint &point, int flag)
{
    QRect dockRect = m_appsManager->dockGeometry();
    QRect visiblableRect = m_menuWorker->menuGeometry();

    if (flag == DLauncher::MOUSE_LEFTBUTTON) {
        // 左键点击时
        if (!m_menuWorker->isMenuShown() && !m_appsManager->uninstallDlgShownState()
                && !m_delayHideTimer->isActive() && dockRect.contains(point)) {
            m_delayHideTimer->start();
        }

        if (m_menuWorker->isMenuShown() && !visiblableRect.contains(point)) {
            m_delayHideTimer->start();
        }
    }
}

void FullScreenFrame::showPopupMenu(const QPoint &pos, const QModelIndex &context)
{
    qDebug() << "show menu" << pos << context << context.data(AppsListModel::AppNameRole).toString()
             << "app key:" << context.data(AppsListModel::AppKeyRole).toString();

    const bool isDir = context.data(AppsListModel::ItemIsDirRole).toBool();
    if (!isDir)
        m_menuWorker->showMenuByAppItem(pos, context);
}

void FullScreenFrame::uninstallApp(const QString &desktopPath)
{
    int currentPage = m_multiPagesView->currentPage();
    m_appsManager->uninstallApp(m_multiPagesView->pageModel(currentPage)->indexAt(desktopPath));
}

void FullScreenFrame::refreshPageView(AppsListModel::AppCategory category)
{
    if (!isVisible())
        return;

    if (AppsListModel::Search == category) {
        m_searchModeWidget->setSearchModel(m_filterModel);
    } else {
        m_multiPagesView->updatePageCount(category);
        m_multiPagesView->showCurrentPage(m_multiPagesView->currentPage());
    }
}

void FullScreenFrame::onScreenInfoChange()
{
    m_curScreen->disconnect();
    m_curScreen = m_appsManager->currentScreen();

    setFixedSize(m_curScreen->size());
    scaledBackground();
    scaledBlurBackground();
    update();

    connect(m_curScreen, &QScreen::geometryChanged, this, &FullScreenFrame::onScreenInfoChange);
    connect(m_curScreen, &QScreen::orientationChanged, this, &FullScreenFrame::onScreenInfoChange);
}

void FullScreenFrame::onAppClick(const QModelIndex &index)
{
    if (!index.isValid()) {
        // 点击全屏空白处隐藏文件夹展开窗口
       if (m_drawerWidget->isVisible())
           m_drawerWidget->hide();
       else
           hideLauncher();
        return;
    }

    const ItemInfo_v1 info = index.data(AppsListModel::AppRawItemInfoRole).value<ItemInfo_v1>();

    if (!info.m_isDir) {
        m_appsManager->launchApp(index);
        hideLauncher();
    } else {
        m_drawerWidget->setCurrentIndex(index);
        m_appsManager->setDirAppInfoList(index);
        m_drawerWidget->show();
        m_drawerWidget->refreshDrawerTitle(info.m_name);
    }
}

void FullScreenFrame::onDrawerAppClick(const QModelIndex &index)
{
    // 点击应用文件夹展开窗口空白处, 不响应
    if (!index.isValid())
        return;

    const bool isDir = index.data(AppsListModel::ItemIsDirRole).toBool();
    if (!isDir) {
        m_appsManager->launchApp(index);
        onAppLaunch();
    }
}

void FullScreenFrame::onRequestMouseRelease()
{
    m_mousePressState = false;
}

void FullScreenFrame::onRequestHidePopUp()
{
    if (m_drawerWidget->isVisible())
        m_drawerWidget->hide();
}

void FullScreenFrame::onAppLaunch()
{
    // 启动应用后，隐藏所有窗口
    if (m_drawerWidget->isVisible())
        m_drawerWidget->hide();

    hideLauncher();
}

void FullScreenFrame::onWindowHide()
{
    // 点击列表空白处，应用文件夹展开窗口显示则先关闭，否则隐藏全屏窗口
    if (m_drawerWidget->isVisible()) {
        m_drawerWidget->hide();
        return;
    }

    hideLauncher();
}

void FullScreenFrame::updateDisplayMode(const int mode)
{
    if (m_displayMode == mode)
        return;

    m_displayMode = mode;

   if (m_displayMode == AppsListModel::FullscreenAll) {
        // 隐藏搜索模式
        m_searchModeWidget->setVisible(false);

        // 再显示自由显示模式
        m_appsIconBox->setVisible(true);

        m_multiPagesView->setModel(AppsListModel::FullscreenAll);
    } else if (m_displayMode ==  AppsListModel::Search) {
        // 隐藏自由模式显示
        m_appsIconBox->setVisible(false);
        // 显示搜索模式
        m_searchModeWidget->setVisible(true);
        layoutChanged();
    }

    m_appItemDelegate->setCurrentIndex(QModelIndex());

    // 搜索模式下的文字描述
    hideTips();
}

void FullScreenFrame::updateDockPosition()
{
    const QRect dockGeometry = m_appsManager->dockGeometry();

    int bottomMargin = 20;

    m_topSpacing->setFixedHeight(30);
    m_bottomSpacing->setFixedHeight(bottomMargin);

    switch (m_appsManager->dockPosition()) {
    case DLauncher::DOCK_POS_TOP:
        m_topSpacing->setFixedHeight(30 + dockGeometry.height());
        bottomMargin = m_topSpacing->height() + DLauncher::APPS_AREA_TOP_MARGIN;
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_BOTTOM:
        bottomMargin += dockGeometry.height();
        m_bottomSpacing->setFixedHeight(bottomMargin);
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_LEFT:
        m_searchWidget->setLeftSpacing(dockGeometry.width());
        m_searchWidget->setRightSpacing(0);
        break;
    case DLauncher::DOCK_POS_RIGHT:
        m_searchWidget->setLeftSpacing(0);
        m_searchWidget->setRightSpacing(dockGeometry.width());
        break;
    default:
        break;
    }

    m_calcUtil->calculateAppLayout(m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN), m_displayMode);
}

void FullScreenFrame::nextTabWidget(int key)
{
    if (Qt::Key_Backtab == key) {
        -- m_focusIndex;
        if (m_displayMode == AppsListModel::FullscreenAll) {
            if (m_focusIndex < FirstItem)
                m_focusIndex = CategoryChangeBtn;
        }
    } else if (Qt::Key_Tab == key) {
        ++ m_focusIndex;
        if (m_displayMode == AppsListModel::FullscreenAll) {
            if (m_focusIndex > CategoryChangeBtn)
                m_focusIndex = FirstItem;
        }
    } else {
        return;
    }

    switch (m_focusIndex) {
    case FirstItem: {
        if (m_displayMode == AppsListModel::FullscreenAll)
            m_appItemDelegate->setCurrentIndex(m_multiPagesView->getAppItem(0));
        update();
    }
    break;
    case SearchEdit: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->edit()->lineEdit()->setFocus();
    }
    break;
    case CategoryChangeBtn: {
        m_appItemDelegate->setCurrentIndex(QModelIndex());
        m_searchWidget->toggleModeBtn()->setFocus();
    }
    break;
    }
}

/**
 * @brief FullScreenFrame::mousePressDrag 处理全屏区域按住鼠标拖动翻页
 * @param e
 */
void FullScreenFrame::mousePressDrag(QMouseEvent *e)
{
    m_bMousePress = true;
    m_nMousePos = e->x();
    m_scrollValue = m_multiPagesView->getListArea()->horizontalScrollBar()->value();
    m_scrollStart = m_scrollValue;

////     这里会导致单元测试程序异常崩溃,暂时屏蔽
//     if(e->button() != Qt::RightButton) m_multiPagesView->updateGradient();
}

void FullScreenFrame::mouseMoveDrag(QMouseEvent *e)
{
    int pageCount = m_multiPagesView->pageCount();
    int curPage = m_multiPagesView->currentPage();

    if (!m_bMousePress)
        return;

    int nDiff = m_nMousePos - e->x();

    // 处于首页继续向右滑动
    if (curPage == 0 && nDiff < 0)
        return;

    // 处于尾页继续向左滑动
    if (curPage == pageCount -1 && nDiff > 0)
        return;

    m_multiPagesView->getListArea()->horizontalScrollBar()->setValue(nDiff + m_scrollValue);
}

void FullScreenFrame::mouseReleaseDrag(QMouseEvent *e)
{
    int curPage = m_multiPagesView->currentPage();
    int nDiff = m_nMousePos - e->x();

    if (nDiff > DLauncher::TOUCH_DIFF_THRESH) {
        // 加大范围来避免手指点击触摸屏抖动问题
        m_multiPagesView->showCurrentPage(curPage + 1);
    } else if (nDiff < -DLauncher::TOUCH_DIFF_THRESH) {
        // 加大范围来避免手指点击触摸屏抖动问题
        m_multiPagesView->showCurrentPage(curPage - 1);
    } else {
       int nScroll = m_multiPagesView->getListArea()->horizontalScrollBar()->value();
        //多个分页是点击直接隐藏
        if (nScroll == m_scrollStart && curPage != 1)
            emit m_multiPagesView->getAppGridViewList()[curPage]->clicked(QModelIndex());
        else if (nScroll - m_scrollStart > DLauncher::MOUSE_MOVE_TO_NEXT)
            m_multiPagesView->showCurrentPage(curPage + 1);
        else if (nScroll - m_scrollStart < -DLauncher::MOUSE_MOVE_TO_NEXT)
            m_multiPagesView->showCurrentPage(curPage - 1);
        else
            m_multiPagesView->showCurrentPage(curPage);
    }

    m_bMousePress = false;

    m_multiPagesView->setGradientVisible(false);
}

void FullScreenFrame::layoutChanged()
{
    if (!m_calcUtil->fullscreen()) {
        qDebug() << "now in the FullScreenFrame";
        return;
    }

    QSize boxSize = m_contentFrame->size() - QSize(0, DLauncher::APPS_AREA_TOP_MARGIN);
    if (m_displayMode == AppsListModel::FullscreenAll) {
        m_appsIconBox->setFixedSize(boxSize);
        m_multiPagesView->setFixedSize(boxSize);
        m_multiPagesView->updatePosition(m_displayMode);
    } else {
        // TODO: remainSpacing 临时处理，后面会定制控件封装
        int remainSpacing = m_calcUtil->appItemSpacing() * 7 / 2;
        m_searchModeWidget->setContentsMargins(remainSpacing, 0, remainSpacing, 0);
        m_searchModeWidget->setFixedSize(boxSize);
    }
}

void FullScreenFrame::searchTextChanged(const QString &keywords, bool enableUpdateMode)
{
    if (!enableUpdateMode)
        return;

    if (keywords.isEmpty()) {
        updateDisplayMode(AppsListModel::FullscreenAll);
    } else {
        updateDisplayMode(AppsListModel::Search);

        QString keyWord = keywords;
        keyWord = keyWord.remove(QRegExp("\\s"));

        emit searchApp(keyWord);
        QRegExp regExp(keyWord, Qt::CaseInsensitive);
        m_filterModel->setFilterRegExp(regExp);
        m_searchModeWidget->setSearchModel(m_filterModel);
        m_searchModeWidget->selectFirstItem();
    }

    if (m_searchWidget->edit()->lineEdit()->text().isEmpty())
        m_searchWidget->edit()->lineEdit()->clearFocus();
}
