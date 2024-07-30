/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONLOADER_H
#define KICONLOADER_H

#include <QObject>
#include <QSize>
#include <QString>
#include <QStringList>
#include <memory>

#if __has_include(<optional>) && __cplusplus >= 201703L
#include <optional>
#endif

#include <kiconthemes_export.h>

class QIcon;
class QMovie;
class QPixmap;

class KIconColors;
class KIconLoaderPrivate;
class KIconEffect;
class KIconTheme;

/**
 * @class KIconLoader kiconloader.h KIconLoader
 *
 * Iconloader for KDE.
 *
 * @note For most icon loading use cases perfer using QIcon::fromTheme
 *
 * KIconLoader will load the current icon theme and all its base themes.
 * Icons will be searched in any of these themes. Additionally, it caches
 * icons and applies effects according to the user's preferences.
 *
 * In KDE, it is encouraged to load icons by "Group". An icon group is a
 * location on the screen where icons are being used. Standard groups are:
 * Desktop, Toolbar, MainToolbar, Small and Panel. Each group can have some
 * centrally-configured effects applied to its icons. This makes it possible
 * to offer a consistent icon look in all KDE applications.
 *
 * The standard groups are defined below.
 *
 * @li KIconLoader::Desktop: Icons in the iconview of konqueror, kdesktop and similar apps.
 * @li KIconLoader::Toolbar: Icons in toolbars.
 * @li KIconLoader::MainToolbar: Icons in the main toolbars.
 * @li KIconLoader::Small: Various small (typical 16x16) places: titlebars, listviews
 * and menu entries.
 * @li KIconLoader::Panel: Icons in kicker's panel
 *
 * The icons are stored on disk in an icon theme or in a standalone
 * directory. The icon theme directories contain multiple sizes and/or
 * depths for the same icon. The iconloader will load the correct one based
 * on the icon group and the current theme. Icon themes are stored globally
 * in share/icons, or, application specific in share/apps/$appdir/icons.
 *
 * The standalone directories contain just one version of an icon. The
 * directories that are searched are: $appdir/pics and $appdir/toolbar.
 * Icons in these directories can be loaded by using the special group
 * "User".
 *
 */
class KICONTHEMES_EXPORT KIconLoader : public QObject
{
    Q_OBJECT

public:
    /**
     * Defines the context of the icon.
     */
    enum Context {
        Any, ///< Some icon with unknown purpose.
        Action, ///< An action icon (e.g. 'save', 'print').
        Application, ///< An icon that represents an application.
        Device, ///< An icon that represents a device.
        MimeType, ///< An icon that represents a mime type (or file type).
        Animation, ///< An icon that is animated.
        Category, ///< An icon that represents a category.
        Emblem, ///< An icon that adds information to an existing icon.
        Emote, ///< An icon that expresses an emotion.
        International, ///< An icon that represents a country's flag.
        Place, ///< An icon that represents a location (e.g. 'home', 'trash').
        StatusIcon, ///< An icon that represents an event.
    };
    Q_ENUM(Context)

    /**
     * The type of the icon.
     */
    enum Type {
        Fixed, ///< Fixed-size icon.
        Scalable, ///< Scalable-size icon.
        Threshold, ///< A threshold icon.
    };
    Q_ENUM(Type)

    /**
     * The type of a match.
     */
    enum MatchType {
        MatchExact, ///< Only try to find an exact match.
        MatchBest, ///< Take the best match if there is no exact match.
        MatchBestOrGreaterSize, ///< Take the best match or the match with a greater size if there is no exact match. @since 6.0
    };
    Q_ENUM(MatchType)

    /**
     * The group of the icon.
     */
    enum Group {
        /// No group
        NoGroup = -1,
        /// Desktop icons
        Desktop = 0,
        /// First group
        FirstGroup = 0,
        /// Toolbar icons
        Toolbar,
        /// Main toolbar icons
        MainToolbar,
        /// Small icons, e.g. for buttons
        Small,
        /// Panel (Plasma Taskbar) icons
        // TODO KF6: remove this (See https://phabricator.kde.org/T14340)
        Panel,
        /// Icons for use in dialog titles, page lists, etc
        Dialog,
        /// Last group
        LastGroup,
        /// User icons
        User,
    };
    Q_ENUM(Group)

    /**
     * These are the standard sizes for icons.
     */
    enum StdSizes {
        /// small icons for menu entries
        SizeSmall = 16,
        /// slightly larger small icons for toolbars, panels, etc
        SizeSmallMedium = 22,
        /// medium sized icons for the desktop
        SizeMedium = 32,
        /// large sized icons for the panel
        SizeLarge = 48,
        /// huge sized icons for iconviews
        SizeHuge = 64,
        /// enormous sized icons for iconviews
        SizeEnormous = 128,
    };
    Q_ENUM(StdSizes)

    /**
     * Defines the possible states of an icon.
     */
    enum States {
        DefaultState, ///< The default state.
        ActiveState, ///< Icon is active.
        DisabledState, ///< Icon is disabled.
        SelectedState, ///< Icon is selected. @since 5.22
        LastState, ///< Last state (last constant)
    };
    Q_ENUM(States)

    /**
     * Constructs an iconloader.
     * @param appname Add the data directories of this application to the
     * icon search path for the "User" group. The default argument adds the
     * directories of the current application.
     * @param extraSearchPaths additional search paths, either absolute or relative to GenericDataLocation
     *
     * Usually, you use the default iconloader, which can be accessed via
     * KIconLoader::global(), so you hardly ever have to create an
     * iconloader object yourself. That one is the application's iconloader.
     */
    explicit KIconLoader(const QString &appname = QString(), const QStringList &extraSearchPaths = QStringList(), QObject *parent = nullptr);

    /**
     * Cleanup
     */
    ~KIconLoader() override;

    /**
     * Returns the global icon loader initialized with the application name.
     * @return global icon loader
     */
    static KIconLoader *global();

    /**
     * Adds @p appname to the list of application specific directories with @p themeBaseDir as its base directory.
     * Assume the icons are in /home/user/app/icons/hicolor/48x48/my_app.png, the base directory would be
     * /home/user/app/icons; KIconLoader automatically searches @p themeBaseDir + "/hicolor"
     * This directory must contain a dir structure as defined by the XDG icons specification
     * @param appname The application name.
     * @param themeBaseDir The base directory of the application's theme (eg. "/home/user/app/icons")
     */
    void addAppDir(const QString &appname, const QString &themeBaseDir = QString());

    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See KIconLoader::StdSizes.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param overlays a list of emblem icons to overlay, by name
     *                 @see drawOverlays
     * @param path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found @p path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     * @param canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. <em>Note:</em> a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     * @return the QPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     */
    QPixmap loadIcon(const QString &name,
                     KIconLoader::Group group,
                     int size = 0,
                     int state = KIconLoader::DefaultState,
                     const QStringList &overlays = QStringList(),
                     QString *path_store = nullptr,
                     bool canReturnNull = false) const;

    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See KIconLoader::StdSizes.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param overlays a list of emblem icons to overlay, by name
     *                 @see drawOverlays
     * @param path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found @p path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     * @param canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. <em>Note:</em> a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     * @return the QPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     * @since 5.48
     */
    // TODO KF6 merge loadIcon() and loadScaledIcon()
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           int size = 0,
                           int state = KIconLoader::DefaultState,
                           const QStringList &overlays = QStringList(),
                           QString *path_store = nullptr,
                           bool canReturnNull = false) const;

    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See KIconLoader::StdSizes. The icon will be fit into @p size
     *             without changing the aspect ratio, which particularly matters
     *             for non-square icons.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param overlays a list of emblem icons to overlay, by name
     *                 @see drawOverlays
     * @param path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found @p path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     * @param canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. <em>Note:</em> a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     * @return the QPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     * @since 5.81
     */
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           const QSize &size = {},
                           int state = KIconLoader::DefaultState,
                           const QStringList &overlays = QStringList(),
                           QString *path_store = nullptr,
                           bool canReturnNull = false) const;

#if __has_include(<optional>) && __cplusplus >= 201703L
    /**
     * Loads an icon. It will try very hard to find an icon which is
     * suitable. If no exact match is found, a close match is searched.
     * If neither an exact nor a close match is found, a null pixmap or
     * the "unknown" pixmap is returned, depending on the value of the
     * @p canReturnNull parameter.
     *
     * @param name The name of the icon, without extension.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param scale The scale of the icon group to use. If no icon exists in the
     * scaled group, a 1x icon with its size multiplied by the scale will be
     * loaded instead.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See KIconLoader::StdSizes. The icon will be fit into @p size
     *             without changing the aspect ratio, which particularly matters
     *             for non-square icons.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param overlays a list of emblem icons to overlay, by name
     *                 @see drawOverlays
     * @param path_store If not null, the path of the icon is stored here,
     *        if the icon was found. If the icon was not found @p path_store
     *        is unaltered even if the "unknown" pixmap was returned.
     * @param canReturnNull Can return a null pixmap? If false, the
     *        "unknown" pixmap is returned when no appropriate icon has been
     *        found. <em>Note:</em> a null pixmap can still be returned in the
     *        event of invalid parameters, such as empty names, negative sizes,
     *        and etc.
     * @param colorScheme will define the stylesheet used to color this icon.
     *        Note this will only work if @p name is an svg file.
     * @return the QPixmap. Can be null when not found, depending on
     *         @p canReturnNull.
     * @since 5.88
     */
    QPixmap loadScaledIcon(const QString &name,
                           KIconLoader::Group group,
                           qreal scale,
                           const QSize &size,
                           int state,
                           const QStringList &overlays,
                           QString *path_store,
                           bool canReturnNull,
                           const std::optional<KIconColors> &colorScheme) const;
#endif

    /**
     * Loads an icon for a mimetype.
     * This is basically like loadIcon except that extra desktop themes are loaded if necessary.
     *
     * Consider using QIcon::fromTheme() with a fallback to "application-octet-stream" instead.
     *
     * @param iconName The name of the icon, without extension, usually from KMimeType.
     * @param group The icon group. This will specify the size of and effects to
     * be applied to the icon.
     * @param size If nonzero, this overrides the size specified by @p group.
     *             See KIconLoader::StdSizes.
     * @param state The icon state: @p DefaultState, @p ActiveState or
     * @p DisabledState. Depending on the user's preferences, the iconloader
     * may apply a visual effect to hint about its state.
     * @param path_store If not null, the path of the icon is stored here.
     * @param overlays a list of emblem icons to overlay, by name
     *                 @see drawOverlays
     * @return the QPixmap. Can not be null, the
     * "unknown" pixmap is returned when no appropriate icon has been found.
     */
    QPixmap loadMimeTypeIcon(const QString &iconName,
                             KIconLoader::Group group,
                             int size = 0,
                             int state = KIconLoader::DefaultState,
                             const QStringList &overlays = QStringList(),
                             QString *path_store = nullptr) const;

    /**
     * Returns the path of an icon.
     * @param name The name of the icon, without extension. If an absolute
     * path is supplied for this parameter, iconPath will return it
     * directly.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     * @param canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     * @return the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on @p canReturnNull.
     */
    QString iconPath(const QString &name, int group_or_size, bool canReturnNull = false) const;

    /**
     * Returns the path of an icon.
     * @param name The name of the icon, without extension. If an absolute
     * path is supplied for this parameter, iconPath will return it
     * directly.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     * @param canReturnNull Can return a null string? If not, a path to the
     *                      "unknown" icon will be returned.
     * @param scale The scale of the icon group.
     * @return the path of an icon, can be null or the "unknown" icon when
     *         not found, depending on @p canReturnNull.
     * @since 5.48
     */
    // TODO KF6 merge iconPath() with and without "scale" and move that argument after "group_or_size"
    QString iconPath(const QString &name, int group_or_size, bool canReturnNull, qreal scale) const;

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /**
     * Loads an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See KIconLoader::StdSizes.
     * @param parent The parent object of the returned QMovie.
     * @return A QMovie object. Can be null if not found or not valid.
     *         Ownership is passed to the caller.
     * @deprecated since 6.5, use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QMovie *loadMovie(const QString &name, KIconLoader::Group group, int size = 0, QObject *parent = nullptr) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /**
     * Returns the path to an animated icon.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See KIconLoader::StdSizes.
     * @return the full path to the movie, ready to be passed to QMovie's constructor.
     * Empty string if not found.
     * @deprecated since 6.5, use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QString moviePath(const QString &name, KIconLoader::Group group, int size = 0) const;
#endif

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /**
     * Loads an animated icon as a series of still frames. If you want to load
     * a .mng animation as QMovie instead, please use loadMovie() instead.
     * @param name The name of the icon.
     * @param group The icon group. See loadIcon().
     * @param size Override the default size for @p group.
     *             See KIconLoader::StdSizes.
     * @return A QStringList containing the absolute path of all the frames
     * making up the animation.
     *
     * @deprecated since 6.5, use QMovie API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use QMovie API")
    QStringList loadAnimated(const QString &name, KIconLoader::Group group, int size = 0) const;
#endif

    /**
     * Queries all available icons for a specific group, having a specific
     * context.
     * @param group_or_size If positive, search icons whose size is
     * specified by the icon group @p group_or_size. If negative, search
     * icons whose size is - @p group_or_size.
     *             See KIconLoader::Group and KIconLoader::StdSizes
     * @param context The icon context.
     * @return a list of all icons
     */
    QStringList queryIcons(int group_or_size, KIconLoader::Context context = KIconLoader::Any) const;

    /**
     * Queries all available icons for a specific context.
     * @param group_or_size The icon preferred group or size. If available
     * at this group or size, those icons will be returned, in other case,
     * icons of undefined size will be returned. Positive numbers are groups,
     * negative numbers are negated sizes. See KIconLoader::Group and
     * KIconLoader::StdSizes
     * @param context The icon context.
     * @return A QStringList containing the icon names
     * available for that context
     */
    QStringList queryIconsByContext(int group_or_size, KIconLoader::Context context = KIconLoader::Any) const;

    /**
     * @internal
     */
    bool hasContext(KIconLoader::Context context) const;

    /**
     * Returns a list of all icons (*.png or *.xpm extension) in the
     * given directory.
     * @param iconsDir the directory to search in
     * @return A QStringList containing the icon paths
     */
    QStringList queryIconsByDir(const QString &iconsDir) const;

    /**
     * Returns all the search paths for this icon loader, either absolute or
     * relative to GenericDataLocation.
     * Mostly internal (for KIconDialog).
     * \since 5.0
     */
    QStringList searchPaths() const;

    /**
     * Returns the size of the specified icon group.
     * Using e.g. KIconLoader::SmallIcon will return 16.
     * @param group the group to check.
     * @return the current size for an icon group.
     */
    int currentSize(KIconLoader::Group group) const;

    /**
     * Returns a pointer to the current theme. Can be used to query
     * available and default sizes for groups.
     * @note The KIconTheme will change if reconfigure() is called and
     * therefore it's not recommended to store the pointer anywhere.
     * @return a pointer to the current theme. 0 if no theme set.
     */
    KIconTheme *theme() const;

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /**
     * Returns a pointer to the KIconEffect object used by the icon loader.
     * @return the KIconEffect.
     *
     * @deprecated since 6.5, use the static KIconEffect API
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use static KIconEffect API")
    KIconEffect *iconEffect() const;
#endif

    /**
     * Reconfigure the icon loader, for instance to change the associated app name or extra search paths.
     * This also clears the in-memory pixmap cache (even if the appname didn't change, which is useful for unittests)
     * @param appname the application name (empty for the global iconloader)
     * @param extraSearchPaths additional search paths, either absolute or relative to GenericDataLocation
     */
    void reconfigure(const QString &appname, const QStringList &extraSearchPaths = QStringList());

    /**
     * Returns the unknown icon. An icon that is used when no other icon
     * can be found.
     * @return the unknown pixmap
     */
    static QPixmap unknown();

#if KICONTHEMES_ENABLE_DEPRECATED_SINCE(6, 5)
    /**
     * Draws overlays on the specified pixmap, it takes the width and height
     * of the pixmap into consideration
     * @param overlays List of up to 4 overlays to blend over the pixmap. The first overlay
     *                 will be in the bottom right corner, followed by bottom left, top left
     *                 and top right. An empty QString can be used to leave the specific position
     *                 blank.
     * @param pixmap to draw on
     * @since 4.7
     * @deprecated since 6.5, use KIconUtils::addOverlays from KGuiAddons
     */
    KICONTHEMES_DEPRECATED_VERSION(6, 5, "Use KIconUtils::addOverlays from KGuiAddons")
    void drawOverlays(const QStringList &overlays, QPixmap &pixmap, KIconLoader::Group group, int state = KIconLoader::DefaultState) const;
#endif

    bool hasIcon(const QString &iconName) const;

    /**
     * Sets the colors for this KIconLoader.
     * NOTE: if you set a custom palette, if you are using some colors from
     * application's palette, you need to track the application palette changes by yourself.
     * If you no longer wish to use a custom palette, use resetPalette()
     * @see resetPalette
     * @since 5.39
     */
    void setCustomPalette(const QPalette &palette);

    /**
     * The colors that will be used for the svg stylesheet in case the
     * loaded icons are svg-based, icons can be colored in different ways in
     * different areas of the application
     * @return the palette, if any, an invalid one if the user didn't set it
     * @since 5.39
     */
    QPalette customPalette() const;

    /**
     * Resets the custom palette used by the KIconLoader to use the
     * QGuiApplication::palette() again (and to follow it in case the
     * application's palette changes)
     * @since 5.39
     */
    void resetPalette();

    /**
     * @returns whether we have set a custom palette using @f setCustomPalette
     *
     * @since 5.85
     * @see resetPalette, setCustomPalette
     */
    bool hasCustomPalette() const;

public Q_SLOTS:
    // TODO: while marked as deprecated, newIconLoader() is still used:
    // internally by KIconLoadeer as well as by Plasma's Icons kcm module (state: 5.17)
    // this needs some further cleanup work before removing it from the API with KICONTHEMES_ENABLE_DEPRECATED_SINCE
    /**
     * Re-initialize the global icon loader
     *
     * @todo Check deprecation, still used internally.
     * @deprecated Since 5.0, use emitChange(Group)
     */
    KICONTHEMES_DEPRECATED_VERSION(5, 0, "Use KIconLoader::emitChange(Group)") // TODO KF6 remove
    void newIconLoader();

    /**
     * Emits an iconChanged() signal on all the KIconLoader instances in the system
     * indicating that a system's icon group has changed in some way. It will also trigger
     * a reload in all of them to update to the new theme.
     *
     * @p group indicates the group that has changed
     *
     * @since 5.0
     */
    static void emitChange(Group group);

Q_SIGNALS:
    /**
     * Emitted by newIconLoader once the new settings have been loaded
     */
    void iconLoaderSettingsChanged();

    /**
     * Emitted when the system icon theme changes
     *
     * @since 5.0
     */
    void iconChanged(int group);

private:
    friend class KIconLoaderPrivate;
    // @internal the data object
    std::unique_ptr<KIconLoaderPrivate> const d;

    Q_PRIVATE_SLOT(d, void _k_refreshIcons(int group))
};

namespace KDE
{
/**
 * \relates KIconLoader
 * Returns a QIcon with an appropriate
 * KIconEngine to perform loading and rendering.  KIcons thus adhere to
 * KDE style and effect standards.
 * @since 5.0
 */
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, KIconLoader *iconLoader = nullptr);
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader = nullptr);

/**
 * \relates KIconLoader
 * Returns a QIcon for the given icon, with additional overlays.
 * @since 5.0
 */
KICONTHEMES_EXPORT QIcon icon(const QString &iconName, const QStringList &overlays, KIconLoader *iconLoader = nullptr);

}

inline KIconLoader::Group &operator++(KIconLoader::Group &group)
{
    group = static_cast<KIconLoader::Group>(group + 1);
    return group;
}
inline KIconLoader::Group operator++(KIconLoader::Group &group, int)
{
    KIconLoader::Group ret = group;
    ++group;
    return ret;
}

#endif // KICONLOADER_H
