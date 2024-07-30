/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSVG_IMAGESET_P_H
#define KSVG_IMAGESET_P_H

#include "imageset.h"
#include "svg.h"
#include <QHash>

#include <KColorScheme>
#include <KImageCache>
#include <KPluginMetaData>
#include <KSharedDataCache>
#include <QDebug>
#include <QTimer>

#include <KConfigGroup>

namespace KSvg
{
class ImageSet;

enum CacheType {
    NoCache = 0,
    PixmapCache = 1,
    SvgElementsCache = 2,
};
Q_DECLARE_FLAGS(CacheTypes, CacheType)
Q_DECLARE_OPERATORS_FOR_FLAGS(CacheTypes)

class ImageSetPrivate : public QObject, public QSharedData
{
    Q_OBJECT

public:
    explicit ImageSetPrivate(QObject *parent = nullptr);
    ~ImageSetPrivate() override;

    QString imagePath(const QString &theme, const QString &type, const QString &image);
    QString findInImageSet(const QString &image, const QString &theme, bool cache = true);
    void discardCache(CacheTypes caches);
    void scheduleImageSetChangeNotification(CacheTypes caches);
    bool useCache();
    void setImageSetName(const QString &themeName, bool emitChanged);

    QColor namedColor(Svg::StyleSheetColor colorName, const KSvg::Svg *svg);
    const QString svgStyleSheet(KSvg::Svg *svg);

    /**
     * Check if a pixmap already exists in the cache and compare the last modified
     * timestamp of the file with the last modified date of the one in the cache to make sure
     * the cache is still valid.
     *
     * On Linux systems only, if lastModified is not provided or set to 0, then this function
     * uses the boot time as a reference instead. This is notably the case on ostree based
     * systems such as Fedora Kinoite.
     *
     * TODO: timestamp shouldn't be user-provided
     *
     * @param key the name to use in the cache for this image
     * @param pix the pixmap object to populate with the resulting data if found
     * @param lastModified the timestamp of the file which will be compared with the last
     *                                           modified time of the entry in the cache
     *
     * @note Since KF 5.75, a lastModified value of 0 is deprecated on non-Linux systems. If
     *              used, it will now always return false. Use a proper file timestamp instead
     *              so modification can be properly tracked.
     *
     * @return true when pixmap was found and loaded from cache, false otherwise
     **/
    bool findInCache(const QString &key, QPixmap &pix, unsigned int lastModified);

    /**
     * Insert specified pixmap into the cache.
     * If the cache already contains pixmap with the specified key then it is
     * overwritten.
     *
     * @param key the name to use in the cache for this pixmap
     * @param pix the pixmap data to store in the cache
     **/
    void insertIntoCache(const QString &key, const QPixmap &pix);

    /**
     * Insert specified pixmap into the cache.
     * If the cache already contains pixmap with the specified key then it is
     * overwritten.
     * The actual insert is delayed for optimization reasons and the id
     * parameter is used to discard repeated inserts in the delay time, useful
     * when for instance the graphics to insert comes from a quickly resizing
     * object: the frames between the start and destination sizes aren't
     * useful in the cache and just cause overhead.
     *
     * @param key the name to use in the cache for this pixmap
     * @param pix the pixmap data to store in the cache
     * @param id a name that identifies the caller class of this function in an unique fashion.
     *           This is needed to limit disk writes of the cache.
     *           If an image with the same id changes quickly,
     *           only the last size where insertIntoCache was called is actually stored on disk
     **/
    void insertIntoCache(const QString &key, const QPixmap &pix, const QString &id);

    void colorsChanged();

public Q_SLOTS:
    void scheduledCacheUpdate();
    void onAppExitCleanup();
    void notifyOfChanged();

Q_SIGNALS:
    void imageSetChanged(const QString &imageSetName);
    void applicationPaletteChange();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

    static const char defaultImageSet[];
    static const char themeRcFile[];

    // Ref counting of ImageSetPrivate instances
    static ImageSetPrivate *globalImageSet;
    static QHash<QString, ImageSetPrivate *> themes;

    QString imageSetName = QStringLiteral("default");
    QString basePath;
    KPluginMetaData pluginMetaData;
    QList<QString> fallbackImageSets;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme selectionColorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    KColorScheme complementaryColorScheme;
    KColorScheme headerColorScheme;
    KColorScheme tooltipColorScheme;
    QStringList selectors;
    KConfigGroup cfg;
    KImageCache *pixmapCache;
    QHash<QString, QPixmap> pixmapsToCache;
    QHash<QString, QString> keysToCache;
    QHash<QString, QString> idsToCache;
    QHash<qint64, QString> cachedSvgStyleSheets;
    QHash<qint64, QString> cachedSelectedSvgStyleSheets;
    QHash<qint64, QString> cachedInactiveSvgStyleSheets;
    QHash<QString, QString> discoveries;
    QTimer *pixmapSaveTimer;
    QTimer *updateNotificationTimer;
    unsigned cacheSize;
    CacheTypes cachesToDiscard;
    QString themeVersion;
    QString themeMetadataPath;
    QString iconImageSetMetadataPath;

    #if defined(Q_OS_LINUX)
    // Store boot time to be able to compare it to the lastModifiedTime when the timestamp
    // of files is the UNIX epoch.
    time_t bootTime = 0;
    #endif

    bool isDefault : 1;
    bool useGlobal : 1;
    bool cacheImageSet : 1;
    bool fixedName : 1;

    // Version number of Plasma the ImageSet has been designed for
    int apiMajor;
    int apiMinor;
    int apiRevision;
};

}

#endif

extern const QString s;
