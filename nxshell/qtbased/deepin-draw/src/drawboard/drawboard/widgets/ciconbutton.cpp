// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ciconbutton.h"
#include <QPainter>
#include <QDebug>
#ifdef USE_DTK
#include <DApplicationHelper>
#endif

#include <QPalette>
#include <QMouseEvent>

CIconButton::CIconButton(const QMap<int, QMap<EIconButtonSattus, QString> > &pictureMap, const QSize &size,
                         QWidget *parent, bool isCheckLock) :
    ICONBUTTON(parent),
    m_isHover(false),
    m_isChecked(false),
    m_isPressed(false),
    m_isCheckLock(isCheckLock),
    m_currentStatus(Normal),
    m_currentTheme(1/*DGuiApplicationHelper::LightType*/),
    m_pictureMap(pictureMap)
    , m_centerColor(Qt::transparent)
{
    setFixedSize(size);
    setIconSize(size);

#ifdef USE_DTK
    m_currentTheme = DGuiApplicationHelper::instance()->themeType();
#endif

    updateImage();
}

void CIconButton::setChecked(bool checked)
{
    if (checked) {
        m_currentStatus = Active;

    } else {
        if (hasFocus()) {
            clearFocus();
        }
        m_currentStatus = Normal;
    }

    m_isChecked = checked;

    updateImage();
}


bool CIconButton::isChecked() const
{
    return m_isChecked;
}

void CIconButton::mousePressEvent(QMouseEvent *e)
{
//    Q_UNUSED(e)

    if (m_isChecked && m_isCheckLock) {
        return;
    }

    m_isPressed = true;

    m_currentStatus = Press;

    m_isChecked = !m_isChecked;

    updateImage();

    emit buttonClick();
    ICONBUTTON::mousePressEvent(e);
}

void CIconButton::mouseReleaseEvent(QMouseEvent *e)
{
    m_isPressed = false;

    if (m_isChecked) {
        m_currentStatus = Active;
    } else {
        m_currentStatus = Normal;
    }

    updateImage();

    if (e->button() == Qt::LeftButton) {
        emit mouseRelease();
    }

    ICONBUTTON::mouseReleaseEvent(e);
}

void CIconButton::enterEvent(QEvent *e)
{
    m_isHover = true;
    m_tmpStatus = m_currentStatus;
    m_currentStatus = Hover;
    updateImage();
    ICONBUTTON::enterEvent(e);
}

void CIconButton::leaveEvent(QEvent *e)
{
    m_isHover = false;

    if (m_currentStatus == Hover) {
        m_currentStatus = m_tmpStatus;
        updateImage();
    }
    ICONBUTTON::leaveEvent(e);
}

void CIconButton::paintEvent(QPaintEvent *event)
{
    ICONBUTTON::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::SmoothPixmapTransform | QPainter::Antialiasing);
    QPen pen(m_centerColor);
    pen.setCapStyle(Qt::PenCapStyle::RoundCap);
    pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
    pen.setWidth(rect().width() / 4);
    painter.setPen(pen);
    painter.drawPoint(rect().center() + QPoint(1, 1));
    painter.end();
}

void CIconButton::updateImage()
{
    if (!m_iconMode) {
        setIcon(QIcon(m_pictureMap[m_currentTheme][m_currentStatus]));
    }
}

void CIconButton::setTheme(int currentTheme)
{
    m_currentTheme = currentTheme;

    updateImage();

}

void CIconButton::setIconMode()
{
    m_iconMode = true;
}

void CIconButton::setCenterColor(QColor c)
{
    m_centerColor = c;
}

