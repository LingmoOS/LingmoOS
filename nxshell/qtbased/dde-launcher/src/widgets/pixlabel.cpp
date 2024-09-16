// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pixlabel.h"

#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

PixLabel::PixLabel(QWidget *parent)
    : QLabel (parent)
{
}

void PixLabel::setContent(const QPixmap pix, const QString str)
{
    if (pix.isNull()) {
        qDebug() << "pixmap is null";
        return;
    }

    m_pixmap = pix;
    m_text = str;
    update();
}

void PixLabel::paintEvent(QPaintEvent *e)
{
    QLabel::paintEvent(e);

    QPainter painter(this);

    QFontMetrics fontMetric(this->font());
    int textHeight = fontMetric.boundingRect(m_text).height();

    // 文本居中置底, 图片居中
    QTextOption option;
    option.setWrapMode(QTextOption::WordWrap);
    option.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    int width = (this->rect().width() - m_pixmap.width()) / 2;
    int height = (this->rect().height() - m_pixmap.height() - textHeight) / 2;
    QRect targetRect = QRect(this->rect().topLeft() + QPoint(width, height), m_pixmap.size());
    QRect sourceRect = m_pixmap.rect();
    painter.drawPixmap(targetRect, m_pixmap, sourceRect);
    painter.drawText(QRectF(this->rect()), m_text, option);
}
