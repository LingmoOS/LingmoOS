// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DetailViewDelegate.h"

// Qt库文件
#include <QPainter>
#include <QLoggingCategory>
#include <QPainterPath>

// Dtk头文件
#include <DApplication>
#include <DStyle>
#include <DApplicationHelper>
#include <DFontSizeManager>

// 其它头文件
#include "DetailTreeView.h"

DWIDGET_USE_NAMESPACE

DetailViewDelegate::DetailViewDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void DetailViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd)
            cg = DPalette::Inactive;
        else
            cg = DPalette::Active;
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;

    if (opt.features & QStyleOptionViewItem::Alternate)
        background = palette.color(cg, DPalette::ItemBackground);
    else
        background = palette.color(cg, DPalette::Base);

    QRect rect = opt.rect;
    QPainterPath path;
    QRect rectpath = rect;

    // 确定绘制区域的形状，单元格

    // 第一列右空一个像素
    rectpath.setWidth(rect.width() - 1);

    // 最后一行是更多信息按钮行，背景色为白色，单元格上边框要绘制横线以
    if (index.row() == dynamic_cast<DetailTreeView *>(this->parent())->rowCount() - 1
            && index.row() != 0 && dynamic_cast<DetailTreeView *>(this->parent())->hasExpendInfo()) {
        // 上方要空一个像素用来绘制横线
        rectpath.setY(rect.y() + 1);
        path.addRoundedRect(rectpath, 8, 8);

        // 展开 行背景色为白色
        painter->fillPath(path, palette.color(cg, DPalette::Base));


    } else {
        if (rectpath.y() <= 0) {
            QRect tmpRect = rectpath;
            tmpRect.setY(0);
            path.addRoundedRect(tmpRect, 8, 8);

            // 填充第一行空白
            if (rectpath.bottomLeft().y() > 8) {

                // 填充第一列空白
                if (index.column() == 0) {

                    // 左下空白
                    QRect rect1(tmpRect.bottomLeft().x(), tmpRect.bottomLeft().y() - 7, 8, 8);

                    // 第一列后移一个像素
                    rect1.setX(rect1.x() + 1);

                    QPainterPath pathLeftBottom;
                    pathLeftBottom.addRect(rect1);

                    // 左下角绘制区域
                    pathLeftBottom = pathLeftBottom.subtracted(path);
                    painter->fillPath(pathLeftBottom, background);

                    // 右侧空白
                    QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y() + 1, 8, tmpRect.height());

                    QPainterPath pathRight;
                    pathRight.addRect(rect2);

                    // 右侧绘制区域
                    pathRight = pathRight.subtracted(path);
                    painter->fillPath(pathRight, background);
                } else if (index.column() == 1) { // 填充第二列空白
                    // 左侧空白
                    QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y() + 1, 8, tmpRect.height());

                    QPainterPath pathLeft;
                    pathLeft.addRect(rect1);

                    // 左侧绘制区域
                    pathLeft = pathLeft.subtracted(path);
                    painter->fillPath(pathLeft, background);

                    // 右下角
                    QRect rect2(tmpRect.bottomRight().x() - 7, tmpRect.bottomRight().y() - 7, 8, 8);

                    QPainterPath pathRightBottom;
                    pathRightBottom.addRect(rect2);

                    // 左侧绘制区域
                    pathRightBottom = pathRightBottom.subtracted(path);
                    painter->fillPath(pathRightBottom, background);
                }
            }

        } else if (rectpath.y() + rect.height() >= dynamic_cast<DetailTreeView *>(this->parent())->height()) {
            QRect tmpRect = rectpath;
            tmpRect.setHeight(dynamic_cast<DetailTreeView *>(this->parent())->height() - rectpath.y());
            path.addRoundedRect(tmpRect, 8, 8);

            if (tmpRect.height() > 8) {
                // 填充第列空白
                if (index.column() == 0) {
                    // 左上空白
                    QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y(), 8, tmpRect.height() - 8);

                    // 第一列后移一个像素
                    rect1.setX(rect1.x() + 1);

                    QPainterPath pathLeftTop;
                    pathLeftTop.addRect(rect1);

                    // 左下角绘制区域
                    pathLeftTop = pathLeftTop.subtracted(path);
                    painter->fillPath(pathLeftTop, background);

                    // 右侧空白
                    QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y(), 8, tmpRect.height());

                    QPainterPath pathRight;
                    pathRight.addRect(rect2);

                    // 右侧绘制区域
                    pathRight = pathRight.subtracted(path);
                    painter->fillPath(pathRight, background);
                } else if (index.column() == 1) {
                    // 左侧空白
                    QRect rect1(tmpRect.topLeft().x(), tmpRect.topLeft().y(), 8, tmpRect.height());

                    QPainterPath pathLeft;
                    pathLeft.addRect(rect1);

                    // 左侧绘制区域
                    pathLeft = pathLeft.subtracted(path);
                    painter->fillPath(pathLeft, background);

                    // 右上空白
                    QRect rect2(tmpRect.topRight().x() - 7, tmpRect.topRight().y(), 8, tmpRect.height() - 8);

                    QPainterPath pathRightTop;
                    pathRightTop.addRect(rect2);

                    // 右上绘制区域
                    pathRightTop = pathRightTop.subtracted(path);
                    painter->fillPath(pathRightTop, background);
                }
            }

        } else {
            path.addRect(rectpath);
        }
        painter->fillPath(path, background);
    }

    // 绘制文字信息
    QRect textRect = rectpath;
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);   // 边距

    textRect.setX(textRect.x() + margin);
    textRect.setWidth(textRect.width() - margin);

    QFont fo;
    // 表格第一列，字体加粗
    if (index.column() == 0) {
        fo = opt.font;
        fo.setWeight(63);
    } else {
        fo = DFontSizeManager::instance()->t8();
    }

    painter->setFont(fo);
    QFontMetrics fm(fo);
    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());

    // 设备启用禁用的状态 和 可用不可用状态
    if (dynamic_cast<DetailTreeView *>(this->parent())->isCurDeviceEnable()) {
        // 在设备没有被禁用的前提下，如果设备不可用则，则按照不可用要求显示
        if(dynamic_cast<DetailTreeView *>(this->parent())->isCurDeviceAvailable()){
            if (index.row() == 0 && index.column() == 1)
                text = text.remove("(" + tr("Disable") + ")");
        }else{
            if (index.row() == 0 && index.column() == 1) {
                text = "(" + tr("Unavailable") + ")" + text;
                QPen pen = painter->pen();
                pen.setColor(palette.color(cg, DPalette::PlaceholderText));
                painter->setPen(pen);
            }
        }
    } else { // 设备被禁用的情况下的显示效果
        if (index.row() == 0 && index.column() == 1) {
            text = "(" + tr("Disable") + ")" + text;
            QPen pen = painter->pen();
            pen.setColor(QColor("#FF5736"));
            painter->setPen(pen);
        }
    }

    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);

    painter->restore();
}

QWidget *DetailViewDelegate::createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
{
    return nullptr;
}

QSize DetailViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(50, size.height()));

    if (index.column())
        size.setWidth(std::max(150, size.width()));

    return size;
}

void DetailViewDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}
