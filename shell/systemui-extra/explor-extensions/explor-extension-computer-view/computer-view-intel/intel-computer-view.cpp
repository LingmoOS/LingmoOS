/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "intel-computer-view.h"
#include "intel-computer-proxy-model.h"
#include "intel-abstract-computer-item.h"
#include "intel-computer-item-delegate.h"
#include "intel-computer-view-style.h"

#include <QScrollBar>
#include <QPainter>

#include <QMouseEvent>
#include <QRubberBand>

#include <QDebug>

using namespace Intel;

ComputerView::ComputerView(QWidget *parent) : QAbstractItemView(parent)
{
    this->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    setAttribute(Qt::WA_TranslucentBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);

    setItemDelegate(new ComputerItemDelegate(this));

    m_model = ComputerProxyModel::globalInstance();
    setModel(m_model);
    setStyle(ComputerViewStyle::getStyle());

    setStyle(ComputerViewStyle::getStyle());

    m_rubberBand = new QRubberBand(QRubberBand::Shape::Rectangle, this);

    connect(this, &QAbstractItemView::doubleClicked, this, [=](const QModelIndex &index){
        qDebug()<<index.data()<<"double clicked";
    });

    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, [=](){
        this->viewport()->update();
    });

    //fix #18184
    auto volumeManager = Peony::VolumeManager::getInstance();
    connect(volumeManager,&Peony::VolumeManager::volumeRemoved,this,[=](const std::shared_ptr<Peony::Volume> volume){
        this->viewport()->update();
    });

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->viewport()->setMouseTracking(true);
    this->viewport()->installEventFilter(this);

//    for (int row = 0; row < m_model->rowCount(); row++) {
//        m_items.push_back(ComputerViewItem(row, 0, false));
//    }
}

bool ComputerView::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        if (!m_isLeftButtonPressed) {
            auto pos = mapFromGlobal(QCursor::pos());
            auto newIndex = indexAt(pos);
            if (newIndex != m_hoverIndex) {
                m_hoverIndex = newIndex;
                this->viewport()->update();
            }
        } else {
            m_hoverIndex = QModelIndex();
        }
    }
    return false;
}

QRect ComputerView::visualRect(const QModelIndex &index) const
{
    return m_rect_cache.value(index);
}

void ComputerView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    //return;
    //FIXME: scroll to the index more accurecely.
    //auto y = m_rect_cache.value(index).y();
    //verticalScrollBar()->setValue(y);
}

QModelIndex ComputerView::indexAt(const QPoint &point) const
{
    auto pos = point + QPoint(horizontalOffset(), verticalOffset());
    for (auto index : m_rect_cache.keys()) {
        auto rect = m_rect_cache.value(index);
        if (rect.contains(pos))
            return index;
    }
    return QModelIndex();
}

QModelIndex ComputerView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    return QModelIndex();
}

int ComputerView::horizontalOffset() const
{
    return horizontalScrollBar()->value()*m_scrollStep;
}

int ComputerView::verticalOffset() const
{
    return verticalScrollBar()->value()*m_scrollStep;
}

bool ComputerView::isIndexHidden(const QModelIndex &index) const
{
    return false;
}

void ComputerView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    //caculate logic rubber band rect in mouse move event.
    //qDebug()<<"setSelection"<<rect<<command;

    if (m_rubberBand->isVisible()) {
        auto realRect = m_rubberBand->geometry();
        realRect.adjust(horizontalOffset(), verticalOffset(), horizontalOffset(), verticalOffset());

        for (auto index : m_rect_cache.keys()) {
            auto indexRect = m_rect_cache.value(index);
            if (realRect.contains(indexRect.center())) {
                selectionModel()->select(index, QItemSelectionModel::Select);
            } else {
                selectionModel()->select(index, QItemSelectionModel::Deselect);
            }
        }
    } else {
        auto pos = rect.center();
        auto index = indexAt(pos);
        if (!index.isValid()) {
            clearSelection();
            return;
        }
        if (!selectedIndexes().contains(index))
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
    }
}

QRegion ComputerView::visualRegionForSelection(const QItemSelection &selection) const
{
    return QRegion();
}

QString ComputerView::tryGetVolumeUriFromMountTarget(const QString &mountTargetUri)
{
    return m_model->tryGetVolumeUriFromMountTarget(mountTargetUri);
}

void ComputerView::refresh()
{
    m_model->refresh();
}

void ComputerView::updateEditorGeometries()
{
    QAbstractItemView::updateEditorGeometries();

    m_totalHeight = 0;
    m_totalWidth = this->viewport()->width();

    m_rect_cache.clear();
    for (int row = 0; row < m_model->rowCount(); row++) {
        auto index = m_model->index(row, 0);
        auto item = m_model->itemFromIndex(index);
        switch (item->itemType()) {
        case AbstractComputerItem::Volume: {
            layoutVolumeIndexes(index);
            break;
        }
        case AbstractComputerItem::RemoteVolume: {
            layoutRemoteIndexes(index);
            break;
        }
        case AbstractComputerItem::Network: {
            layoutNetworkIndexes(index);
            break;
        }
        default:
            break;
        }
    }

    //confirm total width
    for (auto rect : m_rect_cache.values()) {
        if (rect.right() > m_totalWidth)
            m_totalWidth = rect.right();
    }

    horizontalScrollBar()->setRange(0, qMax(0, m_totalWidth - viewport()->width()));
    verticalScrollBar()->setRange(0, qMax(0, (m_totalHeight - viewport()->height() + 200)/m_scrollStep));

    //update tab index rect width
    for (auto index : m_rect_cache.keys()) {
        if (!index.parent().isValid()) {
            auto rect = m_rect_cache.value(index);
            rect.setWidth(m_totalWidth);
            m_rect_cache.remove(index);
            m_rect_cache.insert(index, rect);
        }
    }
}

void ComputerView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    updateEditorGeometries();
}

void ComputerView::paintEvent(QPaintEvent *e)
{
    QPainter p(this->viewport());
//    p.fillRect(this->rect(), palette().base());
    //p.fillRect(QRect(0, 0, m_totalWidth, m_totalHeight), Qt::blue);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::transparent);
    p.setBrush(this->palette().base());
    p.drawRoundedRect(this->rect(),24,24, Qt::AbsoluteSize);
    p.drawRect(0, 0, 24, 24);
    p.drawRect(0, rect().height() - 24, 24, 24);
    p.drawRect(rect().width() - 24, 0, 24, 24);
    auto w = window();
    QPoint l = this->mapTo(w,QPoint(this->rect().right(),this->rect().height()));
    if(w->rect().right()-l.x()>100||window()->isMaximized()){
        p.drawRect(rect().width()-24,rect().height()-24,24,24);
    }
    p.save();
    p.translate(-horizontalOffset(), -verticalOffset());

    for (auto rect : m_rect_cache) {
        //qDebug()<<rect;
        auto opt = viewOptions();
        auto index = m_rect_cache.key(rect);
        if (selectedIndexes().contains(index)) {
//            opt.state.setFlag(QStyle::State_Selected);
            opt.state |= QStyle::State_Selected;
        }
        //p.drawText(rect.center(), QString::number(m_rect_cache.key(rect).row()));
        if (index == m_hoverIndex) {
//            opt.state.setFlag(QStyle::State_MouseOver);
            opt.state |= QStyle::State_MouseOver;
        }

        opt.rect = rect;
        opt.icon = QIcon::fromTheme("folder");
        itemDelegate()->paint(&p, opt, index);
    }
    p.restore();
}

void ComputerView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isLeftButtonPressed = true;
        m_rubberBand->hide();
        m_lastPressedPoint = event->pos();
        m_lastPressedLogicPoint = event->pos() + QPoint(horizontalOffset(), verticalOffset());

        auto index = indexAt(event->pos());
        if (!index.parent().isValid() && index.isValid()) {
            if (isExpanded(index)) {
                expand(index, false);
            }
            else {
                expand(index, true);
            }
            updateEditorGeometries();
            this->viewport()->update();
            return;
        }
    }
    else
        m_rubberBand->hide();

    QAbstractItemView::mousePressEvent(event);

    this->viewport()->update();
}

void ComputerView::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractItemView::mouseMoveEvent(event);
    if (m_isLeftButtonPressed) {
        auto pos = event->pos();
        auto offset = QPoint(horizontalOffset(), verticalOffset());
        auto logicPos = pos + offset;
        QRect logicRect = QRect(logicPos, m_lastPressedLogicPoint);
        m_logicRect = logicRect.normalized();

        int dx = -horizontalOffset();
        int dy = -verticalOffset();
        auto realRect = m_logicRect.adjusted(dx, dy, dx ,dy);
        if (!m_rubberBand->isVisible())
            m_rubberBand->show();
        m_rubberBand->setGeometry(realRect);
    } else {
        m_rubberBand->hide();
    }
}

void ComputerView::mouseReleaseEvent(QMouseEvent *event)
{
    m_rubberBand->hide();
    m_isLeftButtonPressed = false;
    QAbstractItemView::mouseReleaseEvent(event);
}

void ComputerView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
    this->viewport()->update();
}

void ComputerView::layoutVolumeIndexes(const QModelIndex &volumeParentIndex)
{
    int rowCount = m_model->rowCount(volumeParentIndex);
    if (rowCount <= 0)
        return;

    //add tab
    m_rect_cache.insert(volumeParentIndex, QRect(QPoint(0, m_totalHeight), QSize(this->viewport()->width(), m_tabPadding)));
    m_totalHeight += m_tabPadding + 10;

    if (!isExpanded(volumeParentIndex))
        return;

    //layout indexes
    int tmpX = 0;

    int maxColumnCount = 0;
    //if view's width is not enough, expand the m_totalWidth to ensure scroallble
    if (m_totalWidth < 2 * (m_hSpacing + m_volumeItemFixedSize.width())) {
        maxColumnCount = 1;
    } else {
        maxColumnCount = this->viewport()->width()/(m_hSpacing + m_volumeItemFixedSize.width());// - 1;
    }

    int cloumn = 0;
    for (int row = 0; row < rowCount; row++) {
        //layout next row
        if (cloumn > maxColumnCount - 1) {
            //m_totalWidth = tmpX + m_hSpacing + m_volumeItemFixedSize.width();
            cloumn = 1;
            tmpX = m_hSpacing;
            m_totalHeight = m_totalHeight + m_volumeItemFixedSize.height() + m_vSpacing;
        } else {
            tmpX = m_hSpacing + cloumn*(m_hSpacing+m_volumeItemFixedSize.width());
            cloumn++;
        }
        auto index = m_model->index(row, 0, volumeParentIndex);
        m_rect_cache.insert(index, QRect(QPoint(tmpX, m_totalHeight), m_volumeItemFixedSize));
    }

    m_totalHeight = m_totalHeight + m_volumeItemFixedSize.height() + m_vSpacing;
}

void ComputerView::layoutRemoteIndexes(const QModelIndex &remoteParentIndex)
{
    int rowCount = m_model->rowCount(remoteParentIndex);
    if (rowCount <= 0)
        return;

    //add tab
    m_rect_cache.insert(remoteParentIndex, QRect(QPoint(0, m_totalHeight), QSize(this->viewport()->width(), m_tabPadding)));
    m_totalHeight += m_tabPadding + 10;

    if (!isExpanded(remoteParentIndex))
        return;

    //layout indexes
    int tmpX = 0;

    int maxColumnCount = 0;
    //if view's width is not enough, expand the m_totalWidth to ensure scroallble
    if (m_totalWidth < 2 * (m_hSpacing + m_remoteItemFixedSize.width())) {
        //m_totalWidth = m_hSpacing + m_remoteItemFixedSize.width();
        maxColumnCount = 1;
    } else {
        maxColumnCount = this->viewport()->width()/(m_hSpacing + m_remoteItemFixedSize.width());// - 1;
    }

    int cloumn = 0;
    for (int row = 0; row < rowCount; row++) {
        //layout next row
        if (cloumn > maxColumnCount - 1) {
            //m_totalWidth = tmpX + m_hSpacing + m_volumeItemFixedSize.width();
            cloumn = 1;
            tmpX = m_hSpacing;
            m_totalHeight = m_totalHeight + m_remoteItemFixedSize.height() + m_vSpacing;
        } else {
            tmpX = m_hSpacing + cloumn*(m_hSpacing+m_remoteItemFixedSize.width());
            cloumn++;
        }
        auto index = m_model->index(row, 0, remoteParentIndex);
        m_rect_cache.insert(index, QRect(QPoint(tmpX, m_totalHeight), m_remoteItemFixedSize));
    }

    m_totalHeight = m_totalHeight + m_remoteItemFixedSize.height() + m_vSpacing;
}

void ComputerView::layoutNetworkIndexes(const QModelIndex &networkParentIndex)
{
    int rowCount = m_model->rowCount(networkParentIndex);
    if (rowCount <= 0)
        return;

    //add tab
    m_rect_cache.insert(networkParentIndex, QRect(QPoint(0, m_totalHeight), QSize(this->viewport()->width(), m_tabPadding)));
    m_totalHeight += m_tabPadding + 10;

    if(!isExpanded(networkParentIndex))
        return;

    //layout indexes
    int tmpX = 0;

    int maxColumnCount = 0;
    //if view's width is not enough, expand the m_totalWidth to ensure scroallble
    if (m_totalWidth < 2 * (m_hSpacing + m_networkItemFixedSize.width())) {
        maxColumnCount = 1;
    } else {
        maxColumnCount = this->viewport()->width()/(m_hSpacing + m_networkItemFixedSize.width());// - 1;
    }

    int cloumn = 0;
    for (int row = 0; row < rowCount; row++) {
        //layout next row
        if (cloumn > maxColumnCount - 1) {
            //m_totalWidth = tmpX + m_hSpacing + m_volumeItemFixedSize.width();
            cloumn = 1;
            tmpX = m_hSpacing;
            m_totalHeight = m_totalHeight + m_networkItemFixedSize.height() + m_vSpacing;
        } else {
            tmpX = m_hSpacing + cloumn*(m_hSpacing+m_networkItemFixedSize.width());
            cloumn++;
        }
        auto index = m_model->index(row, 0, networkParentIndex);
        m_rect_cache.insert(index, QRect(QPoint(tmpX, m_totalHeight), m_networkItemFixedSize));
    }

    m_totalHeight = m_totalHeight + m_networkItemFixedSize.height() + m_vSpacing;
}

bool ComputerView::isExpanded(const QModelIndex &index)
{
    auto item = m_model->itemFromIndex(index);
    return item->isExpanded();
}

void ComputerView::expand(const QModelIndex &index, bool expand)
{
    auto item = m_model->itemFromIndex(index);
    if (item == nullptr)
        return;
    item->expand(expand);
}
