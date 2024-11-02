/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "detailwidget.h"
#include <QDebug>

#define ITEM_HEIGHT 36
#define ITEM_MARGINS 18,0,16,0
#define MIN_LABEL_WIDTH 146
#define MAX_LABEL_WIDTH 154
#define MAX_WIDGET_WIDTH 270

DetailWidget::DetailWidget(QWidget *valueWidget, QWidget *parent, QWidget *buttonWidget)
    : m_valueWidget(valueWidget) , QWidget(parent) , m_copyButton(buttonWidget)
{
//    m_valueWidget = valueWidget;
    initUI();
}

DetailWidget::~DetailWidget()
{

}

void DetailWidget::initUI()
{
    this->setFixedHeight(ITEM_HEIGHT);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(ITEM_MARGINS);

    m_keyLabel = new FixLabel(this);
    m_keyLabel->setMinimumWidth(MIN_LABEL_WIDTH);
    m_keyLabel->setMaximumWidth(MAX_LABEL_WIDTH);
    m_keyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_mainLayout->addWidget(m_keyLabel);
    m_mainLayout->addStretch();
    if (m_copyButton != nullptr) {
        m_copyButton->setMaximumWidth(ITEM_HEIGHT-8);
        m_copyButton->setMaximumHeight(ITEM_HEIGHT-8);
        m_mainLayout->addWidget(m_copyButton);
    }
    m_mainLayout->addWidget(m_valueWidget);
    m_valueWidget->setMaximumWidth(MAX_WIDGET_WIDTH);
}

void DetailWidget::setKey(const QString &keyLabel)
{
    m_keyLabel->setLabelText(keyLabel);
}
