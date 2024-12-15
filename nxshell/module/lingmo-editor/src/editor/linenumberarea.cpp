// SPDX-FileCopyrightText: 2017-2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "linenumberarea.h"
#include "dtextedit.h"
#include "leftareaoftextedit.h"
#include <QDebug>
#include <QMouseEvent>

LineNumberArea::LineNumberArea(LeftAreaTextEdit *leftAreaWidget)
{
    m_leftAreaWidget = leftAreaWidget;
    setContentsMargins(0, 0, 0, 0);
    //m_textEdit = textEdit;
}

LineNumberArea::~LineNumberArea()
{
}

void LineNumberArea::paintEvent(QPaintEvent *e)
{
    //qDebug() << "LineNumberArea::paintEvent ";
    m_leftAreaWidget->lineNumberAreaPaintEvent(e);
    //m_textEdit->lineNumberAreaPaintEvent(e);
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(m_leftAreaWidget->lineNumberAreaWidth(), 0);
}


void LineNumberArea::mousePressEvent(QMouseEvent *e)
{
//    m_pressPoint = e->pos();
//    m_leftAreaWidget->update();
    m_leftAreaWidget->getEdit()->onPressedLineNumber(e->pos());
    QWidget::mousePressEvent(e);
}

QPoint LineNumberArea::getPressPoint()
{
    return m_pressPoint;
}
