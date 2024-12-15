// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolbutton.h"
#include "frame/cviewmanagement.h"
#include <QPainter>
#include <QStylePainter>
#include <QStyleOptionButton>
#include <qtoolbutton.h>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

ToolButton::ToolButton(QWidget *parent, ToolButonStyle style) : QPushButton(parent)
    , m_bShowText(true)
    , m_style(style)
    , m_alignment(Qt::AlignLeft)
{
    setMinimumWidth(50);
}

void ToolButton::setBtnStyle(ToolButonStyle style)
{
    m_style = style;
    update();
}

void ToolButton::setShowText(bool bShowText)
{
    m_bShowText = bShowText;
}

void ToolButton::setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;
}

void ToolButton::paintEvent(QPaintEvent *e)
{
    if (m_style == BUTTON_STYLE) {
        if (m_bShowText) {
            return QPushButton::paintEvent(e);
        }

        QStylePainter p(this);
        QStyleOptionButton option;

        initStyleOption(&option);
        option.text = "";
        p.drawControl(QStyle::CE_PushButton, option);

    } else {
        QStylePainter p(this);
        QStyleOptionButton option;
        initStyleOption(&option);

        QPainter painter(this);
        //绘制背景
        painter.save();
        //非禁用状态绘制背景
        if (option.state & QStyle::State_Enabled) {
            //! \modified 菜单栏统一高亮效果 [bug] https://pms.uniontech.com/bug-view-130621.html
            // 获取系统主题颜色，高亮和Hover效果不同
            if (option.state & QStyle::State_On) {
                painter.setBrush(option.palette.highlight().color());
            } else if (option.state & QStyle::State_MouseOver) {
                painter.setBrush(option.palette.shadow().color());
            } else {}

            painter.setPen(Qt::NoPen);
            painter.drawRect(this->rect());
        }
        painter.restore();
        painter.setPen(getPen(option));
        painter.save();

        QSize iconSz(16, 16);
        int space = 10;

        if (icon().isNull()) {
            iconSz = QSize(0, 0);
            space = 0;
        }

        QSize textSz(painter.fontMetrics().width(text()), painter.fontMetrics().height());
        int iconX = (width() - (iconSz.width() + space + textSz.width())) / 2;
        int iconY = (height() - iconSz.height()) / 2;

        if (m_alignment == Qt::AlignLeft) {
            iconX = 20;
        }

        QRect iconRect(QPoint(iconX, iconY), iconSz);

        // 绘制ICON
        if (iconSz.isValid()) {
            painter.save();

            // 设置Hover时Icon颜色不变更
            QPen pen = painter.pen();
            pen.setColor(option.palette.foreground().color());
            painter.setPen(pen);

            QTransform matrix;
            matrix.translate(iconRect.center().x(), iconRect.center().y());
            matrix.scale(45. / 16., 45. / 16.);
            matrix.translate(-iconRect.center().x(), -iconRect.center().y());
            icon().paint(&painter, matrix.mapRect(iconRect));
            painter.restore();
        }

        //绘制文字
        painter.save();
        QPen pen = painter.pen();
        // 不同状态下使用不同颜色文字，高亮和Hover效果不同
        if (option.state & QStyle::State_On) {
            pen.setColor(option.palette.highlightedText().color());
        } else if (option.state & QStyle::State_MouseOver) {
            pen.setColor(option.palette.text().color());
        } else {}
        painter.setPen(pen);

        // 计算绘制区域并绘制文字
        QRect textRect(QRect(iconX + iconSz.width() + space, 0, textSz.width() + 1, height()));
        painter.drawText(textRect, text(), QTextOption(Qt::AlignCenter));
        painter.restore();

        painter.restore();
    }
}

QPen ToolButton::getPen(const QStyleOptionButton option)
{
    QPen pen;
    if (DGuiApplicationHelper::instance()->themeType() == 1) {
        //控件禁用样式
        if (!(option.state & QStyle::State_Enabled)) {
            pen = QPen(QColor("#9C9C9C"));
        }

        //鼠标悬停画笔颜色
        else if (option.state & QStyle::State_MouseOver) {
            pen = QPen(QColor(Qt::white));
        }

        //鼠标按下画笔颜色
        else if (option.state & QStyle::State_Sunken) {
            pen = QPen(QColor("#99cdff"));
        } else {
            pen = QPen(QColor("#343434"));
        }

    } else {

        //控件禁用样式
        if (!(option.state & QStyle::State_Enabled)) {
            pen = QPen(QColor("#8D8D8D"));
        }

        //鼠标悬停画笔颜色
        else if (option.state & QStyle::State_MouseOver) {
            pen = QPen(QColor(Qt::white));
        }

        //鼠标按下画笔颜色
        else if (option.state & QStyle::State_Sunken) {
            pen = QPen(QColor("#99cdff"));
        } else {

            pen = QPen(QColor(Qt::white));
        }
    }
    return pen;
}

QSize ToolButton::sizeHint() const
{

    if (m_style == BUTTON_STYLE && m_bShowText) {
        return QSize(QPushButton::sizeHint().width() + 9, QPushButton::sizeHint().height());
    } else if (m_style == BUTTON_STYLE && !m_bShowText) {
        int w = 0, h = 0;

        QStyleOptionButton opt;
        initStyleOption(&opt);

        if (!m_bShowText) {
            opt.text = "";
        }

        if (!icon().isNull()) {
            int ih = opt.iconSize.height();
            int iw = opt.iconSize.width();
            w += iw;
            h = qMax(h, ih);
        }

        opt.rect.setSize(QSize(w, h));
#if QT_CONFIG(menu)
        if (menu())
            w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
#endif
        return QSize(w, h);
    } else {

        int w = 0, h = 0;

        QStyleOptionButton opt;
        initStyleOption(&opt);

        if (!m_bShowText) {
            opt.text = "";
        }

        if (!icon().isNull()) {
            int ih = opt.iconSize.height();
            int iw = opt.iconSize.width() + 4;
            w += iw;
            h = qMax(h, ih);
        }

        QString s(/*text()*/opt.text);
        bool empty = s.isEmpty();
        if (empty)
            s = QStringLiteral("XXXX");
        QFontMetrics fm = fontMetrics();
        QSize sz = fm.size(Qt::TextShowMnemonic, s);
        if (!empty || !w)
            w += sz.width();
        if (!empty || !h)
            h = qMax(h, sz.height());
        opt.rect.setSize(QSize(w, h)); // PM_MenuButtonIndicator depends on the height
#if QT_CONFIG(menu)
        if (menu())
            w += style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &opt, this);
#endif
        return style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), this);
    }
}











