// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistview.h"
#include "constants.h"
#include "applistdelegate.h"
#include "appslistmodel.h"
#include "iteminfo.h"

#include <QStyleOptionViewItem>
#include <QPropertyAnimation>
#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>
#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QDebug>
#include <QDrag>
#include <QScroller>

/**
 * @brief AppListView::AppListView 启动器左侧小窗口列表
 * @param parent
 */
AppListView::AppListView(QWidget *parent)
    : DListView(parent)
    , m_dropThresholdTimer(new QTimer(this))
    , m_touchMoveFlag(false)
    , m_scrollAni(new QPropertyAnimation(verticalScrollBar(), "value", this))
    , m_updateEnableSelectionByMouseTimer(nullptr)
    , m_updateEnableShowSelectionByMouseTimer(nullptr)
{
    this->setAccessibleName("Form_AppList");
    viewport()->setAutoFillBackground(false);
    m_scrollAni->setEasingCurve(QEasingCurve::OutQuint);
    m_scrollAni->setDuration(800);

    horizontalScrollBar()->setEnabled(false);
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameStyle(NoFrame);
    setVerticalScrollMode(ScrollPerPixel);
    setSelectionMode(SingleSelection);
    setSpacing(0);
    setViewportMargins(0, 0, 11, 0);
    setMouseTracking(true);
    verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);

    // support drag and drop.
    setDragDropMode(QAbstractItemView::DragDrop);
    setMovement(QListView::Free);
    setDragEnabled(true);
    setDropIndicatorShown(false);// 解决拖拽释放前有小黑点出现的问题

    // init drop threshold timer.
    m_dropThresholdTimer->setInterval(DLauncher::APP_DRAG_SWAP_THRESHOLD);
    m_dropThresholdTimer->setSingleShot(true);

#ifndef DISABLE_DRAG_ANIMATION
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::prepareDropSwap, Qt::QueuedConnection);
#else
    connect(m_dropThresholdTimer, &QTimer::timeout, this, &AppListView::dropSwap);
#endif

    onThemeChanged(DGuiApplicationHelper::instance()->themeType());

    connect(m_scrollAni, &QPropertyAnimation::valueChanged, this, &AppListView::handleScrollValueChanged);
    connect(m_scrollAni, &QPropertyAnimation::finished, this, &AppListView::handleScrollFinished);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &AppListView::onThemeChanged);
}

const QModelIndex AppListView::indexAt(const int index) const
{
    return model()->index(index, 0, QModelIndex());
}

/**
 * @brief AppListView::wheelEvent 鼠标滑轮事件触发滑动区域控件动画
 * @param e 鼠标滑轮事件指针对象
 */
void AppListView::wheelEvent(QWheelEvent *e)
{
    // 解决蓝牙连接时触摸板斜对角方向双指按住滑动时滑条滚动异常问题
    if (e->orientation() == Qt::Horizontal)
        return;

    int offset = -e->delta();

    m_scrollAni->stop();
    m_scrollAni->setStartValue(verticalScrollBar()->value());
    m_scrollAni->setEndValue(verticalScrollBar()->value() + offset * m_speedTime);
    m_scrollAni->start();
}

void AppListView::mouseMoveEvent(QMouseEvent *e)
{
    if (e->source() == Qt::MouseEventSynthesizedByQt) {
        m_touchMoveFlag = true;

        // 在滑动时禁止拖拽
        if (QScroller::hasScroller(this)) {
            return;
        }

        // 拖拽时间少于200ms 禁止拖拽
        if (m_updateEnableSelectionByMouseTimer && m_updateEnableSelectionByMouseTimer->isActive()) {
            QListView::mouseMoveEvent(e);
            return;
        }
    } else {
        if (m_touchMoveFlag == false)
            emit requestEnter(true);
    }

    setState(NoState);
    blockSignals(false);

    const QModelIndex &index = indexAt(e->pos());
    const QPoint pos = e->pos();

    if (index.isValid() && !m_enableDropInside)
        Q_EMIT entered(index);
    else
        Q_EMIT entered(QModelIndex());

    if (e->buttons() != Qt::LeftButton)
        return;

    // 列表标题不允许被拖拽
    bool isTitle = index.data(AppsListModel::AppItemTitleRole).toBool();

    if (!isTitle && (qAbs(pos.x() - m_dragStartPos.x()) > DLauncher::DRAG_THRESHOLD ||
        qAbs(pos.y() - m_dragStartPos.y()) > DLauncher::DRAG_THRESHOLD)) {
        return startDrag(indexAt(m_dragStartPos));
    }

    QListView::mouseMoveEvent(e);
}

void AppListView::mousePressEvent(QMouseEvent *e)
{
    m_lastTouchBeginPos = e->pos();
    m_dragStartPos = e->pos();
    m_fullscreenStartPos = QCursor::pos();

    if (e->source() == Qt::MouseEventSynthesizedByQt) {
        emit requestEnter(false);
        m_scrollAni->stop();

        if (m_updateEnableShowSelectionByMouseTimer) {
            m_updateEnableShowSelectionByMouseTimer->stop();
        } else {
            m_updateEnableShowSelectionByMouseTimer = new QTimer(this);
            m_updateEnableShowSelectionByMouseTimer->setSingleShot(true);
            m_updateEnableShowSelectionByMouseTimer->setInterval(100);

            connect(m_updateEnableShowSelectionByMouseTimer, &QTimer::timeout, this, [=] {
                QPoint currentPos = QCursor::pos();
                int diff_x = qAbs(currentPos.x() - m_fullscreenStartPos.x());
                int diff_y = qAbs(currentPos.y() - m_fullscreenStartPos.y());
                if (diff_x < 5 && diff_y < 5) // 触摸按压抖动限制范围
                    emit requestEnter(true);
                    
                m_updateEnableShowSelectionByMouseTimer->deleteLater();
                m_updateEnableShowSelectionByMouseTimer = nullptr;
            });
        }
        m_updateEnableShowSelectionByMouseTimer->start();

        if (m_updateEnableSelectionByMouseTimer) {
            m_updateEnableSelectionByMouseTimer->stop();
        } else {
            m_updateEnableSelectionByMouseTimer = new QTimer(this);
            m_updateEnableSelectionByMouseTimer->setSingleShot(true);
            m_updateEnableSelectionByMouseTimer->setInterval(200); // 拖拽应用允许的最短时间

            connect(m_updateEnableSelectionByMouseTimer, &QTimer::timeout, this, [=] {
                m_updateEnableSelectionByMouseTimer->deleteLater();
                m_updateEnableSelectionByMouseTimer = nullptr;
            });
        }
        m_updateEnableSelectionByMouseTimer->start();
        QListView::mousePressEvent(e);
        return;
    }

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid())
        e->ignore();

    bool isTitle = index.data(AppsListModel::AppItemTitleRole).toBool();

    if (e->button() == Qt::RightButton && !isTitle) {
        const QPoint rightClickPoint = mapToGlobal(e->pos());
        const QModelIndex &clickedIndex = QListView::indexAt(e->pos());

        if (clickedIndex.isValid()) {
            qInfo() << "app listview popupMenuRequested emited....";
            emit popupMenuRequested(rightClickPoint, clickedIndex);
        }
    }

    if (e->button() == Qt::LeftButton)
        m_dragStartRow = indexAt(e->pos()).row();

    QListView::mousePressEvent(e);
}

void AppListView::mouseReleaseEvent(QMouseEvent *e)
{
    if (QScroller::hasScroller(this)) {
        QDBusInterface inPutInter("org.deepin.dde.InputDevices1",
                             "/org/deepin/dde/InputDevices1",
                             "org.deepin.dde.InputDevices1",
                             QDBusConnection::sessionBus() ,this);

        const auto wheelSpeed = inPutInter.property("WheelSpeed").toInt();
        int offset = m_lastTouchBeginPos.y() - e->pos().y();
        m_scrollAni->stop();
        m_scrollAni->setStartValue(verticalScrollBar()->value());
        m_scrollAni->setEndValue(verticalScrollBar()->value() + offset * wheelSpeed);
        m_scrollAni->start();
        QScroller::scroller(this)->deleteLater();
        return;
    }

    const QModelIndex &index = indexAt(e->pos());
    if (!index.isValid()){
        e->ignore();
        return;
    }

    QListView::mouseReleaseEvent(e);
}

/**
 * @brief AppListView::dragEnterEvent 在除搜索模式和无效的拖动模式下接收处理拖动进入事件
 * @param e 鼠标拖动进入事件指针对象
 */
void AppListView::dragEnterEvent(QDragEnterEvent *e)
{
    const QModelIndex index = indexAt(e->pos());

    if (model()->canDropMimeData(e->mimeData(), e->dropAction(), index.row(), index.column(), QModelIndex())) {
        return e->accept();
    }
}


void AppListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_lastFakeAni)
        return;

    const QModelIndex dropIndex = QListView::indexAt(e->pos());
    if (dropIndex.isValid())
        m_dropToRow = dropIndex.row();

    m_dropThresholdTimer->stop();

    const QPoint pos = e->pos();
    const QRect rect = this->rect();

    // 小窗口模式下 拖动item移动触发列表上下滑动
    if (pos.y() < DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollUp();
    } else if (pos.y() > rect.height() - DRAG_SCROLL_THRESHOLD) {
        Q_EMIT requestScrollDown();
    } else {
        Q_EMIT requestScrollStop();

        //窗口模式拖动交换位置
        //m_dropThresholdTimer->start();
    }

    // drag move does not allow to have selected effect.
    Q_EMIT entered(QModelIndex());
}

void AppListView::dragLeaveEvent(QDragLeaveEvent *e)
{
    e->accept();

    // drag leave will also restore opacity.
    m_dropThresholdTimer->stop();

    Q_EMIT requestScrollStop();
}

void AppListView::dropEvent(QDropEvent *e)
{
    e->accept();

    //窗口模式禁止拖动交换排序
    m_enableDropInside = false;
}

void AppListView::enterEvent(QEvent *event)
{
    QListView::enterEvent(event);

    // QScroller::hasScroller用于判断listview是否处于滑动状态，滑动状态不触发paint相关操作，否则滑动动画异常
    if (!QScroller::hasScroller(this)) {
        emit requestEnter(true);
    }
}

void AppListView::leaveEvent(QEvent *event)
{
    QListView::leaveEvent(event);

    if (!QScroller::hasScroller(this)) {
        emit requestEnter(false);
    }
}

/**
 * @brief AppListView::startDrag 当前页的拖动处理
 * @param index 拖动item的对应的模型索引
 */
void AppListView::startDrag(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    // 获取当前页视图列表对应的模型
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());

    const QModelIndex &dragIndex = index;

    const QPixmap dropPixmap = index.data(AppsListModel::AppDragIconRole).value<QPixmap>();
    QPoint hotSpot = dropPixmap.rect().center();
    QDrag *drag = new QDrag(this);
    drag->setMimeData(model()->mimeData(QModelIndexList() << dragIndex));
    drag->setPixmap(dropPixmap);
    drag->setHotSpot(hotSpot);

    // request remove current item.
    m_dropToRow = index.row();
    listModel->setDraggingIndex(index);

    setState(DraggingState);
    drag->exec(Qt::MoveAction);// 开启拖动item的子事件循环

    // disable animation when finally dropped
    m_dropThresholdTimer->stop();

    // disable auto scroll
    Q_EMIT requestScrollStop();


    // 小窗口不允许拖动交换位置, 重置模型索引,使dropSwap直接返回
    if (qobject_cast<AppsListModel*>(model())->category()) {
        listModel->clearDraggingIndex();
    }

    if (!m_lastFakeAni) {
        if (m_enableDropInside)
            listModel->dropSwap(m_dropToRow);
        else
            listModel->dropSwap(m_dragStartRow);

        listModel->clearDraggingIndex();
    } else {
        connect(m_lastFakeAni, &QPropertyAnimation::finished, listModel, &AppsListModel::clearDraggingIndex);
    }

    m_enableDropInside = false;
}

void AppListView::handleScrollValueChanged()
{
    if (m_touchMoveFlag)
        emit requestEnter(false);

    QScrollBar *vscroll = verticalScrollBar();

    if (vscroll->value() == vscroll->maximum() ||
        vscroll->value() == vscroll->minimum()) {
        blockSignals(false);
    } else {
        blockSignals(true);
    }
}

void AppListView::handleScrollFinished()
{
    blockSignals(false);
    if (m_touchMoveFlag) {
        emit requestEnter(false);
        m_touchMoveFlag = false;
    }
}

void AppListView::prepareDropSwap()
{
    if (m_lastFakeAni || m_dropThresholdTimer->isActive())
        return;

    const QModelIndex dropIndex = indexAt(m_dropToRow);
    if (!dropIndex.isValid())
        return;

    const QModelIndex dragStartIndex = indexAt(m_dragStartRow);
    if (dropIndex == dragStartIndex)
        return;

    if (!dragStartIndex.isValid()) {
        m_dragStartRow = dropIndex.row();
        return;
    }

    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());
    if (!listModel)
        return;

    listModel->clearDraggingIndex();
    listModel->setDraggingIndex(dragStartIndex);
    listModel->setDragDropIndex(dropIndex);

    const int startIndex = dragStartIndex.row();
    const bool moveToNext = startIndex <= m_dropToRow;
    const int start = moveToNext ? startIndex : m_dropToRow;
    const int end = !moveToNext ? startIndex : m_dropToRow;

    if (start == end)
        return;

    for (int i = start + moveToNext; i != end - !moveToNext; ++i) {
        createFakeAnimation(i, moveToNext);
    }

    // last animation.
    createFakeAnimation(end - !moveToNext, moveToNext, true);

    m_dragStartRow = dropIndex.row();
}

void AppListView::createFakeAnimation(const int pos, const bool moveNext, const bool isLastAni)
{
    const QModelIndex index(indexAt(pos));

    QLabel *floatLabel = new QLabel(this);
    QPropertyAnimation *animation = new QPropertyAnimation(floatLabel, "pos", floatLabel);

    const auto ratio = devicePixelRatioF();
    const QSize rectSize(300, 36);

    QPixmap pixmap(rectSize * ratio);
    pixmap.fill(Qt::transparent);
    pixmap.setDevicePixelRatio(ratio);

    QStyleOptionViewItem item;
    item.rect = QRect(QPoint(0, 0), rectSize);
    item.features |= QStyleOptionViewItem::HasDisplay;

    QPainter painter(&pixmap);
    itemDelegate()->paint(&painter, item, index);

    floatLabel->setFixedSize(rectSize);
    floatLabel->setPixmap(pixmap);
    floatLabel->show();

    animation->setStartValue(visualRect(index).topLeft());
    animation->setEndValue(visualRect(indexAt(moveNext ? pos - 1 : pos + 1)).topLeft());
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->setDuration(200);

    connect(animation, &QPropertyAnimation::finished, floatLabel, &QLabel::deleteLater);

    if (isLastAni) {
        m_lastFakeAni = animation;
        connect(animation, &QPropertyAnimation::finished, this, &AppListView::dropSwap, Qt::QueuedConnection);
        connect(animation, &QPropertyAnimation::valueChanged, m_dropThresholdTimer, &QTimer::stop);
    }

    animation->start();
}

void AppListView::dropSwap()
{
    AppsListModel *listModel = qobject_cast<AppsListModel *>(model());

    if (!listModel)
        return;

    listModel->dropSwap(m_dropToRow);

    m_lastFakeAni = nullptr;
    m_dragStartRow = m_dropToRow;

    setState(NoState);
}

void AppListView::menuHide()
{
    const QPoint pos = mapFromGlobal(QCursor::pos());
    const QModelIndex &index = indexAt(pos);
    if (index.isValid())
        Q_EMIT entered(index);
    else
        Q_EMIT entered(QModelIndex());
}

void AppListView::onThemeChanged(DGuiApplicationHelper::ColorType)
{
    update();
}
