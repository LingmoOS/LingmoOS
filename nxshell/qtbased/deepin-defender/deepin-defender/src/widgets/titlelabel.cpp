// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "titlelabel.h"

#include <DFontSizeManager>

#include <QFont>
#include <QEvent>

DWIDGET_USE_NAMESPACE

TitleLabel::TitleLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    auto tf = this->font();
    tf.setWeight(QFont::Medium);
    setFont(tf);

    DFontSizeManager::instance()->bind(this,DFontSizeManager::T5);
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
