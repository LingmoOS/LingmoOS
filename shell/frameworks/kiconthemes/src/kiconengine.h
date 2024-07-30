/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONENGINE_H
#define KICONENGINE_H

#include "kiconthemes_export.h"
#include <QIconEngine>
#include <QPointer>

class KIconColors;
class KIconLoader;
class KIconEnginePrivate;

/**
 * @class KIconEngine kiconengine.h KIconEngine
 *
 * \short A class to provide rendering of KDE icons.
 *
 * This is mostly used to provide Qt's icon loading in plasma-integration
 *
 * Application developers should use QIcon::fromTheme instead of using it directly.
 */
class KICONTHEMES_EXPORT KIconEngine : public QIconEngine // exported for plasma-integration
{
public:
    /**
     * Constructs an icon engine for a named icon.
     *
     * @param iconName the name of the icon to load
     * @param iconLoader The icon loader that this engine is to use.
     * @param overlays Add one or more overlays to the icon. See KIconLoader::Overlays.
     *
     * @sa KIconLoader
     */
    KIconEngine(const QString &iconName, KIconLoader *iconLoader, const QStringList &overlays);

    /**
     * \overload
     */
    KIconEngine(const QString &iconName, KIconLoader *iconLoader);

    /**
     * Constructs an icon engine for a KDE named icon with a specific palette.
     *
     * @param iconName the name of the icon to load
     * @param colors defines the colors we want to be applied on this icon
     * @param iconLoader The KDE icon loader that this engine is to use.
     */
    KIconEngine(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader);

    /**
     * Constructs an icon engine for a KDE named icon with a specific palette and overlays.
     *
     * @param iconName the name of the icon to load
     * @param colors defines the colors we want to be applied on this icon
     * @param iconLoader The KDE icon loader that this engine is to use.
     * @param overlays Add one or more overlays to the icon. See KIconLoader::Overlays.
     *
     * @since 6.1
     */
    KIconEngine(const QString &iconName, const KIconColors &colors, KIconLoader *iconLoader, const QStringList &overlays);

    /**
     * Destructor.
     */
    ~KIconEngine() override;

    /// Reimplementation
    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    /// Reimplementation
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    /// Reimplementation
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override;

    /// Reimplementation
    QString iconName() override;
    /// Reimplementation
    QList<QSize> availableSizes(QIcon::Mode mode, QIcon::State state) override;

    bool isNull() override;

    QString key() const override;
    QIconEngine *clone() const override;
    bool read(QDataStream &in) override;
    bool write(QDataStream &out) const override;

private:
    // TODO KF6: move those into the d-pointer
    QPixmap createPixmap(const QSize &size, qreal scale, QIcon::Mode mode, QIcon::State state);
    QString mIconName;
    QStringList mOverlays;
    KIconEnginePrivate *const d;
};

#endif
