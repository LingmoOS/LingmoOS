// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "tipswidget.h"

#include <QPainter>
#include <QAccessible>
#include <QTextDocument>

#define PADDING 4

TipsWidget::TipsWidget(QWidget *parent)
    : QFrame(parent)
    , m_type(SingleLine)
{
}

void TipsWidget::setText(const QString &text)
{
    m_type = TipsWidget::SingleLine;
    m_text = text;

    setFixedSize(fontMetrics().horizontalAdvance(m_text) + 20, fontMetrics().boundingRect(m_text).height() + PADDING);

    update();
}

void TipsWidget::setTextList(const QStringList &textList)
{
    m_type = TipsWidget::MultiLine;
    m_textList = textList;

    int width = 0;
    int height = 0;
    for (const QString& text : m_textList) {
        width = qMax(width, fontMetrics().horizontalAdvance(text));
        height += fontMetrics().boundingRect(text).height();
    }

    setFixedSize(width + 20, height + PADDING);

    update();
}

/**
 * @brief TipsWidget::paintEvent 任务栏插件提示信息绘制
 * @param event
 */
void TipsWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(palette().brightText(), 1));

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    switch (m_type) {
    case SingleLine: {
        painter.drawText(rect(), m_text, option);
    }
        break;
    case MultiLine: {
        int x = rect().x();
        int y = rect().y();
        if (m_textList.size() != 1) {
            x += 10;
            option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        for (const QString& text : m_textList) {
            int lineHeight = fontMetrics().boundingRect(text).height();
            painter.drawText(QRect(x, y, rect().width(), lineHeight), text, option);
            y += lineHeight;
        }
    } break;
    }
}

bool TipsWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        switch (m_type) {
        case SingleLine:
        {
            setText(m_text);
            break;
        }
        case MultiLine:
        {
            setTextList(m_textList);
            break;
        }
        }
    } else if (event->type() == QEvent::MouseButtonRelease
               && static_cast<QMouseEvent *>(event)->button() == Qt::RightButton) {
        return true;
    }
    return QFrame::event(event);
}
