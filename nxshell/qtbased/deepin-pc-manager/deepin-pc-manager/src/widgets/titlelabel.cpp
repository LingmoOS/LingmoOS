// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlelabel.h"

#include <DFontSizeManager>

#include <QEvent>
#include <QFont>

DWIDGET_USE_NAMESPACE

TitleLabel::TitleLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    auto tf = this->font();
    tf.setWeight(QFont::Medium);
    tf.setPixelSize(17);
    setFont(tf);
}

TitleLabel::TitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : TitleLabel(parent, f)
{
    setText(text);
}

bool TitleLabel::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationFontChange) {
        auto tf = this->font();
        tf.setWeight(QFont::Medium);
        setFont(tf);
    }

    return QLabel::event(e);
}
