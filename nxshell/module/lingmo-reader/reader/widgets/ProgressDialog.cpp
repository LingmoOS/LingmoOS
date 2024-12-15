// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ProgressDialog.h"

#include <QPainter>
#include <QProgressBar>
#include <QLabel>
#include <DLabel>

#include <DFontSizeManager>

ProgressDialog::ProgressDialog(QWidget *parent): DDialog(parent)
{
    setFixedSize(440, 100);

    QMargins mar(0, 0, 0, 30);
    setContentLayoutContentsMargins(mar);

    Dtk::Widget::DLabel *label = new Dtk::Widget::DLabel(this);
    label->setText(tr("Converting"));
    label->move(20, 10);
    Dtk::Widget::DFontSizeManager::instance()->bind(label, Dtk::Widget::DFontSizeManager::T6, true);

    m_progress = new QProgressBar(this);
    m_progress->resize(400, 20);
    m_progress->move(20, 60);
    m_progress->setValue(50);
}
