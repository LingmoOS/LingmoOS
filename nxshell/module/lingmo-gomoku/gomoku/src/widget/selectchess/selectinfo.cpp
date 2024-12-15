// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "selectinfo.h"
#include "globaltool.h"

Selectinfo::Selectinfo(QWidget *parent)
    : DLabel(parent)
{
    //设置大小
    setFixedSize(226, 40);
}

/**
 * @brief Selectinfo::paintEvent 绘图函数
 * @param event paintevent
 */
void Selectinfo::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setPen("#02412c");
    QFont font;
    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Bold);
    font.setPixelSize(Globaltool::instacne()->getFontSize().dialogLabel
                      - (Globaltool::instacne()->getFontSize().dialogOffset * 2) - 1); //选子弹窗标签字体大小为18
    painter.setFont(font);
    QString str = tr("Play as");
    painter.drawText(this->rect(), Qt::AlignHCenter | Qt::AlignBottom, str);
    painter.restore();
    DLabel::paintEvent(event);
}
