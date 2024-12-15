// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logspinnerwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

LogSpinnerWidget::LogSpinnerWidget(QWidget *parent)
    : Dtk::Widget::DWidget(parent)
{
    initUI();
}
/**
 * @brief LogSpinnerWidget::initUI  主表加载数据时的spinner
 * 把DSpinner通过弹簧放在布局中间
 */
void LogSpinnerWidget::initUI()
{
    m_spinner = new DSpinner(this);
    m_spinner->setFixedSize(QSize(32, 32));

    //    QHBoxLayout *hh = new QHBoxLayout(this);

    QHBoxLayout *h = new QHBoxLayout();
    h->addStretch(1);

    QVBoxLayout *v = new QVBoxLayout();
    v->addStretch(1);
    v->addWidget(m_spinner);
    v->addStretch(1);

    h->addLayout(v);
    h->addStretch(1);

    //    hh->addLayout(h);
    this->setLayout(h);
}

void LogSpinnerWidget::spinnerStart()
{
    m_spinner->start();
}

void LogSpinnerWidget::spinnerStop()
{
    m_spinner->stop();
}
