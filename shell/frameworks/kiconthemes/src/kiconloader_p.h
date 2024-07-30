/*
    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONLOADER_P_H
#define KICONLOADER_P_H

#include <QCache>
#include <QElapsedTimer>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringList>

#include "kiconcolors.h"
#include "kiconeffect.h"
#include "kiconloader.h"

class KIconThemeNode;

/*** KIconGroup: Icon type description. ***/

struct KIconGroup {
    int size;
};

/**
 * Holds a QPixmap for this process, along with its associated path on disk.
 */
struct PixmapWithPath {
    QPixmap pixmap;
    QString path;
};

/*** d pointer for KIconLoader. ***/
class KIconLoaderPrivate
{
public:
    KIconLoaderPrivate(const QString &_appname, const QStringList &extraSearchPaths, KIconLoader *qq);
    ~KIconLoaderPrivate();

    static KIconLoaderPrivate *get(KIconLoader *loader);

    void clear();

    /**
     * @internal
     */
    void init(const QString &_appname, const QStringList &extraSearchPaths = QStringList());

    /**
     * @internal
     */
    void initIconThemes();

    /**
     * @internal
     * tries to find an icon with the name. It tries some extension and
     * match strategies
     */
    QString findMatchingIcon(const QString &name, int size, qreal scale) const;

    /**
     * @internal
     * tries to find an icon with the name.
     * This is one layer above findMatchingIcon -- it also implements generic fallbacks
     * such as generic icons for mimetypes.
     */
    QString findMatchingIconWithGenericFallbacks(const QString &name, int size, qreal scale) const;

    /**
     * @internal
     * returns the preferred icon path for an icon with the name.
     * Can be used for a quick "hasIcon" check since it caches
     * that an icon was not found.
     */
    QString preferredIconPath(const QString &name);

    /**
     * @internal
     * Adds themes installed in the application's directory.
     **/
    void addAppThemes(const QString &appname, const QString &themeBaseDir = QString());

    /**
     * @internal
     * Adds all themes that are part of this node and the themes
     * below (the fallbacks of the theme) into the tree.
     */
    void addBaseThemes(KIconThemeNode *node, const QString &appname);

    /**
     * @internal
     * Recursively adds all themes that are specified in the "Inherits"
     * property of the given theme into the tree.
     */
    void addInheritedThemes(KIconThemeNode *node, const QString &appname);

    /**
     * @internal
     * Creates a KIconThemeNode out of a theme name, and adds this theme
     * as well as all its inherited themes into the tree. Themes that already
     * exist in the tree will be ignored and not added twice.
     */
    void addThemeByName(const QString &themename, const QString &appname);

    /**
     * Adds all the default themes from other desktops at the end of
     * the list of icon themes.
     */
    void addExtraDesktopThemes();

    /**
     * @internal
     * return the path for the unknown icon in that size
     */
    QString unknownIconPath(int size, qreal scale) const;

    /**
     * Checks if name ends in one of the supported icon formats (i.e. .png)
     * and returns the name without the extension if it does.
     */
    QString removeIconExtension(const QString &name) const;

    /**
     * @internal
     * Used with KIconLoader::loadIcon to convert the given name, size, group,
     * and icon state information to valid states. All parameters except the
     * name can be modified as well to be valid.
     */
    void normalizeIconMetadata(KIconLoader::Group &group, QSize &size, int &state) const;

    /**
     * @internal
     * Used with KIconLoader::loadIcon to get a base key name from the given
     * icon metadata. Ensure the metadata is normalized first.
     */
    QString makeCacheKey(const QString &name,
                         KIconLoader::Group group,
                         const QStringList &overlays,
                         const QSize &size,
                         qreal scale,
                         int state,
                         const KIconColors &colors) const;

    /**
     * @internal
     * If the icon is an SVG file, process it generating a stylesheet
     * following the current color scheme. in this case the icon can use named colors
     * as text color, background color, highlight color, positive/neutral/negative color
     * @see KColorScheme
     */
    QByteArray processSvg(const QString &path, KIconLoader::States state, const KIconColors &colors) const;

    /**
     * @internal
     * Creates the QImage for @p path, using SVG rendering as appropriate.
     * @p size is only used for scalable images, but if non-zero non-scalable
     * images will be resized anyways.
     */
    QImage createIconImage(const QString &path, const QSize &size, qreal scale, KIconLoader::States state, const KIconColors &colors);

    /**
     * @internal
     * Adds an QPixmap with its associated path to the shared icon cache.
     */
    void insertCachedPixmapWithPath(const QString &key, const QPixmap &data, const QString &path);

    /**
     * @internal
     * Retrieves the path and pixmap of the given key from the shared
     * icon cache.
     */
    bool findCachedPixmapWithPath(const QString &key, QPixmap &data, QString &path);

    /**
     * Find the given file in the search paths.
     */
    QString locate(const QString &fileName);

    /**
     * @internal
     * React to a global icon theme change
     */
    void _k_refreshIcons(int group);

    bool shouldCheckForUnknownIcons();

    KIconLoader *const q;

    QStringList mThemesInTree;
    KIconGroup *mpGroups = nullptr;
    KIconThemeNode *mpThemeRoot = nullptr;
    QStringList searchPaths;
#if KICONTHEMES_BUILD_DEPRECATED_SINCE(6, 5)
    KIconEffect mpEffect;
#endif
    QList<KIconThemeNode *> links;

    // This caches rendered QPixmaps in just this process.
    QCache<QString, PixmapWithPath> mPixmapCache;

    bool extraDesktopIconsLoaded : 1;
    // lazy loading: initIconThemes() is only needed when the "links" list is needed
    // mIconThemeInited is used inside initIconThemes() to init only once
    bool mIconThemeInited : 1;
    QString m_appname;

    void drawOverlays(const KIconLoader *loader, KIconLoader::Group group, int state, QPixmap &pix, const QStringList &overlays);

    QHash<QString, QString> mIconAvailability; // icon name -> actual icon name (not null if known to be available)
    QElapsedTimer mLastUnknownIconCheck; // recheck for unknown icons after kiconloader_ms_between_checks
    // the colors used to recolor svg icons stylesheets
    KIconColors mColors;
    QPalette mPalette;
    // to keep track if we are using a custom palette or just falling back to qApp;
    bool mCustomColors = false;
};

#endif // KICONLOADER_P_H
