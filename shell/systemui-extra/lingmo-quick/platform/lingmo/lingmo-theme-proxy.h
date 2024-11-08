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

#ifndef LINGMO_QUICK_ITEMS_LINGMO_THEME_PROXY_H
#define LINGMO_QUICK_ITEMS_LINGMO_THEME_PROXY_H

#include <QObject>
#include <QFont>
#include <QPalette>
#include <qqml.h>

namespace LingmoUIQuick {

class ThemePrivate;

class Theme : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QFont font READ font NOTIFY fontChanged)
    Q_PROPERTY(QPalette palette READ palette NOTIFY paletteChanged)
    Q_PROPERTY(qreal fontSize READ fontSize NOTIFY fontChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY fontChanged)
    Q_PROPERTY(QString themeName READ themeName NOTIFY themeNameChanged)
    Q_PROPERTY(bool isDarkTheme READ isDarkTheme NOTIFY themeNameChanged)
    Q_PROPERTY(QString themeColor READ themeColor NOTIFY themeColorChanged)
    Q_PROPERTY(qreal themeTransparency READ themeTransparency NOTIFY themeTransparencyChanged)
    Q_PROPERTY(int maxRadius READ maxRadius NOTIFY themeRadiusChanged)
    Q_PROPERTY(int normalRadius READ normalRadius NOTIFY themeRadiusChanged)
    Q_PROPERTY(int minRadius READ minRadius NOTIFY themeRadiusChanged)
    Q_PROPERTY(int windowRadius READ windowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection NOTIFY layoutDirectionChanged)
public:
    enum ColorGroup { Active, Disabled, Inactive };
    Q_ENUM(ColorGroup)

    enum ColorRole { Window, WindowText, Base, Text, AlternateBase,
        Button, ButtonText, Light, MidLight, Dark, Mid, Shadow,
        Highlight, HighlightedText, BrightText, Link, LinkVisited,
        ToolTipBase, ToolTipText, PlaceholderText
    };
    Q_ENUM(ColorRole)

    ~Theme() override;

    static Theme *instance();
    // Attached Prop
    static Theme *qmlAttachedProperties(QObject *object);

    // func
    QFont font() const;
    QPalette palette() const;
    Q_INVOKABLE QColor color(Theme::ColorRole role, Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor color(Theme::ColorRole role, Theme::ColorGroup group, qreal alpha) const;

    // 获取带有主题透明度的颜色
    Q_INVOKABLE QColor colorWithThemeTransparency(Theme::ColorRole role, Theme::ColorGroup group = Theme::Active) const;

    // 获取自定义透明度的颜色
    Q_INVOKABLE QColor colorWithCustomTransparency(Theme::ColorRole role, Theme::ColorGroup group, qreal alphaF) const;

    // color
    Q_INVOKABLE QColor window(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor windowText(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor base(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor text(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor alternateBase(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor button(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor buttonText(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor light(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor midLight(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor dark(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor mid(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor shadow(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor highlight(Theme::ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor highlightedText(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor separator(Theme::ColorGroup group = Theme::Active) const;

    Q_INVOKABLE QColor brightText(ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor link(ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor linkVisited(ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor toolTipBase(ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor toolTipText(ColorGroup group = Theme::Active) const;
    Q_INVOKABLE QColor placeholderText(ColorGroup group = Theme::Active) const;

    //  font
    qreal fontSize() const;
    QString fontFamily() const;

    // theme
    QString themeName() const;
    QString themeColor() const;

    // only lingmo
    bool isDarkTheme() const;
    qreal themeTransparency() const;

    // theme radius
    int maxRadius() const;
    int normalRadius() const;
    int minRadius() const;

    //window radius
    int windowRadius() const;

    //system layoutDirection
    Qt::LayoutDirection layoutDirection() const;

Q_SIGNALS:
    void fontChanged();
    void paletteChanged();
    void themeNameChanged();
    void themeColorChanged();
    void themeTransparencyChanged();
    void themeRadiusChanged();
    void iconThemeChanged();
    void windowRadiusChanged();
    void layoutDirectionChanged();

private:
    explicit Theme(QObject *parent = nullptr);
    inline QPalette::ColorGroup switchColorGroup(ColorGroup group) const;

private:
    ThemePrivate *d {nullptr};
};

} // LingmoUIQuick
Q_DECLARE_METATYPE(LingmoUIQuick::Theme::ColorRole)
Q_DECLARE_METATYPE(LingmoUIQuick::Theme::ColorGroup)
QML_DECLARE_TYPEINFO(LingmoUIQuick::Theme, QML_HAS_ATTACHED_PROPERTIES)


#endif //LINGMO_QUICK_ITEMS_LINGMO_THEME_PROXY_H
