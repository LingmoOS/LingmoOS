// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hboxlayoutwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <DArrowLineExpand>
#include <DWidget>
#include <DSplitter>
#include <QComboBox>

#include "boxlayoutwidget.h"
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
HBoxLayoutWidget::HBoxLayoutWidget(QWidget *parent): AttributeWgt(-1, parent)
{
    initUi();
}

void HBoxLayoutWidget::setTitle(const QString &title)
{
    m_label->setText(title);
}

void HBoxLayoutWidget::addWidget(QWidget *widget, int stretch)
{
    m_lay->addWidget(widget, stretch);
}

QHBoxLayout *HBoxLayoutWidget::getLayout()
{
    return m_lay;
}

DLabel *HBoxLayoutWidget::getLabel()
{
    return m_label;
}

void HBoxLayoutWidget::initUi()
{
    m_label = new DLabel(this);
    m_label->setText("arrow");
    QVBoxLayout *fillLayout = new QVBoxLayout(this);
    fillLayout->setMargin(0);
    fillLayout->setContentsMargins(0, 0, 0, 0);
    m_lay = new QHBoxLayout;
    m_lay->setContentsMargins(0, 0, 0, 0);
    fillLayout->addWidget(m_label);
    fillLayout->addLayout(m_lay);
    setLayout(fillLayout);
}

