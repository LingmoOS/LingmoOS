/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <QVariant>
#include <QPalette>
#include <QColor>

#include "theme_management.hpp"
#include "log.hpp"

namespace kdk
{
namespace kabase
{
namespace
{
constexpr char minIconName[] = "window-minimize-symbolic";
constexpr char maxIconName[] = "window-maximize-symbolic";
constexpr char closeIconName[] = "window-close-symbolic";
} // namespace

ThemeManagement::ThemeManagement() = default;

ThemeManagement::~ThemeManagement() = default;

bool ThemeManagement::setProperty(QObject *object, Property property)
{
    switch (property) {
    case Property::MinButton || Property::MaxButton:
        return object->setProperty("isWindowButton", QVariant(0x1))
               && object->setProperty("useIconHighlightEffect", QVariant(0x2));
    case Property::CloseButton:
        return object->setProperty("isWindowButton", QVariant(0x2))
               && object->setProperty("useIconHighlightEffect", QVariant(0x8));
    case Property::ButtonThreeStateGrey:
        return object->setProperty("useButtonPalette", QVariant(true));
    case Property::ButtonThreeStateBlue:
        return object->setProperty("isImportant", QVariant(true));
    case Property::IconDefaultHighlight:
        return object->setProperty("useIconHighlightEffect", QVariant(0x2));
    case Property::IconSelectClickHighlight:
        return object->setProperty("useIconHighlightEffect", QVariant(0x4));
    case Property::IconDefaultSelectClickHighlight:
        return object->setProperty("useIconHighlightEffect", QVariant(0x8));
    case Property::IconNonSolidColorHighlight:
        return object->setProperty("useIconHighlightEffect", QVariant(0x10));
    default:
        return false;
    }

    return false;
}

bool ThemeManagement::setPaletteColor(QWidget *widget)
{
    if (widget == nullptr) {
        error << "kabase : pointer is nullptr";
        return false;
    }

    QPalette palette = widget->palette();
    QColor color = palette.color(QPalette::Active, QPalette::Base);
    palette.setColor(QPalette::Window, color);
    widget->setPalette(palette);

    return true;
}

bool ThemeManagement::setMinIcon(QAbstractButton *button)
{
    if (button == nullptr) {
        error << "kabase : pointer is nullptr";
        return false;
    }

    button->setIcon(QIcon::fromTheme(minIconName));

    return true;
}

bool ThemeManagement::setMaxIcon(QAbstractButton *button)
{
    if (button == nullptr) {
        error << "kabase : pointer is nullptr";
        return false;
    }

    button->setIcon(QIcon::fromTheme(maxIconName));

    return true;
}

bool ThemeManagement::setCloseIcon(QAbstractButton *button)
{
    if (button == nullptr) {
        error << "kabase : pointer is nullptr";
        return false;
    }

    button->setIcon(QIcon::fromTheme(closeIconName));

    return true;
}

} /* namespace kabase */
} /* namespace kdk */
