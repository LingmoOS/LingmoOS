// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "netview.h"

#include "netitem.h"
#include "netmanager.h"
#include "private/netdelegate.h"
#include "private/netmodel.h"

#include <DStyle>
#include <DStyleOption>

#include <QEvent>
#include <QHoverEvent>
#include <QScrollBar>
#include <QScroller>
#include <QSortFilterProxyModel>
#include <QTimer>

DWIDGET_USE_NAMESPACE

namespace dde {
namespace network {

NetView::NetView(NetManager *manager)
    : QTreeView(nullptr)
    , m_manager(manager)
    , m_closeOnClear(true)
    , m_shouldUpdateExpand(true)
    , m_maxHeight(400)
{
#ifdef QT_SCROLL_WHEEL_ANI
    QScrollBar *bar = verticalScrollBar();
    bar->setSingleStep(1);
    setVerticalScrollBarPolicy(Qt::ScrollBarSlideAnimationOn);
#endif
    setAccessibleName("tree_network");
    setForegroundRole(QPalette::BrightText);
    setFrameShape(QFrame::NoFrame);

    m_model = new NetModel(this);
    m_model->setRoot(m_manager->root());

    m_proxyModel = new QSortFilterProxyModel(m_model);
    m_proxyModel->setSortRole(SORTROLE);
    m_proxyModel->setSourceModel(m_model);

    setModel(m_proxyModel);
    sortByColumn(0, Qt::AscendingOrder);
    connect(m_proxyModel, &QSortFilterProxyModel::rowsRemoved, this, &NetView::updateGeometries);

    m_delegate = new NetDelegate(this);
    setItemDelegate(m_delegate);

    connect(m_delegate, &NetDelegate::requestUpdateLayout, this, &NetView::updateLayout, Qt::QueuedConnection);
    connect(m_delegate, &NetDelegate::requestShow, this, &NetView::scrollToItem, Qt::QueuedConnection);
    connect(m_delegate, &NetDelegate::requestShow, this, &NetView::requestShow);
    connect(m_delegate, &NetDelegate::requestExec, this, &NetView::onExec);
    connect(m_manager, &NetManager::request, m_delegate, &NetDelegate::onRequest);

    setFixedWidth(330);

    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    setSelectionMode(QAbstractItemView::NoSelection);
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setHeaderHidden(true);
    setAnimated(false);
    setIndentation(0);
    setContentsMargins(0, 0, 0, 0);
    expandAll();

    viewport()->setAutoFillBackground(false);
    viewport()->setFixedWidth(320);

    connect(this, &NetView::clicked, this, &NetView::activated);
    connect(this, &NetView::activated, this, &NetView::onActivated);
}

NetView::~NetView() = default;

void NetView::setForegroundRole(QPalette::ColorRole role)
{
    QTreeView::setForegroundRole(role);
    viewport()->setForegroundRole(role);
}

bool NetView::closeOnClear() const
{
    return m_closeOnClear;
}

void NetView::setCloseOnClear(bool closeOnClear)
{
    m_closeOnClear = closeOnClear;
}

void NetView::clear()
{
    if (isVisible())
        return;

    m_manager->exec(NetManager::UserCancelRequest, "");
    scrollTo(model()->index(0, 0));
    m_shouldUpdateExpand = true;
}

void NetView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QAbstractItemModel *m = model();
    QList<QModelIndex> indexes;
    indexes.append(parent);
    while (!indexes.isEmpty()) {
        QModelIndex i = indexes.takeFirst();
        if (!isPersistentEditorOpen(i))
            openPersistentEditor(i);
        for (int j = 0; j < m->rowCount(i); j++) {
            indexes.append(m->index(j, 0, i));
        }
    }
    QTreeView::rowsInserted(parent, start, end);

    QModelIndex newIndex = m->index(start, 0, parent);
    QModelIndex sourceIndex = m_proxyModel->mapToSource(newIndex);
    NetItem *item = m_model->toObject(sourceIndex);

    if (!item)
        return;

    switch (item->itemType()) {
    case NetItemType::WirelessOtherItem: {
        NetWirelessOtherItem *otherItem = NetItem::toItem<NetWirelessOtherItem>(item);
        if (otherItem) {
            updateItemExpand(otherItem);
            connect(otherItem, &NetWirelessOtherItem::expandedChanged, this, &NetView::onExpandStatusChanged);
        }
    } break;
    case NetItemType::WirelessMineItem:
        updateItemExpand(item);
        break;
    case NetItemType::VPNControlItem: {
        NetVPNControlItem *vpnControlItem = NetItem::toItem<NetVPNControlItem>(item);
        if (vpnControlItem) {
            connect(vpnControlItem, &NetVPNControlItem::expandedChanged, this, &NetView::onExpandStatusChanged);
            connect(vpnControlItem, &NetVPNControlItem::enabledChanged, this, &NetView::onExpandStatusChanged);
            updateItemExpand(vpnControlItem);
        }
    } break;
    case NetItemType::WirelessControlItem:
    case NetItemType::WiredControlItem:
    case NetItemType::WirelessDeviceItem:
    case NetItemType::WiredDeviceItem: {
        NetDeviceItem *dev = NetItem::toItem<NetDeviceItem>(item);
        if (dev) {
            updateItemExpand(dev);
            connect(dev, &NetDeviceItem::enabledChanged, this, &NetView::onExpandStatusChanged);
            if (dev->itemType() == NetItemType::WirelessDeviceItem)
                connect(NetItem::toItem<NetWirelessDeviceItem>(dev), &NetWirelessDeviceItem::apModeChanged, this, &NetView::onExpandStatusChanged);
        }
    } break;
    default:
        break;
    }
}

bool NetView::viewportEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverLeave: {
        setCurrentIndex(QModelIndex());
    } break;
    case QEvent::HoverEnter:
    case QEvent::HoverMove: {
        QHoverEvent *he = static_cast<QHoverEvent *>(event);
        QModelIndex newIndex = indexAt(he->pos());
        setCurrentIndex(newIndex);
        break;
    }
    default:
        return QTreeView::viewportEvent(event);
    }
    return true;
}

void NetView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || !index.flags().testFlag(Qt::ItemIsEnabled)) {
        QAbstractItemView::mousePressEvent(event);
    } else {
        QTreeView::mousePressEvent(event);
    }
}

void NetView::mouseReleaseEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid() || !index.flags().testFlag(Qt::ItemIsEnabled)) {
        QAbstractItemView::mouseReleaseEvent(event);
    } else {
        QTreeView::mouseReleaseEvent(event);
    }
}

void NetView::updateByScrollbar()
{
    if (m_updateCurrent) {
        QPoint posInVp = viewport()->mapFromGlobal(QCursor::pos());
        if (viewport()->rect().contains(posInVp))
            setCurrentIndex(indexAt(posInVp));
    }
    m_updateCurrent = true;
}

void NetView::onExec(NetManager::CmdType cmd, const QString &id, const QVariantMap &param)
{
    m_manager->exec(cmd, id, param);
}

void NetView::onActivated(const QModelIndex &index)
{
    switch (index.data(TYPEROLE).value<NetItemType>()) {
    case NetItemType::WirelessOtherItem:
        m_manager->exec(NetManager::ToggleExpand, index.data(IDROLE).toString());
        break;
    default:
        m_manager->exec(NetManager::Connect, index.data(IDROLE).toString());
        break;
    }
}

void NetView::updateLayout()
{
    scheduleDelayedItemsLayout();
    if (!isVisible())
        updateGeometries();
}

void NetView::onExpandStatusChanged()
{
    NetItem *item = qobject_cast<NetItem *>(sender());
    if (item)
        updateItemExpand(item);
}

void NetView::updateItemExpand(NetItem *item)
{
    bool expandItem = false;
    switch (item->itemType()) {
    case NetItemType::WiredDeviceItem: {
        NetWiredDeviceItem *dev = NetItem::toItem<NetWiredDeviceItem>(item);
        expandItem = dev->isEnabled();
    } break;
    case NetItemType::WirelessDeviceItem: { // 无线禁用或热点模式时折叠
        NetWirelessDeviceItem *dev = NetItem::toItem<NetWirelessDeviceItem>(item);
        expandItem = dev->isEnabled() && !dev->isApMode();
    } break;
    case NetItemType::WirelessMineItem: {
        expandItem = true;
    } break;
    case NetItemType::WirelessOtherItem: {
        NetWirelessOtherItem *dev = NetItem::toItem<NetWirelessOtherItem>(item);
        expandItem = dev->isExpanded();
    } break;
    case NetItemType::VPNControlItem: {
        NetVPNControlItem *dev = NetItem::toItem<NetVPNControlItem>(item);
        expandItem = dev->isExpanded();
    } break;
    default:
        return;
    }
    QModelIndex sIndex = m_model->index(item);
    QModelIndex index = m_proxyModel->mapFromSource(sIndex);
    if (isExpanded(index) != expandItem) {
        setExpanded(index, expandItem);
        updateGeometries();
        viewport()->update();
    }
}

void NetView::scrollToItem(const QString &id)
{
    QAbstractItemModel *m = model();
    QList<QModelIndex> indexes;
    indexes.append(QModelIndex());
    while (!indexes.isEmpty()) {
        QModelIndex index = indexes.takeFirst();
        if (index.data(IDROLE).toString() == id) {
            scrollTo(index);
            return;
        }
        if (!index.isValid() || isExpanded(index)) {
            int row = m->rowCount(index);
            while (row--) {
                indexes.prepend(m->index(row, 0, index));
            }
        }
    }
}

QModelIndex NetView::traverseAndSearch(const QModelIndex &parent, const QString &id)
{
    int row = m_proxyModel->rowCount(parent);
    while (row--) {
        QModelIndex index = m_proxyModel->index(row, 0, parent);
        if (index.isValid() && index.data(IDROLE).toString() == id) {
            return index;
        }
        QModelIndex retIndex = traverseAndSearch(index, id);
        if (retIndex.isValid())
            return retIndex;
    }
    return QModelIndex();
}

void NetView::verticalScrollbarValueChanged(int)
{
    QMetaObject::invokeMethod(this, "updateByScrollbar", Qt::QueuedConnection);
}

void NetView::horizontalScrollbarValueChanged(int)
{
    QMetaObject::invokeMethod(this, "updateByScrollbar", Qt::QueuedConnection);
}

void NetView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTreeView::currentChanged(current, previous);
    if (previous.isValid()) {
        QRect rect = visualRect(previous);
        rect.setRect(0, rect.y() - 1, viewport()->width(), rect.height() + 2);
        viewport()->update(rect);
    }
    if (current.isValid()) {
        QRect rect = visualRect(current);
        rect.setRect(0, rect.y() - 1, viewport()->width(), rect.height() + 2);
        viewport()->update(rect);
    }
    m_updateCurrent = false;
}

void NetView::updateGeometries()
{
    QTreeView::updateGeometries();

    QAbstractItemModel *m = model();
    QList<QModelIndex> indexes;
    indexes.append(QModelIndex());
    int count = 0;
    int h = 0;
    while (!indexes.isEmpty()) {
        QModelIndex index = indexes.takeFirst();
        if (index.isValid()) {
            h += rowHeight(index);
        }
        count++;
        if (h > m_maxHeight) {
            h = m_maxHeight;
            break;
        }
        if (!index.isValid() || isExpanded(index)) {
            int row = m->rowCount(index);
            while (row--) {
                indexes.prepend(m->index(row, 0, index));
            }
        }
    }
    setFixedHeight(h);
    Q_EMIT updateSize();
}

QModelIndex NetView::indexAt(const QPoint &p) const
{
    QModelIndex index = QTreeView::indexAt(p);
    if (!index.isValid())
        return index;
    ItemSpacing itemSpacing = m_delegate->getItemSpacing(index);
    QPoint pos = p;
    if (itemSpacing.bottom != 0) {
        pos.setY(p.y() + itemSpacing.bottom);
        QModelIndex indexTmp = QTreeView::indexAt(pos);
        if (indexTmp != index)
            return QModelIndex();
    }
    if (itemSpacing.top != 0) {
        pos.setY(p.y() - itemSpacing.top);
        QModelIndex indexTmp = QTreeView::indexAt(pos);
        if (indexTmp != index)
            return QModelIndex();
    }
    return index;
}

void NetView::showEvent(QShowEvent *event)
{
    QTreeView::showEvent(event);
    m_manager->setAutoScanEnabled(true);
    // 首次打开，需要刷新展开状态
    if (m_shouldUpdateExpand)
        m_manager->exec(NetManager::ToggleExpand, "");
    m_shouldUpdateExpand = false;
}

void NetView::hideEvent(QHideEvent *event)
{
    QTreeView::hideEvent(event);
    if (closeOnClear()) {
        // 延时调用,防止密码拉起时收到多个hide事件,误触发clear,
        QTimer::singleShot(10, this, &NetView::clear);
    }
    m_shouldUpdateExpand = false;
    m_manager->exec(NetManager::ToggleExpand, "");
    Q_EMIT updateSize();
    m_manager->setAutoScanEnabled(false);
}

void NetView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Space:
    case Qt::Key_Select:
    case Qt::Key_O:
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        QAbstractItemModel *m = model();
        QList<QModelIndex> indexes;
        indexes.append(QModelIndex());

        while (!indexes.isEmpty()) {
            QModelIndex index = indexes.takeFirst();
            if (index.isValid() && rowHeight(index) > 80) { // 输入框展开状态跳过键盘事件
                return;
            }
            if (!index.isValid() || isExpanded(index)) {
                int row = m->rowCount(index);
                while (row--) {
                    indexes.prepend(m->index(row, 0, index));
                }
            }
        }
    } break;
    default:
        break;
    }
    QTreeView::keyPressEvent(event);
}

void NetView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    viewport()->setFixedWidth(320);
}

} // namespace network
} // namespace dde
