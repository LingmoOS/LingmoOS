// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dselectedtexttooltip.h"

#include <QWindow>
#include <QPainter>
#include <QPaintEvent>
#include <QGuiApplication>
#include <QFontMetrics>

#include <QPalette>
#include <QDebug>

#define TEXT_SPACINGWIDGET 20
#define TEXT_SPACINGHEIGHT 10
#define WINDOWBOARD 1

DPP_BEGIN_NAMESPACE

DSelectedTextTooltip::DSelectedTextTooltip()
    : QRasterWindow()
    , m_borderColor(0, 0, 0, 0.05 *255)
{
    setFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus);

    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);

    // 这里借用QLineEdit的翻译
    m_textInfoVec.push_back({SelectAll, 0, qApp->translate("QLineEdit", "Select All")});
    m_textInfoVec.push_back({Cut, 0, qApp->translate("QLineEdit", "Cu&t").split("(").at(0)});
    m_textInfoVec.push_back({Copy, 0, qApp->translate("QLineEdit", "&Copy").split("(").at(0)});
    m_textInfoVec.push_back({Paste, 0, qApp->translate("QLineEdit", "&Paste").split("(").at(0)});

    updateColor();
    connect(qApp, &QGuiApplication::fontChanged, this, &DSelectedTextTooltip::onFontChanged);

    // 更新文本信息
    onFontChanged();
}

DSelectedTextTooltip::~DSelectedTextTooltip()
{
}

void DSelectedTextTooltip::onFontChanged()
{
    QFontMetrics font_metrics(qApp->font());
    int tooltip_width = 0;
    for (auto &font_info : m_textInfoVec) {
        int tmp_width = font_metrics.horizontalAdvance(font_info.optName);
        font_info.textWidth = tmp_width + 2 * TEXT_SPACINGWIDGET;
        tooltip_width += font_info.textWidth;
    }

    m_textInfoVec.first().textWidth += WINDOWBOARD;
    m_textInfoVec.last().textWidth += WINDOWBOARD;
    resize(tooltip_width + 2*WINDOWBOARD, font_metrics.height() + 2 * TEXT_SPACINGHEIGHT + 2*WINDOWBOARD);
}

void DSelectedTextTooltip::updateColor()
{
    // 参考DtkGui
    QColor rgb_color = qApp->palette().window().color().toRgb();

    float luminance = 0.299 * rgb_color.redF() + 0.587 * rgb_color.greenF() + 0.114 * rgb_color.blueF();

    if (qRound(luminance * 255) > 191) {
        m_backgroundColor = QColor("#fafafa");
        m_dividerColor = QColor("#d6d6d6");
        return;
    }

    m_backgroundColor = QColor("#434343");
    m_dividerColor = QColor("#4f4f4f");
    return;
}

DSelectedTextTooltip::OptionType DSelectedTextTooltip::getOptionType(const QPoint &pos) const
{
    int tmp_width = 0;
    for (const auto &info : m_textInfoVec) {
        tmp_width += info.textWidth;
        if (pos.x() < tmp_width) {
            return info.optType;
        }
    }

    return None;
}

void DSelectedTextTooltip::mousePressEvent(QMouseEvent *event)
{
    Q_EMIT optAction(getOptionType(event->pos()));
}

void DSelectedTextTooltip::paintEvent(QPaintEvent *pe)
{
    updateColor();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setBrush(m_backgroundColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(pe->rect().adjusted(1, 1, -1, -1), 8, 8);

    painter.setBrush(Qt::NoBrush);
    painter.setPen(m_borderColor);
    painter.drawRoundedRect(pe->rect(), 8, 8);

    painter.setFont(qApp->font());
    painter.setPen(QPen(qApp->palette().brightText().color(), 1));

    int pos_x = 0;
    for (int index = 0; index < m_textInfoVec.size(); ++index) {
        if (index == 0 || index == m_textInfoVec.count() -1) {
            pos_x += WINDOWBOARD;
        }
        QRect text_rect(pos_x, WINDOWBOARD, m_textInfoVec[index].textWidth, pe->rect().height() - WINDOWBOARD);
        pos_x += m_textInfoVec[index].textWidth;
        painter.drawText(text_rect, Qt::AlignCenter, m_textInfoVec[index].optName);
        if (index == m_textInfoVec.count() -1)
            break;
        painter.save();
        painter.setPen(m_dividerColor);
        painter.drawLine(text_rect.topRight(), text_rect.bottomRight());
        painter.restore();
    }
}

DPP_END_NAMESPACE
