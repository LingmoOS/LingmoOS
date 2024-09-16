// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifylistview.h"
#include "notification/bubbletool.h"
#include "bubbleitem.h"
#include "overlapwidet.h"
#include "bubbletitlewidget.h"
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

#define RefreshTime 900

NotifyListView::NotifyListView(QWidget *parent)
    : DListView(parent)
    , m_scrollAni(new QPropertyAnimation(verticalScrollBar(), "value" ,this))
    , m_refreshTimer(new QTimer(this))
{
    qApp->installEventFilter(this);
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
    ListItem appGroup = notifyModel->getAppData(item->getEntity()->appName());

    int moveValue = -bubbleItemHeight - BubbleSpacing;
    if (!appGroup.hideList.isEmpty() && appGroup.showList.size() == 1) {
        moveValue = 0;
    }
    if (!appGroup.hideList.isEmpty() && canShow(appGroup.hideList.first())) {
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

void NotifyListView::createExpandAnimation(int idx, const ListItem appItem)
{
    // 获取动画的起始位置
    const QModelIndex &index = this->model()->index(idx, 0);
    QWidget *currentWidget = this->indexWidget(index);
    if (!currentWidget)
        return;

    QSequentialAnimationGroup *insertAniGroup = new QSequentialAnimationGroup(this);    // 逐个插入的串行动画
    QParallelAnimationGroup *downMoveAniGroup = new QParallelAnimationGroup(this);      // 整体向下移动的动画

    connect(insertAniGroup, &QParallelAnimationGroup::finished, this, [ = ] {
        m_aniState = false;
        Q_EMIT expandAniFinished(appItem.appName);
    });

    QPoint startPos = currentWidget->pos();
    const int bubbleItemHight = BubbleItem::bubbleItemHeight();
    int maxCount = (height() - startPos.y()) / (bubbleItemHight + BubbleSpacing);
    int needCount = appItem.hideList.size() > maxCount ? maxCount : appItem.hideList.size();

    for (int i = 0; i < needCount; i++) {
        BubbleItem *item = new BubbleItem(this, appItem.hideList[i]);
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
        connect(insertAniGroup, &QParallelAnimationGroup::finished, item, &BubbleItem::deleteLater);
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

void NotifyListView::createAddedAnimation(EntityPtr entity, const ListItem appItem)
{
    const QModelIndex &index = this->model()->index(1, 0);
    QWidget *currentWidget = this->indexWidget(index);
    if (!currentWidget)
        return;

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

    if (appItem.showList.size() != 3 && canShow(appItem.showList.last())) {
        for (int i = 1; i < this->model()->rowCount(QModelIndex()); ++i) {
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
        for (int i = 0; i < appItem.showList.size(); i++) {
            if (i > 1 || !canShow(appItem.showList[i])) {
                break;
            }
            QWidget *widget = this->indexWidget(this->model()->index(1 + i, 0));
            QPropertyAnimation *downMoveAni = new QPropertyAnimation(widget, "pos", this);
            downMoveAni->setStartValue(widget->pos());
            downMoveAni->setEndValue(widget->pos() + QPoint(0, bubbleItemHight + BubbleSpacing));
            downMoveAni->setDuration(AnimationTime);
            addedAniGroup->addAnimation(downMoveAni);
        }
        QWidget *lastWidget = this->indexWidget(this->model()->index(appItem.showList.size(), 0));
        OverLapWidet *overLapWidget = qobject_cast<OverLapWidet *> (lastWidget);
        QWidget *faceWidget = nullptr;
        if (overLapWidget != nullptr) {
            faceWidget = overLapWidget->getFaceItem();
        } else {
            faceWidget = qobject_cast<BubbleItem *> (lastWidget);
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

void NotifyListView::mousePressEvent(QMouseEvent *event)
{
    if (m_aniState)
        return;
    return QListView::mousePressEvent(event);
}

void NotifyListView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QModelIndex index = this->model()->index(m_currentIndex, 0);
        QWidget *widget = this->indexWidget(index);

        if (qobject_cast<DIconButton *> (m_prevElement) != nullptr) {
            DIconButton *itemCloseBtn = qobject_cast<DIconButton *> (m_prevElement);
            itemCloseBtn->clicked();
            m_currentIndex --;
        } else if (qobject_cast<Button *> (m_prevElement) != nullptr) {
            Button *actionBtn = qobject_cast<Button *> (m_prevElement);
            actionBtn->clicked();
            m_currentIndex --;
        } else {
            OverLapWidet *overLabWidget = qobject_cast<OverLapWidet *> (widget);
            if (overLabWidget != nullptr) {
                overLabWidget->expandAppGroup();
            }
        }

    }
    return QListView::keyPressEvent(event);
}

bool NotifyListView::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = static_cast<QKeyEvent *>(event);
        if (key->key() == Qt::Key_Tab) {
            if (m_aniState)
                return true;
            return tabKeyEvent(object, key);
        } else if (key->key() == Qt::Key_Down) {
            return true;
        } else if (key->key() == Qt::Key_Up) {
            return true;
        } else if (key->key() == Qt::Key_Backtab){
            m_currentIndex --;
            if (m_currentIndex < 0) {
                m_currentIndex = 0;
            }
            QModelIndex index = this->model()->index(m_currentIndex, 0);
            setCurrentIndex(index);
            return true;
        }
    }
    return QListView::eventFilter(object, event);
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

bool NotifyListView::tabKeyEvent(QObject *object, QKeyEvent *event)
{
    Q_UNUSED(object)
    Q_UNUSED(event)
    if (m_currentIndex >= model()->rowCount()) {
        m_currentIndex = 0;
        Q_EMIT focusOnButton();
        return true;
    }
    QModelIndex index = this->model()->index(m_currentIndex, 0);
    QWidget *widget = this->indexWidget(index);
    scrollTo(index);
    if (widget == nullptr)
        return true;
    // 更新按钮列表
    widget->setFocus();
    QList<QPointer<QWidget>> elements;
    if (qobject_cast<BubbleTitleWidget *> (widget) != nullptr) {
        m_currentIndex ++;
        index = this->model()->index(m_currentIndex, 0);
        widget = this->indexWidget(index);
    }
    setCurrentIndex(index);
    m_prevElement = nullptr;
    if (qobject_cast<BubbleItem *> (widget) != nullptr) {
        BubbleItem *itemWidget = qobject_cast<BubbleItem *> (widget);
        elements = itemWidget->bubbleElements();
    } else if (qobject_cast<OverLapWidet *> (widget) != nullptr) {
        OverLapWidet *overLabWidget = qobject_cast<OverLapWidet *> (widget);
        elements = overLabWidget->bubbleElements();
        overLabWidget->setFocus();
    }
    if (m_currentElement == nullptr && !elements.isEmpty()) {
        m_currentElement = elements.first();
    } else {
        if (qobject_cast<DIconButton *> (m_currentElement) != nullptr) {
            DIconButton *itemCloseBtn = qobject_cast<DIconButton *> (m_currentElement);
            QFocusEvent event(QEvent::Enter, Qt::MouseFocusReason);
            QApplication::sendEvent (itemCloseBtn, &event);
        } else if (qobject_cast<Button *> (m_currentElement) != nullptr) {
            Button *actionBtn = qobject_cast<Button *> (m_currentElement);
            actionBtn->setFocus();
        }
        m_prevElement = m_currentElement;
        const int last_pos = elements.indexOf(m_currentElement) + 1;
        if (last_pos < elements.size()) {
            m_currentElement = elements.at(last_pos);
        } else { // 到item最后一个按钮
            m_currentElement = nullptr;
            m_currentIndex ++;
        }
    }
    return true;
}

void NotifyListView::wheelEvent(QWheelEvent *event)
{
    if (m_aniState)
        return;

    QPoint offset_point = - event->angleDelta();
    int offset = std::abs(offset_point.x()) > std::abs(offset_point.y()) ? offset_point.x() : offset_point.y();

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
