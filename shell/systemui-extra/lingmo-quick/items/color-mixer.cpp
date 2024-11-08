/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "color-mixer.h"

namespace LingmoUIQuick {

ColorMix::ColorMix(QObject *parent) : QObject(parent)
{
    connect(Theme::instance(), &Theme::paletteChanged, this, &ColorMix::updateColor);
    connect(Theme::instance(), &Theme::themeTransparencyChanged, this, [this] {
        // 不在[0, 1]范围内，跟随主题透明度
        if (m_backColorAlpha < 0 || m_backColorAlpha > 1 || m_foreColorAlpha < 0 || m_foreColorAlpha > 1) {
            updateColor();
        }
    });
}

QColor ColorMix::color()
{
    return m_color;
}

ColorMix::MixMode ColorMix::mixMode() const
{
    return m_mixMode;
}

void ColorMix::setMixMode(ColorMix::MixMode mixMode)
{
    if (m_mixMode == mixMode) {
        return;
    }

    m_mixMode = mixMode;
    updateColor();

    Q_EMIT mixModeChanged();
}

void ColorMix::updateColor()
{
    if (m_mixMode == NoMixer) {
        m_color = ColorMix::backColor();
    } else {
        QColor fore = ColorMix::foreColor();
        QColor back = ColorMix::backColor();

        switch (m_mixMode) {
            default:
            case Normal: {
              qreal tiny = 1 - fore.alphaF();
              qreal alpha = fore.alphaF() + back.alphaF() * tiny;

              qreal r = (fore.redF() * fore.alphaF() +
                         back.redF() * back.alphaF() * tiny) /
                        alpha;
              qreal g = (fore.greenF() * fore.alphaF() +
                         back.greenF() * back.alphaF() * tiny) /
                        alpha;
              qreal b = (fore.blueF() * fore.alphaF() +
                         back.blueF() * back.alphaF() * tiny) /
                        alpha;

              m_color = QColor::fromRgbF(r, g, b, alpha);
              break;
            }
            case Lighten:
              break;
            case Darken:
              break;
            case Deepen:
              break;
            case Dodge:
              break;
            case Overlay:
              break;
            case Exclusion:
              break;
        }
    }

    Q_EMIT colorChanged();
}

Theme::ColorRole ColorMix::backColorRole() const
{
    return m_backColorRole;
}

void ColorMix::setBackColorRole(Theme::ColorRole role)
{
    if (m_backColorRole == role) {
        return;
    }

    m_backColorRole = role;
    updateColor();
}

Theme::ColorRole ColorMix::foreColorRole() const
{
    return m_foreColorRole;
}

void ColorMix::setForeColorRole(Theme::ColorRole role)
{
    if (m_foreColorRole == role) {
        return;
    }

    m_foreColorRole = role;
    updateColor();
}

Theme::ColorGroup ColorMix::backColorGroup() const
{
    return m_backColorGroup;
}

void ColorMix::setBackColorGroup(Theme::ColorGroup group)
{
    if (m_backColorGroup == group) {
        return;
    }

    m_backColorGroup = group;
    updateColor();
}

Theme::ColorGroup ColorMix::foreColorGroup() const
{
    return m_foreColorGroup;
}

void ColorMix::setForeColorGroup(Theme::ColorGroup group)
{
    if (m_foreColorGroup == group) {
        return;
    }

    m_foreColorGroup = group;
    updateColor();
}

qreal ColorMix::backColorAlpha() const
{
    return m_backColorAlpha;
}

void ColorMix::setBackColorAlpha(qreal alpha)
{
    if (m_backColorAlpha == alpha) {
        return;
    }

    m_backColorAlpha = alpha;
    updateColor();
}

qreal ColorMix::foreColorAlpha() const
{
    return m_foreColorAlpha;
}

void ColorMix::setForeColorAlpha(qreal alpha)
{
    if (m_foreColorAlpha == alpha) {
        return;
    }

    m_foreColorAlpha = alpha;
    updateColor();
}

QColor ColorMix::backColor()
{
    return Theme::instance()->color(m_backColorRole, m_backColorGroup, m_backColorAlpha);
}

QColor ColorMix::foreColor()
{
    return Theme::instance()->color(m_foreColorRole, m_foreColorGroup, m_foreColorAlpha);
}

} // LingmoUIQuick
