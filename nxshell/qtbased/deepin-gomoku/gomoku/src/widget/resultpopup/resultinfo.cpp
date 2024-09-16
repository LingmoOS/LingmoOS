// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resultinfo.h"
#include "globaltool.h"

Resultinfo::Resultinfo(QWidget *parent)
    : DLabel(parent)
    , mResult(true)
    , strResult("")
{
    //设置大小
    setFixedSize(332, 48);
}

Resultinfo::~Resultinfo()
{

}

/**
 * @brief Resultinfo::setResult 设置对局结果
 * @param result 对局结果:true为赢, false为输
 */
void Resultinfo::setResult(bool result)
{
    mResult = result;
    if (mResult) {
        strResult = tr("You did great!");
    } else {
        strResult = tr("Game Over");
    }
}

/**
 * @brief Resultinfo::paintEvent 绘图事件
 * @param event
 */
void Resultinfo::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.save();
    painter.setPen("#353535");
    QFont font;
    font.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    font.setWeight(QFont::Bold);
    font.setPixelSize(Globaltool::instacne()->getFontSize().dialogLabel
                      - Globaltool::instacne()->getFontSize().dialogOffset);
    painter.setFont(font);
    painter.drawText(this->rect(), Qt::AlignHCenter | Qt::AlignVCenter, strResult);
    painter.restore();

    DLabel::paintEvent(event);
}
