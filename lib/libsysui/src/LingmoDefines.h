/**
 *  @brief This file contains commonly used types, etc.
 *         for LingmoUI
 */
#ifndef LINGMODEFINES_H
#define LINGMODEFINES_H

#include <QObject>
#include <QQmlEngine>

/**
 * @brief LingmoWindowType
 */
namespace LingmoWindowType {
Q_NAMESPACE

enum LaunchMode {
    Standard = 0,
    SingleTask = 1,
    SingleInstance = 2
};
Q_ENUM_NS(LaunchMode)

QML_NAMED_ELEMENT(LingmoWindowType)
}

namespace LingmoThemeType {
Q_NAMESPACE
enum DarkMode {
    System = 0x0000,
    Light = 0x0001,
    Dark = 0x0002,
};

Q_ENUM_NS(DarkMode)

QML_NAMED_ELEMENT(LingmoThemeType)
}

// Used in LingmoCustomDialog
namespace LingmoCustomDialogType {
Q_NAMESPACE
enum ButtonFlag { NeutralButton = 0x0001,
    NegativeButton = 0x0002,
    PositiveButton = 0x0004 };

Q_ENUM_NS(ButtonFlag)

QML_NAMED_ELEMENT(LingmoCustomDialogType)
}

// Used in LingmoTabView
namespace LingmoTabViewType {
    Q_NAMESPACE
    enum TabWidthBehavior { Equal = 0x0000, SizeToContent = 0x0001, Compact = 0x0002 };

    Q_ENUM_NS(TabWidthBehavior)

    enum CloseButtonVisibility { Never = 0x0000, Always = 0x0001, OnHover = 0x0002 };

    Q_ENUM_NS(CloseButtonVisibility)

    QML_NAMED_ELEMENT(LingmoTabViewType)
}

namespace LingmoNavigationViewType {
    Q_NAMESPACE
    enum DisplayMode { Open = 0x0000, Compact = 0x0001, Minimal = 0x0002, Auto = 0x0004 };

    Q_ENUM_NS(DisplayMode)

    enum PageMode { Stack = 0x0000, NoStack = 0x0001 };

    Q_ENUM_NS(PageMode)

    QML_NAMED_ELEMENT(LingmoNavigationViewType)
}

namespace LingmoPageType {
    Q_NAMESPACE
    enum LaunchMode {
        Standard = 0x0000,
        SingleTask = 0x0001,
        SingleTop = 0x0002,
        SingleInstance = 0x0004
    };

    Q_ENUM_NS(LaunchMode)

    QML_NAMED_ELEMENT(LingmoPageType)
}

#endif // LINGMODEFINES_H
