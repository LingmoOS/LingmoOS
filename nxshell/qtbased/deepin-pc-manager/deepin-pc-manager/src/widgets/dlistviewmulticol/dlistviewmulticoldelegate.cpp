// Copyright (C) 2019 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlistviewmulticoldelegate.h"

#include "dlistviewmulticolcommon.h"

#include <DPaletteHelper>

#include <QAbstractItemView>
#include <QApplication>
#include <QHelpEvent>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QToolTip>

DWIDGET_USE_NAMESPACE

DListViewMultiColDelegate::DListViewMultiColDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DListViewMultiColDelegate::paint(QPainter *painter,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    const QAbstractItemModel *ItemModel = index.model();

    QStyleOptionViewItem styleOption = option;
    int left = styleOption.rect.left();
    int top = styleOption.rect.top();
    int width = styleOption.rect.width();
    int height = styleOption.rect.height();

    // 画背景
    QRect backgroundRect = QRect(left + 5, top, width - 10, height);
    // 判断奇偶
    bool drawBackground = ((index.row() & 1) == 1);
    if (drawBackground) {
        QPainterPath path;
        path.addRoundedRect(backgroundRect, 8, 8);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);

        DPalette pa = DPaletteHelper::instance()->palette(styleOption.widget);
        QBrush backgroundBrush = pa.brush(DPalette::ColorType::ItemBackground);

        painter->fillPath(path, backgroundBrush);
        painter->restore();
    }

    // 画当鼠标悬浮此处时的背景
    bool isHovering = (QStyle::StateFlag::State_MouseOver & option.state);
    if (isHovering) {
        QPainterPath path;
        path.addRoundedRect(backgroundRect, 8, 8);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);

        DPalette pa = DPaletteHelper::instance()->palette(styleOption.widget);
        QBrush backgroundBrush = pa.brush(DPalette::ColorType::ObviousBackground);

        painter->fillPath(path, backgroundBrush);
        painter->restore();
    }

    // 画当被选中时的背景
    bool isSelection = (QStyle::StateFlag::State_Selected & option.state);
    if (isSelection) {
        QPainterPath path;
        path.addRoundedRect(backgroundRect, 8, 8);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);

        DPalette pa = DPaletteHelper::instance()->palette(styleOption.widget);
        QBrush backgroundBrush = pa.brush(DPalette::ColorRole::Highlight);

        painter->fillPath(path, backgroundBrush);
        painter->restore();
    }

    // 画本行各列内容
    int leftColWidth = 0;
    QFontMetrics fontMetrics = styleOption.fontMetrics;
    QList<int> headerWidthList =
        ItemModel->property(PROPERTY_KEY_HEADER_SECTION_WIDTH_LIST).value<QList<int>>();
    for (int i = 0; i < headerWidthList.size(); ++i) {
        int thisColWidth = headerWidthList[i];

        // 获取列数据
        QModelIndex colIndex = ItemModel->index(index.row(), i);

        // icon
        painter->save();
        int iconSectionWidth = 0;
        QVariant decorationVar = ItemModel->data(colIndex, Qt::ItemDataRole::DecorationRole);
        bool haveIcon = (QVariant::Type::Icon == decorationVar.type());
        if (haveIcon) {
            QIcon icon = ItemModel->data(colIndex, Qt::ItemDataRole::DecorationRole).value<QIcon>();
            QSize iconSize = ItemModel->property(PROPERTY_KEY_ITEM_ICON_SIZE).value<QSize>();
            int iconYOffset = (height - iconSize.height()) / 2; // 为垂直居中画图标
            int iconLeftSpace = 10;
            QRect iconRect(iconLeftSpace + left + leftColWidth,
                           top + iconYOffset,
                           iconSize.width(),
                           iconSize.height());
            iconSectionWidth = iconLeftSpace + iconSize.width();
            icon.paint(painter, iconRect);
        }
        painter->restore();

        // 画文字
        // 文字颜色
        painter->save();
        QVariant colorVar = ItemModel->data(colIndex, Qt::ForegroundRole);
        if (QVariant::Type::Color == colorVar.type()) {
            painter->setPen(colorVar.value<QColor>());
        }

        // 文字左侧空隙
        int textLeftSpace = 10;
        // 判断是否为首列
        if (0 == i) {
            textLeftSpace = 18;
        }
        // 随选中状态设置画笔颜色
        if (isSelection) {
            painter->setPen(option.palette.color(QPalette::ColorRole::HighlightedText));
        }
        int textYOffset = (height - fontMetrics.height()) / 2; // 为垂直居中画文字
        QRect textRect(left + leftColWidth + iconSectionWidth + textLeftSpace,
                       top + textYOffset,
                       thisColWidth - iconSectionWidth - textLeftSpace,
                       height);
        QString displayText = ItemModel->data(colIndex, Qt::ItemDataRole::DisplayRole).toString();
        QString elidedDisplayText =
            fontMetrics.elidedText(displayText,
                                   Qt::TextElideMode::ElideMiddle,
                                   thisColWidth - iconSectionWidth - textLeftSpace);
        // 为防止文本换行，设置Qt::TextFlag::TextSingleLine属性
        painter->drawText(textRect, Qt::TextFlag::TextSingleLine, elidedDisplayText);
        leftColWidth += thisColWidth;
        painter->restore();
    }
}

bool DListViewMultiColDelegate::helpEvent(QHelpEvent *event,
                                          QAbstractItemView *view,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index)
{
    const QAbstractItemModel *ItemModel = index.model();
    if (!ItemModel) {
        return QStyledItemDelegate::helpEvent(event, view, option, index);
    }
    QList<int> headerWidthList =
        ItemModel->property(PROPERTY_KEY_HEADER_SECTION_WIDTH_LIST).value<QList<int>>();
    if (event->type() == QEvent::ToolTip) {
        int cursorPosX = event->pos().x();
        // 获取表头水平偏移，定表头初始x坐标
        int selectionLeftX = ItemModel->property(PROPERTY_KEY_HEADER_X_OFFSET).toInt();
        int colCurrentPos = 0;
        // 判断当前处于的列数
        for (int i = 0; i < headerWidthList.size(); ++i) {
            selectionLeftX += headerWidthList[i];
            if (cursorPosX < selectionLeftX) {
                colCurrentPos = i;
                break;
            }
        }
        // 关闭提示标签
        QWidgetList qwl = QApplication::topLevelWidgets();
        for (QWidget *qw : qwl) {
            if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
                qw->close();
            }
        }

        // 重新显示标签
        QModelIndex currentIndex = ItemModel->index(index.row(), colCurrentPos);
        const QString tooltip = currentIndex.data(Qt::ToolTipRole).toString();
        if (!tooltip.isEmpty()) {
            int tooltipsize = tooltip.size();
            const int nlong = 32;
            int lines = tooltipsize / nlong + 1;
            QString strtooltip;
            for (int i = 0; i < lines; ++i) {
                strtooltip.append(tooltip.mid(i * nlong, nlong));
                strtooltip.append("\n");
            }
            strtooltip.chop(1);
            QToolTip::showText(event->globalPos(), strtooltip, view);
        }

        return false;
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize DListViewMultiColDelegate::sizeHint(const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
{
    // 原始尺寸
    QSize originSize = QStyledItemDelegate::sizeHint(option, index);
    // 设定的高度
    int rowHeight = index.model()->property(PROPERTY_KEY_ROW_HEIGHT).toInt();

    // 为自适应，最小设定高度为原始高度
    if (originSize.height() > rowHeight) {
        rowHeight = originSize.height();
    }

    return QSize(originSize.width(), rowHeight);
}

QWidget *DListViewMultiColDelegate::createEditor(QWidget *parent,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index) const
{
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void DListViewMultiColDelegate::updateEditorGeometry(QWidget *editor,
                                                     const QStyleOptionViewItem &option,
                                                     const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void DListViewMultiColDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QStyledItemDelegate::setEditorData(editor, index);
}
