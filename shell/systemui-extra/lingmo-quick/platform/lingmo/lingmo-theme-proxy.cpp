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

#include "lingmo-theme-proxy.h"

#include <QGSettings>
#include <QGuiApplication>
#include <QApplication>
#include <QStyle>

#define CONTROL_CENTER_SETTING          "org.lingmo.control-center.personalise"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"

#define LINGMO_STYLE_SETTING          "org.lingmo.style"
#define LINGMO_STYLE_NAME_KEY         "styleName"
#define LINGMO_STYLE_THEME_COLOR_KEY  "themeColor"
#define LINGMO_STYLE_SYSTEM_FONT_KEY  "systemFont"
#define LINGMO_STYLE_SYSTEM_FONT_SIZE "systemFontSize"
#define LINGMO_STYLE_ICON_THEME_NAME_KEY "iconThemeName"
#define LINGMO_STYLE_WINDOW_RADIUS_KEY "windowRadius"

namespace LingmoUIQuick {

class ThemePrivate
{
public:
    explicit ThemePrivate(Theme *parent);

    void initTransparency();
    void initStyleSetting();
    void initThemeRadius();
    void initLayoutDirection();

    Theme *q {nullptr};

    QFont font;
    QPalette palette;
    Qt::LayoutDirection layoutDirection;

    qreal transparency {1.0};

    QString themeName {"lingmo-light"};
    QString themeColor {"daybreakBlue"};

    int maxRadius = 0;
    int normalRadius = 0;
    int minRadius = 0;
    int windowRadius = 0;
};

ThemePrivate::ThemePrivate(Theme *parent) : q(parent), font(QGuiApplication::font()),
    palette(QGuiApplication::palette()), layoutDirection(QGuiApplication::layoutDirection())
{
    initTransparency();
    initStyleSetting();
    initThemeRadius();
}

void ThemePrivate::initTransparency()
{
    const QByteArray id(CONTROL_CENTER_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), q);

        QStringList keys = settings->keys();
        if (keys.contains(CONTROL_CENTER_TRANSPARENCY_KEY)) {
            transparency = settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
        }

        QObject::connect(settings, &QGSettings::changed, q, [this, settings] (const QString &key) {
            if (key == CONTROL_CENTER_TRANSPARENCY_KEY) {
                transparency = settings->get(key).toReal();
                Q_EMIT q->themeTransparencyChanged();
            }
        });
    }
}

void ThemePrivate::initStyleSetting()
{
    const QByteArray id(LINGMO_STYLE_SETTING);
    if (QGSettings::isSchemaInstalled(id)) {
        auto settings = new QGSettings(id, QByteArray(), q);

        QStringList keys = settings->keys();
        if (keys.contains(LINGMO_STYLE_NAME_KEY)) {
            themeName = settings->get(LINGMO_STYLE_NAME_KEY).toString();
        }

        if (keys.contains(LINGMO_STYLE_THEME_COLOR_KEY)) {
            themeColor = settings->get(LINGMO_STYLE_THEME_COLOR_KEY).toString();
        }
        if (keys.contains(LINGMO_STYLE_WINDOW_RADIUS_KEY)) {
            windowRadius = settings->get(LINGMO_STYLE_WINDOW_RADIUS_KEY).toInt();
        }

        QObject::connect(settings, &QGSettings::changed, q, [this, settings] (const QString &key) {
            if (key == LINGMO_STYLE_NAME_KEY) {
                themeName = settings->get(key).toString();
                initThemeRadius();
                Q_EMIT q->themeRadiusChanged();
                Q_EMIT q->themeNameChanged();
                Q_EMIT q->paletteChanged();
                return;
            }

            if (key == LINGMO_STYLE_THEME_COLOR_KEY) {
                themeColor = settings->get(key).toString();
                Q_EMIT q->themeColorChanged();
                Q_EMIT q->paletteChanged();
                return;
            }
            if (key == LINGMO_STYLE_ICON_THEME_NAME_KEY) {
                Q_EMIT q->iconThemeChanged();
            }
            if (key == LINGMO_STYLE_WINDOW_RADIUS_KEY) {
                windowRadius = settings->get(LINGMO_STYLE_WINDOW_RADIUS_KEY).toInt();
                Q_EMIT q->windowRadiusChanged();
            }
        });
    }
}

void ThemePrivate::initThemeRadius()
{
    // QApplication程序
    if (qobject_cast<QApplication*>(qApp)) {
        maxRadius = qApp->style()->property("maxRadius").toInt();
        normalRadius = qApp->style()->property("normalRadius").toInt();
        minRadius = qApp->style()->property("minRadius").toInt();
    } else {
        maxRadius = 8;
        normalRadius = 6;
        minRadius = 4;
    }
}

// ====== Theme ====== //
Theme *Theme::qmlAttachedProperties(QObject *object)
{
    Q_UNUSED(object)
    return Theme::instance();
}

Theme *Theme::instance()
{
    static Theme theme;
    return &theme;
}

Theme::Theme(QObject *parent) : QObject(parent), d(new ThemePrivate(this))
{
    qRegisterMetaType<QFont>("QFont");
    qRegisterMetaType<QPalette>("QPalette");

    qRegisterMetaType<LingmoUIQuick::Theme::ColorRole>("Theme::ColorRole");
    qRegisterMetaType<LingmoUIQuick::Theme::ColorGroup>("Theme::ColorGroup");

    connect(qGuiApp, &QGuiApplication::fontChanged, this, [this] (const QFont &font) {
        d->font = font;
        Q_EMIT fontChanged();
    });
    connect(qGuiApp, &QGuiApplication::paletteChanged, this, [this] (const QPalette &palette) {
        d->palette = palette;
        Q_EMIT paletteChanged();
    });
    connect(qGuiApp, &QGuiApplication::layoutDirectionChanged, this, [this] (const Qt::LayoutDirection &layoutDirection) {
        d->layoutDirection = layoutDirection;
        Q_EMIT layoutDirectionChanged();
    });
}

Theme::~Theme()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QFont Theme::font() const
{
    return d->font;
}

QPalette Theme::palette() const
{
    return d->palette;
}

QColor Theme::color(Theme::ColorRole role, Theme::ColorGroup group) const
{
    switch (role) {
        default:
        case Window:
            return window(group);
        case WindowText:
            return windowText(group);
        case Base:
            return base(group);
        case Text:
            return text(group);
        case AlternateBase:
            return alternateBase(group);
        case Button:
            return button(group);
        case ButtonText:
            return buttonText(group);
        case Light:
            return light(group);
        case MidLight:
            return midLight(group);
        case Dark:
            return dark(group);
        case Mid:
            return mid(group);
        case Shadow:
            return shadow(group);
        case Highlight:
            return highlight(group);
        case HighlightedText:
            return highlightedText(group);
        case BrightText:
            return brightText(group);
        case Link:
            return link(group);
        case LinkVisited:
            return linkVisited(group);
        case ToolTipBase:
            return toolTipBase(group);
        case ToolTipText:
            return toolTipText(group);
        case PlaceholderText:
            return placeholderText(group);
    }
}

QColor Theme::color(Theme::ColorRole role, Theme::ColorGroup group, qreal alpha) const
{
    if (alpha < 0 || alpha > 1) {
        return colorWithThemeTransparency(role, group);
    }

    return colorWithCustomTransparency(role, group, alpha);
}

QColor Theme::colorWithThemeTransparency(Theme::ColorRole role, Theme::ColorGroup group) const
{
    QColor c = Theme::color(role, group);
    c.setAlphaF(d->transparency);
    return c;
}

QColor Theme::colorWithCustomTransparency(Theme::ColorRole role, Theme::ColorGroup group, qreal alphaF) const
{
    QColor c = Theme::color(role, group);
    c.setAlphaF(alphaF < 0 ? 0 : alphaF > 1 ? 1 : alphaF);
    return c;
}
QColor Theme::window(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Window);
}

QColor Theme::windowText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::WindowText);
}

QColor Theme::text(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Text);
}

QColor Theme::base(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Base);
}

QColor Theme::alternateBase(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::AlternateBase);
}

QColor Theme::button(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Button);
}

QColor Theme::buttonText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::ButtonText);
}

QColor Theme::light(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Light);
}

QColor Theme::midLight(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Midlight);
}

QColor Theme::dark(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Dark);
}

QColor Theme::mid(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Mid);
}

QColor Theme::shadow(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Shadow);
}

QColor Theme::highlight(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Highlight);
}

QColor Theme::highlightedText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::HighlightedText);
}

QColor Theme::separator(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Window);
}

QColor Theme::brightText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::BrightText);
}

QColor Theme::link(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::Link);
}

QColor Theme::linkVisited(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::LinkVisited);
}

QColor Theme::toolTipBase(ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::ToolTipBase);
}

QColor Theme::toolTipText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::ToolTipText);
}

QColor Theme::placeholderText(Theme::ColorGroup group) const
{
    return d->palette.color(switchColorGroup(group), QPalette::PlaceholderText);
}

inline QPalette::ColorGroup Theme::switchColorGroup(Theme::ColorGroup group) const
{
    switch (group) {
        default:
        case Active:
            return QPalette::Active;
        case Disabled:
            return QPalette::Disabled;
        case Inactive:
            return QPalette::Inactive;
    }
}

qreal Theme::fontSize() const
{
    return d->font.pointSize();
}

QString Theme::fontFamily() const
{
    return d->font.family();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::themeColor() const
{
    return d->themeColor;
}

bool Theme::isDarkTheme() const
{
    return d->themeName == "lingmo-dark";
}

qreal Theme::themeTransparency() const
{
    return d->transparency;
}

int Theme::maxRadius() const
{
    return d->maxRadius;
}

int Theme::normalRadius() const
{
    return d->normalRadius;
}

int Theme::minRadius() const
{
    return d->minRadius;
}

int Theme::windowRadius() const
{
    return d->windowRadius;
}

Qt::LayoutDirection Theme::layoutDirection() const
{
    return d->layoutDirection;
}

} // LingmoUIQuick
