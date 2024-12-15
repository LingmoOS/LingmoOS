// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "autoindexstatus.h"

#include <DFontSizeManager>
#include <DIconButton>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

AutoIndexStatus::AutoIndexStatus(QWidget *parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 0, 10, 0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_text = new QLabel(this);
    m_text->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_text->setWordWrap(true);
    DFontSizeManager::instance()->bind(m_text, DFontSizeManager::T8);

    m_icon = new QLabel(this);
    m_icon->setFixedSize(30, 30);
    m_icon->setMargin(0);

    QWidget *spinnerContainer = new QWidget(this);
    {
        spinnerContainer->setFixedSize(30, 20); //10px space on right.
        m_spinner = new DSpinner(spinnerContainer);
        m_spinner->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_spinner->setFocusPolicy(Qt::NoFocus);
        m_spinner->setFixedSize(QSize(20, 20));
        m_spinner->setGeometry(5, 0, 20, 20);
        spinnerContainer->hide();
    }

    m_layout->addWidget(spinnerContainer);
    m_layout->addWidget(m_icon);
    m_layout->addSpacing(4);
    m_layout->addWidget(m_text);
}

void AutoIndexStatus::updateContent(AutoIndexStatus::Status st, const QString &text)
{
    m_text->setText(text);

    switch (st) {
    case Success:
        m_icon->setPixmap(QIcon::fromTheme("icon_ok").pixmap(m_icon->size()));
        m_icon->show();
        m_spinner->parentWidget()->hide();
        m_spinner->stop();
        break;
    case Updating:
        m_icon->hide();
        m_spinner->parentWidget()->show();
        m_spinner->start();
        break;
    case Fail:
        m_icon->setPixmap(QIcon::fromTheme("icon_warning").pixmap(m_icon->size()));
        m_icon->show();
        m_spinner->parentWidget()->hide();
        m_spinner->stop();
        break;
    default:
        break;
    }
}
