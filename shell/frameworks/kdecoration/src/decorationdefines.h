/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

namespace KDecoration2
{
/**
 * The DecorationButtonType is a helper type for the DecorationButton.
 * A Decoration should provide a DecorationButton for each of the types,
 * if it wants to provide further buttons it should use the Custom type.
 * The DecorationButton gets configured depending on the type. E.g. the
 * Close button gets disabled if the DecoratedClient is not closeable.
 **/
enum class DecorationButtonType {
    /**
     * The Menu button requests showing the window menu on left or right click.
     **/
    Menu,
    /**
     * The ApplicationMenu button requests showing the application's menu on left or right click.
     */
    ApplicationMenu,
    /**
     * The OnAllDesktops button requests toggling the DecoratedClient's on all desktops state.
     * The DecoratedButton is only visible if multiple virtual desktops are available.
     **/
    OnAllDesktops,
    /**
     * The Minimize button requests minimizing the DecoratedClient. The DecorationButton is only
     * enabled if the DecoratedClient is minimizeable.
     **/
    Minimize,
    /**
     * The Maximize button requests maximizing the DecoratedClient. The DecorationButton is checkable
     * and if the DecoratedClient is maximized the DecorationButton is checked. The DecorationButton
     * supports multiple mouse buttons to change horizontal, vertical and overall maximized state.
     *
     * The DecorationButton is only enabled if the DecoratedClient is maximizeable.
     **/
    Maximize,
    /**
     * The Close button requests closing the DecoratedClient. The DecorationButton is only enabled
     * if the DecoratedClient is closeable.
     **/
    Close,
    /**
     * The ContextHelp button requests entering the context help mode. The DecorationButton is only
     * visible if the DecoratedClient provides context help.
     **/
    ContextHelp,
    /**
     * The Shade button requests toggling the DecoratedClient's shaded state. The DecoratedButton
     * is only enabled if the DecoratedClient is shadeable.
     **/
    Shade,
    /**
     * The KeepBelow button requests toggling the DecoratedClient's keep below state.
     **/
    KeepBelow,
    /**
     * The KeepAbove button requests toggling the DecoratedClient's keep above state.
     **/
    KeepAbove,
    /**
     * The Custom type allows a Decoration to provide custom DecorationButtons.
     **/
    Custom,
    /**
     * The Spacer button provides some space between buttons.
     */
    Spacer,
};

/**
 * Border sizes are a combination of visual and accessibility features.
 * Larger borders should be used to increase the non title bar borders to
 * make it easier to resize the decoration
 **/
enum class BorderSize {
    /**
     * Border sizes of all non title bar sides should be set to 0.
     **/
    None,
    /**
     * Border sizes of the sides should be set to 0. Title bar and
     * the border on opposite side of the title bar should follow the
     * Normal settings.
     **/
    NoSides,
    /**
     * Borders should be smaller than Normal, e.g. a factor of 0.5.
     **/
    Tiny,
    /**
     * The default border size with borders on each side. This should
     * be the base for calculating other border sizes.
     **/
    Normal,
    /**
     * Increased border sizes, considered a factor of 1.5.
     **/
    Large,
    /**
     * Increased border sizes, considered a factor of 2.0.
     **/
    VeryLarge,
    /**
     * Increased border sizes, considered a factor of 2.5.
     **/
    Huge,
    /**
     * Increased border sizes, considered a factor of 3.0.
     **/
    VeryHuge,
    /**
     * Increased border sizes, considered a factor of 5.0.
     **/
    Oversized,
};

/**
 * Color groups are used for DecoratedClient::color().
 * @since 5.3
 **/
enum class ColorGroup {
    /**
     * Inactive color, used for unfocused windows.
     **/
    Inactive,
    /**
     * Active color, used for focused windows.
     **/
    Active,
    /**
     * Warning color, can only be used with ColorRole::Foreground. If used with other roles,
     * a invalid QColor is returned. It can be used for close buttons and is typically red.
     **/
    Warning,
};

/**
 * Color roles are used for DecoratedClient::color().
 * @since 5.3
 **/
enum class ColorRole {
    /**
     * The decoration's frame background color.
     **/
    Frame,
    /**
     * The decoration's title bar background color
     **/
    TitleBar,
    /**
     * The decoration's title bar forground color
     **/
    Foreground,
};

}
