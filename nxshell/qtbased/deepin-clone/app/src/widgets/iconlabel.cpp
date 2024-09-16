// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "iconlabel.h"

#include <QLabel>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>

IconLabel::IconLabel(QWidget *parent, bool wordWrap)
    : QWidget(parent)
{
    m_icon = new QLabel(this);
    m_icon->hide();
    m_label = new QLabel(this);
    m_label->setWordWrap(wordWrap);
    m_label->hide();

    QHBoxLayout *layout = new QHBoxLayout(this);

    layout->addWidget(m_icon, 0, Qt::AlignHCenter);
    layout->addWidget(m_label, 0, Qt::AlignHCenter);
    layout->addStretch();

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMouseTracking(true);
}

QString IconLabel::title() const
{
    return m_label->text();
}

void IconLabel::setTitle(const QString &title)
{
    m_label->setText(title);
    m_label->setVisible(!title.isEmpty());
}

void IconLabel::setIcon(const QIcon &icon, int extent)
{
    if (extent <= 0) {
        QBoxLayout::Direction d = static_cast<QBoxLayout*>(layout())->direction();

        if (d == QBoxLayout::LeftToRight || d == QBoxLayout::RightToLeft)
            extent = m_label->sizeHint().height();
        else
            extent = 50;
    }

    QPixmap pixmap = icon.pixmap(extent);

    pixmap.setDevicePixelRatio(pixmap.width() / qreal(extent));

    m_icon->setPixmap(pixmap);
    m_icon->setVisible(!icon.isNull());
}

void IconLabel::setDirection(QBoxLayout::Direction direction)
{
    static_cast<QBoxLayout*>(layout())->setDirection(direction);
}

void IconLabel::setChecked(bool checked)
{
    if (m_isChecked == checked)
        return;

    m_isChecked = checked;

    update();
}

void IconLabel::setHoverBackground(const QColor &color)
{
    m_background = color;
}

void IconLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter pa(this);

    pa.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;

    path.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);

    if (m_background.isValid() && underMouse()) {
        pa.fillPath(path, m_background);
    }

    if (m_isChecked) {
        QPen pen = pa.pen();

        pen.setColor(QColor("#2ca7f8"));
        pen.setWidth(2);
        pen.setJoinStyle(Qt::RoundJoin);
        pa.setPen(pen);

        pa.drawPath(path);
    }
}

void IconLabel::enterEvent(QEvent *event)
{
    if (m_background.isValid())
        update();

    return QWidget::enterEvent(event);
}

void IconLabel::leaveEvent(QEvent *event)
{
    if (m_background.isValid())
        update();

    return QWidget::leaveEvent(event);
}
