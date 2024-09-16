// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TransparentTextEdit.h"
#include "Application.h"
#include "MsgHeader.h"

#include <DFontSizeManager>

#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QMimeData>
#include <QMenu>
#include <QPointF>

TransparentTextEdit::TransparentTextEdit(DWidget *parent)
    : QTextEdit(parent)
{
    this->setObjectName("TransparentTextEdit");

    this->setAcceptRichText(false);

    this->setWordWrapMode(QTextOption::WrapAnywhere);

    this->setViewportMargins(0, 0, 10, 0);

    // background color
    QPalette pText = this->palette();

    pText.setColor(QPalette::Base, QColor(255, 251, 225, 0));

    pText.setColor(QPalette::Text, QColor(0, 0, 0));

    this->setPalette(pText);

    DFontSizeManager::instance()->bind(this, DFontSizeManager::T8);

    this->setFrameStyle(QFrame::NoFrame);

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, SIGNAL(textChanged()), this, SLOT(slotTextEditMaxContantNum()));
}

void TransparentTextEdit::slotTextEditMaxContantNum()
{
    QString textContent = this->toPlainText();

    int length = textContent.count();

    if (length > m_nMaxContantLen) {
        int position = this->textCursor().position();

        QTextCursor textCursor = this->textCursor();

        textContent.remove(position - (length - m_nMaxContantLen), length - m_nMaxContantLen);

        this->setText(textContent);

        textCursor.setPosition(position - (length - m_nMaxContantLen));

        this->setTextCursor(textCursor);

        sigNeedShowTips(tr("Input limit reached"), 1);
    }
}

void TransparentTextEdit::paintEvent(QPaintEvent *event)
{
    QTextEdit::paintEvent(event);

    QPainter painter(this->viewport());

    painter.setRenderHints(QPainter::Antialiasing);

    int maxLineHeight = 2;

    int totalheight = this->viewport()->height() - maxLineHeight;

    const QFontMetricsF &fontmetricsf = QFontMetricsF(this->font());

    qreal lineheight = fontmetricsf.height();

    painter.setBrush(Qt::NoBrush);

    QPen pen(QColor(219, 189, 119), maxLineHeight);

    painter.setPen(pen);

    int startLine = static_cast<int>(this->document()->documentMargin() - this->verticalScrollBar()->value());

    painter.drawLine(2, startLine, this->viewport()->width() - 4, startLine);

    pen.setWidth(1);

    painter.setPen(pen);

    qreal curh;

    for (curh = startLine + lineheight; curh <= totalheight; curh += lineheight) {
        painter.drawLine(QPointF(2.0, curh), QPointF(this->viewport()->width() * 1.0 - 4.0, curh));
    }

    if (this->verticalScrollBar()->maximum() - this->verticalScrollBar()->value() < maxLineHeight) {
        pen.setWidth(maxLineHeight);

        painter.setPen(pen);

        curh -= lineheight;

        painter.drawLine(QPointF(2.0, curh), QPointF(this->viewport()->width() * 1.0 - 4.0, curh));
    }
}

void TransparentTextEdit::insertFromMimeData(const QMimeData *source)
{
    if (!source->text().isEmpty())
        this->insertPlainText(source->text());
}

void TransparentTextEdit::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_M && (keyEvent->modifiers() & Qt::AltModifier) && !keyEvent->isAutoRepeat()) {
        QMenu *menu =  this->createStandardContextMenu();

        if (menu) {
            menu->exec(this->cursor().pos());
            delete  menu;
            menu = nullptr;
        }
        return;
    }

    QTextEdit::keyPressEvent(keyEvent);
}

void TransparentTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextEdit::focusOutEvent(event);

    Q_EMIT sigCloseNoteWidget();
}
