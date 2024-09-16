// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"

#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <qtoolbutton.h>

#ifdef USE_DTK
#include <DGuiApplicationHelper>
DGUI_USE_NAMESPACE
#endif

ToolButton::ToolButton(QWidget *parent) : QPushButton(parent)
{

}

void ToolButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);

    QPainter painter(this);
    //绘制背景
    painter.save();
    //非禁用状态绘制背景
    if (option.state & QStyle::State_Enabled) {
        if (option.state & QStyle::State_MouseOver) {
            painter.setPen(Qt::NoPen);
            //获取系统主题颜色
            QColor hovertColor(option.palette.highlight().color());
            painter.setBrush(hovertColor);
            painter.drawRect(this->rect());
        }
    }
    painter.restore();

    painter.setPen(getPen(option));

    painter.save();
    painter.translate(20, 0);

    // 绘制图片
    icon().paint(&painter, QRect(-14, (height() - 45) / 2, 45, 45));

    //绘制文字
    QFont ft = QFont();
    painter.setFont(ft);
    QRect textRect(28, 5, width() - 28, height() - 10);
    painter.drawText(textRect, text(), QTextOption(Qt::AlignVCenter | Qt::AlignLeft));
    painter.restore();
}

QPen ToolButton::getPen(const QStyleOptionButton option)
{
    QPen pen;
    int style = 1;
#ifdef USE_DTK
    style = DGuiApplicationHelper::instance()->themeType();
#endif
    if (style == 1) {
        //控件禁用样式
        if (!(option.state & QStyle::State_Enabled)) {
            pen = QPen(QColor("#9C9C9C"));
        }

        //鼠标悬停画笔颜色
        else if (option.state & QStyle::State_MouseOver) {
            pen = QPen(QColor(Qt::white));
        }

        //鼠标按下画笔颜色
        else if (option.state & QStyle::State_Sunken) {
            pen = QPen(QColor("#99cdff"));
        } else {
            pen = QPen(QColor("#343434"));
        }

    } else {

        //控件禁用样式
        if (!(option.state & QStyle::State_Enabled)) {
            pen = QPen(QColor("#8D8D8D"));
        }

        //鼠标悬停画笔颜色
        else if (option.state & QStyle::State_MouseOver) {
            pen = QPen(QColor(Qt::white));
        }

        //鼠标按下画笔颜色
        else if (option.state & QStyle::State_Sunken) {
            pen = QPen(QColor("#99cdff"));
        } else {

            pen = QPen(QColor(Qt::white));
        }
    }
    return pen;
}










