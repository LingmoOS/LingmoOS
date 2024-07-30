/*
    SPDX-FileCopyrightText: 2008-2010 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "framesvg.h"
#include "private/framesvg_p.h"

#include <string>

#include <QAtomicInt>
#include <QBitmap>
#include <QCryptographicHash>
#include <QPainter>
#include <QRegion>
#include <QSizeF>
#include <QStringBuilder>
#include <QTimer>

#include <QDebug>

#include "debug_p.h"
#include "imageset.h"
#include "private/framesvg_helpers.h"
#include "private/imageset_p.h"
#include "private/svg_p.h"

namespace KSvg
{
QHash<ImageSetPrivate *, QHash<uint, QWeakPointer<FrameData>>> FrameSvgPrivate::s_sharedFrames;

// Any attempt to generate a frame whose width or height is larger than this
// will be rejected
static const int MAX_FRAME_SIZE = 100000;

FrameData::~FrameData()
{
    FrameSvgPrivate::s_sharedFrames[imageSet].remove(cacheId);
}

FrameSvg::FrameSvg(QObject *parent)
    : Svg(parent)
    , d(new FrameSvgPrivate(this))
{
    connect(this, &FrameSvg::repaintNeeded, this, std::bind(&FrameSvgPrivate::updateNeeded, d));
}

FrameSvg::~FrameSvg()
{
    delete d;
}

void FrameSvg::setImagePath(const QString &path)
{
    if (path == imagePath()) {
        return;
    }

    clearCache();

    setContainsMultipleImages(true);
    Svg::d->setImagePath(path);
    if (!d->repaintBlocked) {
        d->updateFrameData(Svg::d->lastModified);
    }
}

void FrameSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->enabledBorders) {
        return;
    }

    d->enabledBorders = borders;

    if (!d->repaintBlocked) {
        d->updateFrameData(Svg::d->lastModified);
    }
}

FrameSvg::EnabledBorders FrameSvg::enabledBorders() const
{
    return d->enabledBorders;
}

void FrameSvg::setElementPrefix(KSvg::FrameSvg::LocationPrefix location)
{
    switch (location) {
    case TopEdge:
        setElementPrefix(QStringLiteral("north"));
        break;
    case BottomEdge:
        setElementPrefix(QStringLiteral("south"));
        break;
    case LeftEdge:
        setElementPrefix(QStringLiteral("west"));
        break;
    case RightEdge:
        setElementPrefix(QStringLiteral("east"));
        break;
    default:
        setElementPrefix(QString());
        break;
    }

    d->location = location;
}

void FrameSvg::setElementPrefix(const QString &prefix)
{
    if (prefix.isEmpty() || !hasElement(prefix % QLatin1String("-center"))) {
        d->prefix.clear();
    } else {
        d->prefix = prefix;
        if (!d->prefix.isEmpty()) {
            d->prefix += QLatin1Char('-');
        }
    }
    d->requestedPrefix = prefix;

    d->location = FrameSvg::Floating;

    if (!d->repaintBlocked) {
        d->updateFrameData(Svg::d->lastModified);
    }
}

bool FrameSvg::hasElementPrefix(const QString &prefix) const
{
    // for now it simply checks if a center element exists,
    // because it could make sense for certain themes to not have all the elements
    if (prefix.isEmpty()) {
        return hasElement(QStringLiteral("center"));
    }
    if (prefix.endsWith(QLatin1Char('-'))) {
        return hasElement(prefix % QLatin1String("center"));
    }

    return hasElement(prefix % QLatin1String("-center"));
}

bool FrameSvg::hasElementPrefix(KSvg::FrameSvg::LocationPrefix location) const
{
    switch (location) {
    case TopEdge:
        return hasElementPrefix(QStringLiteral("north"));
    case BottomEdge:
        return hasElementPrefix(QStringLiteral("south"));
    case LeftEdge:
        return hasElementPrefix(QStringLiteral("west"));
    case RightEdge:
        return hasElementPrefix(QStringLiteral("east"));
    default:
        return hasElementPrefix(QString());
    }
}

QString FrameSvg::prefix()
{
    return d->requestedPrefix;
}

void FrameSvg::resizeFrame(const QSizeF &size)
{
    if (imagePath().isEmpty()) {
        return;
    }

    if (size.isEmpty()) {
#ifndef NDEBUG
        // qCDebug(LOG_KSVG) << "Invalid size" << size;
#endif
        return;
    }

    if (d->frame && size.toSize() == d->frame->frameSize) {
        return;
    }
    d->pendingFrameSize = size.toSize();

    if (!d->repaintBlocked) {
        d->updateFrameData(Svg::d->lastModified, FrameSvgPrivate::UpdateFrame);
    }
}

QSizeF FrameSvg::frameSize() const
{
    if (!d->frame) {
        return QSizeF(-1, -1);
    } else {
        return d->frameSize(d->frame.data());
    }
}

qreal FrameSvg::marginSize(const FrameSvg::MarginEdge edge) const
{
    if (!d->frame) {
        return .0;
    }

    if (d->frame->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case FrameSvg::TopMargin:
        return d->frame->topMargin;

    case FrameSvg::LeftMargin:
        return d->frame->leftMargin;

    case FrameSvg::RightMargin:
        return d->frame->rightMargin;

    // KSvg::BottomMargin
    default:
        return d->frame->bottomMargin;
    }
}

qreal FrameSvg::insetSize(const FrameSvg::MarginEdge edge) const
{
    if (!d->frame) {
        return .0;
    }

    if (d->frame->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case FrameSvg::TopMargin:
        return d->frame->insetTopMargin;

    case FrameSvg::LeftMargin:
        return d->frame->insetLeftMargin;

    case FrameSvg::RightMargin:
        return d->frame->insetRightMargin;

    // KSvg::BottomMargin
    default:
        return d->frame->insetBottomMargin;
    }
}

qreal FrameSvg::fixedMarginSize(const FrameSvg::MarginEdge edge) const
{
    if (!d->frame) {
        return .0;
    }

    if (d->frame->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case FrameSvg::TopMargin:
        return d->frame->fixedTopMargin;

    case FrameSvg::LeftMargin:
        return d->frame->fixedLeftMargin;

    case FrameSvg::RightMargin:
        return d->frame->fixedRightMargin;

    // KSvg::BottomMargin
    default:
        return d->frame->fixedBottomMargin;
    }
}

void FrameSvg::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    if (!d->frame || d->frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = d->frame->topMargin;
    left = d->frame->leftMargin;
    right = d->frame->rightMargin;
    bottom = d->frame->bottomMargin;
}

void FrameSvg::getFixedMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    if (!d->frame || d->frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = d->frame->fixedTopMargin;
    left = d->frame->fixedLeftMargin;
    right = d->frame->fixedRightMargin;
    bottom = d->frame->fixedBottomMargin;
}

void FrameSvg::getInset(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    if (!d->frame || d->frame->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = d->frame->insetTopMargin;
    left = d->frame->insetLeftMargin;
    right = d->frame->insetRightMargin;
    bottom = d->frame->insetBottomMargin;
}

QRectF FrameSvg::contentsRect() const
{
    if (d->frame) {
        QRectF rect(QPoint(0, 0), d->frame->frameSize);
        return rect.adjusted(d->frame->leftMargin, d->frame->topMargin, -d->frame->rightMargin, -d->frame->bottomMargin);
    } else {
        return QRectF();
    }
}

QPixmap FrameSvg::alphaMask() const
{
    // FIXME: the distinction between overlay and
    return d->alphaMask();
}

QRegion FrameSvg::mask() const
{
    QRegion result;
    if (!d->frame) {
        return result;
    }

    size_t id = qHash(d->cacheId(d->frame.data(), QString()), SvgRectsCache::s_seed);

    QRegion *obj = d->frame->cachedMasks.object(id);

    if (!obj) {
        QPixmap alphaMask = d->alphaMask();
        const qreal dpr = alphaMask.devicePixelRatio();

        // region should always be in logical pixels, resize pixmap to be in the logical sizes
        if (alphaMask.devicePixelRatio() != 1.0) {
            alphaMask = alphaMask.scaled(alphaMask.width() / dpr, alphaMask.height() / dpr);
        }

        // mask() of a QPixmap without alpha Channel will be null
        // but if our mask has no lpha at all, we want instead consider the entire area as the mask
        if (alphaMask.hasAlphaChannel()) {
            obj = new QRegion(QBitmap(alphaMask.mask()));
        } else {
            obj = new QRegion(alphaMask.rect());
        }

        result = *obj;
        d->frame->cachedMasks.insert(id, obj);
    } else {
        result = *obj;
    }
    return result;
}

void FrameSvg::setCacheAllRenderedFrames(bool cache)
{
    if (d->cacheAll && !cache) {
        clearCache();
    }

    d->cacheAll = cache;
}

bool FrameSvg::cacheAllRenderedFrames() const
{
    return d->cacheAll;
}

void FrameSvg::clearCache()
{
    if (d->frame) {
        d->frame->cachedBackground = QPixmap();
        d->frame->cachedMasks.clear();
    }
    if (d->maskFrame) {
        d->maskFrame->cachedBackground = QPixmap();
        d->maskFrame->cachedMasks.clear();
    }
}

QPixmap FrameSvg::framePixmap()
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
    }

    return d->frame->cachedBackground;
}

void FrameSvg::paintFrame(QPainter *painter, const QRectF &target, const QRectF &source)
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
        if (d->frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(target, d->frame->cachedBackground, source.isValid() ? source : target);
}

void FrameSvg::paintFrame(QPainter *painter, const QPointF &pos)
{
    if (d->frame->cachedBackground.isNull()) {
        d->generateBackground(d->frame);
        if (d->frame->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(pos, d->frame->cachedBackground);
}

int FrameSvg::minimumDrawingHeight()
{
    if (d->frame) {
        return d->frame->fixedTopHeight + d->frame->fixedBottomHeight;
    }
    return 0;
}

int FrameSvg::minimumDrawingWidth()
{
    if (d->frame) {
        return d->frame->fixedRightWidth + d->frame->fixedLeftWidth;
    }
    return 0;
}

//#define DEBUG_FRAMESVG_CACHE
FrameSvgPrivate::~FrameSvgPrivate() = default;

QPixmap FrameSvgPrivate::alphaMask()
{
    QString maskPrefix;

    if (q->hasElement(QLatin1String("mask-") % prefix % QLatin1String("center"))) {
        maskPrefix = QStringLiteral("mask-");
    }

    if (maskPrefix.isNull()) {
        if (frame->cachedBackground.isNull()) {
            generateBackground(frame);
        }
        return frame->cachedBackground;
    }

    // We are setting the prefix only temporary to generate
    // the needed mask image
    const QString maskRequestedPrefix = requestedPrefix.isEmpty() ? QStringLiteral("mask") : maskPrefix % requestedPrefix;
    maskPrefix = maskPrefix % prefix;

    if (!maskFrame) {
        maskFrame = lookupOrCreateMaskFrame(frame, maskPrefix, maskRequestedPrefix);
        if (!maskFrame->cachedBackground.isNull()) {
            return maskFrame->cachedBackground;
        }
        updateSizes(maskFrame);
        generateBackground(maskFrame);
        return maskFrame->cachedBackground;
    }

    const bool shouldUpdate = (maskFrame->enabledBorders != frame->enabledBorders //
                               || maskFrame->frameSize != frameSize(frame.data()) //
                               || maskFrame->imagePath != frame->imagePath);
    if (shouldUpdate) {
        maskFrame = lookupOrCreateMaskFrame(frame, maskPrefix, maskRequestedPrefix);
        if (!maskFrame->cachedBackground.isNull()) {
            return maskFrame->cachedBackground;
        }
        updateSizes(maskFrame);
    }

    if (maskFrame->cachedBackground.isNull()) {
        generateBackground(maskFrame);
    }

    return maskFrame->cachedBackground;
}

QSharedPointer<FrameData>
FrameSvgPrivate::lookupOrCreateMaskFrame(const QSharedPointer<FrameData> &frame, const QString &maskPrefix, const QString &maskRequestedPrefix)
{
    const size_t key = qHash(cacheId(frame.data(), maskPrefix));
    QSharedPointer<FrameData> mask = s_sharedFrames[q->imageSet()->d].value(key);

    // See if we can find a suitable candidate in the shared frames.
    // If there is one, use it.
    if (mask) {
        return mask;
    }

    mask.reset(new FrameData(*frame.data()));
    mask->prefix = maskPrefix;
    mask->requestedPrefix = maskRequestedPrefix;
    mask->imageSet = q->imageSet()->d;
    mask->imagePath = frame->imagePath;
    mask->enabledBorders = frame->enabledBorders;
    mask->frameSize = frameSize(frame).toSize();
    mask->cacheId = key;
    mask->lastModified = frame->lastModified;
    s_sharedFrames[q->imageSet()->d].insert(key, mask);

    return mask;
}

void FrameSvgPrivate::generateBackground(const QSharedPointer<FrameData> &frame)
{
    if (!frame->cachedBackground.isNull() || !q->hasElementPrefix(frame->prefix)) {
        return;
    }

    const size_t id = qHash(cacheId(frame.data(), frame->prefix));

    bool frameCached = !frame->cachedBackground.isNull();
    bool overlayCached = false;

    const bool overlayAvailable = !frame->prefix.startsWith(QLatin1String("mask-")) && q->hasElement(frame->prefix % QLatin1String("overlay"));
    QPixmap overlay;
    if (q->isUsingRenderingCache()) {
        frameCached = q->imageSet()->d->findInCache(QString::number(id), frame->cachedBackground, frame->lastModified) && !frame->cachedBackground.isNull();
        if (frameCached) {
            frame->cachedBackground.setDevicePixelRatio(q->devicePixelRatio());
        }

        if (overlayAvailable) {
            const size_t overlayId = qHash(cacheId(frame.data(), frame->prefix % QLatin1String("overlay")));
            overlayCached = q->imageSet()->d->findInCache(QString::number(overlayId), overlay, frame->lastModified) && !overlay.isNull();
            if (overlayCached) {
                overlay.setDevicePixelRatio(q->devicePixelRatio());
            }
        }
    }

    if (!frameCached) {
        generateFrameBackground(frame);
    }

    // Overlays
    QSizeF overlaySize;
    QPointF actualOverlayPos = QPointF(0, 0);
    if (overlayAvailable && !overlayCached) {
        overlaySize = q->elementSize(frame->prefix % QLatin1String("overlay")).toSize();

        if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-pos-right"))) {
            actualOverlayPos.setX(frame->frameSize.width() - overlaySize.width());
        } else if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-pos-bottom"))) {
            actualOverlayPos.setY(frame->frameSize.height() - overlaySize.height());
            // Stretched or Tiled?
        } else if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-stretch"))) {
            overlaySize = frameSize(frame).toSize();
        } else {
            if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-horizontal"))) {
                overlaySize.setWidth(frameSize(frame).width());
            }
            if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-vertical"))) {
                overlaySize.setHeight(frameSize(frame).height());
            }
        }

        overlay = alphaMask();
        QPainter overlayPainter(&overlay);
        overlayPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        // Tiling?
        if (q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-horizontal"))
            || q->hasElement(frame->prefix % QLatin1String("hint-overlay-tile-vertical"))) {
            QSizeF s = q->size().toSize();
            q->resize(q->elementSize(frame->prefix % QLatin1String("overlay")));

            overlayPainter.drawTiledPixmap(QRectF(QPointF(0, 0), overlaySize), q->pixmap(frame->prefix % QLatin1String("overlay")));
            q->resize(s);
        } else {
            q->paint(&overlayPainter, QRectF(actualOverlayPos, overlaySize), frame->prefix % QLatin1String("overlay"));
        }

        overlayPainter.end();
    }

    if (!frameCached) {
        cacheFrame(frame->prefix, frame->cachedBackground, overlayCached ? overlay : QPixmap());
    }

    if (!overlay.isNull()) {
        QPainter p(&frame->cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(actualOverlayPos, overlay, QRectF(actualOverlayPos, overlaySize));
    }
}

void FrameSvgPrivate::generateFrameBackground(const QSharedPointer<FrameData> &frame)
{
    // qCDebug(LOG_KSVG) << "generating background";
    const QSizeF size = frameSize(frame) * q->devicePixelRatio();

    if (!size.isValid()) {
#ifndef NDEBUG
        // qCDebug(LOG_KSVG) << "Invalid frame size" << size;
#endif
        return;
    }
    if (size.width() >= MAX_FRAME_SIZE || size.height() >= MAX_FRAME_SIZE) {
        qCWarning(LOG_KSVG) << "Not generating frame background for a size whose width or height is more than" << MAX_FRAME_SIZE << size;
        return;
    }

    // Don't cut away pieces of the frame
    frame->cachedBackground = QPixmap(QSize(std::ceil(size.width()), std::ceil(size.height())));
    frame->cachedBackground.fill(Qt::transparent);
    QPainter p(&frame->cachedBackground);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QRectF contentRect = contentGeometry(frame, size);
    paintCenter(p, frame, contentRect, size);

    paintCorner(p, frame, FrameSvg::LeftBorder | FrameSvg::TopBorder, contentRect);
    paintCorner(p, frame, FrameSvg::RightBorder | FrameSvg::TopBorder, contentRect);
    paintCorner(p, frame, FrameSvg::LeftBorder | FrameSvg::BottomBorder, contentRect);
    paintCorner(p, frame, FrameSvg::RightBorder | FrameSvg::BottomBorder, contentRect);

    // Sides
    const qreal leftHeight = q->elementSize(frame->prefix % QLatin1String("left")).height();
    paintBorder(p, frame, FrameSvg::LeftBorder, QSizeF(frame->leftWidth, leftHeight) * q->devicePixelRatio(), contentRect);
    const qreal rightHeight = q->elementSize(frame->prefix % QLatin1String("right")).height();
    paintBorder(p, frame, FrameSvg::RightBorder, QSizeF(frame->rightWidth, rightHeight) * q->devicePixelRatio(), contentRect);

    const qreal topWidth = q->elementSize(frame->prefix % QLatin1String("top")).width();
    paintBorder(p, frame, FrameSvg::TopBorder, QSizeF(topWidth, frame->topHeight) * q->devicePixelRatio(), contentRect);
    const qreal bottomWidth = q->elementSize(frame->prefix % QLatin1String("bottom")).width();
    paintBorder(p, frame, FrameSvg::BottomBorder, QSizeF(bottomWidth, frame->bottomHeight) * q->devicePixelRatio(), contentRect);
    p.end();

    // Set the devicePixelRatio only at the end, drawing all happened in device pixels
    frame->cachedBackground.setDevicePixelRatio(q->devicePixelRatio());
}

QRectF FrameSvgPrivate::contentGeometry(const QSharedPointer<FrameData> &frame, const QSizeF &size) const
{
    const QSizeF contentSize(size.width() - frame->leftWidth * q->devicePixelRatio() - frame->rightWidth * q->devicePixelRatio(),
                             size.height() - frame->topHeight * q->devicePixelRatio() - frame->bottomHeight * q->devicePixelRatio());
    QRectF contentRect(QPointF(0, 0), contentSize);
    if (frame->enabledBorders & FrameSvg::LeftBorder && q->hasElement(frame->prefix % QLatin1String("left"))) {
        contentRect.translate(frame->leftWidth * q->devicePixelRatio(), 0);
    }

    // Corners
    if (frame->enabledBorders & FrameSvg::TopBorder && q->hasElement(frame->prefix % QLatin1String("top"))) {
        contentRect.translate(0, frame->topHeight * q->devicePixelRatio());
    }
    return contentRect;
}

void FrameSvgPrivate::updateFrameData(uint lastModified, UpdateType updateType)
{
    auto fd = frame;
    uint newKey = 0;

    if (fd) {
        const uint oldKey = fd->cacheId;

        const QString oldPath = fd->imagePath;
        const FrameSvg::EnabledBorders oldBorders = fd->enabledBorders;
        const QSizeF currentSize = fd->frameSize;

        fd->enabledBorders = enabledBorders;
        fd->frameSize = pendingFrameSize;
        fd->imagePath = q->imagePath();

        newKey = qHash(cacheId(fd.data(), prefix));

        // reset frame to old values
        fd->enabledBorders = oldBorders;
        fd->frameSize = currentSize;
        fd->imagePath = oldPath;

        // FIXME: something more efficient than string comparison?
        if (oldKey == newKey) {
            return;
        }

        // qCDebug(LOG_KSVG) << "looking for" << newKey;
        auto newFd = FrameSvgPrivate::s_sharedFrames[q->imageSet()->d].value(newKey);
        if (newFd) {
            // qCDebug(LOG_KSVG) << "FOUND IT!" << newFd->refcount;
            // we've found a match, use that one
            Q_ASSERT(newKey == newFd.lock()->cacheId);
            frame = newFd;
            return;
        }

        fd.reset(new FrameData(*fd));
    } else {
        fd.reset(new FrameData(q, QString()));
    }

    frame = fd;
    fd->prefix = prefix;
    fd->requestedPrefix = requestedPrefix;
    // updateSizes();
    fd->enabledBorders = enabledBorders;
    fd->frameSize = pendingFrameSize;
    fd->imagePath = q->imagePath();
    fd->lastModified = lastModified;
    // was fd just created empty now?
    if (newKey == 0) {
        newKey = qHash(cacheId(fd.data(), prefix));
    }

    // we know it isn't in s_sharedFrames due to the check above, so insert it now
    FrameSvgPrivate::s_sharedFrames[q->imageSet()->d].insert(newKey, fd);
    fd->cacheId = newKey;
    fd->imageSet = q->imageSet()->d;
    if (updateType == UpdateFrameAndMargins) {
        updateAndSignalSizes();
    } else {
        updateSizes(frame);
    }
}

void FrameSvgPrivate::paintCenter(QPainter &p, const QSharedPointer<FrameData> &frame, const QRectF &contentRect, const QSizeF &fullSize)
{
    // fullSize and contentRect are in device pixels
    if (!contentRect.isEmpty()) {
        const QString centerElementId = frame->prefix % QLatin1String("center");
        if (frame->tileCenter) {
            QSizeF centerTileSize = q->elementSize(centerElementId);
            QPixmap center(centerTileSize.toSize());
            center.fill(Qt::transparent);

            QPainter centerPainter(&center);
            centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&centerPainter, QRectF(QPointF(0, 0), centerTileSize), centerElementId);

            if (frame->composeOverBorder) {
                p.drawTiledPixmap(QRectF(QPointF(0, 0), fullSize), center);
            } else {
                p.drawTiledPixmap(FrameSvgHelpers::sectionRect(FrameSvg::NoBorder, contentRect, fullSize * q->devicePixelRatio()), center);
            }
        } else {
            if (frame->composeOverBorder) {
                q->paint(&p, QRectF(QPointF(0, 0), fullSize), centerElementId);
            } else {
                q->paint(&p, FrameSvgHelpers::sectionRect(FrameSvg::NoBorder, contentRect, fullSize * q->devicePixelRatio()), centerElementId);
            }
        }
    }

    if (frame->composeOverBorder) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRectF(QPointF(0, 0), fullSize), alphaMask(), QRectF(QPointF(0, 0), alphaMask().size()));
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
}

void FrameSvgPrivate::paintBorder(QPainter &p,
                                  const QSharedPointer<FrameData> &frame,
                                  const FrameSvg::EnabledBorders borders,
                                  const QSizeF &size,
                                  const QRectF &contentRect) const
{
    // size and contentRect are in device pixels
    QString side = frame->prefix % FrameSvgHelpers::borderToElementId(borders);
    if (frame->enabledBorders & borders && q->hasElement(side) && !size.isEmpty()) {
        if (frame->stretchBorders) {
            q->paint(&p, FrameSvgHelpers::sectionRect(borders, contentRect, frame->frameSize * q->devicePixelRatio()), side);
        } else {
            QSize grownSize(std::ceil(size.width()), std::ceil(size.height()));
            QPixmap px(grownSize);
            // QPixmap px(QSize(std::ceil(size.width()), std::ceil(size.height())));
            px.fill(Qt::transparent);

            QPainter sidePainter(&px);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            // A QRect as we have to exactly fill a QPixmap of integer size, prefer going slightly outside it to not have empty edges in the pixmap to tile
            q->paint(&sidePainter, QRect(QPoint(0, 0), grownSize), side);

            // We are composing QPixmaps here, so all objects with integer size
            // Rounding the position and ceiling the size is the way that gives better tiled results
            auto r = FrameSvgHelpers::sectionRect(borders, contentRect, frame->frameSize * q->devicePixelRatio());
            r.setTopLeft(r.topLeft().toPoint());
            r.setSize(QSizeF(std::ceil(r.size().width()), std::ceil(r.size().height())));

            p.drawTiledPixmap(r, px);
        }
    }
}

void FrameSvgPrivate::paintCorner(QPainter &p, const QSharedPointer<FrameData> &frame, KSvg::FrameSvg::EnabledBorders border, const QRectF &contentRect) const
{
    // contentRect is in device pixels
    // Draw the corner only if both borders in both directions are enabled.
    if ((frame->enabledBorders & border) != border) {
        return;
    }
    const QString corner = frame->prefix % FrameSvgHelpers::borderToElementId(border);
    if (q->hasElement(corner)) {
        auto r = FrameSvgHelpers::sectionRect(border, contentRect, frame->frameSize * q->devicePixelRatio());
        // We are composing QPixmaps here, so all objects with integer size
        // Rounding the position and ceiling the size is the way that gives better tiled results
        r.setTopLeft(r.topLeft().toPoint());
        r.setSize(QSizeF(std::ceil(r.size().width()), std::ceil(r.size().height())));
        q->paint(&p, r.toRect(), corner);
    }
}

SvgPrivate::CacheId FrameSvgPrivate::cacheId(FrameData *frame, const QString &prefixToSave) const
{
    const QSize size = frameSize(frame).toSize();
    return SvgPrivate::CacheId{double(size.width()),
                               double(size.height()),
                               frame->imagePath,
                               prefixToSave,
                               q->status(),
                               q->devicePixelRatio(),
                               q->colorSet(),
                               (uint)frame->enabledBorders,
                               0,
                               q->Svg::d->lastModified};
}

void FrameSvgPrivate::cacheFrame(const QString &prefixToSave, const QPixmap &background, const QPixmap &overlay)
{
    if (!q->isUsingRenderingCache()) {
        return;
    }

    // insert background
    if (!frame) {
        return;
    }

    const size_t id = qHash(cacheId(frame.data(), prefixToSave));

    // qCDebug(LOG_KSVG)<<"Saving to cache frame"<<id;

    q->imageSet()->d->insertIntoCache(QString::number(id), background, QString::number((qint64)q, 16) % prefixToSave);

    if (!overlay.isNull()) {
        // insert overlay
        const size_t overlayId = qHash(cacheId(frame.data(), frame->prefix % QLatin1String("overlay")));
        q->imageSet()->d->insertIntoCache(QString::number(overlayId), overlay, QString::number((qint64)q, 16) % prefixToSave % QLatin1String("overlay"));
    }
}

void FrameSvgPrivate::updateSizes(FrameData *frame) const
{
    // qCDebug(LOG_KSVG) << "!!!!!!!!!!!!!!!!!!!!!! updating sizes" << prefix;
    Q_ASSERT(frame);

    QSizeF s = q->size();
    q->resize();
    if (!frame->cachedBackground.isNull()) {
        frame->cachedBackground = QPixmap();
    }

    // This function needs to do a lot of string creation, since we have four
    // sides with matching margins and insets. Rather than creating a new string
    // every time for these, create a single buffer that can contain a full
    // element name and pass that around using views, so we save a lot of
    // allocations.
    QString nameBuffer;
    const auto offset = frame->prefix.length();
    nameBuffer.reserve(offset + 30);
    nameBuffer.append(frame->prefix);

    // This uses UTF16 literals to avoid having to create QLatin1String and then
    // converting that to a QString temporary for the replace operation.
    // Additionally, we use a template parameter to provide us the compile-time
    // length of the literal so we don't need to calculate that.
    auto createName = [&nameBuffer, offset]<std::size_t length>(const char16_t(&name)[length]) {
        nameBuffer.replace(offset, length - 1, reinterpret_cast<const QChar *>(name), length);
        return QStringView(nameBuffer).mid(0, offset + length - 1);
    };

    // This has the same size regardless the border is enabled or not
    frame->fixedTopHeight = q->elementSize(createName(u"top")).height();

    if (auto topMargin = q->elementRect(createName(u"hint-top-margin")); topMargin.isValid()) {
        frame->fixedTopMargin = topMargin.height();
    } else {
        frame->fixedTopMargin = frame->fixedTopHeight;
    }

    // The same, but its size depends from the margin being enabled
    if (frame->enabledBorders & FrameSvg::TopBorder) {
        frame->topMargin = frame->fixedTopMargin;
        frame->topHeight = frame->fixedTopHeight;
    } else {
        frame->topMargin = frame->topHeight = 0;
    }

    if (auto topInset = q->elementRect(createName(u"hint-top-inset")); topInset.isValid()) {
        frame->insetTopMargin = topInset.height();
    } else {
        frame->insetTopMargin = -1;
    }

    frame->fixedLeftWidth = q->elementSize(createName(u"left")).width();

    if (auto leftMargin = q->elementRect(createName(u"hint-left-margin")); leftMargin.isValid()) {
        frame->fixedLeftMargin = leftMargin.width();
    } else {
        frame->fixedLeftMargin = frame->fixedLeftWidth;
    }

    if (frame->enabledBorders & FrameSvg::LeftBorder) {
        frame->leftMargin = frame->fixedLeftMargin;
        frame->leftWidth = frame->fixedLeftWidth;
    } else {
        frame->leftMargin = frame->leftWidth = 0;
    }

    if (auto leftInset = q->elementRect(createName(u"hint-left-inset")); leftInset.isValid()) {
        frame->insetLeftMargin = leftInset.width();
    } else {
        frame->insetLeftMargin = -1;
    }

    frame->fixedRightWidth = q->elementSize(createName(u"right")).width();

    if (auto rightMargin = q->elementRect(createName(u"hint-right-margin")); rightMargin.isValid()) {
        frame->fixedRightMargin = rightMargin.width();
    } else {
        frame->fixedRightMargin = frame->fixedRightWidth;
    }

    if (frame->enabledBorders & FrameSvg::RightBorder) {
        frame->rightMargin = frame->fixedRightMargin;
        frame->rightWidth = frame->fixedRightWidth;
    } else {
        frame->rightMargin = frame->rightWidth = 0;
    }

    if (auto rightInset = q->elementRect(createName(u"hint-right-inset")); rightInset.isValid()) {
        frame->insetRightMargin = rightInset.width();
    } else {
        frame->insetRightMargin = -1;
    }

    frame->fixedBottomHeight = q->elementSize(createName(u"bottom")).height();

    if (auto bottomMargin = q->elementRect(createName(u"hint-bottom-margin")); bottomMargin.isValid()) {
        frame->fixedBottomMargin = bottomMargin.height();
    } else {
        frame->fixedBottomMargin = frame->fixedBottomHeight;
    }

    if (frame->enabledBorders & FrameSvg::BottomBorder) {
        frame->bottomMargin = frame->fixedBottomMargin;
        frame->bottomHeight = frame->fixedBottomHeight;
    } else {
        frame->bottomMargin = frame->bottomHeight = 0;
    }

    if (auto bottomInset = q->elementRect(createName(u"hint-bottom-inset")); bottomInset.isValid()) {
        frame->insetBottomMargin = bottomInset.height();
    } else {
        frame->insetBottomMargin = -1;
    }

    static const QString maskPrefix = QStringLiteral("mask-");
    static const QString hintTileCenter = QStringLiteral("hint-tile-center");
    static const QString hintNoBorderPadding = QStringLiteral("hint-no-border-padding");
    static const QString hintStretchBorders = QStringLiteral("hint-stretch-borders");

    frame->composeOverBorder = (q->hasElement(createName(u"hint-compose-over-border")) && q->hasElement(maskPrefix % createName(u"center")));

    // since it's rectangular, topWidth and bottomWidth must be the same
    // the ones that don't have a frame->prefix is for retrocompatibility
    frame->tileCenter = (q->hasElement(hintTileCenter) || q->hasElement(createName(u"hint-tile-center")));
    frame->noBorderPadding = (q->hasElement(hintNoBorderPadding) || q->hasElement(createName(u"hint-no-border-padding")));
    frame->stretchBorders = (q->hasElement(hintStretchBorders) || q->hasElement(createName(u"hint-stretch-borders")));
    q->resize(s);
}

void FrameSvgPrivate::updateNeeded()
{
    q->setElementPrefix(requestedPrefix);
    // frame not created yet?
    if (!frame) {
        return;
    }
    q->clearCache();
    updateSizes(frame);
}

void FrameSvgPrivate::updateAndSignalSizes()
{
    // frame not created yet?
    if (!frame) {
        return;
    }
    updateSizes(frame);
    Q_EMIT q->repaintNeeded();
}

QSizeF FrameSvgPrivate::frameSize(FrameData *frame) const
{
    if (!frame) {
        return QSizeF();
    }

    if (!frame->frameSize.isValid()) {
        updateSizes(frame);
        frame->frameSize = q->size().toSize();
    }

    return frame->frameSize;
}

QString FrameSvg::actualPrefix() const
{
    return d->prefix;
}

bool FrameSvg::isRepaintBlocked() const
{
    return d->repaintBlocked;
}

void FrameSvg::setRepaintBlocked(bool blocked)
{
    d->repaintBlocked = blocked;

    if (!blocked) {
        d->updateFrameData(Svg::d->lastModified);
    }
}

} // KSvg namespace

#include "moc_framesvg.cpp"
