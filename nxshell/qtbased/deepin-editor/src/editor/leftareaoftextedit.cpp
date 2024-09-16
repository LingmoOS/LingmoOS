// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "leftareaoftextedit.h"
#include "linenumberarea.h"
#include "bookmarkwidget.h"
#include "codeflodarea.h"
#include "dtextedit.h"

#include <QHBoxLayout>
#include <QDebug>


LeftAreaTextEdit::LeftAreaTextEdit(TextEdit *textEdit) :
    m_pTextEdit(textEdit)
{
    QHBoxLayout *pHLayout = new QHBoxLayout(this);
    m_pLineNumberArea = new LineNumberArea(this);
    m_pBookMarkArea = new BookMarkWidget(this);
    m_pFlodArea = new CodeFlodArea(this);

    m_pBookMarkArea->setContentsMargins(0, 0, 0, 0);
    m_pFlodArea->setContentsMargins(0, 0, 0, 0);
    m_pLineNumberArea->setContentsMargins(0, 0, 0, 0);
    //m_pBookMarkArea->setFixedWidth(14);
   // m_pFlodArea->setFixedWidth(18);
    //m_pBookMarkArea->setFixedWidth(textEdit->cursorRect(textEdit->textCursor()).height());
   // m_pFlodArea->setFixedWidth(textEdit->cursorRect(textEdit->textCursor()).height());

    pHLayout->addWidget(m_pBookMarkArea);
    pHLayout->addWidget(m_pLineNumberArea);
    pHLayout->addWidget(m_pFlodArea);
    pHLayout->setContentsMargins(0, 0, 0, 0);
    pHLayout->setSpacing(0);
    this->setLayout(pHLayout);
}

LeftAreaTextEdit::~LeftAreaTextEdit()
{

}

void LeftAreaTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    m_pTextEdit->lineNumberAreaPaintEvent(event);
}

int LeftAreaTextEdit::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, m_pTextEdit->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    return 13 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits + 40;
}


void LeftAreaTextEdit::bookMarkAreaPaintEvent(QPaintEvent *event)
{
    m_pTextEdit->bookMarkAreaPaintEvent(event);
}

void LeftAreaTextEdit::codeFlodAreaPaintEvent(QPaintEvent *event)
{
    m_pTextEdit->codeFLodAreaPaintEvent(event);
}

void LeftAreaTextEdit::updateLineNumber()
{
    if (m_pLineNumberArea) m_pLineNumberArea->update();
}

void LeftAreaTextEdit::updateBookMark()
{
    if (m_pBookMarkArea) m_pBookMarkArea->update();
}

void LeftAreaTextEdit::updateCodeFlod()
{
    if (m_pFlodArea) m_pFlodArea->update();
}

void LeftAreaTextEdit::updateAll()
{
    if (m_pLineNumberArea) m_pLineNumberArea->update();
    if (m_pBookMarkArea) m_pBookMarkArea->update();
    if (m_pFlodArea) m_pFlodArea->update();
}

void LeftAreaTextEdit::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QColor bdColor;
    if (m_pTextEdit->getBackColor().lightness() < 128) {
        bdColor = palette().brightText().color();
        bdColor.setAlphaF(0.06);
    } else {
        bdColor = palette().brightText().color();
        bdColor.setAlphaF(0.03);
    }
    painter.fillRect(event->rect(), bdColor);
}


TextEdit* LeftAreaTextEdit::getEdit()
{
    return m_pTextEdit;
}


