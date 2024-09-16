// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "endpage.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QDebug>

EndPage::EndPage(Mode mode, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *icon = new QLabel(this);

    if (mode == Success)
        icon->setPixmap(QIcon(":/icons/success.svg").pixmap(128));
    else if (mode == Fail)
        icon->setPixmap(QIcon(":/icons/fail.svg").pixmap(128));
    else
        icon->setPixmap(QIcon(":/icons/warning.svg").pixmap(128));

    m_title = new QLabel(this);
    m_message = new QLabel(this);
    m_message->setObjectName("MessageLabel");
    m_message->setWordWrap(true);
    m_message->setAlignment(Qt::AlignHCenter);

    layout->addStretch();
    layout->addWidget(icon, 0, Qt::AlignCenter);
    layout->addWidget(m_title, 0, Qt::AlignCenter);
    layout->addWidget(m_message, 0, Qt::AlignCenter);
    layout->addStretch();
}

void EndPage::setTitle(const QString &title)
{
    m_title->setText(title);
}

void EndPage::setMessage(const QString &message)
{
    m_message->setText(message);
}

void EndPage::resizeEvent(QResizeEvent *e)
{
    int width = this->width() * 0.606;

    m_message->setFixedSize(width, m_message->heightForWidth(width));

    return QWidget::resizeEvent(e);
}
