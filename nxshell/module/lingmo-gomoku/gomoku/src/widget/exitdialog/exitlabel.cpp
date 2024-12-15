// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "exitlabel.h"
#include "globaltool.h"

ExitLabel::ExitLabel(QWidget *parent) : DLabel(parent)
{

    setWordWrap(true);
    setMinimumWidth(204);
    setMinimumHeight(66);
    setText(tr("Are you sure you want to exit the game?"));
    setAlignment(Qt::AlignCenter);

    QPalette palCancel;
    palCancel.setColor(QPalette::WindowText, QColor("#024526"));
    setPalette(palCancel);

    QFont tipFont;
    tipFont.setFamily(Globaltool::instacne()->loadFontFamilyFromFiles(":/resources/font/ResourceHanRoundedCN-Bold.ttf"));
    tipFont.setWeight(QFont::Bold);
    tipFont.setPixelSize(Globaltool::instacne()->getFontSize().dialogLabel
                            - Globaltool::instacne()->getFontSize().dialogOffset); //阻塞弹窗标签字体大小为22
    setFont(tipFont);
}
