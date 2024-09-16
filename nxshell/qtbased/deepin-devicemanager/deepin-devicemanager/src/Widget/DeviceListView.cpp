// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceListView.h"

// Dtk头文件
#include <DApplicationHelper>
#include <DApplication>

// Qt库文件
#include <qdrawutil.h>
#include <QPainter>
#include <QLoggingCategory>
#include <QKeyEvent>
#include <QPainterPath>

// 其它头文件
#include "MacroDefinition.h"

DWIDGET_USE_NAMESPACE



DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{
    //setItemSpacing(10);
}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().toString() == "Separator") {
        return paintSeparator(painter, option);
    }
    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().toString() == "Separator") {
        return QSize(option.rect.width(), 5);
    }
    return DStyledItemDelegate::sizeHint(option, index);
}

void DeviceListviewDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();


    int yPoint = option.rect.top() + option.rect.height() / 2;

    QRect paintRect(0, yPoint, option.rect.width(), 2);
    QPainterPath path;
    path.addRect(paintRect);

    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter->fillPath(path, bgBrush);
    painter->restore();
}



DeviceListView::DeviceListView(QWidget *parent)
    : DListView(parent), mp_ItemModel(new QStandardItemModel(this))
{
    // 设置View的Model
    setModel(mp_ItemModel);

    // 设置item的代理
    setItemDelegate(new DeviceListviewDelegate(this));

    // 设置item的其它属性
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemSpacing(0);
    setAutoFillBackground(true); //与背景色有关

    // bug51444左侧菜单和滚动条不重叠重叠
    setViewportMargins(11, 2, 11, 5);

    setMovement(QListView::Static);
}

DeviceListView::~DeviceListView()
{
    DELETE_PTR(mp_ItemModel);
}

void DeviceListView::addItem(const QString &name, const QString &iconFile)
{
    QStringList lst = iconFile.split("##");
    if (lst.size() != 2) {
        return;
    }

    DStandardItem *item = new DStandardItem;
    item->setData(name, Qt::DisplayRole);
    item->setData(lst[1], Qt::UserRole);
    item->setTextAlignment(Qt::AlignVCenter);
    if (name != QString("Separator")) {
        item->setToolTip(name);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);
    } else {
        item->setFlags(Qt::ItemNeverHasChildren);
    }
    item->setIcon(QIcon::fromTheme(lst[0]));
    setIconSize(QSize(20, 20));
    mp_ItemModel->appendRow(item);
}

void DeviceListView::setCurItemEnable(bool enable)
{
    const QModelIndex &index =  this->currentIndex();
    int rowNum = index.row();
    QStandardItem *item = mp_ItemModel->item(rowNum);
    if (item)
        item->setEnabled(enable);
}

void DeviceListView::setCurItem(const QString &str)
{
    if (!mp_ItemModel) {
        return;
    }

    QList<QStandardItem *> lstItems = mp_ItemModel->findItems(str);
    if (lstItems.size() != 1) {
        return;
    }

    QModelIndex index = mp_ItemModel->indexFromItem(lstItems[0]);
    setCurrentIndex(index);
}

QString DeviceListView::getConcatenateStrings(const QModelIndex &index)
{
    QStandardItem *item = mp_ItemModel->item(index.row());
    if (item) {
        return item->data(Qt::DisplayRole).toString();
    }
    return "";
}

void DeviceListView::clearItem()
{
    mp_ItemModel->clear();
}

void DeviceListView::paintEvent(QPaintEvent *event)
{
    // 让背景色适合主题颜色
    DPalette pa;
    pa = DApplicationHelper::instance()->palette(this);
    pa.setBrush(DPalette::ItemBackground, pa.brush(DPalette::Base));
    DApplicationHelper::instance()->setPalette(this, pa);

    DListView::paintEvent(event);
}

void DeviceListView::mousePressEvent(QMouseEvent *event)
{
    if ((QApplication::keyboardModifiers() == Qt::ControlModifier) && (event->button() == Qt::LeftButton)) {
        // 当键盘按住ctrl，不响应鼠标点击事件
    } else {
        DListView::mousePressEvent(event);
    }
}

void DeviceListView::mouseMoveEvent(QMouseEvent *event)
{
    if ((QApplication::keyboardModifiers() == Qt::ControlModifier)/* && (event->button() == Qt::LeftButton)*/) {
        // 当键盘按住ctrl，不响应鼠标移动事件
    } else {
        DListView::mouseMoveEvent(event);
    }
}

void DeviceListView::keyPressEvent(QKeyEvent *keyEvent)
{
    DListView::keyPressEvent(keyEvent);

    // 当前Item 为Separator时，需要跳过Separator
    if (this->currentIndex().data(Qt::DisplayRole) == "Separator") {
        // 按键：下一个
        if (keyEvent->key() == Qt::Key_Down) {
            int curRow = this->currentIndex().row();

            // 当前 Separator 不是最后一个Item，显示下一个Item
            if (curRow != mp_ItemModel->rowCount() - 1) {
                QModelIndex index = this->currentIndex().siblingAtRow(curRow + 1);
                this->setCurrentIndex(index);
            }
        }

        // 按键：上一个
        if (keyEvent->key() == Qt::Key_Up) {
            int curRow = this->currentIndex().row();
            // 当前 Separator 不是第一个Item，显示上一个Item
            if (curRow != 0) {
                QModelIndex index = this->currentIndex().siblingAtRow(curRow - 1);
                this->setCurrentIndex(index);
            }
        }
    }

    // 切换对应设备信息页面
    emit clicked(this->currentIndex());
}
