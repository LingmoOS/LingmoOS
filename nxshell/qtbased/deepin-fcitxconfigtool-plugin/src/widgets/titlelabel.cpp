// Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlelabel.h"

#include <DFontSizeManager>

#include <QFont>
#include <QEvent>

DWIDGET_USE_NAMESPACE

FcitxTitleLabel::FcitxTitleLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    auto tf = this->font();
    tf.setWeight(QFont::Medium);
    setFont(tf);

    //DFontSizeManager::instance()->bind(this, DFontSizeManager::T5);
}

FcitxTitleLabel::FcitxTitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f)
    : FcitxTitleLabel(parent, f)
{
    setText(text);
}

bool FcitxTitleLabel::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationFontChange) {
        auto tf = this->font();
        tf.setWeight(QFont::Medium);
        setFont(tf);
    }

    return QLabel::event(e);
}
