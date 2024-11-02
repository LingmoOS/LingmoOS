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

#ifndef LINGMO_QUICK_ITEMS_COLOR_MIXER_H
#define LINGMO_QUICK_ITEMS_COLOR_MIXER_H

#include "lingmo/lingmo-theme-proxy.h"
#include <QQuickItem>
#include <QColor>

namespace LingmoUIQuick {

/**
 * @class ColorMix
 *
 * 通过设置前景色和背景色，根据特定的模式生成新的颜色。
 * 颜色与主题色进行绑定，自动跟随主题切换。
 *
 * 前景色，背景色，混合模式
 */
class ColorMix : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
    Q_PROPERTY(LingmoUIQuick::ColorMix::MixMode mixMode READ mixMode WRITE setMixMode NOTIFY mixModeChanged)
    Q_PROPERTY(qreal backColorAlpha READ backColorAlpha WRITE setBackColorAlpha)
    Q_PROPERTY(qreal foreColorAlpha READ foreColorAlpha WRITE setForeColorAlpha)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorRole backColorRole READ backColorRole WRITE setBackColorRole)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorRole foreColorRole READ foreColorRole WRITE setForeColorRole)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorGroup backColorGroup READ backColorGroup WRITE setBackColorGroup)
    Q_PROPERTY(LingmoUIQuick::Theme::ColorGroup foreColorGroup READ foreColorGroup WRITE setForeColorGroup)
public:
    enum MixMode {
        NoMixer = 0,/**> 禁用混合 */
        Normal,     /**> 普通混合 */
        Lighten,    /**> 变亮 */
        Darken,     /**> 变暗 */
        Deepen,     /**> 加深 */
        Dodge,      /**> 减淡 */
        Overlay,    /**> 叠加 */
        Exclusion   /**> 排除 */
    };
    Q_ENUM(MixMode)

    explicit ColorMix(QObject *parent = nullptr);

    QColor color();
    QColor backColor();
    QColor foreColor();

    qreal backColorAlpha() const;
    void setBackColorAlpha(qreal alpha);

    qreal foreColorAlpha() const;
    void setForeColorAlpha(qreal alpha);

    ColorMix::MixMode mixMode() const;
    void setMixMode(ColorMix::MixMode mixMode);

    Theme::ColorRole backColorRole() const;
    void setBackColorRole(Theme::ColorRole role);

    Theme::ColorRole foreColorRole() const;
    void setForeColorRole(Theme::ColorRole role);

    Theme::ColorGroup backColorGroup() const;
    void setBackColorGroup(Theme::ColorGroup group);

    Theme::ColorGroup foreColorGroup() const;
    void setForeColorGroup(Theme::ColorGroup group);

Q_SIGNALS:
    void colorChanged();
    void mixModeChanged();

private Q_SLOTS:
    void updateColor();

private:
    MixMode m_mixMode {NoMixer};
    QColor m_color {Qt::white};

    qreal m_backColorAlpha {0}; // 不在[0, 1]范围内，跟随主题透明度
    Theme::ColorRole m_backColorRole {Theme::Base};
    Theme::ColorGroup m_backColorGroup {Theme::Active};

    qreal m_foreColorAlpha {0};
    Theme::ColorRole m_foreColorRole {Theme::Base};
    Theme::ColorGroup m_foreColorGroup {Theme::Active};
};

//class ColorMixerAttached : public QObject
//{
//    Q_OBJECT
//  public:
//    static ColorMix *qmlAttachedProperties(QObject *object);
//};

} // LingmoUIQuick

Q_DECLARE_METATYPE(LingmoUIQuick::ColorMix::MixMode)
//QML_DECLARE_TYPEINFO(LingmoUIQuick::ColorMixerAttached, QML_HAS_ATTACHED_PROPERTIES)

#endif //LINGMO_QUICK_ITEMS_COLOR_MIXER_H
