// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "custompushbutton.h"

#include <DFontSizeManager>

#include <QHBoxLayout>
#include <QIcon>

CustomPushbutton::CustomPushbutton(QWidget *parent)
    : QWidget(parent)
    , m_button(nullptr)
{
    // 布局
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);
    m_button = new DPushButton(this);
    QString buttonAccName = this->accessibleName().append("_button");
    m_button->setAccessibleName(buttonAccName);
    m_button->setIconSize(QSize(16, 16));
    m_button->setText("ssssss");
    m_button->setFlat(true);
    QFont ft = m_button->font();
    ft.setPixelSize(12);
    m_button->setFont(ft);
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

QFont CustomPushbutton::getButtonFont()
{
    return m_button->font();
}

void CustomPushbutton::enterEvent(QEvent *)
{
    m_button->setFlat(false);
}

void CustomPushbutton::leaveEvent(QEvent *)
{
    m_button->setFlat(true);
}
