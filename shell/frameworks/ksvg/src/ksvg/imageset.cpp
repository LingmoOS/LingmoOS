/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imageset.h"
#include "private/imageset_p.h"
#include "private/svg_p.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutableListIterator>
#include <QPair>
#include <QStringBuilder>
#include <QThread>
#include <QTimer>

#include <KConfigGroup>
#include <KDirWatch>
#include <KImageCache>

#include "debug_p.h"

namespace KSvg
{
ImageSet::ImageSet(QObject *parent)
    : QObject(parent)
{
    if (!ImageSetPrivate::globalImageSet) {
        ImageSetPrivate::globalImageSet = new ImageSetPrivate;
        if (QCoreApplication::instance()) {
            connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, ImageSetPrivate::globalImageSet, &ImageSetPrivate::onAppExitCleanup);
        }
    }
    ImageSetPrivate::globalImageSet->ref.ref();
    d = ImageSetPrivate::globalImageSet;

    connect(d, &ImageSetPrivate::imageSetChanged, this, &ImageSet::imageSetChanged);
}

ImageSet::ImageSet(const QString &imageSetName, const QString &basePath, QObject *parent)
    : QObject(parent)
{
    auto &priv = ImageSetPrivate::themes[imageSetName];
    if (!priv) {
        priv = new ImageSetPrivate;
        if (QCoreApplication::instance()) {
            connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, priv, &ImageSetPrivate::onAppExitCleanup);
        }
    }

    priv->ref.ref();
    d = priv;

    // turn off caching so we don't accidentally trigger unnecessary disk activity at this point
    bool useCache = d->cacheImageSet;
    d->cacheImageSet = false;
    if (!basePath.isEmpty()) {
        d->basePath = basePath;
        if (!d->basePath.endsWith(QDir::separator())) {
            d->basePath += QDir::separator();
        }
    }
    d->setImageSetName(imageSetName, false);
    d->cacheImageSet = useCache;
    d->fixedName = true;
    connect(d, &ImageSetPrivate::imageSetChanged, this, &ImageSet::imageSetChanged);
}

ImageSet::~ImageSet()
{
    if (d == ImageSetPrivate::globalImageSet) {
        if (!d->ref.deref()) {
            disconnect(ImageSetPrivate::globalImageSet, nullptr, this, nullptr);
            delete ImageSetPrivate::globalImageSet;
            ImageSetPrivate::globalImageSet = nullptr;
            d = nullptr;
        }
    } else {
        if (!d->ref.deref()) {
            delete ImageSetPrivate::themes.take(d->imageSetName);
        }
    }
}

void ImageSet::setBasePath(const QString &basePath)
{
    if (d->basePath == basePath) {
        return;
    }

    d->basePath = basePath;
    if (!d->basePath.endsWith(QDir::separator())) {
        d->basePath += QDir::separator();
    }

    // Don't use scheduleImageSetChangeNotification as we want things happening immediately there,
    // we don't want in the client code to be setting things like the svg size right after thing just to
    // be reset right after in an async fashion
    d->discardCache(PixmapCache | SvgElementsCache);
    d->cachesToDiscard = NoCache;

    Q_EMIT basePathChanged(basePath);
    Q_EMIT imageSetChanged(d->imageSetName);
}

QString ImageSet::basePath() const
{
    return d->basePath;
}

void ImageSet::setSelectors(const QStringList &selectors)
{
    d->selectors = selectors;
    d->scheduleImageSetChangeNotification(PixmapCache | SvgElementsCache);
}

QStringList ImageSet::selectors() const
{
    return d->selectors;
}

void ImageSet::setImageSetName(const QString &imageSetName)
{
    if (d->imageSetName == imageSetName) {
        return;
    }

    if (d != ImageSetPrivate::globalImageSet) {
        disconnect(QCoreApplication::instance(), nullptr, d, nullptr);
        if (!d->ref.deref()) {
            delete ImageSetPrivate::themes.take(d->imageSetName);
        }

        auto &priv = ImageSetPrivate::themes[imageSetName];
        if (!priv) {
            priv = new ImageSetPrivate;
            if (QCoreApplication::instance()) {
                connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, priv, &ImageSetPrivate::onAppExitCleanup);
            }
        }
        priv->ref.ref();
        d = priv;
        connect(d, &ImageSetPrivate::imageSetChanged, this, &ImageSet::imageSetChanged);
    }

    d->setImageSetName(imageSetName, true);
}

QString ImageSet::imageSetName() const
{
    return d->imageSetName;
}

QString ImageSet::imagePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains(QLatin1String("../")) || name.isEmpty()) {
        // we don't support relative paths
        // qCDebug(LOG_KSVG) << "ImageSet says: bad image path " << name;
        return QString();
    }

    const QString svgzName = name % QLatin1String(".svgz");
    QString path = d->findInImageSet(svgzName, d->imageSetName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        const QString svgName = name % QLatin1String(".svg");
        path = d->findInImageSet(svgName, d->imageSetName);

        // search in fallback themes if necessary
        for (int i = 0; path.isEmpty() && i < d->fallbackImageSets.count(); ++i) {
            if (d->imageSetName == d->fallbackImageSets[i]) {
                continue;
            }

            // try a compressed svg file in the fallback theme
            path = d->findInImageSet(svgzName, d->fallbackImageSets[i]);

            if (path.isEmpty()) {
                // try an uncompressed svg file in the fallback theme
                path = d->findInImageSet(svgName, d->fallbackImageSets[i]);
            }
        }
    }

    return path;
}

QString ImageSet::filePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains(QLatin1String("../")) || name.isEmpty()) {
        // we don't support relative paths
        // qCDebug(LOG_KSVG) << "ImageSet says: bad image path " << name;
        return QString();
    }

    QString path = d->findInImageSet(name, d->imageSetName);

    if (path.isEmpty()) {
        // search in fallback themes if necessary
        for (int i = 0; path.isEmpty() && i < d->fallbackImageSets.count(); ++i) {
            if (d->imageSetName == d->fallbackImageSets[i]) {
                continue;
            }

            path = d->findInImageSet(name, d->fallbackImageSets[i]);
        }
    }

    return path;
}

bool ImageSet::currentImageSetHasImage(const QString &name) const
{
    if (name.contains(QLatin1String("../"))) {
        // we don't support relative paths
        return false;
    }

    QString path = d->findInImageSet(name % QLatin1String(".svgz"), d->imageSetName);
    if (path.isEmpty()) {
        path = d->findInImageSet(name % QLatin1String(".svg"), d->imageSetName);
    }
    return path.contains(d->basePath % d->imageSetName);
}

void ImageSet::setUseGlobalSettings(bool useGlobal)
{
    if (d->useGlobal == useGlobal) {
        return;
    }

    d->useGlobal = useGlobal;
    d->cfg = KConfigGroup();
    d->imageSetName.clear();
}

bool ImageSet::useGlobalSettings() const
{
    return d->useGlobal;
}

void ImageSet::setCacheLimit(int kbytes)
{
    d->cacheSize = kbytes;
    delete d->pixmapCache;
    d->pixmapCache = nullptr;
}

KPluginMetaData ImageSet::metadata() const
{
    return d->pluginMetaData;
}

}

#include "moc_imageset.cpp"
