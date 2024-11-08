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

#include "computer-view.h"
#include "computer-proxy-model.h"
#include "abstract-computer-item.h"
#include "computer-item-delegate.h"
#include "computer-view-style.h"

#include <QScrollBar>
#include <QPainter>

#include <QMouseEvent>
#include <QRubberBand>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QApplication>
#include <QGSettings/qgsettings.h>
#include <global-settings.h>

#include <QTimer>

#include <QDebug>

ComputerView::ComputerView(QWidget *parent) : QAbstractItemView(parent)
{
    m_touch_active_timer = new QTimer(this);
    m_touch_active_timer->setInterval(2000);
    m_touch_active_timer->setSingleShot(true);

    setDragDropMode(QAbstractItemView::DropOnly);
    setItemDelegate(new ComputerItemDelegate(this));

    m_model = ComputerProxyModel::globalInstance();
    setModel(m_model);
    setStyle(ComputerViewStyle::getStyle());
    model()->sort(0, Qt::AscendingOrder);

    m_rubberBand = new QRubberBand(QRubberBand::Shape::Rectangle, this);

    connect(m_model, &ComputerProxyModel::updateLocationRequest, this, &ComputerView::updateLocationRequest);
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

    if (QGSettings::isSchemaInstalled("org.lingmo.style"))
    {
        adjustLayout();
        //font monitor, adjust to font size to fix lauout issue, link to bug#65238
        QGSettings *fontSetting = new QGSettings(FONT_SETTINGS, QByteArray(), this);
        connect(fontSetting, &QGSettings::changed, this, [=](const QString &key){
            qDebug() << "fontSetting changed:" << key;
            if (key == "systemFontSize") {
                adjustLayout();
            }
        });
    }

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->viewport()->setMouseTracking(true);
    this->viewport()->installEventFilter(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(0);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [=]{
        doLayout();
    });

    connect(m_model, &ComputerProxyModel::updateRequest, this, [=]{
        doLayout();
    });
    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, this, [=](const QString& key){
        if (SHOW_NETWORK == key) {
            doLayout();
        }
    });
}

void ComputerView::adjustLayout()
{
    int fontSize = this->font().pointSize();
    if (fontSize <= 0) {
        fontSize = this->font().pixelSize();
    }
    int width = 256 + (fontSize -11) * 64/5;
    int height = 108 + (fontSize -11) * 36/5;
    m_volumeItemFixedSize = QSize(width, height);

    int other_width = 108 + (fontSize -11) * 36/5;
    int other_height = 144 + (fontSize -11) * 48/5;
    m_remoteItemFixedSize = QSize(other_width, other_height);
    m_networkItemFixedSize = QSize(other_width, other_height);
}

void ComputerView::doLayout()
{
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
            m_isShowNetwork = Peony::GlobalSettings::getInstance()->isExist(SHOW_NETWORK) ?
                        Peony::GlobalSettings::getInstance()->getValue(SHOW_NETWORK).toBool() : true;
            if (m_isShowNetwork) {
                layoutNetworkIndexes(index);
            }
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

    viewport()->update();
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
        if (rect.contains(pos) && index.parent().isValid()){
            return index;
        }
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
            if (realRect.contains(indexRect.center())
                    && index.parent().isValid()) {
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
//        if (!selectedIndexes().contains(index))
//            selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
            setCurrentIndex(index);
    }
}

QRegion ComputerView::visualRegionForSelection(const QItemSelection &selection) const
{
    return QRegion();
}

void ComputerView::setModel(QAbstractItemModel *model)
{
//    if (this->model()) {
//        disconnect(this->model(), &QAbstractItemModel::rowsAboutToBeRemoved, 0, 0);
//        disconnect(this->model(), &QAbstractItemModel::rowsInserted, 0, 0);
//    }
//    connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, &ComputerView::rowsAboutToBeRemoved);
//    connect(model, &QAbstractItemModel::rowsInserted, this, &ComputerView::rowsInserted);

    QAbstractItemView::setModel(model);
}

QString ComputerView::tryGetVolumeUriFromMountTarget(const QString &mountTargetUri)
{
    return m_model->tryGetVolumeUriFromMountTarget(mountTargetUri);
}

QString ComputerView::tryGetVolumeRealUriFromUri(const QString &uri)
{
    return m_model->tryGetVolumeRealUriFromUri(uri);
}

void ComputerView::refresh()
{
    m_model->refresh();
}

bool ComputerView::getRightDoubleClickState()
{
    return m_isRightButonDClick;
}

void ComputerView::setRightDoubleClickState(bool flag)
{
    m_isRightButonDClick = flag;
}

void ComputerView::updateEditorGeometries()
{
    QAbstractItemView::updateEditorGeometries();
}

void ComputerView::updateGeometries()
{
    doLayout();
}

void ComputerView::resizeEvent(QResizeEvent *event)
{
    QAbstractItemView::resizeEvent(event);

    updateEditorGeometries();
}

void ComputerView::paintEvent(QPaintEvent *e)
{
    QPainter p(this->viewport());
    p.fillRect(this->rect(), palette().base());
    //p.fillRect(QRect(0, 0, m_totalWidth, m_totalHeight), Qt::blue);
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
    bool singleClicked = qApp->style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick);
    if (singleClicked) {
        if (!m_touch_active_timer->isActive()) {
            m_touch_active_timer->start(1100);
        }
    }
    if (event->button() == Qt::LeftButton) {
        m_isLeftButtonPressed = true;
        m_rubberBand->hide();
        m_lastPressedPoint = event->pos();
        m_lastPressedLogicPoint = event->pos() + QPoint(horizontalOffset(), verticalOffset());
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

void ComputerView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton){
        m_isRightButonDClick = true;
    }else{
        m_isRightButonDClick = false;
    }
    QAbstractItemView::mouseDoubleClickEvent(event);
}

void ComputerView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);
    this->viewport()->update();
}

void ComputerView::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void ComputerView::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
    auto newIndex = indexAt(event->pos());
    if (newIndex != m_hoverIndex) {
        m_hoverIndex = newIndex;
        this->viewport()->update();
    }
}

void ComputerView::dropEvent(QDropEvent *event)
{
    bool isMoveOp = event->keyboardModifiers() & Qt::ShiftModifier;
    auto index = indexAt(event->pos());
    if (index.isValid()) {
        m_model->dropMimeData(event->mimeData(), isMoveOp? Qt::MoveAction: Qt::CopyAction, index.row(), index.column(), index.parent());
    }
}

void ComputerView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    m_timer->start();
    QAbstractItemView::rowsInserted(parent, start, end);
}

void ComputerView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    m_timer->start();
    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void ComputerView::layoutVolumeIndexes(const QModelIndex &volumeParentIndex)
{
    int rowCount = m_model->rowCount(volumeParentIndex);
    if (rowCount <= 0)
        return;

    //add tab
//    m_rect_cache.insert(volumeParentIndex, QRect(QPoint(0, m_totalHeight), QSize(this->viewport()->width(), m_tabPadding)));
    m_totalHeight += m_tabPadding;

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
