// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "listview.h"
#include "itemdelegate.h"
#include "itemmodel.h"

#include <QDebug>
#include <QEvent>
#include <QHoverEvent>

#define ITEM_KEY "item"

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

#define VIEWPORT_MAGINS QMargins(0, 10, 0, 10)
#define MAXITEM_PERLINE 6
#define CLOSEBTN_SIZE 22

ListView::ListView(QWidget *parent) : QListView(parent)
{

}

void ListView::initialize()
{
    setSelectionBehavior(SelectItems);
    setSelectionMode(NoSelection);

    setDragEnabled(false);
    setDragDropMode(NoDragDrop);
    setFrameShape(NoFrame);
    setSpacing(0);
    setViewportMargins(VIEWPORT_MAGINS);

    delegate = new ItemDelegate(this);
    setItemDelegate(delegate);
    setViewMode(IconMode);
    setMovement(Static);

    setGridSize(delegate->sizeHint(QStyleOptionViewItem(), QModelIndex()));
    setResizeMode(Adjust);
    setFlow(LeftToRight);
    setMinimumHeight(gridSize().height() + 20); // add top and bottom margin

    deleteBtn = new DIconButton(DStyle::SP_CloseButton, this);
    deleteBtn->setFixedSize(CLOSEBTN_SIZE, CLOSEBTN_SIZE);
    deleteBtn->setIconSize(QSize(CLOSEBTN_SIZE, CLOSEBTN_SIZE));
    deleteBtn->setFlat(true);
    deleteBtn->setFocusPolicy(Qt::NoFocus);
    deleteBtn->hide();

    connect(deleteBtn, &DIconButton::clicked, this, &ListView::onButtonClicked);
    connect(this, &ListView::clicked, this, &ListView::onItemClicked);
    connect(model(), &QAbstractItemModel::rowsAboutToBeRemoved, this, &ListView::onItemRemoved);
    connect(model(), &QAbstractItemModel::rowsInserted, this, &ListView::updateMaxHeight);
    connect(model(), &QAbstractItemModel::rowsRemoved, this, &ListView::updateMaxHeight);
    connect(model(), &QAbstractItemModel::modelReset, this, &ListView::updateMaxHeight);
}

ItemModel *ListView::itemModel() const
{
    return qobject_cast<ItemModel *>(model());
}

void ListView::setShowDelete(bool enable)
{
    enableDelete = enable;
    if (!enable) {
        deleteBtn->hide();
        deleteBtn->setProperty(ITEM_KEY, QString());
    }
}

bool ListView::hasHeightForWidth() const
{
    return true;
}

int ListView::heightForWidth(int w) const
{
    const QSize itemSize = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());
    auto vm = viewportMargins();
    // the size of viewport is used to layout items.
    // minus 1px is for layout that list view calc rect error.
    auto viewportWidth = w - 1;
    int column = viewportWidth / itemSize.width();
    if (column > MAXITEM_PERLINE)
        column = MAXITEM_PERLINE;
    else if (column < 0)
        column = 1;

    const int itemCount = itemModel()->rowCount();
    int row = (itemCount / column) + (itemCount % column > 0 ? 1 : 0);
    if (row < 0)
        row = 1;

    int maxHeight = row * itemSize.height() + vm.bottom() + vm.top();
    return maxHeight;
}

void ListView::updateMaxHeight()
{
    auto max = heightForWidth(width());
    if (max != maximumHeight())
        setMaximumHeight(max);
}

void ListView::updateMargins()
{
    const QSize itemSize = delegate->sizeHint(QStyleOptionViewItem(), QModelIndex());

    auto defalutMargins = VIEWPORT_MAGINS;
    // the size of viewport is used to layout items.
    // minus 1px is for layout that list view calc rect error.
    auto viewportWidth = width() - 1;

    int margin = 0;
    if ((viewportWidth / itemSize.width()) > MAXITEM_PERLINE)
        margin = (viewportWidth - itemSize.width() * MAXITEM_PERLINE) / 2;
    else
        margin = (viewportWidth % itemSize.width()) / 2;

    defalutMargins.setLeft(defalutMargins.left() + margin);
    defalutMargins.setRight(defalutMargins.right() + margin);

    if (defalutMargins != viewportMargins()) {
        setViewportMargins(defalutMargins);
    }
}

void ListView::updateButton(const QModelIndex &index)
{
    auto item = itemModel()->itemNode(index);
    if (item && item->deletable && !selectionModel()->isSelected(index)) {
        deleteBtn->setProperty(ITEM_KEY, item->item);
        QPoint pos = visualRect(index).topRight() - QPoint(CLOSEBTN_SIZE / 2 + LISTVIEW_ICON_MARGIN, CLOSEBTN_SIZE / 2 - LISTVIEW_ICON_MARGIN);
        deleteBtn->move(viewport()->mapTo(this, pos));
        deleteBtn->show();
    } else {
        deleteBtn->hide();
        deleteBtn->setProperty(ITEM_KEY, QString());
    }
}

bool ListView::viewportEvent(QEvent *event)
{
    if (enableDelete) {
        switch (event->type()) {
        case QEvent::HoverMove:
        case QEvent::HoverEnter:
        case QEvent::HoverLeave:
        case QEvent::Leave: {
            // use cursor pos instead of event->pos() to resolve
            // that deleteBtn blinking bt accepting Leave event when hover on itself.
            auto pos = viewport()->mapFromGlobal(QCursor::pos());
            //auto pos = dynamic_cast<QHoverEvent *>(event)->pos();
            auto idx = indexAt(pos);
            updateButton(idx);
        }
            break;
        default:
            break;
        }
    }

    return QListView::viewportEvent(event);
}

bool ListView::event(QEvent *e)
{
    bool ret = QListView::event(e);

    // the resizeEvent of QAbstractScrollArea is not view's event but viewport.
    // so need to get resize event of QAbstractScrollArea at here.
    if (e->type() == QEvent::Resize) {
        updateMargins();
        updateMaxHeight();
    } else if (e->type() == QEvent::Leave) {
        updateButton(QModelIndex());
    }

    return ret;
}

void ListView::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (index.flags() & Qt::ItemIsSelectable) {
        selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        setCurrentIndex(index);
    }

    auto ptr = itemModel()->itemNode(index);
    if (!ptr.isNull())
        emit itemClicked(ptr);
}

void ListView::onButtonClicked()
{
    auto key = deleteBtn->property(ITEM_KEY).toString();
    qInfo() << "delete btn clicked" << key;

    auto m = itemModel();
    if (auto ptr = m->itemNode(m->itemIndex(key)))
        emit deleteButtonClicked(ptr);
}

void ListView::onItemRemoved(const QModelIndex &parent, int first, int last)
{
    auto key = deleteBtn->property(ITEM_KEY).toString();
    auto idx = itemModel()->itemIndex(key);
    if (idx.isValid() && idx.row() == first) {
          deleteBtn->hide();
          deleteBtn->setProperty(ITEM_KEY, QString());
     }
}
