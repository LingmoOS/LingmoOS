// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drivertableview.h"
#include "MacroDefinition.h"

#include <DApplication>
#include <DStyle>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QApplication>
#include <QMouseEvent>
#include <QLoggingCategory>
#include <QPainter>
#include <QHeaderView>
#include <QPainterPath>


static const int kSpacingMargin = 4;

DriverItemDelegate::DriverItemDelegate(QAbstractItemView *parent)
    : QStyledItemDelegate(parent)
{
}

void DriverItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
    QBrush background;
    QPen forground;
    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            background = palette.color(cg, DPalette::Highlight);
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
        }
    }
    painter->setPen(forground);

    QRect rect = opt.rect;

    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;

    QRect textRect = rect;

    QRect iconRect = rect;
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning &&
            index.data(Qt::DecorationRole).isValid()) {
        iconRect.setX(rect.x() - margin);
        iconRect.setWidth(64);
        QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();
        ic.paint(painter, iconRect);
    }

    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning) {
        textRect.setX(textRect.x() + margin * 2);
    } else {
        textRect.setX(textRect.x() + margin);
    }

    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

QSize DriverItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(DRIVER_TABLE_ROW_HEIGHT, size.height()));
    return size;
}



// DriverHeaderView *********************************************************************************


DriverHeaderView::DriverHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    viewport()->setAutoFillBackground(false);
}
void DriverHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    DPalette::ColorGroup cg;
    if (Qt::ApplicationActive == DApplication::applicationState()) {
        cg = DPalette::Active;
    } else {
        cg = DPalette::Inactive;
    }

    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;

    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height(), rect.width(),
                       rect.height() - contentRect.height());

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // vertical spacing
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin, m_spacing,
                        rect.height() - kSpacingMargin * 2);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

    painter->fillRect(hSpacingRect, clearBrush);
    painter->fillRect(hSpacingRect, hSpacingBrush);

    if (visualIndex(logicalIndex) > 0) {
        painter->fillRect(vSpacingRect, clearBrush);
        painter->fillRect(vSpacingRect, vSpacingBrush);
    }

    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    // TODO: dropdown icon (8x5)
    QRect textRect;
    if (sortIndicatorSection() == logicalIndex) {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin * 3 - 8,
                    contentRect.height()
                   };
    } else {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin,
                    contentRect.height()
                   };
    }
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    //    int align = model()->headerData(logicalIndex, orientation(),
    //    Qt::TextAlignmentRole).toInt();
    int align = Qt::AlignLeft | Qt::AlignVCenter;

    painter->setPen(forground);

    if (logicalIndex == 0) {
        QRect col0Rect = textRect;
        col0Rect.setX(textRect.x() + margin - 2);
        painter->drawText(col0Rect, static_cast<int>(align), title);
    } else {
        painter->drawText(textRect, static_cast<int>(align), title);
    }

    // sort indicator
    if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
        // TODO: arrow size (8x5)
        QRect sortIndicator(textRect.x() + textRect.width() + margin,
                            textRect.y() + (textRect.height() - 5) / 2, 8, 5);
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }

    painter->restore();
}

void DriverHeaderView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();

    DPalette::ColorGroup cg;
#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }
#else
    cg = DPalette::Active;
#endif

    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionHeader option;
    initStyleOption(&option);
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int margin = 1;

    QRect rect = QRect(viewport()->rect().x() + margin, viewport()->rect().y() + margin, viewport()->rect().width() - margin * 2, viewport()->rect().height() - margin * 2);
    QRectF clipRect(rect.x(), rect.y(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    clipPath.addRoundedRect(clipRect, radius, radius);
    subPath.addRect(subRect);
    clipPath = clipPath.subtracted(subPath);

    painter.fillPath(clipPath, bgBrush);

    painter.restore();
    DHeaderView::paintEvent(event);
}

QSize DriverHeaderView::sizeHint() const
{
    return QSize(width(), 36 + m_spacing);
}

int DriverHeaderView::sectionSizeHint(int logicalIndex) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QFontMetrics fm(DApplication::font());
    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
    if (sortIndicatorSection() == logicalIndex) {
        return fm.width(buf) + margin * 3 + 8;
    } else {
        return fm.width(buf) + margin * 2;
    }
}


// DriverTableView *********************************************************************************


DriverTableView::DriverTableView(DWidget *parent)
    : DTreeView(parent)
    , mp_Delegate(new DriverItemDelegate(this))
    , mp_HeadView(new DriverHeaderView(Qt::Horizontal, this))
    , mp_Model(new QStandardItemModel(this))
    , mp_HeaderCb(nullptr)
{
    setModel(mp_Model);
    setHeader(mp_HeadView);
    setItemDelegate(mp_Delegate);
    connect(mp_HeadView, &DriverHeaderView::sectionResized, this, &DriverTableView::slotHeaderSectionResized);


    //隐藏根节点项前的图标（展开折叠图标）
    setRootIsDecorated(false);

    // this is setting is necessary,because scrollperpixel is default in dtk!!
//    setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    // 设置整行选中
    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置不可编辑
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Item 不可扩展
    setItemsExpandable(false);

    // 设置无边框
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);

    setAlternatingRowColors(false);
    setAllColumnsShowFocus(false);
}

void DriverTableView::initHeaderView(const QStringList &headerList, bool firstCheckbox)
{
    // 水平右对齐
    mp_HeadView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    mp_HeadView->setStretchLastSection(true);
    mp_HeadView->setMinimumSectionSize(40);
    DFontSizeManager::instance()->bind(mp_HeadView, DFontSizeManager::T6);

    QAbstractItemModel *model = mp_HeadView->model();
    mp_Model->setHorizontalHeaderLabels(headerList);

    if (firstCheckbox) {
        if (mp_HeaderCb) {
            mp_HeaderCb->setChecked(true);
            mp_HeaderCb->setCbEnable(true);
        } else {
            mp_HeaderCb = new DriverCheckItem(this, true);
            mp_HeadView->setIndexWidget(model->index(0, 0), mp_HeaderCb);
            connect(mp_HeaderCb, &DriverCheckItem::sigChecked, this, &DriverTableView::slotAllItemChecked);
        }
    }

    setFixedHeight(40);
}

void DriverTableView::appendRowItems(int column)
{

    QList<QStandardItem *> lstItem;
    for (int i = 0; i < column; i++) {
        QStandardItem *item = new QStandardItem();
        lstItem.append(item);
    }

    mp_Model->appendRow(lstItem);

    setFixedHeight(height() + DRIVER_TABLE_ROW_HEIGHT);
}

void DriverTableView::setWidget(int row, int column, DWidget *widget)
{
    QModelIndex index = mp_Model->index(row, column);
    setIndexWidget(index, widget);

    // 选中的操作函数处理
    DriverCheckItem *cbItem = qobject_cast<DriverCheckItem *>(widget);
    if (cbItem) {
        connect(cbItem, &DriverCheckItem::sigChecked, this, [this, row](bool checked) {
            // 获取此行的index
            DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(row, 1)));
            if (name) {
                emit itemChecked(name->index(), checked);
            }
        });
    }

    // 点击安装或者更新的槽函数
    // 做三件事，1:当前行的勾选框置灰，2:当前行的按钮置灰，3:其它行如果选中则取消选中且为可安装或可更新状态
    DriverOperationItem *orItem = qobject_cast<DriverOperationItem *>(widget);
    if (orItem) {
        connect(orItem, &DriverOperationItem::clicked, this, [this, orItem](bool checked) {
            Q_UNUSED(checked)
            int rowCount = mp_Model->rowCount();
            for (int i = 0; i < rowCount; i++) {
                if (DriverOperationItem::RESTORE == orItem->mode()) {
                    DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 0)));
                    DriverOperationItem *curItem = dynamic_cast<DriverOperationItem *>(indexWidget(mp_Model->index(i, 3)));
                    if (curItem  && name && curItem == orItem) {
                        emit operatorClicked(name->index(), i, orItem->mode());
                    }
                } else {
                    DriverCheckItem *cb = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
                    DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 1)));
                    DriverStatusItem *status = dynamic_cast<DriverStatusItem *>(indexWidget(mp_Model->index(i, 4)));
                    DriverOperationItem *curItem = dynamic_cast<DriverOperationItem *>(indexWidget(mp_Model->index(i, 5)));
                    if (curItem && cb && name && status) {
                        if (curItem == orItem) {
                            // 当前行按钮置灰
                            curItem->setBtnEnable(false);
                            // 当前行的勾选框置灰
                            cb->setCbEnable(false);
                            cb->setChecked(true);
                            // 状态修改为等待中
                            status->setStatus(ST_WAITING);
                            // 发送信号出去
                            emit operatorClicked(name->index(), i, orItem->mode());
                        } else {
                            // 此时如果选中，则取消选中
                            if (cb->checked() && (status->getStatus() == ST_NOT_INSTALL || status->getStatus() == ST_CAN_UPDATE)) {
                                cb->setChecked(false);
                            }
                        }
                    }

                }
            }
        });
    }
}

void DriverTableView::setHeaderCbStatus(bool checked)
{
    if (!mp_HeaderCb)
        return;

    // 如果取消选中，则取消表头选中
    if (!checked) {
        mp_HeaderCb->setChecked(checked, true);
        return;
    }

    // 如果是选中，则要判断其它的是不是都选中，如果其它的都选中，那表头选中
    bool allChecked = true;
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverCheckItem *item = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
        if (item && item->checked() == false) {
            allChecked = false;
            break;
        }
    }

    if (allChecked) {
        mp_HeaderCb->setChecked(true);
    }
}

void DriverTableView::setHeaderCbEnable(bool enable)
{
    mp_HeaderCb->setCbEnable(enable);
}

void DriverTableView::setItemCbEnable(int row, bool enable)
{
    DriverCheckItem *item = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(row, 0)));
    if (item) {
        item->setCbEnable(enable);
    }
}

void DriverTableView::setAllItemCbEanble(bool enable)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        setItemCbEnable(i, enable);
    }
}

void DriverTableView::setCheckedCBDisable()
{
    if (mp_HeaderCb->checked()) {
        mp_HeaderCb->setCbEnable(false);
    }

    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverCheckItem *item = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
        if (item && item->checked()) {
            // 设置不能选中
            item->setCbEnable(false);

            DriverStatusItem *status = dynamic_cast<DriverStatusItem *>(indexWidget(mp_Model->index(i, 4)));
            DriverOperationItem *opera = dynamic_cast<DriverOperationItem *>(indexWidget(mp_Model->index(i, 5)));

            // 设置不可操作
            if (opera) {
                opera->setBtnEnable(false);
            }

            // 设置状态为等待
            if (status && status->getStatus() != ST_DOWNLOADING && status->getStatus() != ST_INSTALL && status->getStatus() != ST_DRIVER_BACKING_UP) {
                status->setStatus(ST_WAITING);
            }
        }
    }
}

void DriverTableView::setItemOperationEnable(int index, bool enable)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 0)));
        if (name && name->index() == index) {
            DriverOperationItem *opera = dynamic_cast<DriverOperationItem *>(indexWidget(mp_Model->index(i, 3)));

            if (opera) {
                opera->setBtnEnable(enable);
            }
        }
    }
}

void DriverTableView::getCheckedDriverIndex(QList<int> &lstIndex)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverCheckItem *item = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
        DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 1)));
        if (item && item->checked() && name) {
            lstIndex.append(name->index());
        }
    }
}

void DriverTableView::setItemStatus(int index, Status s)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 1)));
        DriverStatusItem *status = dynamic_cast<DriverStatusItem *>(indexWidget(mp_Model->index(i, 4)));

        if (name && status && name->index() == index) {
            status->setStatus(s);

            // 如果是安装成功则取消选中且不可选
            if (ST_SUCESS == s || ST_FAILED == s|| ST_DRIVER_BACKUP_FAILED == s || ST_DRIVER_BACKUP_SUCCESS == s
                    || ST_DRIVER_NOT_BACKUP == s || ST_CAN_UPDATE == s || ST_NOT_INSTALL == s) {
                DriverCheckItem *cb = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
                if (cb) {
                    cb->setCbEnable(ST_FAILED == s || ST_DRIVER_BACKUP_FAILED == s || ST_DRIVER_NOT_BACKUP == s  || ST_CAN_UPDATE == s || ST_NOT_INSTALL == s ? true : false);
                    cb->setChecked(false);
                }
                DriverOperationItem *opera = dynamic_cast<DriverOperationItem *>(indexWidget(mp_Model->index(i, 5)));
                if (opera) {
                    opera->setBtnEnable(ST_FAILED == s || ST_DRIVER_BACKUP_FAILED == s || ST_DRIVER_NOT_BACKUP == s  || ST_CAN_UPDATE == s || ST_NOT_INSTALL == s ? true : false);
                }
            }
            break;
        }
    }
}

void DriverTableView::setErrorMsg(int index, const QString &msg)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverNameItem *name = dynamic_cast<DriverNameItem *>(indexWidget(mp_Model->index(i, 1)));
        DriverStatusItem *status = dynamic_cast<DriverStatusItem *>(indexWidget(mp_Model->index(i, 4)));

        if (name && status && name->index() == index) {
            status->setErrorMsg(msg);
            break;
        }
    }
}

bool DriverTableView::hasItemDisabled()
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverCheckItem *cb = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
        if (cb && !cb->isEnabled()) {
            return true;
        }
    }
    return false;
}

void DriverTableView::clear()
{
    int rowCount = mp_Model->rowCount();
    int columnCount = mp_Model->columnCount();
    for (int row = 0; row < rowCount; row++) {
        for (int column = 0; column < columnCount; column++)
            removeItemAndWidget(row, column);// 删除item和widget
    }

    if (mp_Model) {
        mp_Model->removeRows(0, mp_Model->rowCount());
    }
}

void DriverTableView::removeItemAndWidget(int row, int column)
{
    QWidget *widget = indexWidget(mp_Model->index(row, column));
    if (widget) {
        delete widget;
        widget = nullptr;
    }
}

void DriverTableView::resizeColumn(int column)
{
    mp_HeadView->sectionResized(column, mp_HeadView->sectionSize(column), mp_HeadView->sectionSize(column));
}

void DriverTableView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    auto palette = DGuiApplicationHelper::instance()->applicationPalette();;

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionFrame option;
    initStyleOption(&option);

    // 计算绘制背景色有问题
    QRect rect = viewport()->rect();
    int hMargin = 1; // 水平方向的内边距
    rect.setX(rect.x() + hMargin);
    rect.setWidth(rect.width() - hMargin * 2);
    QPainterPath clipPath = getTopRadiusPath(rect);

    // 填充背景色

    painter.fillPath(clipPath, bgBrush);
    painter.restore();

    DTreeView::paintEvent(event);
}

void DriverTableView::showEvent(QShowEvent *event)
{
    return DTreeView::showEvent(event);
}

void DriverTableView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        cg = DPalette::Active;
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;
    // 圆角以及边距
    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = 6;

    // modify background color acorrding to UI designer
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
    QBrush background;

    // 隔行变色
    if (!(index.row() & 1)) {
        background = palette.color(cg, DPalette::Base);
    } else {
        background = palette.color(cg, DPalette::ItemBackground);
    }

    // 选中状态背景色
    if (options.state & DStyle::State_Enabled) {
        if (selectionModel()->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // 绘制背景色
    QPainterPath path;
    QRect rowRect = options.rect;
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    painter->restore();

    QTreeView::drawRow(painter, options, index);
}

void DriverTableView::slotAllItemChecked(bool checked)
{
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        DriverCheckItem *item = dynamic_cast<DriverCheckItem *>(indexWidget(mp_Model->index(i, 0)));
        if (item && item->isEnabled())
            item->setChecked(checked);
    }
}

void DriverTableView::slotHeaderSectionResized(int logicalIndex, int oldSize, int newSize)
{
    Q_UNUSED(oldSize)
    int rowCount = mp_Model->rowCount();
    for (int i = 0; i < rowCount; i++) {
        QWidget *widget = indexWidget(mp_Model->index(i, logicalIndex));
        if (widget) {
            if (logicalIndex == 1 && newSize < 106) {
                newSize = 106;
                setColumnWidth(logicalIndex, newSize);
            } else if (logicalIndex == 2 && newSize < 80) {
                newSize = 80;
                setColumnWidth(logicalIndex, newSize);
            }
            widget->setMaximumWidth(newSize);
        }
    }
}

QPainterPath DriverTableView::getTopRadiusPath(const QRect &rect)
{
    QPainterPath path;
    //设置圆角半径
    const int radius = 8;
    path.moveTo(rect.bottomRight().x() - radius, rect.bottomRight().y() - radius);
    path.arcTo(QRect(QPoint(rect.bottomRight().x() - radius * 2, rect.bottomRight().y() - radius * 2), QSize(radius * 2, radius * 2)), 270, 90);
    path.lineTo(rect.topRight());
    path.lineTo(rect.topLeft());
    path.lineTo(QPoint(rect.bottomLeft().x(), rect.bottomLeft().y() - radius * 2));
    path.arcTo(QRect(QPoint(rect.bottomLeft().x(), rect.bottomLeft().y() - radius * 2), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(QPoint(rect.bottomRight().x() - radius, rect.bottomRight().y()));
    return path;
}
