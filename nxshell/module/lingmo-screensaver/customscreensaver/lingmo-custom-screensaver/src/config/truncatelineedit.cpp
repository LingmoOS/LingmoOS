// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "truncatelineedit.h"

#include <QResizeEvent>
#include <QFontMetrics>
#include <QStyleOptionFrame>
#include <QPainter>
#include <QDebug>

DWIDGET_USE_NAMESPACE

TruncateLineEdit::TruncateLineEdit(QWidget *parent)
    : DLineEdit(parent)
{
    this->lineEdit()->setReadOnly(true);
    setFocusPolicy(Qt::NoFocus);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void TruncateLineEdit::setText(const QString &text)
{
    m_fullText = text;
    updateShowText();
}

QString TruncateLineEdit::getText() const
{
    return m_fullText;
}

void TruncateLineEdit::setElideMode(Qt::TextElideMode elideMode)
{
    m_elideMode = elideMode;
    updateShowText();
}

Qt::TextElideMode TruncateLineEdit::elideMode() const
{
    return m_elideMode;
}

void TruncateLineEdit::resizeEvent(QResizeEvent *event)
{
    DLineEdit::resizeEvent(event);

    updateShowText();
}

void TruncateLineEdit::updateShowText()
{
    QStyleOptionFrame panel;
    panel.init(this->lineEdit());
    QRect contentsRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this->lineEdit());

    // QLineEdit绘制显示时，将扣除内容边距，所以计算截断文本时，需要使用内容宽度，而不是控件宽度
    QFontMetrics fontMetrics(this->font());
    m_elideText = fontMetrics.elidedText(m_fullText, m_elideMode, contentsRect.width());

    DLineEdit::setText(m_elideText);
    DLineEdit::setToolTip(m_fullText);
}
