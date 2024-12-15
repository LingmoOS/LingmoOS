// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QFont>
#include <QColor>
#include <QPixmap>

#include "errormessagewidget.h"
#include "statusicon.h"

ErrorMessageWidget::ErrorMessageWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_iconLabel(new SimpleLabel())
    , m_titleLabel(new SimpleLabel())
    , m_errorTextEdit(new DTextEdit(this))
{
    initUI();
}

void ErrorMessageWidget::initUI()
{
    QIcon iconFailed(":/icons/icon_fail.svg");
    m_iconLabel->setPixmap(iconFailed.pixmap(128, 128));
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T4, QFont::Bold);

    m_titleLabel->setForegroundRole(DPalette::TextTitle);

    m_errorTextEdit->setCurrentFont(QFont("SourceHanSansSC", 9, QFont::Normal));
    m_errorTextEdit->setForegroundRole(QPalette::Text);
    QPalette p = m_errorTextEdit->viewport()->palette();
    p.setColor(DPalette::Window, QColor("#07000000"));
    m_errorTextEdit->viewport()->setPalette(p);
    m_errorTextEdit->setAutoFillBackground(false);
    m_errorTextEdit->setReadOnly(true);
    m_errorTextEdit->setFixedSize(472, 122);

    m_mainLayout->setContentsMargins(0, 50, 0, 50);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(30);
    m_mainLayout->addWidget(m_errorTextEdit, 0, Qt::AlignCenter);
}
