// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "replicablelabel.h"
#include "global/accessibility/acintelfunctions.h"

#include <DGuiApplicationHelper>
#include <DStyle>

#include <QMouseEvent>
#include <QResizeEvent>

using namespace GrandSearch;
DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

const static int LineEditHorizontalMargin = 2;

ReplicableLabel::ReplicableLabel(QWidget *parent)
    : QLineEdit(parent)
{
    this->setStyleSheet("QLineEdit{background:transparent;border-width:0;border-style:outset;}");

    setReadOnly(true);
    setContextMenuPolicy(Qt::NoContextMenu);
}

void ReplicableLabel::setText(const QString &text)
{
    m_fullText = text;
}

void ReplicableLabel::setElideMode(Qt::TextElideMode elideMode)
{
    m_elideMode = elideMode;
}

Qt::TextElideMode ReplicableLabel::elideMode() const
{
    return m_elideMode;
}

bool ReplicableLabel::event(QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        m_pressed = false;

        QLineEdit::setText(m_elideText);

        update();
    }

    return QLineEdit::event(event);
}

void ReplicableLabel::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    setFocus();

    return QLineEdit::mousePressEvent(event);
}

void ReplicableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;

    return QLineEdit::mouseReleaseEvent(event);
}

void ReplicableLabel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed) {
        m_pressed = false;

        QLineEdit::setText(m_fullText);

        QPoint pos = event->pos();
        int point = cursorPositionAt(pos);
        setCursorPosition(point);
    }

    return QLineEdit::mouseMoveEvent(event);
}

void ReplicableLabel::resizeEvent(QResizeEvent *event)
{
    QFontMetrics fontMetrics(this->font());

    // 截断文本的显示宽度，需要扣除输入框控件两边的绘制间距
    int drawTextWidth = event->size().width() - LineEditHorizontalMargin * 2;
    m_elideText = fontMetrics.elidedText(m_fullText, m_elideMode, drawTextWidth);

    QLineEdit::setText(m_elideText);

    AC_SET_ACCESSIBLE_NAME(this, m_elideText);

    return QLineEdit::resizeEvent(event);
}
