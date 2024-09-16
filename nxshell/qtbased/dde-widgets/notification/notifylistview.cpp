// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifylistview.h"
#include "notification/bubbletool.h"
#include "bubbleitem.h"
#include "overlapwidet.h"
#include "bubbletitlewidget.h"
#include "itemdelegate.h"
#include "notification/iconbutton.h"
#include "notification/button.h"

#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QPropertyAnimation>
#include <QDebug>
#include <QTimer>
#include <QKeyEvent>
#include <DIconButton>
#include <QDateTime>
#include <QScrollBar>
#include <QScroller>
#include <QTimer>
#include <QBasicTimer>

#define RefreshTime 900

// TODO: need to setFixedHeight Mini Height, other it's default contentSize is 150 px.
static const int ListViewMinHeight = 1;
NotifyListView::NotifyListView(QWidget *parent)
    : DListView(parent)
    , m_scrollAni(new QPropertyAnimation(verticalScrollBar(), "value" ,this))
    , m_refreshTimer(new QTimer(this))
    , m_layoutRequestTimer(new QBasicTimer())
{
//    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setAccessibleName("List_Notifications");
    m_scrollAni->setEasingCurve(QEasingCurve::OutQuint);
    m_scrollAni->setDuration(800);

    m_refreshTimer->setInterval(RefreshTime);

    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(m_refreshTimer, &QTimer::timeout, this, &NotifyListView::refreshItemTime);
    connect(m_scrollAni, &QPropertyAnimation::valueChanged, this, &NotifyListView::handleScrollValueChanged);
    connect(m_scrollAni, &QPropertyAnimation::finished, this, &NotifyListView::handleScrollFinished);

    QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(this);
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootAlwaysOff);
    scroller->setScrollerProperties(sp);
}

NotifyListView::~NotifyListView()
{
}

void NotifyListView::createRemoveAnimation(BubbleItem *item)
{
    QParallelAnimationGroup *removeAniGroup = new QParallelAnimationGroup(this);
    connect(removeAniGroup, &QParallelAnimationGroup::finished, this, [ = ] {
        m_aniState = false;
        Q_EMIT removeAniFinished(item->getEntity());
    });

    QPropertyAnimation *rightMoveAni = new QPropertyAnimation(item, "pos", this);
    rightMoveAni->setStartValue(item->pos());
    rightMoveAni->setEndValue(item->pos() + QPoint(OSD::BubbleWidth(OSD::ShowStyle::BUBBLEWIDGET), 0));
    rightMoveAni->setDuration(AnimationTime);
    removeAniGroup->addAnimation(rightMoveAni);

    const int bubbleItemHeight = BubbleItem::bubbleItemHeight();

    NotifyModel *notifyModel= qobject_cast<NotifyModel *> (model());
    ListItemPtr appGroup = notifyModel->getAppData(item->getEntity()->appName());

    int moveValue = -bubbleItemHeight - BubbleSpacing;
    if (appGroup->hideCount() > 0 && appGroup->showCount() == 1) {
        moveValue = 0;
    }
    if (appGroup->hideCount() > 0 && canShow(appGroup->hideFirst())) {
        moveValue = 0;
    }
    for (int i = item->indexRow() + 1; i < this->model()->rowCount(QModelIndex()); ++i) {
        const QModelIndex index = this->model()->index(i, 0);
        QWidget *widget = this->indexWidget(index);
        if (!widget) {
            break;
        }
        QPropertyAnimation *topMoveAni = new QPropertyAnimation(widget, "pos", this);
        topMoveAni->setStartValue(widget->pos());
        topMoveAni->setEndValue(widget->pos() + QPoint(0, moveValue));
        topMoveAni->setDuration(AnimationTime);
        removeAniGroup->addAnimation(topMoveAni);
    }

    removeAniGroup->start(QPropertyAnimation::DeleteWhenStopped);
    m_aniState = true;
}

void NotifyListView::createExpandAnimation(int idx, const ListItemPtr appItem)
{
    // 正在进行动画
    if (m_aniState)
        return;

    // 获取动画的起始位置
    const QModelIndex &index = this->model()->index(idx, 0);
    QWidget *currentWidget = this->indexWidget(index);
    if (!currentWidget)
        return;

    QPoint startPos = currentWidget->pos();
    const int bubbleItemHight = BubbleItem::bubbleItemHeight();
    int maxCount = (height() - startPos.y()) / (bubbleItemHight + BubbleSpacing);
    int needCount = appItem->hideCount() > maxCount ? maxCount : appItem->hideCount();

    // 当不需要插入时，不进行动画，当QSequentialAnimationGroup的动画个数为0时而start后，不会触发finished.
    if (needCount <= 0)
        return;

    QSequentialAnimationGroup *insertAniGroup = new QSequentialAnimationGroup(this);    // 逐个插入的串行动画
    QParallelAnimationGroup *downMoveAniGroup = new QParallelAnimationGroup(this);      // 整体向下移动的动画

    connect(insertAniGroup, &QSequentialAnimationGroup::finished, this, [ = ] {
        m_aniState = false;
        Q_EMIT expandAniFinished(appItem->appName());
    });

    for (int i = 0; i < needCount; i++) {
        BubbleItem *item = new BubbleItem(this, appItem->hideAt(i));
        item->setAccessibleName("BubbleItem");
        QPoint itemStartPos = startPos + QPoint(0, (bubbleItemHight + BubbleSpacing) * i);
        QPoint itemEndPos = itemStartPos + QPoint(0, bubbleItemHight + BubbleSpacing);
        QPropertyAnimation *ani = new QPropertyAnimation(item, "pos", this);
        connect(ani, &QPropertyAnimation::stateChanged, item, [ = ] (QAbstractAnimation::State newState, QAbstractAnimation::State oldState) {
            if (newState == QAbstractAnimation::Running && oldState == QAbstractAnimation::Stopped) {
                item->move(startPos);
                item->show();
            }
        });
        connect(insertAniGroup, &QSequentialAnimationGroup::finished, item, &BubbleItem::deleteLater);
        ani->setStartValue(itemStartPos);
        ani->setEndValue(itemEndPos);
        ani->setDuration(ExpandAnimationTime);
        insertAniGroup->addAnimation(ani);
    }

    QPoint offsetPos = QPoint(0, (bubbleItemHight + BubbleSpacing) * needCount - 22);
    // 向下移动的动画
    for (int i = idx; i < this->model()->rowCount(QModelIndex()); ++i) {
        QWidget *moveDownWidget = this->indexWidget(this->model()->index(i + 1, 0));
        if (!moveDownWidget) {
            break;
        }
        QPropertyAnimation *ani = new QPropertyAnimation(moveDownWidget, "pos", this);
        ani->setStartValue(moveDownWidget->pos());
        ani->setEndValue(moveDownWidget->pos() + offsetPos);
        ani->setDuration(ExpandAnimationTime * needCount);
        downMoveAniGroup->addAnimation(ani);
    }
    insertAniGroup->start(QPropertyAnimation::DeleteWhenStopped);
    downMoveAniGroup->start(QPropertyAnimation::DeleteWhenStopped);
    m_aniState = true;
}

void NotifyListView::createAddedAnimation(EntityPtr entity, const ListItemPtr appItem)
{
    // 正在进行动画
    if (m_aniState)
        return;

    NotifyModel *notifyModel= qobject_cast<NotifyModel *> (model());
    const int firstNotificationRow = notifyModel->rowOfFirstNotification();
    const QModelIndex &index = this->model()->index(firstNotificationRow, 0);
    QWidget *currentWidget = this->indexWidget(index);
    if (!currentWidget) {
        qWarning() << "createAddedAnimation() it maybe lost notification:" << entity->appName() << entity->id();
        return;
    }

    QParallelAnimationGroup *addedAniGroup = new QParallelAnimationGroup(this);
    connect(addedAniGroup, &QParallelAnimationGroup::finished, this, [ = ] { //动画完成发出动画完成的信号
        m_aniState = false;
        Q_EMIT addedAniFinished(entity);
    });

    QPoint startPos = currentWidget->pos();    //动画基准位置
    const int bubbleItemHight = BubbleItem::bubbleItemHeight();

    BubbleItem *newItem = new BubbleItem(this, entity);
    newItem->setAccessibleName("BubbleItem");
    newItem->show();
    QRect startRect = QRect(startPos, QSize(currentWidget->width(), 0));
    QRect endRect = QRect(startPos, QSize(currentWidget->width(), currentWidget->height()));
    QPropertyAnimation *addAni = new QPropertyAnimation(newItem, "geometry", this);
    connect(addAni, &QPropertyAnimation::finished, newItem, [ = ] {
        newItem->deleteLater();
    });
    addAni->setStartValue(startRect);
    addAni->setEndValue(endRect);
    addAni->setDuration(AnimationTime);
    addedAniGroup->addAnimation(addAni);

    if (!appItem->isCollapse() && canShow(appItem->showLast())) {
        for (int i = firstNotificationRow; i < this->model()->rowCount(QModelIndex()); ++i) {
            QWidget *widget = this->indexWidget(this->model()->index(i, 0));
            if (!widget) {
                break;
            }
            QPropertyAnimation *downMoveAni = new QPropertyAnimation(widget, "pos", this);
            downMoveAni->setStartValue(widget->pos());
            downMoveAni->setEndValue(widget->pos() + QPoint(0, bubbleItemHight + BubbleSpacing));
            downMoveAni->setDuration(AnimationTime);
            addedAniGroup->addAnimation(downMoveAni);
        }
    } else {
        for (int i = 0; i < appItem->showCount(); i++) {
            if (!canShow(appItem->at(i))) {
                break;
            }
            QWidget *widget = this->indexWidget(this->model()->index(i, 0));
            QPropertyAnimation *downMoveAni = new QPropertyAnimation(widget, "pos", this);
            downMoveAni->setStartValue(widget->pos());
            downMoveAni->setEndValue(widget->pos() + QPoint(0, bubbleItemHight + BubbleSpacing));
            downMoveAni->setDuration(AnimationTime);
            addedAniGroup->addAnimation(downMoveAni);
        }
        QWidget *lastWidget = this->indexWidget(this->model()->index(appItem->showLastRow(), 0));
        OverLapWidet *overLapWidget = qobject_cast<OverLapWidet *> (lastWidget);
        QWidget *faceWidget = nullptr;
        if (overLapWidget != nullptr) {
            faceWidget = overLapWidget->getFaceItem();
        } else {
            faceWidget = qobject_cast<BubbleItem *> (lastWidget);
        }

        if (!faceWidget) {
            qWarning() << "createAddedAnimation() error for the notify:" << appItem->appName()
                       << ", faceWidget is null, and it is illegal.";
            return;
        }

        QRect startRect1 = QRect(faceWidget->pos(), faceWidget->size());
        QRect endRect1 = QRect(startRect1.x(), startRect1.y() + bubbleItemHight, startRect.width(), 0);
        QPropertyAnimation *removeAni = new QPropertyAnimation(faceWidget, "geometry", this);
        removeAni->setStartValue(startRect1);
        removeAni->setEndValue(endRect1);
        removeAni->setDuration(AnimationTime);
        addedAniGroup->addAnimation(removeAni);
    }
    addedAniGroup->start(QPropertyAnimation::DeleteWhenStopped);
    m_aniState = true;
}

void NotifyListView::setCurrentRow(int row)
{
    m_currentIndex = row;
}

QWidget *NotifyListView::lastItemView() const
{
    if (auto delegate = qobject_cast<ItemDelegate *>(itemDelegate()))
        return delegate->lastItemView();

    return nullptr;
}

void NotifyListView::mousePressEvent(QMouseEvent *event)
{
    if (m_aniState)
        return;
    return QListView::mousePressEvent(event);
}

void NotifyListView::showEvent(QShowEvent *event)
{
    m_refreshTimer->start();

    return QListView::showEvent(event);
}

void NotifyListView::hideEvent(QHideEvent *event)
{
    m_currentIndex = 0;
    m_currentElement = nullptr;
    m_prevElement = nullptr;
    verticalScrollBar()->setValue(0);
    m_refreshTimer->stop();

    return QListView::hideEvent(event);
}

void NotifyListView::wheelEvent(QWheelEvent *event)
{
    if (m_aniState)
        return;

    int offset = -event->delta();

    m_scrollAni->stop();
    m_scrollAni->setStartValue(verticalScrollBar()->value());
    m_scrollAni->setEndValue(verticalScrollBar()->value() + offset * m_speedTime);
    m_scrollAni->start();
    return QWidget::wheelEvent(event);
}

bool NotifyListView::canShow(EntityPtr ptr)
{
    QDateTime t = QDateTime::fromMSecsSinceEpoch(ptr->ctime().toLongLong());
    return t.secsTo(QDateTime::currentDateTime()) < OVERLAPTIMEOUT_4_HOUR;
}

void NotifyListView::handleScrollValueChanged()
{
    QScrollBar *vscroll = verticalScrollBar();

    if (vscroll->value() == vscroll->maximum() ||
        vscroll->value() == vscroll->minimum()) {
        blockSignals(false);
    } else {
        blockSignals(true);
    }
}

void NotifyListView::handleScrollFinished()
{
    blockSignals(false);

    QPoint pos = mapFromGlobal(QCursor::pos());
    emit entered(indexAt(pos));
}

void NotifyListView::updateViewHeight()
{
    if (model() && model()->rowCount() <= 0) {
        setFixedHeight(ListViewMinHeight);
    } else {
        setFixedHeight(contentsSize().height());
    }
}

bool NotifyListView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Polish:
        setFixedHeight(ListViewMinHeight);
        break;
    case QEvent::LayoutRequest: {
        if (!m_layoutRequestTimer->isActive())
            m_layoutRequestTimer->start(0, this);
    } break;
    default:
        break;
    }
    return DListView::event(event);
}

void NotifyListView::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_layoutRequestTimer->timerId()) {
        updateViewHeight();
        m_layoutRequestTimer->stop();
        e->accept();
    }
    return DListView::timerEvent(e);
}
