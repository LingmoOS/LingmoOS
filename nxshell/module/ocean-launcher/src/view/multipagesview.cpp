// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multipagesview.h"
#include "constants.h"
#include "fullscreenframe.h"
#include "editlabel.h"

#include <QHBoxLayout>

#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

/**
 * @brief MultiPagesView::MultiPagesView 全屏模式下多页列表控件类
 * @param categoryModel 应用分类类型
 * @param parent 父控件指针对象
 */
MultiPagesView::MultiPagesView(AppsListModel::AppCategory categoryModel, QWidget *parent)
    : QWidget(parent)
    , m_pLeftGradient(new GradientLabel(this))
    , m_pRightGradient(new GradientLabel(this))
    , m_appsManager(AppsManager::instance())
    , m_calcUtil(CalculateUtil::instance())
    , m_appListArea(new AppListArea)
    , m_viewBox(new DHBoxWidget)
    , m_delegate(Q_NULLPTR)
    , m_titleLabel(new EditLabel(this))
    , m_pageControl(new PageControl)
    , m_category(categoryModel)
    , m_pageCount(0)
    , m_pageIndex(0)
    , m_bDragStart(false)
    , m_bMousePress(false)
    , m_nMousePos(0)
    , m_scrollValue(0)
    , m_scrollStart(0)
    , m_changePageDelayTime(nullptr)
{
    m_pRightGradient->setAccessibleName("thisRightGradient");
    m_pLeftGradient->setAccessibleName("thisLeftGradient");
    m_pageControl->setAccessibleName("pageControl");

    // 滚动区域
    m_appListArea->setObjectName("MultiPageBox");
    m_appListArea->viewport()->setAutoFillBackground(false);
    m_appListArea->setWidgetResizable(true);
    m_appListArea->setFocusPolicy(Qt::NoFocus);
    m_appListArea->setFrameStyle(QFrame::NoFrame);
    m_appListArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_appListArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListArea->viewport()->installEventFilter(this);
    m_appListArea->installEventFilter(this);

    // 翻页按钮和动画
    m_pageSwitchAnimation = new QPropertyAnimation(m_appListArea->horizontalScrollBar(), "value", this);
    m_pageSwitchAnimation->setEasingCurve(QEasingCurve::Linear);
    if (!DGuiApplicationHelper::isSpecialEffectsEnvironment()) {
        m_changePageDelayTime = new QElapsedTimer();
        m_pageSwitchAnimation->setDuration(0);
    }

    initUi();
    initConnection();
}

MultiPagesView::~MultiPagesView()
{
    if (m_changePageDelayTime) {
        delete m_changePageDelayTime;
        m_changePageDelayTime = nullptr;
    }
}

void MultiPagesView::refreshTitle(const QString &title, int maxWidth)
{
    m_titleLabel->setText(maxWidth, title);
    m_titleLabel->setVisible(true);
}

/**
 * @brief MultiPagesView::updateGradient 切换分页时屏幕左右两边30pixel的过渡效果
 * @param pixmap 屏幕背景图片对象
 * @param topLeftImg 左侧过渡范围起点
 * @param topRightImg 右侧过渡范围起点
 */
void MultiPagesView::updateGradient(QPixmap &pixmap, QPoint topLeftImg, QPoint topRightImg)
{
    m_pLeftGradient->setDirection(GradientLabel::LeftToRight);
    m_pRightGradient->setDirection(GradientLabel::RightToLeft);

    const qreal ratio = devicePixelRatioF();
    pixmap.setDevicePixelRatio(1);

    int nWidth = DLauncher::TOP_BOTTOM_GRADIENT_HEIGHT * m_calcUtil->getScreenScaleX();
    QSize gradientSize(nWidth, height());

    QPoint topLeft = mapTo(this, QPoint(0, 0));
    QRect topRect(topLeftImg * ratio, gradientSize * ratio);
    QPixmap topCache = pixmap.copy(topRect);
    topCache.setDevicePixelRatio(ratio);

    m_pLeftGradient->setPixmap(topCache);
    m_pLeftGradient->resize(gradientSize);
    m_pLeftGradient->move(topLeft);
    m_pLeftGradient->raise();

    QPoint topRight(topLeft.x() + width() - gradientSize.width(), topLeft.y());
    QPoint imgTopRight(topRightImg.x() - gradientSize.width(), topRightImg.y());

    QRect RightRect(imgTopRight * ratio, gradientSize * ratio);
    QPixmap bottomCache = pixmap.copy(RightRect);

    m_pRightGradient->setPixmap(bottomCache);
    m_pRightGradient->resize(gradientSize);
    m_pRightGradient->move(topRight);
    m_pRightGradient->raise();
    setGradientVisible(true);
}

/**
 * @brief MultiPagesView::updatePageCount 更新分页控件信息
 * @param category 应用分类类型
 */
void MultiPagesView::updatePageCount(AppsListModel::AppCategory category)
{
    int pageCount = m_appsManager->getPageCount(category == AppsListModel::FullscreenAll ? m_category : category);

    if (pageCount == 0)
        setVisible(false);

    if (pageCount < 1)
        pageCount = 1;

    if (pageCount == m_pageCount)
        return;

    if (pageCount > m_pageCount) {
        while (pageCount > m_pageCount) {
            AppsListModel *pModel = new AppsListModel(m_category);
            pModel->setPageIndex(m_pageCount);
            m_pageAppsModelList.push_back(pModel);

            AppGridView *pageView = Q_NULLPTR;
            if (category == AppsListModel::FullscreenAll) {
                pageView = new AppGridView(AppGridView::MainView, this);
            } else {
                // m_category == AppsListModel::Dir的时候
                pageView = new AppGridView(AppGridView::PopupView, this);
            }

            pageView->setModel(pModel);
            pageView->setItemDelegate(m_delegate);
            pageView->setContainerBox(m_appListArea);
            pageView->installEventFilter(this);
            pageView->setDelegate(this);
            m_appGridViewList.push_back(pageView);

            m_viewBox->layout()->insertWidget(m_pageCount, pageView);

            m_pageCount++;

            connect(pageView, &AppGridView::requestScrollLeft, this, &MultiPagesView::dragToLeft);
            connect(pageView, &AppGridView::requestScrollRight, this, &MultiPagesView::dragToRight);
            connect(pageView, &AppGridView::requestScrollStop, [this] {
                m_bDragStart = false;
                setGradientVisible(false);
            });
            connect(pageView, &AppGridView::dragEnd, this, &MultiPagesView::dragStop);
            connect(m_pageSwitchAnimation, &QPropertyAnimation::finished,pageView,&AppGridView::setDragAnimationEnable);
            connect(m_pageSwitchAnimation, &QPropertyAnimation::finished,this, [ = ] {
                    setGradientVisible(false);
            });
            emit connectViewEvent(pageView);
            // 新增的页面需要设置一下大小
            updatePosition();
        }
    } else {
        while (pageCount < m_pageCount) {
            AppGridView *pageView = qobject_cast<AppGridView *>(m_viewBox->layout()->itemAt(m_pageCount - 1)->widget());
            m_viewBox->layout()->removeWidget(pageView);
            pageView->model()->deleteLater();
            pageView->deleteLater();

            m_pageAppsModelList.removeLast();
            m_appGridViewList.removeLast();

            m_pageCount--;
        }
    }

    m_pageControl->setPageCount(m_pageCount > 1 ? pageCount : 0);
}

/**
 * @brief MultiPagesView::dragToLeft 在当前列表页向左拖动item
 * @param index 拖动item对应的模型索引
 */
void MultiPagesView::dragToLeft(const QModelIndex &index)
{
    Q_UNUSED(index);
    if (m_pageIndex <= 0)
        return;

    if (isScrolling() || m_bDragStart)
        return;

    m_appGridViewList[m_pageIndex]->dragOut(-1);

    showCurrentPage(m_pageIndex - 1);

    int lastApp = m_pageAppsModelList[m_pageIndex]->rowCount();
    QModelIndex firstModel = m_appGridViewList[m_pageIndex]->indexAt(lastApp - 1);
    m_appGridViewList[m_pageIndex]->dragIn(firstModel, m_pageSwitchAnimation->state() != QPropertyAnimation::Running);

    // 保存向左拖拽后item回归的终点位置
    const QPoint &dropCursorPoint = m_appGridViewList[m_pageIndex]->appIconRect(firstModel).topLeft();
    m_appGridViewList[m_pageIndex]->setDropAndLastPos(dropCursorPoint);

    m_bDragStart = true;
}

/**
 * @brief MultiPagesView::dragToRight  在当前列表页向右拖动item
 * @param index 拖动item对应的模型索引
 */
void MultiPagesView::dragToRight(const QModelIndex &index)
{
    Q_UNUSED(index);
    if (m_pageIndex >= m_pageCount - 1)
        return;

    if (isScrolling() || m_bDragStart)
        return;

    // 当前页面准备空出最后一个图标
    int newPos = m_calcUtil->appPageItemCount(m_category);
    // 下一页的末尾位置
    m_appGridViewList[m_pageIndex]->dragOut(newPos * 2 - 1);

    // 展开下一页
    showCurrentPage(m_pageIndex + 1);

    // 将最后一个App'挤走'
    int lastApp = m_pageAppsModelList[m_pageIndex]->rowCount();
    QModelIndex lastModel = m_appGridViewList[m_pageIndex]->indexAt(lastApp - 1);
    m_appGridViewList[m_pageIndex]->dragIn(lastModel, m_pageSwitchAnimation->state() != QPropertyAnimation::Running);

    // 保存向右拖拽后item回归的终点位置
    const QPoint &dropCursorPoint = m_appGridViewList[m_pageIndex]->appIconRect(lastModel).topLeft();
    m_appGridViewList[m_pageIndex]->setDropAndLastPos(dropCursorPoint);

    m_bDragStart = true;
}

/**
 * @brief MultiPagesView::dragStop 当前视图中拖拽item触发分页才执行flashDrag()
 */
void MultiPagesView::dragStop()
{
    if (sender() == m_appGridViewList[m_pageIndex])
        return;

    m_appGridViewList[m_pageIndex]->flashDrag();
}

void MultiPagesView::updateAppDrawerTitle(const QModelIndex &index)
{
    const QString title = m_titleLabel->text();
    m_appsManager->updateDrawerTitle(index, title);
    m_appsManager->saveFullscreenUsedSortedList();

    emit m_appsManager->dataChanged(AppsListModel::FullscreenAll);
}

/**
 * @brief MultiPagesView::getAppItem 获取item的模型索引
 * @param index item所在行数
 * @return 返回item对应的模型索引
 */
QModelIndex MultiPagesView::getAppItem(int index)
{
    return m_appGridViewList[m_pageIndex]->indexAt(index);
}

/**
 * @brief MultiPagesView::setDataDelegate 初始化当前视图代理
 * @param delegate
 */
void MultiPagesView::setDataDelegate(QAbstractItemDelegate *delegate)
{
    m_delegate = delegate;
}

/**
 * @brief MultiPagesView::ShowPageView 展现分类应用下视图分页数据
 * @param category 应用分类类型
 */
void MultiPagesView::ShowPageView(AppsListModel::AppCategory category)
{
    int pageCount = m_appsManager->getPageCount(category);
    for (int i = 0; i < qMax(pageCount, m_pageCount); i++) {
        m_appGridViewList[i]->setVisible(i < pageCount);
        m_pageAppsModelList[i]->setCategory(category);
    }
    m_pageControl->setPageCount(pageCount > 1 ? pageCount : 0);
    m_pageCount = pageCount;
    m_category = category;
}

/**
 * @brief MultiPagesView::setModel 给视图设置模型
 * @param category 应用分类类型
 */
void MultiPagesView::setModel(AppsListModel::AppCategory category)
{
    for (int i = 0; i < m_pageCount; i++) {
        m_pageAppsModelList[i]->setCategory(category);
        m_appGridViewList[i]->setModel(m_pageAppsModelList[i]);
    }
}

/**
 * @brief MultiPagesView::updatePosition
 * 给自由模式,分类模式下分别给滑动区域, 透明控件, 视图列表设置大小,
 * 并设置边距,默认回到首页
 * @param mode 模式分类: 自由模式: 0, 分类模式: 1, 搜索模式: 2
 */
void MultiPagesView::updatePosition(int mode)
{
    // 更新全屏两种模式下界面布局的左右边距和间隔
    // TODO: 左右间隔，现在是根据剩余控件进行计算得出的，待优化
    int remainSpacing = m_calcUtil->appItemSpacing() * 7 / 2;

    QSize tmpSize = size() - QSize(remainSpacing, m_pageControl->height() + DLauncher::DRAG_THRESHOLD);
    m_appListArea->setFixedSize(tmpSize);
    m_viewBox->setFixedSize(tmpSize);

    for (auto pView : m_appGridViewList)
        pView->setFixedSize(tmpSize);
    m_viewBox->layout()->setContentsMargins(0, 0, 0, 0);
    m_viewBox->layout()->setSpacing(0);
    m_pageControl->updateIconSize(m_calcUtil->getScreenScaleX(), m_calcUtil->getScreenScaleY());
}

void MultiPagesView::initUi()
{
    m_viewBox->setAttribute(Qt::WA_TranslucentBackground);
    m_appListArea->setWidget(m_viewBox);

    m_pageControl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setContentsMargins(0, 0, 0, DLauncher::DRAG_THRESHOLD);
    layoutMain->setSpacing(0);
    m_titleLabel->setVisible(false);

    layoutMain->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    layoutMain->addWidget(m_appListArea, 0, Qt::AlignHCenter);
    layoutMain->addWidget(m_pageControl, 0, Qt::AlignHCenter);
    setLayout(layoutMain);
}

void MultiPagesView::initConnection()
{
    connect(m_pageControl, &PageControl::onPageChanged, this, &MultiPagesView::showCurrentPage);
    connect(m_titleLabel, &EditLabel::titleChanged, this, &MultiPagesView::titleChanged);
}

void MultiPagesView::showCurrentPage(int currentPage)
{
    m_pageIndex = ((currentPage > 0) ? (currentPage < m_pageCount ? currentPage : m_pageCount - 1) : 0);
    int endValue = ((m_pageIndex == 0) ? 0 : (m_appGridViewList[m_pageIndex]->x()));
    int startValue = m_appListArea->horizontalScrollValue();

    m_appListArea->setProperty("curPage", m_pageIndex);

    m_pageSwitchAnimation->stop();
    m_pageSwitchAnimation->setStartValue(startValue);
    m_pageSwitchAnimation->setEndValue(endValue);
    m_pageSwitchAnimation->start();

    if (m_changePageDelayTime)
        m_changePageDelayTime->start();

    m_pageControl->setCurrent(m_pageIndex);
}

QModelIndex MultiPagesView::selectApp(const int key)
{
    int page = m_pageIndex;
    int itemSelect = -1;
    if (Qt::Key_Left == key || Qt::Key_Up == key) {
        if (page - 1 >= 0) {
            -- page;
            itemSelect = m_calcUtil->appPageItemCount(m_category) - 1;
        } else {
            page = m_pageCount - 1;
            itemSelect = m_pageAppsModelList[page]->rowCount() - 1;
        }
    } else {
        if (page + 1 < m_pageCount) {
            ++ page;
            itemSelect = 0;
        } else {
            page = 0;
            itemSelect = 0;
        }
    }
    if (page != m_pageIndex)
        showCurrentPage(page);

    return m_appGridViewList[m_pageIndex]->indexAt(itemSelect);
}

AppGridView *MultiPagesView::pageView(int pageIndex)
{
    if (pageIndex >= m_pageCount)
        return nullptr;

    return m_appGridViewList[pageIndex];
}

AppsListModel *MultiPagesView::pageModel(int pageIndex)
{
    if (pageIndex >= m_pageCount)
        return nullptr;

    return m_pageAppsModelList[pageIndex];
}

void MultiPagesView::wheelEvent(QWheelEvent *e)
{
    if (isScrolling())
        return;

    int page = m_pageIndex;
    const QPoint angleDelta = e->angleDelta();
    int scrollLen = (qAbs(angleDelta.x()) > qAbs(angleDelta.y()) ? angleDelta.x() : angleDelta.y()) / 8;
    if (scrollLen > 0 && page - 1 >= 0) {
        --page;
    } else if (scrollLen < 0 && page + 1 < m_pageCount) {
        ++page;
    }

    if (page != m_pageIndex)
        showCurrentPage(page);
}

void MultiPagesView::showEvent(QShowEvent *e)
{
    showCurrentPage(m_pageIndex);

    return QWidget::showEvent(e);
}

void MultiPagesView::mousePress(QMouseEvent *e)
{
    m_bMousePress = true;
    m_nMousePos = e->x();
    m_scrollValue = m_appListArea->horizontalScrollValue();
    m_scrollStart = m_scrollValue;

    if(m_pageCount == 1 && m_category != AppsListModel::Search)
        QWidget::mousePressEvent(e);
}

void MultiPagesView::mouseMove(QMouseEvent *e)
{
    if (!m_bMousePress)
        return;

    int nDiff = m_nMousePos - e->x();
    m_scrollValue += nDiff;

    m_appListArea->setHorizontalScrollValue(m_scrollValue);

    if(m_pageCount == 1)
        QWidget::mouseMoveEvent(e);
}

void MultiPagesView::mouseRelease(QMouseEvent *e)
{
    int nDiff = m_nMousePos - e->x();

    if (nDiff > DLauncher::TOUCH_DIFF_THRESH) { // 加大范围来避免手指点击触摸屏抖动问题
        showCurrentPage(m_pageIndex + 1);
    } else if (nDiff < -DLauncher::TOUCH_DIFF_THRESH) { // 加大范围来避免手指点击触摸屏抖动问题
        showCurrentPage(m_pageIndex - 1);
    } else {
        int nScroll = m_appListArea->horizontalScrollValue();
        if (nScroll - m_scrollStart > DLauncher::MOUSE_MOVE_TO_NEXT)
            showCurrentPage(m_pageIndex + 1);
        else if (nScroll - m_scrollStart < -DLauncher::MOUSE_MOVE_TO_NEXT)
            showCurrentPage(m_pageIndex - 1);
        else
            showCurrentPage(m_pageIndex);
    }
    m_bMousePress = false;

    setGradientVisible(false);

    // 移动完成后，更新状态
    if ((m_pageIndex >= 0) && (m_pageIndex <= m_appGridViewList.size() - 1))
        m_appGridViewList[m_pageIndex]->setViewMoveState(false);
}

void MultiPagesView::setGradientVisible(bool visible)
{
    m_pLeftGradient->setVisible(visible);
    m_pRightGradient->setVisible(visible);
}

QPropertyAnimation::State MultiPagesView::getPageSwitchAnimationState()
{
    return m_pageSwitchAnimation->state();
}

QWidget *MultiPagesView::getParentWidget()
{
    // 找到背景widget
    QWidget *backgroundWidget = parentWidget();
    while (backgroundWidget) {
        if (qobject_cast<FullScreenFrame *>(backgroundWidget))
            break;

        backgroundWidget = backgroundWidget->parentWidget();
    }

    return backgroundWidget;
}

QPoint MultiPagesView::calculPadding(MultiPagesView::Direction dir)
{
    // 获取当前屏幕的高度
    QScreen *screen = QGuiApplication::primaryScreen();
    int screenHeight = screen->availableGeometry().height();

    // 计算过渡动画开始的位置 区分左右位置与上下位置
    int paddingL = m_calcUtil->getScreenSize().width() * DLauncher::SIDES_SPACE_SCALE;
    int paddingR = m_calcUtil->getScreenSize().width() - paddingL - 1;
    int topPos = (mapToGlobal(rect().topLeft()).y() > screenHeight) ?
                 (mapToGlobal(rect().topLeft()).y() - screenHeight) : mapToGlobal(rect().topLeft()).y();

    QPoint padding(((dir == MultiPagesView::Left) ? paddingL : paddingR), topPos);

    return padding;
}

AppListArea *MultiPagesView::getListArea()
{
    return m_appListArea;
}

AppGridViewList MultiPagesView::getAppGridViewList()
{
    return m_appGridViewList;
}

AppsListModel::AppCategory MultiPagesView::getCategory()
{
    return m_category;
}

QSize MultiPagesView::calculateWidgetSize()
{
    const AppGridViewList &viewList = m_appGridViewList;
    QSize itemSize = CalculateUtil::instance()->appItemSize() * 5 / 4;

    int leftMargin = CalculateUtil::instance()->appMarginLeft();
    int spacing = CalculateUtil::instance()->appItemSpacing();
    int itemWidth = itemSize.width();
    int itemHeight = itemSize.height();
    int viewWidth;
    int viewHeight = itemHeight * 3;
    if (viewList.size() > 1) {
        viewWidth = itemWidth * 4;
    } else {
        AppGridView *view = viewList.at(0);
        AppsListModel *listModel = qobject_cast<AppsListModel *>(view->model());

        if (!listModel)
            return QSize();

        int itemCount = listModel->rowCount();
        if (itemCount <= 3)
            viewWidth = itemWidth * 1 + leftMargin * 2;
        else if (itemCount <= 6)
            viewWidth = itemWidth * 2 + leftMargin * 2 + spacing * 1;
        else if (itemCount <= 9)
            viewWidth = itemWidth * 3 + leftMargin * 2 + spacing * 2;
        else {
            viewWidth = itemWidth * 4 + leftMargin * 2 + spacing * 3;
        }
    }

    return QSize(viewWidth, viewHeight);
}

// 更新边框渐变，在屏幕变化时需要更新，类别拖动时需要隐藏
void MultiPagesView::updateGradient()
{
    QWidget *backgroundWidget = getParentWidget();
    if (!backgroundWidget)
        return;

    QPixmap background = backgroundWidget->grab();
    updateGradient(background, calculPadding(MultiPagesView::Left), calculPadding(MultiPagesView::Right));
}

bool MultiPagesView::isScrolling()
{
    if (m_changePageDelayTime)
        return m_changePageDelayTime->isValid() && m_changePageDelayTime->elapsed() < DLauncher::CHANGE_PAGE_DELAY_TIME;

    return m_pageSwitchAnimation->state() == QPropertyAnimation::Running;
}

EditLabel *MultiPagesView::getEditLabel()
{
    return m_titleLabel;
}

void MultiPagesView::resetCurPageIndex()
{
    m_pageIndex = 0;
    m_appListArea->setHorizontalScrollValue(0);
}
