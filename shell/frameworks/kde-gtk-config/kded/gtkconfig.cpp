/*
 * SPDX-FileCopyrightText: 2019 Mikhail Zolotukhin <zomial@protonmail.com>
 * SPDX-FileCopyrightText: 2019 Nicolas Fella <nicolas.fella@gmx.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "gtkconfig.h"

#include <KColorScheme>
#include <KPluginFactory>
#include <KWindowSystem>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QFont>
#include <QGuiApplication>
#include <QTimer>

#include <algorithm>
#include <cmath>

#include "config_editor/custom_css.h"
#include "config_editor/gsettings.h"
#include "config_editor/gtk2.h"
#include "config_editor/settings_ini.h"
#include "config_editor/xsettings.h"
#include "gsd-xsettings-manager/gsd-xsettings-manager.h"

K_PLUGIN_CLASS_WITH_JSON(GtkConfig, "gtkconfig.json")

GtkConfig::GtkConfig(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
    , configValueProvider(new ConfigValueProvider())
    , themePreviewer(new ThemePreviewer(this))
    , kdeglobalsConfigWatcher(KConfigWatcher::create(KSharedConfig::openConfig()))
    , kwinConfigWatcher(KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kwinrc"))))
    , kcmfontsConfigWatcher(KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kcmfonts"))))
    , kcminputConfigWatcher(KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kcminputrc"))))
    , breezeConfigWatcher(KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("breezerc"))))
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(QStringLiteral("org.kde.GtkConfig"));
    dbus.registerObject(QStringLiteral("/GtkConfig"), this, QDBusConnection::ExportScriptableSlots);

    if (qgetenv("GTK_USE_PORTAL") != "1" && KWindowSystem::isPlatformWayland()) {
        m_gsdXsettingsManager = new GSDXSettingsManager(this);
    }

    connect(kdeglobalsConfigWatcher.data(), &KConfigWatcher::configChanged, this, &GtkConfig::onKdeglobalsSettingsChange);
    connect(kwinConfigWatcher.data(), &KConfigWatcher::configChanged, this, &GtkConfig::onKWinSettingsChange);
    connect(kcmfontsConfigWatcher.data(), &KConfigWatcher::configChanged, this, &GtkConfig::onKCMFontsSettingsChange);
    connect(kcminputConfigWatcher.data(), &KConfigWatcher::configChanged, this, &GtkConfig::onKCMInputSettingsChange);
    connect(breezeConfigWatcher.data(), &KConfigWatcher::configChanged, this, &GtkConfig::onBreezeSettingsChange);

    Gtk2ConfigEditor::removeLegacyStrings();
    applyAllSettings();
}

GtkConfig::~GtkConfig()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService(QStringLiteral("org.kde.GtkConfig"));
    dbus.unregisterObject(QStringLiteral("/GtkConfig"));
}

void GtkConfig::setGtk2Theme(const QString &themeName, const bool preferDarkTheme) const
{
    // GTK2 does not support using dark variant automatically, so we have to get dark preference and switch based on that
    QString possiblydarkthemeName = themeName;
    if (themeName == QLatin1String("Breeze") && preferDarkTheme) {
        possiblydarkthemeName = QStringLiteral("Breeze-Dark");
    }

    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-theme-name"), possiblydarkthemeName);
    XSettingsEditor::setValue(QStringLiteral("Net/ThemeName"), possiblydarkthemeName);
}

void GtkConfig::setGtkTheme(const QString &themeName) const
{
    setGtk2Theme(themeName, configValueProvider->preferDarkTheme());
    GSettingsEditor::setValue("gtk-theme", themeName);
    SettingsIniEditor::setValue(QStringLiteral("gtk-theme-name"), themeName);

    // Window decorations are part of the theme, in case of Breeze we inject custom ones from KWin
    setWindowDecorationsAppearance();
}

QString GtkConfig::gtkTheme() const
{
    return SettingsIniEditor::value(QStringLiteral("gtk-theme-name"));
}

void GtkConfig::showGtkThemePreview(const QString &themeName) const
{
    themePreviewer->showGtk3App(themeName);
}

void GtkConfig::setFont() const
{
    const QString configFontName = configValueProvider->fontName();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-font-name"), configFontName);
    GSettingsEditor::setValue("font-name", configFontName);
    SettingsIniEditor::setValue(QStringLiteral("gtk-font-name"), configFontName);
    XSettingsEditor::setValue(QStringLiteral("Gtk/FontName"), configFontName);
}

void GtkConfig::setIconTheme() const
{
    const QString iconThemeName = configValueProvider->iconThemeName();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-icon-theme-name"), iconThemeName);
    GSettingsEditor::setValue("icon-theme", iconThemeName);
    SettingsIniEditor::setValue(QStringLiteral("gtk-icon-theme-name"), iconThemeName);
    XSettingsEditor::setValue(QStringLiteral("Net/IconThemeName"), iconThemeName);
}

void GtkConfig::setSoundTheme() const
{
    const QString soundThemeName = configValueProvider->soundThemeName();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-sound-theme-name"), soundThemeName);
    GSettingsEditor::setValue("theme-name", soundThemeName, "org.gnome.desktop.sound");
    SettingsIniEditor::setValue(QStringLiteral("gtk-sound-theme-name"), soundThemeName);
    XSettingsEditor::setValue(QStringLiteral("Net/SoundThemeName"), soundThemeName);
}

void GtkConfig::setEventSoundsEnabled() const
{
    const bool soundsEnabled = configValueProvider->eventSoundsEnabled();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-enable-event-sounds"), soundsEnabled);
    GSettingsEditor::setValue("event-sounds", soundsEnabled, "org.gnome.desktop.sound");
    SettingsIniEditor::setValue(QStringLiteral("gtk-enable-event-sounds"), soundsEnabled);
    XSettingsEditor::setValue(QStringLiteral("Net/EnableEventSounds"), soundsEnabled);
}

void GtkConfig::setCursorTheme() const
{
    const QString cursorThemeName = configValueProvider->cursorThemeName();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-cursor-theme-name"), cursorThemeName);
    GSettingsEditor::setValue("cursor-theme", cursorThemeName);
    SettingsIniEditor::setValue(QStringLiteral("gtk-cursor-theme-name"), cursorThemeName);
    XSettingsEditor::setValue(QStringLiteral("Gtk/CursorThemeName"), cursorThemeName);
}

void GtkConfig::setCursorSize() const
{
    qreal xwaylandScale = 1.0;
    if (KWindowSystem::isPlatformWayland()) {
        xwaylandScale = configValueProvider->x11GlobalScaleFactor();
    }

    const int cursorSize = configValueProvider->cursorSize();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-cursor-theme-size"), cursorSize);
    GSettingsEditor::setValue("cursor-size", cursorSize);
    SettingsIniEditor::setValue(QStringLiteral("gtk-cursor-theme-size"), cursorSize);
    XSettingsEditor::setValue(QStringLiteral("Gtk/CursorThemeSize"), int(cursorSize * xwaylandScale));
}

void GtkConfig::setIconsOnButtons() const
{
    const bool iconsOnButtonsConfigValue = configValueProvider->iconsOnButtons();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-button-images"), iconsOnButtonsConfigValue);
    // Deprecated in GTK 4
    SettingsIniEditor::setValue(QStringLiteral("gtk-button-images"), iconsOnButtonsConfigValue, 3);
    XSettingsEditor::setValue(QStringLiteral("Gtk/ButtonImages"), iconsOnButtonsConfigValue);
}

void GtkConfig::setIconsInMenus() const
{
    const bool iconsInMenusConfigValue = configValueProvider->iconsInMenus();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-menu-images"), iconsInMenusConfigValue);
    // Deprecated in GTK 4
    SettingsIniEditor::setValue(QStringLiteral("gtk-menu-images"), iconsInMenusConfigValue, 3);
    XSettingsEditor::setValue(QStringLiteral("Gtk/MenuImages"), iconsInMenusConfigValue);
}

void GtkConfig::setToolbarStyle() const
{
    const int toolbarStyle = configValueProvider->toolbarStyle();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-toolbar-style"), toolbarStyle);
    GSettingsEditor::setValueAsEnum("toolbar-style", toolbarStyle);
    // Deprecated in GTK 4
    SettingsIniEditor::setValue(QStringLiteral("gtk-toolbar-style"), toolbarStyle, 3);
    XSettingsEditor::setValue(QStringLiteral("Gtk/ToolbarStyle"), toolbarStyle);
}

void GtkConfig::setScrollbarBehavior() const
{
    const bool scrollbarBehavior = configValueProvider->scrollbarBehavior();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-primary-button-warps-slider"), scrollbarBehavior);
    SettingsIniEditor::setValue(QStringLiteral("gtk-primary-button-warps-slider"), scrollbarBehavior);
    XSettingsEditor::setValue(QStringLiteral("Gtk/PrimaryButtonWarpsSlider"), scrollbarBehavior);
}

void GtkConfig::setDoubleClickInterval() const
{
    const int doubleClickInterval = configValueProvider->doubleClickInterval();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-double-click-time"), doubleClickInterval);
    GSettingsEditor::setValue("double-click", doubleClickInterval, "org.gnome.desktop.peripherals.mouse");
    SettingsIniEditor::setValue(QStringLiteral("gtk-double-click-time"), doubleClickInterval);
    XSettingsEditor::setValue(QStringLiteral("Net/DoubleClickTime"), doubleClickInterval);
}

void GtkConfig::setDarkThemePreference() const
{
    const bool preferDarkTheme = configValueProvider->preferDarkTheme();
    SettingsIniEditor::setValue(QStringLiteral("gtk-application-prefer-dark-theme"), preferDarkTheme);
    // https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas/-/blob/master/headers/gdesktop-enums.h
    GSettingsEditor::setValueAsEnum("color-scheme",
                                    preferDarkTheme ? 1 /*G_DESKTOP_COLOR_SCHEME_PREFER_DARK*/ : 0 /*G_DESKTOP_COLOR_SCHEME_DEFAULT*/,
                                    "org.gnome.desktop.interface");
    setGtk2Theme(gtkTheme(), preferDarkTheme);
}

void GtkConfig::setWindowDecorationsAppearance() const
{
    if (gtkTheme() == QStringLiteral("Breeze")) { // Only Breeze GTK supports custom decoration buttons
        const auto windowDecorationsButtonsImages = configValueProvider->windowDecorationsButtonsImages();
        CustomCssEditor::setCustomClientSideDecorations(windowDecorationsButtonsImages);
    } else {
        CustomCssEditor::disableCustomClientSideDecorations();
    }
}

void GtkConfig::setWindowDecorationsButtonsOrder() const
{
    const QString windowDecorationsButtonOrder = configValueProvider->windowDecorationsButtonsOrder();
    GSettingsEditor::setValue("button-layout", windowDecorationsButtonOrder, "org.gnome.desktop.wm.preferences");
    SettingsIniEditor::setValue(QStringLiteral("gtk-decoration-layout"), windowDecorationsButtonOrder);
    XSettingsEditor::setValue(QStringLiteral("Gtk/DecorationLayout"), windowDecorationsButtonOrder);
}

void GtkConfig::setEnableAnimations() const
{
    const bool enableAnimations = configValueProvider->enableAnimations();
    Gtk2ConfigEditor::setValue(QStringLiteral("gtk-enable-animations"), enableAnimations);
    GSettingsEditor::setValue("enable-animations", enableAnimations);
    SettingsIniEditor::setValue(QStringLiteral("gtk-enable-animations"), enableAnimations);
    XSettingsEditor::setValue(QStringLiteral("Gtk/EnableAnimations"), enableAnimations);
    if (m_gsdXsettingsManager) {
        m_gsdXsettingsManager->enableAnimationsChanged();
    }
}

void GtkConfig::setGlobalScale() const
{
    const unsigned scaleFactor = configValueProvider->x11GlobalScaleFactor();
    XSettingsEditor::setValue(QStringLiteral("Gdk/WindowScalingFactor"), scaleFactor);
    GSettingsEditor::setValue("scaling-factor", scaleFactor); // For IntelliJ IDEA
}

void GtkConfig::setTextScale() const
{
    const double x11Scale = configValueProvider->x11GlobalScaleFactor();
    const int x11ScaleIntegerPart = int(x11Scale);

    const int forceFontDpi = configValueProvider->fontDpi();

    int x11TextDpiAbsolute = 96 * 1024;
    double waylandTextScaleFactor = 1.0;

    if (forceFontDpi == 0) {
        x11TextDpiAbsolute = (96 * 1024) * x11Scale;
    } else {
        x11TextDpiAbsolute = (forceFontDpi * 1024);

        if (!KWindowSystem::isPlatformX11()) {
            x11TextDpiAbsolute *= x11Scale;
        }

        waylandTextScaleFactor = double(forceFontDpi) / 96.0;
        waylandTextScaleFactor = std::clamp(waylandTextScaleFactor, 0.5, 3.0);
    }

    XSettingsEditor::unsetValue(QStringLiteral("Xft/DPI"));
    SettingsIniEditor::setValue(QStringLiteral("gtk-xft-dpi"), x11TextDpiAbsolute);
    XSettingsEditor::setValue(QStringLiteral("Gdk/UnscaledDPI"), x11TextDpiAbsolute / x11ScaleIntegerPart);
    GSettingsEditor::setValue("text-scaling-factor", waylandTextScaleFactor);
}

void GtkConfig::setColors() const
{
    CustomCssEditor::addGtkModule(QStringLiteral("colorreload-gtk-module"));
    if (m_gsdXsettingsManager) {
        m_gsdXsettingsManager->modulesChanged();
    }
    // modulesChanged signal will take some time to reach a GTK app, so explicitly wait a moment
    QTimer::singleShot(200, this, [this] {
        const QMap<QString, QColor> colors = configValueProvider->colors();
        CustomCssEditor::setColors(colors);
    });
}

void GtkConfig::applyAllSettings() const
{
    setFont();
    setIconTheme();
    setSoundTheme();
    setCursorTheme();
    setCursorSize();
    setIconsOnButtons();
    setIconsInMenus();
    setToolbarStyle();
    setScrollbarBehavior();
    setDarkThemePreference();
    setWindowDecorationsAppearance();
    setWindowDecorationsButtonsOrder();
    setEnableAnimations();
    setGlobalScale();
    setTextScale();
    setColors();
}

void GtkConfig::onKdeglobalsSettingsChange(const KConfigGroup &group, const QByteArrayList &names) const
{
    if (group.name() == QStringLiteral("KDE")) {
        if (names.contains(QByteArrayLiteral("AnimationDurationFactor"))) {
            setEnableAnimations();
        }
        if (names.contains(QByteArrayLiteral("ShowIconsInMenuItems"))) {
            setIconsInMenus();
        }
        if (names.contains(QByteArrayLiteral("ShowIconsOnPushButtons"))) {
            setIconsOnButtons();
        }
        // ScrollbarLeftClickNavigatesByPage is now the default setting, so when it's
        // true, it won't be present, so we need to check for its absence
        if (!names.contains(QByteArrayLiteral("ScrollbarLeftClickNavigatesByPage"))) {
            setScrollbarBehavior();
        }
        if (names.contains(QByteArrayLiteral("DoubleClickInterval"))) {
            setDoubleClickInterval();
        }
    } else if (group.name() == QStringLiteral("Icons")) {
        if (names.contains(QByteArrayLiteral("Theme"))) {
            setIconTheme();
        }
    } else if (group.name() == QLatin1String("Sounds")) {
        if (names.contains(QByteArrayLiteral("Theme"))) {
            setSoundTheme();
        }
        if (names.contains(QByteArrayLiteral("Enable"))) {
            setEventSoundsEnabled();
        }
    } else if (group.name() == QStringLiteral("General")) {
        if (names.contains(QByteArrayLiteral("font"))) {
            setFont();
        }
        if (names.contains(QByteArrayLiteral("ColorScheme")) || names.contains(QByteArrayLiteral("AccentColor"))) {
            setColors();
            setDarkThemePreference();
            setWindowDecorationsAppearance(); // Decorations' color can depend on the current color scheme
        }
    } else if (group.name() == QStringLiteral("KScreen")) {
        if (names.contains(QByteArrayLiteral("ScaleFactor"))) {
            setGlobalScale();
            // setTextScale() will be called in onKCMFontsSettingsChange
        }
    } else if (group.name() == QStringLiteral("Toolbar style")) {
        if (names.contains(QByteArrayLiteral("ToolButtonStyle"))) {
            setToolbarStyle();
        }
    }
}

void GtkConfig::onKWinSettingsChange(const KConfigGroup &group, const QByteArrayList &names) const
{
    if (group.name() == QStringLiteral("org.kde.kdecoration2")) {
        if (names.contains(QByteArrayLiteral("ButtonsOnRight")) //
            || names.contains(QByteArrayLiteral("ButtonsOnLeft"))) {
            setWindowDecorationsButtonsOrder();
        }
        if (names.contains(QByteArrayLiteral("theme"))) {
            setWindowDecorationsAppearance();
        }
    } else if (group.name() == QStringLiteral("Xwayland")) {
        if (names.contains(QByteArrayLiteral("Scale"))) {
            setGlobalScale();
            setTextScale();
            setCursorSize();
        }
    }
}

void GtkConfig::onKCMFontsSettingsChange(const KConfigGroup &group, const QByteArrayList &names) const
{
    if (group.name() == QStringLiteral("General")) {
        if (names.contains("forceFontDPI")) {
            setTextScale();
        }
    }
}

void GtkConfig::onKCMInputSettingsChange(const KConfigGroup &group, const QByteArrayList &names) const
{
    if (group.name() == QStringLiteral("Mouse")) {
        if (names.contains("cursorTheme")) {
            setCursorTheme();
        }
        if (names.contains("cursorSize")) {
            setCursorSize();
        }
    }
}

void GtkConfig::onBreezeSettingsChange(const KConfigGroup &group, const QByteArrayList &names) const
{
    if (group.name() == QStringLiteral("Common") //
        && names.contains("OutlineCloseButton")) {
        setWindowDecorationsAppearance();
    }
}

#include "gtkconfig.moc"

#include "moc_gtkconfig.cpp"
