// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listview.h"
#include "refreshtimer.h"
#include "constants.h"
#include "itemdata.h"
#include "itemwidget.h"
#include "clipboardmodel.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <QPropertyAnimation>
#include <QScroller>
#include <QDrag>
#include <QMimeData>

ListView::ListView(QWidget *parent)
    : QListView(parent)
    , m_mousePressed(false)
    , m_mimeData(nullptr)
{
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);

    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollMode(QListView::ScrollPerPixel);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(this);
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootWhenScrollable);
    scroller->setScrollerProperties(sp);
}

void ListView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up: {
        QModelIndex currentIndex = this->currentIndex();
        QModelIndex targetIndex = currentIndex.sibling(currentIndex.row() - 1, 0);
        if (!currentIndex.isValid() || !targetIndex.isValid()) {
            targetIndex = model()->index(model()->rowCount() - 1, 0);
        }
        setCurrentIndex(targetIndex);
        QListView::scrollTo(targetIndex);
    }
    break;
    case Qt::Key_Down: {
        QModelIndex currentIndex = this->currentIndex();
        QModelIndex targetIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
        if (!currentIndex.isValid() || !targetIndex.isValid()) {
            targetIndex = model()->index(0, 0);
        }
        setCurrentIndex(targetIndex);
        QListView::scrollTo(targetIndex);
    }
    break;
    default:
        QListView::keyPressEvent(event);
        break;
    }
}

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        setCurrentIndex(index);
    }

    if (!geometry().contains(event->pos()) && m_mousePressed) {
        m_mousePressed = false;
        if (m_mimeData) {
            QDrag *drag = new QDrag(this);
            drag->setMimeData(m_mimeData);
            drag->exec(Qt::CopyAction);
        }
    }

    return QListView::mouseMoveEvent(event);
}

void ListView::showEvent(QShowEvent *event)
{
    RefreshTimer::instance()->forceRefresh();
    activateWindow();

    QTimer::singleShot(0, this, [ = ] {
        scrollToTop();
        setCurrentIndex(model()->index(0, 0));
    });

    return QListView::showEvent(event);
}

void ListView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    Q_UNUSED(index)
    Q_UNUSED(hint)
}

void ListView::startAni(int index)
{
    grabMouse();
    for (int i = index + 1; i < this->model()->rowCount(QModelIndex()); ++i) {
        if(!CreateAnimation(i)) {
            const QModelIndex idx = this->model()->index(i, 0);
            QPointer<ItemData> data = idx.data().value<QPointer<ItemData>>();
            ItemWidget *item = new ItemWidget(data, this);
            item->resize(ItemWidth,ItemHeight);
            item->show();

            const QModelIndex prevIndex = this->model()->index(i - 1, 0);
            QWidget *prevWidget = this->indexWidget(prevIndex);
            QPoint endPos = prevWidget->pos();
            QPoint startPos = endPos + QPoint(0, ItemHeight + ItemMargin);

            QPropertyAnimation *ani = new QPropertyAnimation(item, "pos", this);
            ani->setStartValue(startPos);
            ani->setEndValue(endPos);
            ani->setDuration(AnimationTime);
            ani->start(QPropertyAnimation::DeleteWhenStopped);
            connect(ani, &QPropertyAnimation::finished, item, &ItemWidget::deleteLater);
            break;
        }
    }
    // FIXME:比动画时间稍微长一点，否则可能会造成mouseMoveEvent中崩溃
    QTimer::singleShot(AnimationTime + 10, this, [ = ] {
        releaseMouse();
        QModelIndex currentIndex = this->model()->index(index, 0);
        if (!currentIndex.isValid())
        {
            currentIndex = this->model()->index(index - 1, 0);
        }
        setCurrentIndex(currentIndex);
    });

    //TODO 显示的最下面一个widget应该是其下面的widget通过动画移动上来的，但其下面的widget此时有可能并没有被listview创建，这里需要一个假动画
}

bool ListView::CreateAnimation(int idx)
{
    Q_ASSERT(idx > 0);
    const QModelIndex index = this->model()->index(idx, 0);

    Q_ASSERT(index.isValid());
    QWidget *widget = this->indexWidget(index);
    if (!widget) {
        qDebug() << "index widget not created, should be returned;";
        return false;
    }

    QPropertyAnimation *ani = new QPropertyAnimation(widget, "pos", this);
    ani->setStartValue(widget->pos());
    ani->setEndValue(widget->pos() + QPoint(0, -210));
    ani->setDuration(AnimationTime);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
    return true;
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    QListView::mousePressEvent(event);

    QModelIndex dataIndex = currentIndex();
    if (!dataIndex.isValid())
        return;

    ClipboardModel *model = static_cast<ClipboardModel *>(this->model());
    ItemData *data = model->data().at(dataIndex.row());

    if (!m_mimeData)
        m_mimeData = new QMimeData;

    QMap<QString, QByteArray>::const_iterator itor = data->formatMap().constBegin();
    while (itor != data->formatMap().constEnd()) {
        m_mimeData->setData(itor.key(), itor.value());
        ++itor;
    }

    // 如果是触摸屏，当鼠标拖动到剪切板外部的时候才认为是拖拽行为，否则认为是滑动剪切板列表。
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        m_mousePressed = true;
    } else {
        QDrag *drag = new QDrag(this);
        drag->setMimeData(m_mimeData);
        drag->exec(Qt::CopyAction);
    }
}
