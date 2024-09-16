// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iconbutton.h"

#include <QPainter>
#include <QIcon>
#include <QStyle>
#include <QDebug>
#include <QMouseEvent>

#include <DGuiApplicationHelper>
#include <QImageReader>

DGUI_USE_NAMESPACE

IconButton::IconButton(QWidget *parent)
    : DWidget(parent)
    , m_text("")
    , m_hasFocus(false)
    , m_hover(false)
    , m_hoverOpacity(0)
    , m_pressOpacity(0)
    , m_releaseOpacity(0)
    , m_radius(8)
    , m_currentState(RELEASE)
{
}

IconButton::IconButton(const QString &text, QWidget *parent)
    : IconButton(parent)
{
    setText(text);
}

void IconButton::setText(const QString &text)
{
    m_text = text;

    update();
}

void IconButton::setIcon(const QString &iconPath)
{
    m_iconPath = iconPath;

    update();
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QColor color;
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        color = QColor("#000000");
    } else {
        color = QColor("#FFFFFF");
    }

    switch (m_currentState) {
    case HOVER: color.setAlpha(m_hoverOpacity); break;
    case PRESS: color.setAlpha(m_pressOpacity); break;
    case RELEASE: color.setAlpha(m_releaseOpacity); break;
    }

    color.setAlpha(hasFocus() ? m_hoverOpacity : color.alpha());

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), m_radius, m_radius);

    QImageReader reader;

    reader.setFileName(m_iconPath);
    QPixmap pixmap;
    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size() * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    }
    painter.drawPixmap(QPoint(0, 0),pixmap);
}

void IconButton::setFocusState(bool has)
{
    m_hasFocus = has;

    update();
}

void IconButton::setOpacity(const IconButton::ButtonState state, int opacity)
{
    if (opacity < 0 || opacity > 255)
        return;

    switch (state) {
    case HOVER: m_hoverOpacity = opacity; break;
    case PRESS: m_pressOpacity = opacity; break;
    case RELEASE: m_releaseOpacity = opacity; break;
    }
    update();
}

void IconButton::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_pressPos = event->pos();
    m_currentState = PRESS;
    update();
}

void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (rect().contains(m_pressPos) && rect().contains(event->pos())) {
        m_currentState = HOVER;
        Q_EMIT clicked();
    } else {
        m_currentState = RELEASE;
    }
    update();
}

void IconButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_currentState = HOVER;
    update();

    return DWidget::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_currentState = RELEASE;
    update();

    return DWidget::leaveEvent(event);
}

void IconButton::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        clicked();
    }
}
