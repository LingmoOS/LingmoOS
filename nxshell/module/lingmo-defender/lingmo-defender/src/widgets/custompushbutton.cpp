// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custompushbutton.h"

#include <QHBoxLayout>
#include <QIcon>
#include <DFontSizeManager>

CustomPushbutton::CustomPushbutton(QWidget *parent)
    : QWidget(parent)
    , m_button(nullptr)
{
    // 布局
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);
    m_button = new DPushButton(this);
    m_button->setIconSize(QSize(16, 16));
    m_button->setText("ssssss");
    m_button->setFlat(true);
    QFont ft = m_button->font();
    m_button->setFont(ft);
    DFontSizeManager::instance()->bind(m_button, DFontSizeManager::T9);
    hlayout->addWidget(m_button);
    this->setLayout(hlayout);

    // 字体样式
    QPalette pal;
    pal.setColor(QPalette::Text, QColor("#001A2E"));
    m_button->setPalette(pal);
    // 点击信号
    connect(m_button, &DPushButton::clicked, this, &CustomPushbutton::clicked);
}

void CustomPushbutton::setIcon(QIcon icon)
{
    m_button->setIcon(icon);
}

void CustomPushbutton::setText(QString string)
{
    m_button->setText(string);
}

void CustomPushbutton::enterEvent(QEvent *)
{
    m_button->setFlat(false);
}

void CustomPushbutton::leaveEvent(QEvent *)
{
    m_button->setFlat(true);
}
