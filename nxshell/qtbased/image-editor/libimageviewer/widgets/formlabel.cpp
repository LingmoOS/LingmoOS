// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "formlabel.h"

SimpleFormLabel::SimpleFormLabel(const QString &t, QWidget *parent)
    : QLbtoDLabel(t, parent)
{
    QFont font;
    font.setPixelSize(12);
    setFont(font);
    setWordWrap(true);
}

void SimpleFormLabel::resizeEvent(QResizeEvent *event)
{
    if (wordWrap() && sizePolicy().verticalPolicy() == QSizePolicy::Minimum) {
        // heightForWidth rely on minimumSize to evaulate, so reset it before
        setMinimumHeight(0);
        // define minimum height
        setMinimumHeight(heightForWidth(width()));
    }
    QLbtoDLabel::resizeEvent(event);
}

SimpleFormField::SimpleFormField(QWidget *parent)
    : QLbtoDLabel(parent)
{
    QFont font;
    font.setPixelSize(12);
    setFont(font);
    //取消内容信息标签换行
    // setWordWrap(true);
}

void SimpleFormField::resizeEvent(QResizeEvent *event)
{
    if (wordWrap() && sizePolicy().verticalPolicy() == QSizePolicy::Minimum) {
        // heightForWidth rely on minimumSize to evaulate, so reset it before
        setMinimumHeight(0);
        // define minimum height
        setMinimumHeight(heightForWidth(width()));
    }
    QLbtoDLabel::resizeEvent(event);
}
