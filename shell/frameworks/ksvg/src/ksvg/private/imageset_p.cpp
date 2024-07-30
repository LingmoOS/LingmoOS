/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imageset_p.h"
#include "debug_p.h"
#include "framesvg.h"
#include "framesvg_p.h"
#include "imageset.h"
#include "svg_p.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QMetaEnum>

#include <KDirWatch>
#include <KSharedConfig>
#include <kpluginmetadata.h>

#if defined(Q_OS_LINUX)
#include <sys/sysinfo.h>
#endif

#define DEFAULT_CACHE_SIZE 16384 // value is from the old kconfigxt default value

namespace KSvg
{
const char ImageSetPrivate::defaultImageSet[] = "default";

ImageSetPrivate *ImageSetPrivate::globalImageSet = nullptr;
QHash<QString, ImageSetPrivate *> ImageSetPrivate::themes = QHash<QString, ImageSetPrivate *>();
using QSP = QStandardPaths;

KSharedConfig::Ptr configForImageSet(const QString &basePath, const QString &theme)
{
    const QString baseName = basePath % theme;
    QString configPath = QSP::locate(QSP::GenericDataLocation, baseName + QLatin1String("/config"));
    if (!configPath.isEmpty()) {
        return KSharedConfig::openConfig(configPath, KConfig::SimpleConfig);
    }
    QString metadataPath = QSP::locate(QSP::GenericDataLocation, baseName + QLatin1String("/metadata.desktop"));
    return KSharedConfig::openConfig(metadataPath, KConfig::SimpleConfig);
}

KPluginMetaData metaDataForImageSet(const QString &basePath, const QString &theme)
{
    QString packageBasePath = basePath % theme;
    QDir dir;
    if (!dir.exists(packageBasePath)) {
        packageBasePath = QSP::locate(QSP::GenericDataLocation, basePath % theme, QSP::LocateDirectory);
    }
    if (packageBasePath.isEmpty()) {
        qWarning(LOG_KSVG) << "Could not locate KSvg image set" << theme << "in" << basePath << "using search path"
                           << QSP::standardLocations(QSP::GenericDataLocation);
        return {};
    }
    if (QFileInfo::exists(packageBasePath + QLatin1String("/metadata.json"))) {
        return KPluginMetaData::fromJsonFile(packageBasePath + QLatin1String("/metadata.json"));
    } else if (QFileInfo::exists(packageBasePath + QLatin1String("/metadata.desktop"))) {
        QString metadataPath = packageBasePath + QLatin1String("/metadata.desktop");
        KConfigGroup cg(KSharedConfig::openConfig(packageBasePath + QLatin1String("/metadata.desktop"), KConfig::SimpleConfig),
                        QStringLiteral("Desktop Entry"));
        QJsonObject obj = {};
        for (const QString &key : cg.keyList()) {
            obj[key] = cg.readEntry(key);
        }
        qWarning(LOG_KSVG) << "The theme" << theme << "uses the legacy metadata.desktop. Consider contacting the author and asking them update it to use the newer JSON format.";
        return KPluginMetaData(obj, packageBasePath + QLatin1String("/metadata.desktop"));
    } else {
        qCWarning(LOG_KSVG) << "Could not locate metadata for theme" << theme;
        return {};
    }
}

ImageSetPrivate::ImageSetPrivate(QObject *parent)
    : QObject(parent)
    , colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(nullptr))
    , selectionColorScheme(QPalette::Active, KColorScheme::Selection, KSharedConfigPtr(nullptr))
    , buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(nullptr))
    , viewColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(nullptr))
    , complementaryColorScheme(QPalette::Active, KColorScheme::Complementary, KSharedConfigPtr(nullptr))
    , headerColorScheme(QPalette::Active, KColorScheme::Header, KSharedConfigPtr(nullptr))
    , tooltipColorScheme(QPalette::Active, KColorScheme::Tooltip, KSharedConfigPtr(nullptr))
    , pixmapCache(nullptr)
    , cacheSize(DEFAULT_CACHE_SIZE)
    , cachesToDiscard(NoCache)
    , isDefault(true)
    , useGlobal(true)
    , cacheImageSet(true)
    , fixedName(false)
    , apiMajor(1)
    , apiMinor(0)
    , apiRevision(0)
{
    const QString org = QCoreApplication::organizationName();
    if (!org.isEmpty()) {
        basePath += u'/' + org;
    }
    const QString appName = QCoreApplication::applicationName();
    if (!appName.isEmpty()) {
        basePath += u'/' + appName;
    }
    if (basePath.isEmpty()) {
        basePath = QStringLiteral("ksvg");
    }
    basePath += u"/svgtheme/";
    pixmapSaveTimer = new QTimer(this);
    pixmapSaveTimer->setSingleShot(true);
    pixmapSaveTimer->setInterval(600);
    QObject::connect(pixmapSaveTimer, &QTimer::timeout, this, &ImageSetPrivate::scheduledCacheUpdate);

    updateNotificationTimer = new QTimer(this);
    updateNotificationTimer->setSingleShot(true);
    updateNotificationTimer->setInterval(100);
    QObject::connect(updateNotificationTimer, &QTimer::timeout, this, &ImageSetPrivate::notifyOfChanged);

    QCoreApplication::instance()->installEventFilter(this);

    #if defined(Q_OS_LINUX)
    struct sysinfo x;
    if (sysinfo(&x) == 0)  {
        bootTime = QDateTime::currentSecsSinceEpoch() - x.uptime;
        qCDebug(LOG_KSVG) << "ImageSetPrivate: Using boot time value" << bootTime;
    } else {
        // Should never happen, but just in case, fallback to a sane value
        bootTime = QDateTime::currentSecsSinceEpoch();
        qCWarning(LOG_KSVG) << "ImageSetPrivate: Failed to get uptime from sysinfo. Using current time as boot time" << bootTime;
    }
    #endif
}

ImageSetPrivate::~ImageSetPrivate()
{
    FrameSvgPrivate::s_sharedFrames.remove(this);
    delete pixmapCache;
}

bool ImageSetPrivate::useCache()
{
    bool cachesTooOld = false;

    if (cacheImageSet && !pixmapCache) {
        if (cacheSize == 0) {
            cacheSize = DEFAULT_CACHE_SIZE;
        }
        QString cacheFile = QLatin1String("plasma_theme_") + imageSetName;

        // clear any cached values from the previous theme cache
        themeVersion.clear();

        if (!themeMetadataPath.isEmpty()) {
            KDirWatch::self()->removeFile(themeMetadataPath);
        }

        themeMetadataPath = configForImageSet(basePath, imageSetName)->name();
        const QString cacheFileBase = cacheFile + QLatin1String("*.kcache");

        QString currentCacheFileName;
        if (!themeMetadataPath.isEmpty()) {
            // now we record the theme version, if we can
            const KPluginMetaData data = metaDataForImageSet(basePath, imageSetName);
            if (data.isValid()) {
                themeVersion = data.version();
            }
            if (!themeVersion.isEmpty()) {
                cacheFile += QLatin1String("_v") + themeVersion;
                currentCacheFileName = cacheFile + QLatin1String(".kcache");
            }
        }

        // now we check for, and remove if necessary, old caches
        QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation));
        cacheDir.setNameFilters(QStringList({cacheFileBase}));

        const auto files = cacheDir.entryInfoList();
        for (const QFileInfo &file : files) {
            if (currentCacheFileName.isEmpty() //
                || !file.absoluteFilePath().endsWith(currentCacheFileName)) {
                QFile::remove(file.absoluteFilePath());
            }
        }

        // now we do a sanity check: if the metadata.desktop file is newer than the cache, drop the cache
        if (!themeMetadataPath.isEmpty()) {
            // now we check to see if the theme metadata file itself is newer than the pixmap cache
            // this is done before creating the pixmapCache object since that can change the mtime
            // on the cache file

            // FIXME: when using the system colors, if they change while the application is not running
            // the cache should be dropped; we need a way to detect system color change when the
            // application is not running.
            // check for expired cache
            const QString cacheFilePath =
                QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1Char('/') + cacheFile + QLatin1String(".kcache");
            if (!cacheFilePath.isEmpty()) {
                const QFileInfo cacheFileInfo(cacheFilePath);
                const QFileInfo metadataFileInfo(themeMetadataPath);
                const QFileInfo iconImageSetMetadataFileInfo(iconImageSetMetadataPath);

                cachesTooOld = (cacheFileInfo.lastModified().toSecsSinceEpoch() < metadataFileInfo.lastModified().toSecsSinceEpoch())
                    || (cacheFileInfo.lastModified().toSecsSinceEpoch() < iconImageSetMetadataFileInfo.lastModified().toSecsSinceEpoch());
            }
        }

        pixmapCache = new KImageCache(cacheFile, cacheSize * 1024);
        pixmapCache->setEvictionPolicy(KSharedDataCache::EvictLeastRecentlyUsed);

        if (cachesTooOld) {
            discardCache(PixmapCache | SvgElementsCache);
        }
    }

    return cacheImageSet;
}

void ImageSetPrivate::onAppExitCleanup()
{
    pixmapsToCache.clear();
    delete pixmapCache;
    pixmapCache = nullptr;
    cacheImageSet = false;
}

QString ImageSetPrivate::imagePath(const QString &theme, const QString &type, const QString &image)
{
    QString subdir = basePath % theme % type % image;

    if (QFileInfo::exists(subdir)) {
        return subdir;
    } else {
        return QStandardPaths::locate(QStandardPaths::GenericDataLocation, subdir);
    }
}

QString ImageSetPrivate::findInImageSet(const QString &image, const QString &theme, bool cache)
{
    if (cache) {
        auto it = discoveries.constFind(image);
        if (it != discoveries.constEnd()) {
            return it.value();
        }
    }

    QString search;

    // TODO: use also QFileSelector::allSelectors?
    // TODO: check if the theme supports selectors starting with +
    for (const QString &type : std::as_const(selectors)) {
        search = imagePath(theme, QLatin1Char('/') % type % QLatin1Char('/'), image);
        if (!search.isEmpty()) {
            break;
        }
    }

    // not found in selectors
    if (search.isEmpty()) {
        search = imagePath(theme, QStringLiteral("/"), image);
    }

    if (cache && !search.isEmpty()) {
        discoveries.insert(image, search);
    }

    return search;
}

void ImageSetPrivate::discardCache(CacheTypes caches)
{
    if (caches & PixmapCache) {
        pixmapsToCache.clear();
        pixmapSaveTimer->stop();
        if (pixmapCache) {
            pixmapCache->clear();
        }
    } else {
        // This deletes the object but keeps the on-disk cache for later use
        delete pixmapCache;
        pixmapCache = nullptr;
    }

    cachedSvgStyleSheets.clear();
    cachedSelectedSvgStyleSheets.clear();
    cachedInactiveSvgStyleSheets.clear();

    if (caches & SvgElementsCache) {
        discoveries.clear();
    }
}

void ImageSetPrivate::scheduledCacheUpdate()
{
    if (useCache()) {
        QHashIterator<QString, QPixmap> it(pixmapsToCache);
        while (it.hasNext()) {
            it.next();
            pixmapCache->insertPixmap(idsToCache[it.key()], it.value());
        }
    }

    pixmapsToCache.clear();
    keysToCache.clear();
    idsToCache.clear();
}

void ImageSetPrivate::colorsChanged()
{
    // in the case the theme follows the desktop settings, refetch the colorschemes
    // and discard the svg pixmap cache
    if (!colors) {
        KSharedConfig::openConfig()->reparseConfiguration();
    }
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    selectionColorScheme = KColorScheme(QPalette::Active, KColorScheme::Selection, colors);
    complementaryColorScheme = KColorScheme(QPalette::Active, KColorScheme::Complementary, colors);
    headerColorScheme = KColorScheme(QPalette::Active, KColorScheme::Header, colors);
    tooltipColorScheme = KColorScheme(QPalette::Active, KColorScheme::Tooltip, colors);
    scheduleImageSetChangeNotification(PixmapCache | SvgElementsCache);
    Q_EMIT applicationPaletteChange();
}

void ImageSetPrivate::scheduleImageSetChangeNotification(CacheTypes caches)
{
    cachesToDiscard |= caches;
    updateNotificationTimer->start();
}

void ImageSetPrivate::notifyOfChanged()
{
    // qCDebug(LOG_KSVG) << cachesToDiscard;
    discardCache(cachesToDiscard);
    cachesToDiscard = NoCache;
    Q_EMIT imageSetChanged(imageSetName);
}

QColor ImageSetPrivate::namedColor(Svg::StyleSheetColor colorName, const KSvg::Svg *svg)
{
    const KSvg::Svg::Status status = svg->status();
    KColorScheme *currentScheme = nullptr;

    switch (KColorScheme::ColorSet(svg->colorSet())) {
    case KColorScheme::Button:
        currentScheme = &buttonColorScheme;
        break;
    case KColorScheme::View:
        currentScheme = &viewColorScheme;
        break;
    case KColorScheme::Complementary:
        currentScheme = &complementaryColorScheme;
        break;
    case KColorScheme::Header:
        currentScheme = &headerColorScheme;
        break;
    case KColorScheme::Tooltip:
        currentScheme = &tooltipColorScheme;
        break;
    default:
        currentScheme = &colorScheme;
    }

    switch (colorName) {
    case Svg::Text:
        switch (status) {
        case Svg::Status::Selected:
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        case Svg::Status::Inactive:
            return currentScheme->foreground(KColorScheme::InactiveText).color();
        default:
            return currentScheme->foreground(KColorScheme::NormalText).color();
        }
    case Svg::Background:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return colorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::Highlight:
        return selectionColorScheme.background(KColorScheme::NormalBackground).color();
    case Svg::HighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::PositiveText:
        return currentScheme->foreground(KColorScheme::PositiveText).color();
    case Svg::NeutralText:
        return currentScheme->foreground(KColorScheme::NeutralText).color();
    case Svg::NegativeText:
        return currentScheme->foreground(KColorScheme::NegativeText).color();

    case Svg::ButtonText:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        } else {
            return buttonColorScheme.foreground(KColorScheme::NormalText).color();
        }
    case Svg::ButtonBackground:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return buttonColorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::ButtonHover:
        return buttonColorScheme.decoration(KColorScheme::HoverColor).color();
    case Svg::ButtonFocus:
        return buttonColorScheme.decoration(KColorScheme::FocusColor).color();
    case Svg::ButtonHighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::ButtonPositiveText:
        return buttonColorScheme.foreground(KColorScheme::PositiveText).color();
    case Svg::ButtonNeutralText:
        return buttonColorScheme.foreground(KColorScheme::NeutralText).color();
    case Svg::ButtonNegativeText:
        return buttonColorScheme.foreground(KColorScheme::NegativeText).color();

    case Svg::ViewText:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        } else {
            return viewColorScheme.foreground(KColorScheme::NormalText).color();
        }
    case Svg::ViewBackground:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return viewColorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::ViewHover:
        return viewColorScheme.decoration(KColorScheme::HoverColor).color();
    case Svg::ViewFocus:
        return viewColorScheme.decoration(KColorScheme::FocusColor).color();
    case Svg::ViewHighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::ViewPositiveText:
        return viewColorScheme.foreground(KColorScheme::PositiveText).color();
    case Svg::ViewNeutralText:
        return viewColorScheme.foreground(KColorScheme::NeutralText).color();
    case Svg::ViewNegativeText:
        return viewColorScheme.foreground(KColorScheme::NegativeText).color();

    case Svg::TooltipText:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        } else {
            return tooltipColorScheme.foreground(KColorScheme::NormalText).color();
        }
    case Svg::TooltipBackground:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return tooltipColorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::TooltipHover:
        return tooltipColorScheme.decoration(KColorScheme::HoverColor).color();
    case Svg::TooltipFocus:
        return tooltipColorScheme.decoration(KColorScheme::FocusColor).color();
    case Svg::TooltipHighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::TooltipPositiveText:
        return tooltipColorScheme.foreground(KColorScheme::PositiveText).color();
    case Svg::TooltipNeutralText:
        return tooltipColorScheme.foreground(KColorScheme::NeutralText).color();
    case Svg::TooltipNegativeText:
        return tooltipColorScheme.foreground(KColorScheme::NegativeText).color();

    case Svg::ComplementaryText:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        } else {
            return complementaryColorScheme.foreground(KColorScheme::NormalText).color();
        }
    case Svg::ComplementaryBackground:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return complementaryColorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::ComplementaryHover:
        return complementaryColorScheme.decoration(KColorScheme::HoverColor).color();
    case Svg::ComplementaryFocus:
        return complementaryColorScheme.decoration(KColorScheme::FocusColor).color();
    case Svg::ComplementaryHighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::ComplementaryPositiveText:
        return complementaryColorScheme.foreground(KColorScheme::PositiveText).color();
    case Svg::ComplementaryNeutralText:
        return complementaryColorScheme.foreground(KColorScheme::NeutralText).color();
    case Svg::ComplementaryNegativeText:
        return complementaryColorScheme.foreground(KColorScheme::NegativeText).color();

    case Svg::HeaderText:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.foreground(KColorScheme::NormalText).color();
        } else {
            return headerColorScheme.foreground(KColorScheme::NormalText).color();
        }
    case Svg::HeaderBackground:
        if (status == Svg::Status::Selected) {
            return selectionColorScheme.background(KColorScheme::NormalBackground).color();
        } else {
            return headerColorScheme.background(KColorScheme::NormalBackground).color();
        }
    case Svg::HeaderHover:
        return headerColorScheme.decoration(KColorScheme::HoverColor).color();
    case Svg::HeaderFocus:
        return headerColorScheme.decoration(KColorScheme::FocusColor).color();
    case Svg::HeaderHighlightedText:
        return selectionColorScheme.foreground(KColorScheme::NormalText).color();
    case Svg::HeaderPositiveText:
        return headerColorScheme.foreground(KColorScheme::PositiveText).color();
    case Svg::HeaderNeutralText:
        return headerColorScheme.foreground(KColorScheme::NeutralText).color();
    case Svg::HeaderNegativeText:
        return headerColorScheme.foreground(KColorScheme::NegativeText).color();
    default:
        return {};
    }
}

const QString ImageSetPrivate::svgStyleSheet(KSvg::Svg *svg)
{
    const KSvg::Svg::Status status = svg->status();
    const KColorScheme::ColorSet colorSet = KColorScheme::ColorSet(svg->colorSet());
    const bool useCache = svg->d->colorOverrides.isEmpty();
    QString stylesheet;
    if (useCache) {
        stylesheet = (status == Svg::Status::Selected)
            ? cachedSelectedSvgStyleSheets.value(colorSet)
            : (status == Svg::Status::Inactive ? cachedInactiveSvgStyleSheets.value(colorSet) : cachedSvgStyleSheets.value(colorSet));
    }

    if (stylesheet.isEmpty()) {
        const QList<Svg::StyleSheetColor> namedColors({Svg::Text,
                                                       Svg::Background,
                                                       Svg::Highlight,
                                                       Svg::HighlightedText,
                                                       Svg::PositiveText,
                                                       Svg::NeutralText,
                                                       Svg::NegativeText,

                                                       Svg::ButtonText,
                                                       Svg::ButtonBackground,
                                                       Svg::ButtonHover,
                                                       Svg::ButtonFocus,
                                                       Svg::ButtonHighlightedText,
                                                       Svg::ButtonPositiveText,
                                                       Svg::ButtonNeutralText,
                                                       Svg::ButtonNegativeText,

                                                       Svg::ViewText,
                                                       Svg::ViewBackground,
                                                       Svg::ViewHover,
                                                       Svg::ViewFocus,
                                                       Svg::ViewHighlightedText,
                                                       Svg::ViewPositiveText,
                                                       Svg::ViewNeutralText,
                                                       Svg::ViewNegativeText,

                                                       Svg::TooltipText,
                                                       Svg::TooltipBackground,
                                                       Svg::TooltipHover,
                                                       Svg::TooltipFocus,
                                                       Svg::TooltipHighlightedText,
                                                       Svg::TooltipPositiveText,
                                                       Svg::TooltipNeutralText,
                                                       Svg::TooltipNegativeText,

                                                       Svg::ComplementaryText,
                                                       Svg::ComplementaryBackground,
                                                       Svg::ComplementaryHover,
                                                       Svg::ComplementaryFocus,
                                                       Svg::ComplementaryHighlightedText,
                                                       Svg::ComplementaryPositiveText,
                                                       Svg::ComplementaryNeutralText,
                                                       Svg::ComplementaryNegativeText,

                                                       Svg::HeaderText,
                                                       Svg::HeaderBackground,
                                                       Svg::HeaderHover,
                                                       Svg::HeaderFocus,
                                                       Svg::HeaderHighlightedText,
                                                       Svg::HeaderPositiveText,
                                                       Svg::HeaderNeutralText,
                                                       Svg::HeaderNegativeText});
        const QString skel = QStringLiteral(".ColorScheme-%1{color:%2;}");
        const QMetaEnum metaEnum = QMetaEnum::fromType<Svg::StyleSheetColor>();

        for (const auto colorName : std::as_const(namedColors)) {
            stylesheet += skel.arg(QString::fromUtf8(metaEnum.valueToKey(colorName)), svg->color(colorName).name());
        }

        if (status == Svg::Status::Selected) {
            cachedSelectedSvgStyleSheets.insert(colorSet, stylesheet);
        } else if (status == Svg::Status::Inactive) {
            cachedInactiveSvgStyleSheets.insert(colorSet, stylesheet);
        } else {
            cachedSvgStyleSheets.insert(colorSet, stylesheet);
        }
    }

    return stylesheet;
}

bool ImageSetPrivate::findInCache(const QString &key, QPixmap &pix, unsigned int lastModified)
{
    if (!useCache()) {
        return false;
    }

    qint64 cacheLastModifiedTime = uint(pixmapCache->lastModifiedTime().toSecsSinceEpoch());
    if (lastModified > cacheLastModifiedTime) {
        qCDebug(LOG_KSVG) << "ImageSetPrivate::findInCache: lastModified > cacheLastModifiedTime for" << key;
        return false;
    }
    #if defined(Q_OS_LINUX)
    // If the timestamp is the UNIX epoch (0)  then we compare against the boot time instead.
    // This is notably the case on ostree based systems such as Fedora Kinoite.
    if (lastModified == 0 && bootTime > cacheLastModifiedTime) {
        qCDebug(LOG_KSVG) << "ImageSetPrivate::findInCache: lastModified == 0 && bootTime > cacheLastModifiedTime for" << key;
        return false;
    }
    #else
    if (lastModified == 0) {
        qCWarning(LOG_KSVG) << "findInCache with a lastModified timestamp of 0 is deprecated";
        return false;
    }
    #endif

    qCDebug(LOG_KSVG) << "ImageSetPrivate::findInCache: using cache for" << key;
    const QString id = keysToCache.value(key);
    const auto it = pixmapsToCache.constFind(id);
    if (it != pixmapsToCache.constEnd()) {
        pix = *it;
        return !pix.isNull();
    }

    QPixmap temp;
    if (pixmapCache->findPixmap(key, &temp) && !temp.isNull()) {
        pix = temp;
        return true;
    }

    return false;
}

void ImageSetPrivate::insertIntoCache(const QString &key, const QPixmap &pix)
{
    if (useCache()) {
        pixmapCache->insertPixmap(key, pix);
    }
}

void ImageSetPrivate::insertIntoCache(const QString &key, const QPixmap &pix, const QString &id)
{
    if (useCache()) {
        // Remove old key -> id mapping first
        if (auto key = idsToCache.find(id); key != idsToCache.end()) {
            keysToCache.remove(*key);
        }
        pixmapsToCache[id] = pix;
        keysToCache[key] = id;
        idsToCache[id] = key;

        // always start timer in pixmapSaveTimer's thread
        QMetaObject::invokeMethod(pixmapSaveTimer, "start", Qt::QueuedConnection);
    }
}

void ImageSetPrivate::setImageSetName(const QString &tempImageSetName, bool emitChanged)
{
    QString theme = tempImageSetName;
    if (theme.isEmpty() || theme == imageSetName) {
        // let's try and get the default theme at least
        if (imageSetName.isEmpty()) {
            theme = QLatin1String(ImageSetPrivate::defaultImageSet);
        } else {
            return;
        }
    }

    KPluginMetaData data = metaDataForImageSet(basePath, theme);
    if (!data.isValid()) {
        data = metaDataForImageSet(basePath, QStringLiteral("default"));
        if (!data.isValid()) {
            return;
        }

        theme = QLatin1String(ImageSetPrivate::defaultImageSet);
    }

    // check again as ImageSetPrivate::defaultImageSet might be empty
    if (imageSetName == theme) {
        return;
    }

    imageSetName = theme;

    // load the color scheme config
    const QString colorsFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, basePath % theme % QLatin1String("/colors"));
    // qCDebug(LOG_KSVG) << "we're going for..." << colorsFile << "*******************";

    if (colorsFile.isEmpty()) {
        colors = nullptr;
    } else {
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    selectionColorScheme = KColorScheme(QPalette::Active, KColorScheme::Selection, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    complementaryColorScheme = KColorScheme(QPalette::Active, KColorScheme::Complementary, colors);
    headerColorScheme = KColorScheme(QPalette::Active, KColorScheme::Header, colors);
    tooltipColorScheme = KColorScheme(QPalette::Active, KColorScheme::Tooltip, colors);

    pluginMetaData = metaDataForImageSet(basePath, theme);
    KSharedConfigPtr metadata = configForImageSet(basePath, theme);

    KConfigGroup cg(metadata, QStringLiteral("Settings"));
    QString fallback = cg.readEntry("FallbackImageSet", QString());

    fallbackImageSets.clear();
    while (!fallback.isEmpty() && !fallbackImageSets.contains(fallback)) {
        fallbackImageSets.append(fallback);

        KSharedConfigPtr metadata = configForImageSet(basePath, fallback);
        KConfigGroup cg(metadata, QStringLiteral("Settings"));
        fallback = cg.readEntry("FallbackImageSet", QString());
    }

    if (!fallbackImageSets.contains(QLatin1String(ImageSetPrivate::defaultImageSet))) {
        fallbackImageSets.append(QLatin1String(ImageSetPrivate::defaultImageSet));
    }

    // Check for what Plasma version the theme has been done
    // There are some behavioral differences between KDE4 Plasma and Plasma 5
    const QString apiVersion = pluginMetaData.value(QStringLiteral("X-Plasma-API"));
    apiMajor = 1;
    apiMinor = 0;
    apiRevision = 0;
    if (!apiVersion.isEmpty()) {
        const QList<QStringView> parts = QStringView(apiVersion).split(QLatin1Char('.'));
        if (!parts.isEmpty()) {
            apiMajor = parts.value(0).toInt();
        }
        if (parts.count() > 1) {
            apiMinor = parts.value(1).toInt();
        }
        if (parts.count() > 2) {
            apiRevision = parts.value(2).toInt();
        }
    }

    if (emitChanged) {
        scheduleImageSetChangeNotification(PixmapCache | SvgElementsCache);
    }
}

bool ImageSetPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationPaletteChange) {
            colorsChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}
}

#include "moc_imageset_p.cpp"
