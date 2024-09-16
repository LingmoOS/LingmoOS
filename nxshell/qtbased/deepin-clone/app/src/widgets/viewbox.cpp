// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "viewbox.h"
#include "iconlabel.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>

ViewBox::ViewBox(QWidget *parent)
    : QWidget(parent)
{
    init();
}

void ViewBox::setTitle(const QString &title)
{
    m_title->setTitle(title);
}

void ViewBox::setIcon(const QIcon &icon)
{
    m_title->setIcon(icon);
}

void ViewBox::setContent(QWidget *widget, bool full)
{
    if (m_contentWidget) {
        m_borderFrameLayout->removeWidget(m_contentWidget);
        m_contentWidget->deleteLater();
    }

    m_contentWidget = widget;
    m_fullContent = full;
    m_borderFrameLayout->addWidget(widget, 0, Qt::AlignLeft | Qt::AlignTop);

    if (full) {
        m_contentWidget->setFixedSize((m_borderFrame->rect() - m_borderFrameLayout->contentsMargins()).size());
    }
}

QWidget *ViewBox::content() const
{
    return m_contentWidget;
}

void ViewBox::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_title = new IconLabel(this);
    m_borderFrame = new QFrame(this);
    m_borderFrame->setObjectName("BorderFrame");

    layout->addWidget(m_title);
    layout->addWidget(m_borderFrame);

    m_borderFrameLayout = new QVBoxLayout(m_borderFrame);
    m_borderFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_borderFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ViewBox::resizeEvent(QResizeEvent *e)
{
    if (m_contentWidget && m_fullContent)
        m_contentWidget->setFixedSize((m_borderFrame->rect() - m_borderFrameLayout->contentsMargins()).size());

    return QWidget::resizeEvent(e);
}
