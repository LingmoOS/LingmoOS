// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TipsWidget.h"

#include <DPlatformWindowHandle>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

#include <QPainter>
#include <QLoggingCategory>

TipsWidget::TipsWidget(QWidget *parent) : DWidget(parent)
{
    setWindowFlags(Qt::ToolTip);
    initWidget();
    onUpdateTheme();
}

void TipsWidget::initWidget()
{
    m_tbMargin = 8;
    m_lrMargin = 10;
    m_maxLineCount = 10;
    m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    setFixedWidth(254);
    setWindowOpacity(0.97);
    DPlatformWindowHandle handle(this);
    handle.setWindowRadius(8);

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TipsWidget::onUpdateTheme);
}

void TipsWidget::onUpdateTheme()
{
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Background, plt.color(Dtk::Gui::DPalette::Base));
    setPalette(plt);
}

void TipsWidget::setText(const QString &text)
{
    if (text != m_text) {
        m_text = text;
        m_text.replace(QChar('\n'), QString(""));
        m_text.replace(QChar('\t'), QString(""));
        update();
    }
}

void TipsWidget::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
    update();
}

void TipsWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    QTextOption option;
    const QString &text = m_text;
    adjustContent(text);
    option.setAlignment(m_alignment);
    option.setWrapMode(QTextOption::WrapAnywhere);
    painter.setPen(QPen(DTK_NAMESPACE::Gui::DGuiApplicationHelper::instance()->applicationPalette().text().color()));
    painter.drawText(QRect(m_lrMargin, m_tbMargin, this->width() -  2 * m_lrMargin, this->height() -  2 * m_tbMargin), text, option);
}

void TipsWidget::adjustContent(const QString &text)
{
    QFontMetricsF fontMetris(this->font());
    int wordHeight = static_cast<int>(fontMetris.boundingRect(QRectF(0, 0, this->width() - 2 * m_lrMargin, 0),
                                                              static_cast<int>(m_alignment | Qt::TextWrapAnywhere), text).height() + 2 * m_tbMargin);
    if (this->height() == wordHeight) return;
    setFixedHeight(wordHeight);
}
