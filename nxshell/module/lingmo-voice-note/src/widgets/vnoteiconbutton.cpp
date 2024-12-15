// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/vnoteiconbutton.h"
#include "globaldef.h"

#include <DLog>

#include <QImageReader>

/**
 * @brief VNoteIconButton::VNoteIconButton
 * @param parent
 * @param normal
 * @param hover
 * @param press
 */
VNoteIconButton::VNoteIconButton(QWidget *parent, QString normal, QString hover, QString press)
    : DIconButton(parent)
{
    m_icons[Normal] = normal;
    m_icons[Hover] = hover;
    m_icons[Press] = press;

    updateIcon();

    //TODO:x
    //    Need update when theme change
    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::paletteTypeChanged,
            this, &VNoteIconButton::onThemeChanged);
}

/**
 * @brief VNoteIconButton::~VNoteIconButton
 */
VNoteIconButton::~VNoteIconButton()
{
}

/**
 * @brief VNoteIconButton::setSeparateThemIcons
 * @param separate
 */
void VNoteIconButton::setSeparateThemIcons(bool separate)
{
    m_separateThemeIcon = separate;
}

/**
 * @brief VNoteIconButton::SetDisableIcon
 * @param disableIcon
 */
void VNoteIconButton::SetDisableIcon(const QString &disableIcon)
{
    if (!disableIcon.isEmpty()) {
        m_icons[Disabled] = disableIcon;
    }
}

/**
 * @brief VNoteIconButton::setBtnDisabled
 * @param disabled
 */
void VNoteIconButton::setBtnDisabled(bool disabled)
{
    //The disable icon should be supplied
    if (!m_icons[Disabled].isEmpty()) {
        m_isDisabled = disabled;

        if (m_isDisabled) {
            m_state = Disabled;
        } else {
            m_state = Normal;
        }

        updateIcon();

        setDisabled(m_isDisabled);
    }
}

/**
 * @brief VNoteIconButton::enterEvent
 * @param event
 */
void VNoteIconButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (!m_isDisabled) {
        m_state = Hover;
        this->setFlat(false);
        updateIcon();
    }
}

/**
 * @brief VNoteIconButton::leaveEvent
 * @param event
 */
void VNoteIconButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (!m_isDisabled) {
        m_state = Normal;
        this->setFlat(true);
        updateIcon();
    }
}

/**
 * @brief VNoteIconButton::mousePressEvent
 * @param event
 */
void VNoteIconButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (!m_isDisabled) {
        m_state = Press;

        updateIcon();

        event->accept();
    }
}

/**
 * @brief VNoteIconButton::mouseReleaseEvent
 * @param event
 */
void VNoteIconButton::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();

    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (!m_isDisabled) {
        if (rect().contains(event->pos())) {
            m_state = Hover;

            updateIcon();

            Q_EMIT clicked();
        }
    }
}

/**
 * @brief VNoteIconButton::mouseMoveEvent
 * @param event
 */
void VNoteIconButton::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if (!m_isDisabled) {
        if (m_state != Hover) {
            m_state = Hover;

            updateIcon();
        }
    }
}

/**
 * @brief VNoteIconButton::onThemeChanged
 * @param type
 */
void VNoteIconButton::onThemeChanged(DGuiApplicationHelper::ColorType type)
{
    Q_UNUSED(type);

    updateIcon();
}

/**
 * @brief VNoteIconButton::loadPixmap
 * @param path
 * @return 加载图标
 */
QPixmap VNoteIconButton::loadPixmap(const QString &path)
{
    DGuiApplicationHelper::ColorType theme =
        DGuiApplicationHelper::instance()->themeType();

    QString iconPath(STAND_ICON_PAHT);

    //Default use different icons under
    //different themes
    if (m_separateThemeIcon) {
        if (DGuiApplicationHelper::ColorType::LightType == theme) {
            iconPath += QString("light/");
        } else {
            iconPath += QString("dark/");
        }
    }

    iconPath += path;

    qreal ratio = 1.0;

    const qreal devicePixelRatio = devicePixelRatioF();

    QPixmap pixmap;

    if (!qFuzzyCompare(ratio, devicePixelRatio)) {
        QImageReader reader;
        reader.setFileName(qt_findAtNxFile(iconPath, devicePixelRatio, &ratio));
        if (reader.canRead()) {
            reader.setScaledSize(reader.size() * (devicePixelRatio / ratio));
            pixmap = QPixmap::fromImage(reader.read());
            pixmap.setDevicePixelRatio(devicePixelRatio);
        }
    } else {
        pixmap.load(iconPath);
    }

    return pixmap;
}

/**
 * @brief VNoteIconButton::updateIcon
 */
void VNoteIconButton::updateIcon()
{
    if (!m_icons[m_state].isEmpty()) {
        setIcon(loadPixmap(m_icons[m_state]));
    }
}

/**
 * @brief VNoteIconButton::focusInEvent
 * @param e
 */
void VNoteIconButton::focusInEvent(QFocusEvent *e)
{
    m_focusReason = e->reason();
    DIconButton::focusInEvent(e);
}

/**
 * @brief VNoteIconButton::getFocusReason
 * @return
 */
Qt::FocusReason VNoteIconButton::getFocusReason()
{
    if (!this->hasFocus()) {
        m_focusReason = Qt::NoFocusReason;
    }
    return m_focusReason;
}
