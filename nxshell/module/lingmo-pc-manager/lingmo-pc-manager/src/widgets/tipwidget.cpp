// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipwidget.h"

#include <DPalette>

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE

TipWidget::TipWidget(QWidget *parent)
    : DFrame(parent)
{
    QHBoxLayout *m_layout = new QHBoxLayout;
    setLineWidth(0);
    m_layout->setContentsMargins(10, 2, 10, 2);
    setMinimumSize(336, 48);

    m_leftIcon = new QLabel(this);

    m_tipTitle = new QLabel(this);
    m_tipTitle->setWordWrap(true);
    m_tipTitle->adjustSize();

    m_layout->addWidget(m_leftIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_layout->addWidget(m_tipTitle, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_layout->addStretch(1);
    setLayout(m_layout);
}

void TipWidget::setTitle(QString title)
{
    m_tipTitle->setText(title);
}

void TipWidget::setIcon(QIcon icon)
{
    m_leftIcon->setPixmap(icon.pixmap(20, 20));
}
