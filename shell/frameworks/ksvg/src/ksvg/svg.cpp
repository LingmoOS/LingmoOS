/*
    SPDX-FileCopyrightText: 2006-2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "svg.h"
#include "framesvg.h"
#include "private/imageset_p.h"
#include "private/svg_p.h"

#include <array>
#include <cmath>

#include <QBuffer>
#include <QCoreApplication>
#include <QDir>
#include <QPainter>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <KCompressionDevice>
#include <KConfigGroup>
#include <QDebug>

#include "debug_p.h"
#include "imageset.h"

size_t qHash(const KSvg::SvgPrivate::CacheId &id, size_t seed)
{
    std::array<size_t, 10> parts = {
        ::qHash(id.width),
        ::qHash(id.height),
        ::qHash(id.elementName),
        ::qHash(id.filePath),
        ::qHash(id.status),
        ::qHash(id.scaleFactor),
        ::qHash(id.colorSet),
        ::qHash(id.styleSheet),
        ::qHash(id.extraFlags),
        ::qHash(id.lastModified),
    };
    return qHashRange(parts.begin(), parts.end(), seed);
}

size_t qHash(const QList<QColor> &colors, size_t seed)
{
    std::vector<size_t> parts;
    for (const QColor &c : std::as_const(colors)) {
        parts.push_back(::qHash(c.red()));
        parts.push_back(::qHash(c.green()));
        parts.push_back(::qHash(c.blue()));
        parts.push_back(::qHash(c.alpha()));
    }
    return qHashRange(parts.begin(), parts.end(), seed);
}

namespace KSvg
{
class SvgRectsCacheSingleton
{
public:
    SvgRectsCache self;
};

Q_GLOBAL_STATIC(SvgRectsCacheSingleton, privateSvgRectsCacheSelf)

const size_t SvgRectsCache::s_seed = 0x9e3779b9;

SharedSvgRenderer::SharedSvgRenderer(QObject *parent)
    : QSvgRenderer(parent)
{
}

SharedSvgRenderer::SharedSvgRenderer(const QString &filename, const QString &styleSheet, QHash<QString, QRectF> &interestingElements, QObject *parent)
    : QSvgRenderer(parent)
{
    KCompressionDevice file(filename, KCompressionDevice::GZip);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    m_filename = filename;
    m_styleSheet = styleSheet;
    m_interestingElements = interestingElements;
    load(file.readAll(), styleSheet, interestingElements);
}

SharedSvgRenderer::SharedSvgRenderer(const QByteArray &contents, const QString &styleSheet, QHash<QString, QRectF> &interestingElements, QObject *parent)
    : QSvgRenderer(parent)
{
    load(contents, styleSheet, interestingElements);
}

void SharedSvgRenderer::reload()
{
    KCompressionDevice file(m_filename, KCompressionDevice::GZip);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    load(file.readAll(), m_styleSheet, m_interestingElements);
}

bool SharedSvgRenderer::load(const QByteArray &contents, const QString &styleSheet, QHash<QString, QRectF> &interestingElements)
{
    // Apply the style sheet.
    if (!styleSheet.isEmpty() && contents.contains("current-color-scheme")) {
        QByteArray processedContents;
        processedContents.reserve(contents.size());
        QXmlStreamReader reader(contents);

        QBuffer buffer(&processedContents);
        buffer.open(QIODevice::WriteOnly);
        QXmlStreamWriter writer(&buffer);
        while (!reader.atEnd()) {
            if (reader.readNext() == QXmlStreamReader::StartElement && reader.qualifiedName() == QLatin1String("style")
                && reader.attributes().value(QLatin1String("id")) == QLatin1String("current-color-scheme")) {
                writer.writeStartElement(QLatin1String("style"));
                writer.writeAttributes(reader.attributes());
                writer.writeCharacters(styleSheet);
                writer.writeEndElement();
                while (reader.tokenType() != QXmlStreamReader::EndElement) {
                    reader.readNext();
                }
            } else if (reader.tokenType() != QXmlStreamReader::Invalid) {
                writer.writeCurrentToken(reader);
            }
        }
        buffer.close();
        if (!QSvgRenderer::load(processedContents)) {
            return false;
        }
    } else if (!QSvgRenderer::load(contents)) {
        return false;
    }

    // Search the SVG to find and store all ids that contain size hints.
    const QString contentsAsString(QString::fromLatin1(contents));
    static const QRegularExpression idExpr(QLatin1String("id\\s*?=\\s*?(['\"])(\\d+?-\\d+?-.*?)\\1"));
    Q_ASSERT(idExpr.isValid());

    auto matchIt = idExpr.globalMatch(contentsAsString);
    while (matchIt.hasNext()) {
        auto match = matchIt.next();
        QString elementId = match.captured(2);

        QRectF elementRect = boundsOnElement(elementId);
        if (elementRect.isValid()) {
            interestingElements.insert(elementId, elementRect);
        }
    }

    return true;
}

SvgRectsCache::SvgRectsCache(QObject *parent)
    : QObject(parent)
{
    const QString svgElementsFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1Char('/') + QStringLiteral("ksvg-elements");
    m_svgElementsCache = KSharedConfig::openConfig(svgElementsFile, KConfig::SimpleConfig);

    m_configSyncTimer = new QTimer(this);
    m_configSyncTimer->setSingleShot(true);
    m_configSyncTimer->setInterval(5000);
    connect(m_configSyncTimer, &QTimer::timeout, this, [this]() {
        m_svgElementsCache->sync();
    });
}

SvgRectsCache *SvgRectsCache::instance()
{
    return &privateSvgRectsCacheSelf()->self;
}

void SvgRectsCache::insert(KSvg::SvgPrivate::CacheId cacheId, const QRectF &rect, unsigned int lastModified)
{
    insert(qHash(cacheId, SvgRectsCache::s_seed), cacheId.filePath, rect, lastModified);
}

void SvgRectsCache::insert(size_t id, const QString &filePath, const QRectF &rect, unsigned int lastModified)
{
    const unsigned int savedTime = lastModifiedTimeFromCache(filePath);

    if (savedTime == lastModified && m_localRectCache.contains(id)) {
        return;
    }

    m_localRectCache.insert(id, rect);

    KConfigGroup imageGroup(m_svgElementsCache, filePath);

    if (rect.isValid()) {
        imageGroup.writeEntry(QString::number(id), rect);
    } else {
        m_invalidElements[filePath] << id;
        imageGroup.writeEntry("Invalidelements", m_invalidElements[filePath].values());
    }

    QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));

    if (savedTime != lastModified) {
        m_lastModifiedTimes[filePath] = lastModified;
        imageGroup.writeEntry("LastModified", lastModified);
        Q_EMIT lastModifiedChanged(filePath, lastModified);
    }
}

bool SvgRectsCache::findElementRect(KSvg::SvgPrivate::CacheId cacheId, QRectF &rect)
{
    return findElementRect(qHash(cacheId, SvgRectsCache::s_seed), cacheId.filePath, rect);
}

bool SvgRectsCache::findElementRect(size_t id, QStringView filePath, QRectF &rect)
{
    auto it = m_localRectCache.find(id);

    if (it == m_localRectCache.end()) {
        auto elements = m_invalidElements.value(filePath.toString());
        if (elements.contains(id)) {
            rect = QRectF();
            return true;
        }
        return false;
    }

    rect = *it;

    return true;
}

bool SvgRectsCache::loadImageFromCache(const QString &path, uint lastModified)
{
    if (path.isEmpty()) {
        return false;
    }

    KConfigGroup imageGroup(m_svgElementsCache, path);

    unsigned int savedTime = lastModifiedTimeFromCache(path);

    // Reload even if is older, to support downgrades
    if (lastModified != savedTime) {
        imageGroup.deleteGroup();
        QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
        return false;
    }

    auto &elements = m_invalidElements[path];
    if (elements.isEmpty()) {
        auto list = imageGroup.readEntry("Invalidelements", QList<unsigned int>());
        m_invalidElements[path] = QSet<unsigned int>(list.begin(), list.end());

        for (const auto &key : imageGroup.keyList()) {
            bool ok = false;
            uint keyUInt = key.toUInt(&ok);
            if (ok) {
                const QRectF rect = imageGroup.readEntry(key, QRectF());
                m_localRectCache.insert(keyUInt, rect);
            }
        }
    }
    return true;
}

void SvgRectsCache::dropImageFromCache(const QString &path)
{
    KConfigGroup imageGroup(m_svgElementsCache, path);
    imageGroup.deleteGroup();
    QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
}

QList<QSizeF> SvgRectsCache::sizeHintsForId(const QString &path, const QString &id)
{
    const QString pathId = path % id;

    auto it = m_sizeHintsForId.constFind(pathId);
    if (it == m_sizeHintsForId.constEnd()) {
        KConfigGroup imageGroup(m_svgElementsCache, path);
        const QStringList &encoded = imageGroup.readEntry(id, QStringList());
        QList<QSizeF> sizes;
        for (const auto &token : encoded) {
            const auto &parts = token.split(QLatin1Char('x'));
            if (parts.size() != 2) {
                continue;
            }
            QSize size = QSize(parts[0].toDouble(), parts[1].toDouble());
            if (!size.isEmpty()) {
                sizes << size;
            }
        }
        m_sizeHintsForId[pathId] = sizes;
        return sizes;
    }

    return *it;
}

void SvgRectsCache::insertSizeHintForId(const QString &path, const QString &id, const QSizeF &size)
{
    // TODO: need to make this more efficient
    auto sizeListToString = [](const QList<QSizeF> &list) {
        QString ret;
        for (const auto &s : list) {
            ret += QString::number(s.width()) % QLatin1Char('x') % QString::number(s.height()) % QLatin1Char(',');
        }
        return ret;
    };
    m_sizeHintsForId[path % id].append(size);
    KConfigGroup imageGroup(m_svgElementsCache, path);
    imageGroup.writeEntry(id, sizeListToString(m_sizeHintsForId[path % id]));
    QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
}

QString SvgRectsCache::iconThemePath()
{
    if (!m_iconThemePath.isEmpty()) {
        return m_iconThemePath;
    }

    KConfigGroup imageGroup(m_svgElementsCache, QStringLiteral("General"));
    m_iconThemePath = imageGroup.readEntry(QStringLiteral("IconThemePath"), QString());

    return m_iconThemePath;
}

void SvgRectsCache::setIconThemePath(const QString &path)
{
    m_iconThemePath = path;
    KConfigGroup imageGroup(m_svgElementsCache, QStringLiteral("General"));
    imageGroup.writeEntry(QStringLiteral("IconThemePath"), path);
    QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
}

void SvgRectsCache::setNaturalSize(const QString &path, const QSizeF &size)
{
    KConfigGroup imageGroup(m_svgElementsCache, path);

    // FIXME: needs something faster, perhaps even sprintf
    imageGroup.writeEntry(QStringLiteral("NaturalSize"), size);
    QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
}

QSizeF SvgRectsCache::naturalSize(const QString &path)
{
    KConfigGroup imageGroup(m_svgElementsCache, path);

    // FIXME: needs something faster, perhaps even sprintf
    return imageGroup.readEntry(QStringLiteral("NaturalSize"), QSizeF());
}

QStringList SvgRectsCache::cachedKeysForPath(const QString &path) const
{
    KConfigGroup imageGroup(m_svgElementsCache, path);
    QStringList list = imageGroup.keyList();
    QStringList filtered;

    std::copy_if(list.begin(), list.end(), std::back_inserter(filtered), [](const QString element) {
        bool ok;
        element.toLong(&ok);
        return ok;
    });
    return filtered;
}

unsigned int SvgRectsCache::lastModifiedTimeFromCache(const QString &filePath)
{
    const auto &i = m_lastModifiedTimes.constFind(filePath);
    if (i != m_lastModifiedTimes.constEnd()) {
        return i.value();
    }

    KConfigGroup imageGroup(m_svgElementsCache, filePath);
    const unsigned int savedTime = imageGroup.readEntry("LastModified", 0);
    m_lastModifiedTimes[filePath] = savedTime;
    return savedTime;
}

void SvgRectsCache::updateLastModified(const QString &filePath, unsigned int lastModified)
{
    KConfigGroup imageGroup(m_svgElementsCache, filePath);
    const unsigned int savedTime = lastModifiedTimeFromCache(filePath);

    if (savedTime != lastModified) {
        m_lastModifiedTimes[filePath] = lastModified;
        imageGroup.writeEntry("LastModified", lastModified);
        QMetaObject::invokeMethod(m_configSyncTimer, qOverload<>(&QTimer::start));
        Q_EMIT lastModifiedChanged(filePath, lastModified);
    }
}

SvgPrivate::SvgPrivate(Svg *svg)
    : q(svg)
    , renderer(nullptr)
    , styleCrc(0)
    , lastModified(0)
    , devicePixelRatio(1.0)
    , status(Svg::Status::Normal)
    , multipleImages(false)
    , themed(false)
    , fromCurrentImageSet(false)
    , cacheRendering(true)
    , themeFailed(false)
{
}

SvgPrivate::~SvgPrivate()
{
    eraseRenderer();
}

size_t SvgPrivate::paletteId(const QPalette &palette, const QColor &positive, const QColor &neutral, const QColor &negative) const
{
    std::array<size_t, 4> parts = {
        ::qHash(palette.cacheKey()),
        ::qHash(positive.rgba()),
        ::qHash(neutral.rgba()),
        ::qHash(negative.rgba()),
    };
    return qHashRange(parts.begin(), parts.end(), SvgRectsCache::s_seed);
}

// This function is meant for the rects cache
SvgPrivate::CacheId SvgPrivate::cacheId(QStringView elementId) const
{
    auto idSize = size.isValid() && size != naturalSize ? size : QSizeF{-1.0, -1.0};
    return CacheId{idSize.width(), idSize.height(), path, elementId.toString(), status, devicePixelRatio, -1, 0, 0, lastModified};
}

// This function is meant for the pixmap cache
QString SvgPrivate::cachePath(const QString &id, const QSize &size) const
{
    std::vector<size_t> parts;
    const auto colors = colorOverrides.values();
    for (const QColor &c : std::as_const(colors)) {
        parts.push_back(::qHash(c.red()));
        parts.push_back(::qHash(c.green()));
        parts.push_back(::qHash(c.blue()));
        parts.push_back(::qHash(c.alpha()));
    }
    const size_t colorsHash = qHashRange(parts.begin(), parts.end(), SvgRectsCache::s_seed);

    auto cacheId = CacheId{double(size.width()), double(size.height()), path, id, status, devicePixelRatio, colorSet, colorsHash, 0, lastModified};
    return QString::number(qHash(cacheId, SvgRectsCache::s_seed));
}

bool SvgPrivate::setImagePath(const QString &imagePath)
{
    QString actualPath = imagePath;
    bool isAbsoluteFile = QDir::isAbsolutePath(actualPath);
    if (imagePath.startsWith(QLatin1String("file://"))) {
        // length of file://
        actualPath.remove(0, 7);
        isAbsoluteFile = true;
    }
    // If someone using the QML API uses Qt.resolvedUrl to get the absolute path inside of a QRC,
    // the URI will look something like qrc:/artwork/file.svg
    // In order for file IO to work it needs to be reformatted it needs to be :/artwork/file.svg
    if (imagePath.startsWith(QLatin1String("qrc:/"))) {
        actualPath.replace(QLatin1String("qrc:/"), QLatin1String(":/"));
        isAbsoluteFile = true;
    }

    bool isThemed = !actualPath.isEmpty() && !isAbsoluteFile;

    // lets check to see if we're already set to this file
    if (isThemed == themed && ((themed && themePath == actualPath) || (!themed && path == actualPath))) {
        return false;
    }

    eraseRenderer();

    // if we don't have any path right now and are going to set one,
    // then lets not schedule a repaint because we are just initializing!
    bool updateNeeded = true; //! path.isEmpty() || !themePath.isEmpty();

    QObject::disconnect(imageSetChangedConnection);

    themed = isThemed;
    path.clear();
    themePath.clear();

    bool oldfromCurrentImageSet = fromCurrentImageSet;
    fromCurrentImageSet = isThemed && actualImageSet()->currentImageSetHasImage(imagePath);

    if (fromCurrentImageSet != oldfromCurrentImageSet) {
        Q_EMIT q->fromCurrentImageSetChanged(fromCurrentImageSet);
    }

    if (themed) {
        themePath = actualPath;
        path = actualImageSet()->imagePath(themePath);
        themeFailed = path.isEmpty();
        imageSetChangedConnection = QObject::connect(actualImageSet(), &ImageSet::imageSetChanged, q, [this]() {
            imageSetChanged();
        });
    } else if (QFileInfo::exists(actualPath)) {
        imageSetChangedConnection = QObject::connect(actualImageSet(), &ImageSet::imageSetChanged, q, [this]() {
            imageSetChanged();
        });
        path = actualPath;
    } else {
#ifndef NDEBUG
        // qCDebug(LOG_KSVG) << "file '" << path << "' does not exist!";
#endif
    }

    QDateTime lastModifiedDate;
    if (!path.isEmpty()) {
        const QFileInfo info(path);
        lastModifiedDate = info.lastModified();

        lastModified = lastModifiedDate.toSecsSinceEpoch();

        const bool imageWasCached = SvgRectsCache::instance()->loadImageFromCache(path, lastModified);

        if (!imageWasCached) {
            auto i = s_renderers.constBegin();
            while (i != s_renderers.constEnd()) {
                if (i.key().contains(path)) {
                    i.value()->reload();
                }
                i++;
            }
        }
    }

    // also images with absolute path needs to have a natural size initialized,
    // even if looks a bit weird using ImageSet to store non-themed stuff
    if ((themed && !path.isEmpty() && lastModifiedDate.isValid()) || QFileInfo::exists(actualPath)) {
        naturalSize = SvgRectsCache::instance()->naturalSize(path);
        if (naturalSize.isEmpty()) {
            createRenderer();
            naturalSize = renderer->defaultSize();
            SvgRectsCache::instance()->setNaturalSize(path, naturalSize);
        }
    }

    q->resize();
    Q_EMIT q->imagePathChanged();

    return updateNeeded;
}

ImageSet *SvgPrivate::actualImageSet()
{
    if (!theme) {
        theme = new KSvg::ImageSet(q);
    }

    return theme.data();
}

QPixmap SvgPrivate::findInCache(const QString &elementId, qreal ratio, const QSizeF &s)
{
    QSize size;
    QString actualElementId;

    // Look at the size hinted elements and try to find the smallest one with an
    // identical aspect ratio.
    if (s.isValid() && !elementId.isEmpty()) {
        const QList<QSizeF> elementSizeHints = SvgRectsCache::instance()->sizeHintsForId(path, elementId);

        if (!elementSizeHints.isEmpty()) {
            QSizeF bestFit(-1, -1);

            for (const auto &hint : elementSizeHints) {
                if (hint.width() >= s.width() * ratio && hint.height() >= s.height() * ratio
                    && (!bestFit.isValid() || (bestFit.width() * bestFit.height()) > (hint.width() * hint.height()))) {
                    bestFit = hint;
                }
            }

            if (bestFit.isValid()) {
                actualElementId = QString::number(bestFit.width()) % QLatin1Char('-') % QString::number(bestFit.height()) % QLatin1Char('-') % elementId;
            }
        }
    }

    if (elementId.isEmpty() || !q->hasElement(actualElementId)) {
        actualElementId = elementId;
    }

    if (elementId.isEmpty() || (multipleImages && s.isValid())) {
        size = s.toSize() * ratio;
    } else {
        size = elementRect(actualElementId).size().toSize() * ratio;
    }

    if (size.isEmpty()) {
        return QPixmap();
    }

    const QString id = cachePath(actualElementId, size);

    QPixmap p;
    if (cacheRendering && lastModified == SvgRectsCache::instance()->lastModifiedTimeFromCache(path) && actualImageSet()->d->findInCache(id, p, lastModified)) {
        p.setDevicePixelRatio(ratio);
        // qCDebug(LOG_PLASMA) << "found cached version of " << id << p.size();
        return p;
    }

    createRenderer();

    QRectF finalRect = makeUniform(renderer->boundsOnElement(actualElementId), QRect(QPoint(0, 0), size));

    // don't alter the pixmap size or it won't match up properly to, e.g., FrameSvg elements
    // makeUniform should never change the size so much that it gains or loses a whole pixel
    p = QPixmap(size);

    p.fill(Qt::transparent);
    QPainter renderPainter(&p);

    if (actualElementId.isEmpty()) {
        renderer->render(&renderPainter, finalRect);
    } else {
        renderer->render(&renderPainter, actualElementId, finalRect);
    }

    renderPainter.end();
    p.setDevicePixelRatio(ratio);

    if (cacheRendering) {
        actualImageSet()->d->insertIntoCache(id, p, QString::number((qint64)q, 16) % QLatin1Char('_') % actualElementId);
    }

    SvgRectsCache::instance()->updateLastModified(path, lastModified);

    return p;
}

void SvgPrivate::createRenderer()
{
    if (renderer) {
        return;
    }

    if (themed && path.isEmpty() && !themeFailed) {
        if (path.isEmpty()) {
            path = actualImageSet()->imagePath(themePath);
            themeFailed = path.isEmpty();
            if (themeFailed) {
                qCWarning(LOG_KSVG) << "No image path found for" << themePath;
            }
        }
    }

    QString styleSheet;
    if (!colorOverrides.isEmpty()) {
        if (stylesheetOverride.isEmpty()) {
            stylesheetOverride = actualImageSet()->d->svgStyleSheet(q);
        }
        styleSheet = stylesheetOverride;
    } else {
        styleSheet = actualImageSet()->d->svgStyleSheet(q);
    }

    styleCrc = qChecksum(QByteArrayView(styleSheet.toUtf8().constData(), styleSheet.size()));

    QHash<QString, SharedSvgRenderer::Ptr>::const_iterator it = s_renderers.constFind(styleCrc + path);

    if (it != s_renderers.constEnd()) {
        renderer = it.value();
    } else {
        if (path.isEmpty()) {
            renderer = new SharedSvgRenderer();
        } else {
            QHash<QString, QRectF> interestingElements;
            renderer = new SharedSvgRenderer(path, styleSheet, interestingElements);

            // Add interesting elements to the theme's rect cache.
            QHashIterator<QString, QRectF> i(interestingElements);

            QRegularExpression sizeHintedKeyExpr(QStringLiteral("^(\\d+)-(\\d+)-(.+)$"));

            while (i.hasNext()) {
                i.next();
                const QString &elementId = i.key();
                QString originalId = i.key();
                const QRectF &elementRect = i.value();

                originalId.replace(sizeHintedKeyExpr, QStringLiteral("\\3"));
                SvgRectsCache::instance()->insertSizeHintForId(path, originalId, elementRect.size().toSize());

                const CacheId cacheId{.width = -1.0,
                                      .height = -1.0,
                                      .filePath = path,
                                      .elementName = elementId,
                                      .status = status,
                                      .scaleFactor = devicePixelRatio,
                                      .colorSet = -1,
                                      .styleSheet = 0,
                                      .extraFlags = 0,
                                      .lastModified = lastModified};
                SvgRectsCache::instance()->insert(cacheId, elementRect, lastModified);
            }
        }

        s_renderers[styleCrc + path] = renderer;
    }

    if (size == QSizeF()) {
        size = renderer->defaultSize();
    }
}

void SvgPrivate::eraseRenderer()
{
    if (renderer && renderer->ref.loadRelaxed() == 2) {
        // this and the cache reference it
        s_renderers.erase(s_renderers.find(styleCrc + path));
    }

    renderer = nullptr;
    styleCrc = QChar(0);
}

QRectF SvgPrivate::elementRect(QStringView elementId)
{
    if (themed && path.isEmpty()) {
        if (themeFailed) {
            return QRectF();
        }

        path = actualImageSet()->imagePath(themePath);
        themeFailed = path.isEmpty();

        if (themeFailed) {
            return QRectF();
        }
    }

    if (path.isEmpty()) {
        return QRectF();
    }

    QRectF rect;
    const CacheId cacheId = SvgPrivate::cacheId(elementId);
    bool found = SvgRectsCache::instance()->findElementRect(cacheId, rect);
    // This is a corner case where we are *sure* the element is not valid
    if (!found) {
        rect = findAndCacheElementRect(elementId);
    }

    return rect;
}

QRectF SvgPrivate::findAndCacheElementRect(QStringView elementId)
{
    // we need to check the id before createRenderer(), otherwise it may generate a different id compared to the previous cacheId)( call
    const CacheId cacheId = SvgPrivate::cacheId(elementId);

    createRenderer();

    auto elementIdString = elementId.toString();

    // This code will usually never be run because createRenderer already caches all the boundingRect in the elements in the svg
    QRectF elementRect = renderer->elementExists(elementIdString)
        ? renderer->transformForElement(elementIdString).map(renderer->boundsOnElement(elementIdString)).boundingRect()
        : QRectF();

    naturalSize = renderer->defaultSize();

    qreal dx = size.width() / renderer->defaultSize().width();
    qreal dy = size.height() / renderer->defaultSize().height();

    elementRect = QRectF(elementRect.x() * dx, elementRect.y() * dy, elementRect.width() * dx, elementRect.height() * dy);
    SvgRectsCache::instance()->insert(cacheId, elementRect, lastModified);

    return elementRect;
}

bool Svg::eventFilter(QObject *watched, QEvent *event)
{
    return QObject::eventFilter(watched, event);
}

// Following two are utility functions to snap rendered elements to the pixel grid
// to and from are always 0 <= val <= 1
qreal SvgPrivate::closestDistance(qreal to, qreal from)
{
    qreal a = to - from;
    if (qFuzzyCompare(to, from)) {
        return 0;
    } else if (to > from) {
        qreal b = to - from - 1;
        return (qAbs(a) > qAbs(b)) ? b : a;
    } else {
        qreal b = 1 + to - from;
        return (qAbs(a) > qAbs(b)) ? b : a;
    }
}

QRectF SvgPrivate::makeUniform(const QRectF &orig, const QRectF &dst)
{
    if (qFuzzyIsNull(orig.x()) || qFuzzyIsNull(orig.y())) {
        return dst;
    }

    QRectF res(dst);
    qreal div_w = dst.width() / orig.width();
    qreal div_h = dst.height() / orig.height();

    qreal div_x = dst.x() / orig.x();
    qreal div_y = dst.y() / orig.y();

    // horizontal snap
    if (!qFuzzyIsNull(div_x) && !qFuzzyCompare(div_w, div_x)) {
        qreal rem_orig = orig.x() - (floor(orig.x()));
        qreal rem_dst = dst.x() - (floor(dst.x()));
        qreal offset = closestDistance(rem_dst, rem_orig);
        res.translate(offset + offset * div_w, 0);
        res.setWidth(res.width() + offset);
    }
    // vertical snap
    if (!qFuzzyIsNull(div_y) && !qFuzzyCompare(div_h, div_y)) {
        qreal rem_orig = orig.y() - (floor(orig.y()));
        qreal rem_dst = dst.y() - (floor(dst.y()));
        qreal offset = closestDistance(rem_dst, rem_orig);
        res.translate(0, offset + offset * div_h);
        res.setHeight(res.height() + offset);
    }

    return res;
}

void SvgPrivate::imageSetChanged()
{
    if (q->imagePath().isEmpty()) {
        return;
    }

    QString currentPath = themed ? themePath : path;
    themePath.clear();
    eraseRenderer();
    setImagePath(currentPath);
    q->resize();

    // qCDebug(LOG_KSVG) << themePath << ">>>>>>>>>>>>>>>>>> theme changed";
    Q_EMIT q->repaintNeeded();
    Q_EMIT q->imageSetChanged(q->imageSet());
}

void SvgPrivate::colorsChanged()
{
    eraseRenderer();
    qCDebug(LOG_KSVG) << "repaint needed from colorsChanged";

    Q_EMIT q->repaintNeeded();
}

QHash<QString, SharedSvgRenderer::Ptr> SvgPrivate::s_renderers;
QPointer<ImageSet> SvgPrivate::s_systemColorsCache;

Svg::Svg(QObject *parent)
    : QObject(parent)
    , d(new SvgPrivate(this))
{
    connect(SvgRectsCache::instance(), &SvgRectsCache::lastModifiedChanged, this, [this](const QString &filePath, unsigned int lastModified) {
        if (d->lastModified != lastModified && filePath == d->path) {
            d->lastModified = lastModified;
            Q_EMIT repaintNeeded();
        }
    });
}

Svg::~Svg()
{
    delete d;
}

void Svg::setDevicePixelRatio(qreal ratio)
{
    if (FrameSvg *f = qobject_cast<FrameSvg *>(this)) {
        f->clearCache();
    }

    d->devicePixelRatio = ratio;

    Q_EMIT repaintNeeded();
}

qreal Svg::devicePixelRatio() const
{
    return d->devicePixelRatio;
}

QPixmap Svg::pixmap(const QString &elementID)
{
    if (elementID.isNull() || d->multipleImages) {
        return d->findInCache(elementID, d->devicePixelRatio, size());
    } else {
        return d->findInCache(elementID, d->devicePixelRatio);
    }
}

QImage Svg::image(const QSize &size, const QString &elementID)
{
    QPixmap pix(d->findInCache(elementID, d->devicePixelRatio, size));
    return pix.toImage();
}

void Svg::paint(QPainter *painter, const QPointF &point, const QString &elementID)
{
    Q_ASSERT(painter->device());
    const qreal ratio = painter->device()->devicePixelRatio();
    QPixmap pix((elementID.isNull() || d->multipleImages) ? d->findInCache(elementID, ratio, size()) : d->findInCache(elementID, ratio));

    if (pix.isNull()) {
        return;
    }

    painter->drawPixmap(QRectF(point, size()), pix, QRectF(QPointF(0, 0), pix.size()));
}

void Svg::paint(QPainter *painter, int x, int y, const QString &elementID)
{
    paint(painter, QPointF(x, y), elementID);
}

void Svg::paint(QPainter *painter, const QRectF &rect, const QString &elementID)
{
    Q_ASSERT(painter->device());
    const qreal ratio = painter->device()->devicePixelRatio();
    QPixmap pix(d->findInCache(elementID, ratio, rect.size()));

    painter->drawPixmap(rect, pix, QRect(QPoint(0, 0), pix.size()));
}

void Svg::paint(QPainter *painter, int x, int y, int width, int height, const QString &elementID)
{
    Q_ASSERT(painter->device());
    const qreal ratio = painter->device()->devicePixelRatio();
    QPixmap pix(d->findInCache(elementID, ratio, QSizeF(width, height)));
    painter->drawPixmap(x, y, pix, 0, 0, pix.size().width(), pix.size().height());
}

QSizeF Svg::size() const
{
    if (d->size.isEmpty()) {
        d->size = d->naturalSize;
    }

    return {std::round(d->size.width()), std::round(d->size.height())};
}

void Svg::resize(qreal width, qreal height)
{
    resize(QSize(width, height));
}

void Svg::resize(const QSizeF &size)
{
    if (qFuzzyCompare(size.width(), d->size.width()) && qFuzzyCompare(size.height(), d->size.height())) {
        return;
    }

    d->size = size;
    Q_EMIT sizeChanged();
}

void Svg::resize()
{
    if (qFuzzyCompare(d->naturalSize.width(), d->size.width()) && qFuzzyCompare(d->naturalSize.height(), d->size.height())) {
        return;
    }

    d->size = d->naturalSize;
    Q_EMIT sizeChanged();
}

QSizeF Svg::elementSize(const QString &elementId) const
{
    const QSizeF s = d->elementRect(elementId).size();
    return {std::round(s.width()), std::round(s.height())};
}

QSizeF Svg::elementSize(QStringView elementId) const
{
    const QSizeF s = d->elementRect(elementId).size();
    return {std::round(s.width()), std::round(s.height())};
}

QRectF Svg::elementRect(const QString &elementId) const
{
    return d->elementRect(elementId);
}

QRectF Svg::elementRect(QStringView elementId) const
{
    return d->elementRect(elementId);
}

bool Svg::hasElement(const QString &elementId) const
{
    return hasElement(QStringView(elementId));
}

bool Svg::hasElement(QStringView elementId) const
{
    if (elementId.isEmpty() || (d->path.isNull() && d->themePath.isNull())) {
        return false;
    }

    return d->elementRect(elementId).isValid();
}

bool Svg::isValid() const
{
    if (d->path.isNull() && d->themePath.isNull()) {
        return false;
    }

    // try very hard to avoid creation of a parser
    QSizeF naturalSize = SvgRectsCache::instance()->naturalSize(d->path);
    if (!naturalSize.isEmpty()) {
        return true;
    }

    if (d->path.isEmpty() || !QFileInfo::exists(d->path)) {
        return false;
    }
    d->createRenderer();
    return d->renderer->isValid();
}

void Svg::setContainsMultipleImages(bool multiple)
{
    d->multipleImages = multiple;
}

bool Svg::containsMultipleImages() const
{
    return d->multipleImages;
}

void Svg::setImagePath(const QString &svgFilePath)
{
    if (d->setImagePath(svgFilePath)) {
        Q_EMIT repaintNeeded();
    }
}

QString Svg::imagePath() const
{
    return d->themed ? d->themePath : d->path;
}

void Svg::setUsingRenderingCache(bool useCache)
{
    d->cacheRendering = useCache;
    Q_EMIT repaintNeeded();
}

bool Svg::isUsingRenderingCache() const
{
    return d->cacheRendering;
}

bool Svg::fromCurrentImageSet() const
{
    return d->fromCurrentImageSet;
}

void Svg::setImageSet(KSvg::ImageSet *theme)
{
    if (!theme || theme == d->theme.data()) {
        return;
    }

    if (d->theme) {
        disconnect(d->theme.data(), nullptr, this, nullptr);
    }

    d->theme = theme;
    connect(theme, SIGNAL(imageSetChanged(QString)), this, SLOT(imageSetChanged()));
    d->imageSetChanged();
}

ImageSet *Svg::imageSet() const
{
    return d->actualImageSet();
}

void Svg::setStatus(KSvg::Svg::Status status)
{
    if (status == d->status) {
        return;
    }

    d->status = status;
    d->eraseRenderer();
    Q_EMIT statusChanged(status);
    Q_EMIT repaintNeeded();
}

Svg::Status Svg::status() const
{
    return d->status;
}

void Svg::setColorSet(KSvg::Svg::ColorSet colorSet)
{
    const KColorScheme::ColorSet convertedSet = KColorScheme::ColorSet(colorSet);
    if (convertedSet == d->colorSet) {
        return;
    }

    d->colorSet = convertedSet;
    d->eraseRenderer();
    Q_EMIT colorSetChanged(colorSet);
    Q_EMIT repaintNeeded();
}

Svg::ColorSet Svg::colorSet() const
{
    return Svg::ColorSet(d->colorSet);
}

QColor Svg::color(StyleSheetColor colorName) const
{
    auto it = d->colorOverrides.constFind(colorName);
    if (it != d->colorOverrides.constEnd()) {
        return *it;
    }
    return d->actualImageSet()->d->namedColor(colorName, this);
}

void Svg::setColor(StyleSheetColor colorName, const QColor &color)
{
    if (d->colorOverrides.value(colorName) == color) {
        return;
    }

    if (color.isValid()) {
        d->colorOverrides[colorName] = color;
    } else {
        d->colorOverrides.remove(colorName);
    }
    d->stylesheetOverride.clear();

    d->eraseRenderer();
    Q_EMIT repaintNeeded();
}

void Svg::clearColorOverrides()
{
    d->colorOverrides.clear();
    d->stylesheetOverride.clear();
    d->eraseRenderer();
    Q_EMIT repaintNeeded();
}

} // KSvg namespace

#include "moc_svg.cpp"
#include "private/moc_svg_p.cpp"
