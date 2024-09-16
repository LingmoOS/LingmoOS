// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "prolistdelegate.h"

#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

#include "prolistmodel.h"
#include "prolistview.h"
#include "../utils.h"

DWIDGET_USE_NAMESPACE

const int LEFT_MARGIN = 12; //prolistview左边距

ProListDelegate::ProListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

ProListDelegate::~ProListDelegate()
{

}

void ProListDelegate::setThemeType(int type)
{
    m_themeType = type;
}

void ProListDelegate::currentfocusindex(QModelIndex index)
{
    m_focusindex = index;
}

void ProListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
    const QString expression = index.data(ProListModel::ExpressionRole).toString();
    ProListView *listview = qobject_cast<ProListView *>(option.styleObject);
    QRect selectrect(option.rect.x() + LEFT_MARGIN, option.rect.y() + 2, 3, option.rect.height() - 4);  //被选中行选中标记
    QRect hoverrect(option.rect.x() + LEFT_MARGIN, option.rect.y() + 1, 427, option.rect.height() - 2);  //hover背景色
    QRect textrect(option.rect.x() + 80 + LEFT_MARGIN, option.rect.y(), 268, 24);  //textrect
    QRect systemrect(option.rect.x() + 10 + LEFT_MARGIN, option.rect.y(), 24, 24);  //进制字体框
    QRect systemrectbin(option.rect.x() + 10 + LEFT_MARGIN, option.rect.y(), 24, 28); //BIN进制字体框

    QColor hoverbackground, fontColor, systemfontColor;
    if (m_themeType == 1) {
        hoverbackground = QColor(0, 0, 0);
        hoverbackground.setAlphaF(0.03);
        fontColor = QColor("#838483");
        systemfontColor = QColor("#303030");
    } else {
        hoverbackground = QColor(255, 255, 255);
        hoverbackground.setAlphaF(0.05);
        fontColor = QColor("#B4B4B4");
        systemfontColor = QColor("#C0C6D4");
    }

    QFont font; //结果字体
    font.setPixelSize(14);
//    font.setLetterSpacing(QFont::PercentageSpacing, 0); //字间距0，未发现有效果
    QFont systemfont; //进制字体
    systemfont.setPixelSize(12);

    //选中框绘制
    if (index == listview->currentIndex()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color()));
        painter->drawRoundedRect(selectrect, 2, 2);
    }

    //进制字体绘制
    painter->setFont(systemfont);
    if (index == listview->currentIndex())
        painter->setPen(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color());
    else
        painter->setPen(systemfontColor);
    switch (index.row()) {
    case 0:
        painter->drawText(systemrect, "HEX", Qt::AlignHCenter | Qt::AlignVCenter);
        break;
    case 1:
        painter->drawText(systemrect, "DEC", Qt::AlignHCenter | Qt::AlignVCenter);
        break;
    case 2:
        painter->drawText(systemrect, "OCT", Qt::AlignHCenter | Qt::AlignVCenter);
        break;
    case 3:
        painter->drawText(systemrectbin, "BIN", Qt::AlignHCenter | Qt::AlignVCenter);
        break;

    }

    //悬浮背景绘制
    if (option.state & QStyle::State_MouseOver) {
        painter->setBrush(hoverbackground);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(hoverrect, 4, 4);
    }

    //结果字体绘制
    painter->setFont(font);
    painter->setPen(fontColor);
    if (index.row() == 3) {
        if (expression.size() > 39) {
            //显示64位时
            //第一行
            painter->drawText(QRect(option.rect.x() + 80 + LEFT_MARGIN, option.rect.y(), 280, 14),
                              expression.mid(0, 39), Qt::AlignLeft | Qt::AlignVCenter);
            //第二行
            painter->drawText(QRect(option.rect.x() + 80 + LEFT_MARGIN, option.rect.y() + 14, 280, 14),
                              expression.mid(40, -1), Qt::AlignLeft | Qt::AlignVCenter);
        } else {
            painter->drawText(QRect(option.rect.x() + 80 + LEFT_MARGIN, option.rect.y(), 280, 28), expression, Qt::AlignLeft | Qt::AlignVCenter);
        }
    } else
        painter->drawText(textrect, expression, Qt::AlignLeft | Qt::AlignVCenter);
    drawFocusStatus(painter, option); //焦点边框
}

QSize ProListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    if (index.row() == 3)
        return QSize(451, 28);
    else
        return QSize(451, 24);
}

bool ProListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(event);
    Q_UNUSED(model);
    Q_UNUSED(option);
    emit obtainingHistorical(index);
    return true;
}

void ProListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void ProListDelegate::drawFocusStatus(QPainter *painter, const QStyleOptionViewItem &option) const
{
    ProListView *listview = qobject_cast<ProListView *>(option.styleObject);
    if (listview->hasFocus()) {
        QRectF itemrect(listview->visualRect(m_focusindex));
        QRectF frame(itemrect.left() + 1, itemrect.top() + 1, itemrect.width() - 2, itemrect.height() - 2);
        QPen pen;
        pen.setColor(Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color().name());
        pen.setWidth(2);
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(frame); //focus边框
    }
}
