// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tipcontentwidget.h"

#include <QPainter>
#include <QAccessible>
#include <QTextDocument>

#define NEWLINE_FLAG "\n"

TipContentWidget::TipContentWidget(QWidget *parent)
    : QFrame(parent)
    , m_type(SingleLine)
{

}

void TipContentWidget::setText(const QString &text, bool multiLine)
{
    QString showText = text;
    // 去掉所有行尾换行符
    while (showText.count() && showText.endsWith(NEWLINE_FLAG)) {
        showText.chop(1);
    }

    // 去掉行首换行符
    while (showText.count() && showText.at(0) == NEWLINE_FLAG) {
        showText.remove(0, 1);
    }

    if (showText.isEmpty()) {
        return;
    }

    if (text.contains(NEWLINE_FLAG) && multiLine) {
        setTextList(showText.split(NEWLINE_FLAG));
    } else {
        showSingleLineText(text);
    }
}

void TipContentWidget::setTextList(const QStringList &textList)
{
    m_type = TipContentWidget::MultiLine;
    m_textList = textList;

    int width = 0;
    int height = 0;
    for (QString text : m_textList) {
        width = qMax(width, fontMetrics().boundingRect(text).width() + 20);
        height += fontMetrics().boundingRect(text).height();
    }

    setFixedSize(width, height);

    update();
}

void TipContentWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QPainter painter(this);
    painter.setPen(QPen(palette().windowText(), 1));

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    switch (m_type) {
    case SingleLine:
        painter.drawText(rect(), m_text, option);
        break;
    case MultiLine: {
        int y = 0;
        if (m_textList.size() != 1)
            option.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        for (QString text : m_textList) {
            int lineHeight = fontMetrics().boundingRect(text).height();
            painter.drawText(QRect(10, y, rect().width(), lineHeight), text, option);
            y += lineHeight;
        }
    }
        break;
    }
}

bool TipContentWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        switch (m_type) {
        case SingleLine: {
            setText(m_text);
            break;
        }
        case MultiLine: {
            setTextList(m_textList);
            break;
        }
        }
    }

    return QFrame::event(event);
}

void TipContentWidget::showSingleLineText(const QString &text)
{
    m_type = TipContentWidget::SingleLine;
    // 如果传递的是富文本，获取富文本中的纯文本内容进行显示
    QTextDocument document;
    document.setHtml(text);
    // 同时去掉两边的空白信息，例如qBittorrent的提示
    m_text = document.toPlainText().simplified();

#if 0 //测试时可以使用下面的语句
    // FIXME:藏语字体绘制会有异常，设置高度时需要使用fontMetrics().boundingRect()去获取整体的边界矩形的高度，
    // 使用fontMetrics().height()去获取时，针对藏语这种字体，其高度和实际显示区域并不等同
    m_text = "བོད་སྐད་ཡིག་གཟུགས་ཚད་ལེན་ཚོད་ལྟའི་སྐོར་གྱི་རྗོད་ཚིག";
#endif

    setFixedSize(fontMetrics().boundingRect(m_text).width() + 20, fontMetrics().boundingRect(m_text).height());

    update();

#ifndef QT_NO_ACCESSIBILITY
    if (accessibleName().isEmpty()) {
        QAccessibleEvent event(this, QAccessible::NameChanged);
        QAccessible::updateAccessibility(&event);
    }
#endif
}
