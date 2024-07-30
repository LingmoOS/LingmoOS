/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kiconengine.h"

#include "kiconloader_p.h"
#include <kiconloader.h>

#include "kiconcolors.h"
#include <KIconTheme>
#include <QFileInfo>
#include <QPainter>
#include <qscopeguard.h>

class KIconEnginePrivate
{
public:
    QPointer<KIconLoader> mIconLoader;
    bool mCustomColors = false;
    KIconColors mColors;
    QString mActualIconName;
};

KIconEngine::KIconEngine(const QString &iconName, KIconLoader *iconLoader, const QStringList &overlays)
    : mIconName(iconName)
    , mOverlays(overlays)
    , d(new KIconEnginePrivate{iconLoader, false, {}, {}})
{
}

KIconEngine::KIconEngine(const QString &iconName, KIconLoader *iconLoader)
    : mIconName(iconName)
    , d(new KIconEnginePrivate{iconLoader, false, {}, {}})
{
}

KIconEngine::KIconEngine(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader)
    : mIconName(iconName)
    , d(new KIconEnginePrivate{iconLoader, true, colors, {}})
{
}

KIconEngine::KIconEngine(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader, const QStringList &overlays)
    : mIconName(iconName)
    , mOverlays(overlays)
    , d(new KIconEnginePrivate{iconLoader, true, colors, {}})
{
}

KIconEngine::~KIconEngine()
{
    delete d;
}

static inline int qIconModeToKIconState(QIcon::Mode mode)
{
    switch (mode) {
    case QIcon::Normal:
        return KIconLoader::DefaultState;
    case QIcon::Active:
        return KIconLoader::ActiveState;
    case QIcon::Disabled:
        return KIconLoader::DisabledState;
    case QIcon::Selected:
        return KIconLoader::SelectedState;
    default:
        return KIconLoader::DefaultState;
    }
}

QSize KIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return QIconEngine::actualSize(size, mode, state);
}

void KIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    if (!d->mIconLoader) {
        return;
    }

    const qreal dpr = painter->device()->devicePixelRatioF();
    const QPixmap pix = createPixmap(rect.size() * dpr, dpr, mode, state);
    painter->drawPixmap(rect, pix);
}

QPixmap KIconEngine::createPixmap(const QSize &size, qreal scale, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(state)

    if (scale < 1) {
        scale = 1;
    }

    if (size.isEmpty()) {
        return QPixmap();
    }

    if (!d->mIconLoader) {
        QPixmap pm(size);
        pm.setDevicePixelRatio(scale);
        pm.fill(Qt::transparent);
        return pm;
    }

    const QSize scaledSize = size / scale;

    QString iconPath;

    const int kstate = qIconModeToKIconState(mode);
    QPixmap pix = d->mIconLoader->loadScaledIcon(mIconName,
                                                 KIconLoader::Desktop,
                                                 scale,
                                                 scaledSize,
                                                 kstate,
                                                 mOverlays,
                                                 &iconPath,
                                                 false,
                                                 d->mCustomColors ? std::make_optional(d->mColors) : std::nullopt);

    if (!iconPath.isEmpty() && !d->mActualIconName.isEmpty()) {
        d->mActualIconName = QFileInfo(iconPath).completeBaseName();
    }

    if (pix.size() == size) {
        return pix;
    }

    QPixmap pix2(size);
    pix2.setDevicePixelRatio(scale);
    pix2.fill(QColor(0, 0, 0, 0));

    QPainter painter(&pix2);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    const QSizeF targetSize = pix.size().scaled(scaledSize, Qt::KeepAspectRatio);
    QRectF targetRect({0, 0}, targetSize);
    targetRect.moveCenter(QRectF(pix2.rect()).center() / scale);
    painter.drawPixmap(targetRect, pix, pix.rect());

    return pix2;
}

QPixmap KIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    return createPixmap(size, 1 /*scale*/, mode, state);
}

QPixmap KIconEngine::scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale)
{
    return createPixmap(size, scale, mode, state);
}

QString KIconEngine::iconName()
{
    if (!d->mActualIconName.isEmpty()) {
        return d->mActualIconName;
    }

    if (!d->mIconLoader) {
        return QString();
    }

    const QString iconPath = KIconLoaderPrivate::get(d->mIconLoader)->preferredIconPath(mIconName);
    if (iconPath.isEmpty()) {
        return QString();
    }

    d->mActualIconName = QFileInfo(iconPath).completeBaseName();
    return d->mActualIconName;
}

Q_GLOBAL_STATIC_WITH_ARGS(QList<QSize>,
                          sSizes,
                          (QList<QSize>() << QSize(16, 16) << QSize(22, 22) << QSize(32, 32) << QSize(48, 48) << QSize(64, 64) << QSize(128, 128)
                                          << QSize(256, 256)))

QList<QSize> KIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode);
    Q_UNUSED(state);

    if (!d->mIconLoader) {
        return QList<QSize>();
    }

    const bool found = d->mIconLoader->hasIcon(mIconName);
    return found ? *sSizes : QList<QSize>();
}

QString KIconEngine::key() const
{
    return QStringLiteral("KIconEngine");
}

QIconEngine *KIconEngine::clone() const
{
    return new KIconEngine(mIconName, d->mIconLoader, mOverlays);
}

bool KIconEngine::read(QDataStream &in)
{
    in >> mIconName >> mOverlays;
    return true;
}

bool KIconEngine::write(QDataStream &out) const
{
    out << mIconName << mOverlays;
    return true;
}

bool KIconEngine::isNull()
{
    return !d->mIconLoader || !d->mIconLoader->hasIcon(mIconName);
}
