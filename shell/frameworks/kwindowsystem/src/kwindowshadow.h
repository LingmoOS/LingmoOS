/*
    SPDX-FileCopyrightText: 2019 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KWINDOWSHADOW_H
#define KWINDOWSHADOW_H

#include "kwindowsystem_export.h"

#include <QImage>
#include <QMargins>
#include <QSharedPointer>
#include <QWindow>

class KWindowShadowPrivate;
class KWindowShadowTilePrivate;

/**
 * The KWindowShadowTile class provides a platform-indendent shadow tile representation.
 */
class KWINDOWSYSTEM_EXPORT KWindowShadowTile
{
public:
    using Ptr = QSharedPointer<KWindowShadowTile>;

    KWindowShadowTile();
    ~KWindowShadowTile();

    /**
     * Returns the image stored in the KWindowShadowTile.
     */
    QImage image() const;

    /**
     * Sets the image on the KWindowShadowTile.
     *
     * Notice that once the native platform resources have been allocated for the tile, you are
     * not allowed to change the image. In order to do so, you need to create a new tile.
     */
    void setImage(const QImage &image);

    /**
     * Returns @c true if the platform resources associated with the tile have been allocated.
     */
    bool isCreated() const;

    /**
     * Allocates the native platform resources associated with the KWindowShadowTile.
     *
     * Normally it should not be necessary to call this method as KWindowShadow will implicitly
     * call create() on your behalf.
     *
     * Returns @c true if the creation succeeded, otherwise returns @c false.
     */
    bool create();

private:
    QScopedPointer<KWindowShadowTilePrivate> d;

    friend class KWindowShadowTilePrivate;
};

/**
 * The KWindowShadow class represents a drop-shadow that is drawn by the compositor.
 *
 * The KWindowShadow is composed of multiple tiles. The top left tile, the top right tile, the bottom
 * left tile, and the bottom right tile are rendered as they are. The top tile and the bottom tile are
 * stretched in x direction; the left tile and the right tile are stretched in y direction. Several
 * KWindowShadow objects can share shadow tiles to reduce memory usage. You have to specify padding()
 * along the shadow tiles. The padding values indicate how much the KWindowShadow sticks outside the
 * decorated window.
 *
 * Once the KWindowShadow is created, you're not allowed to attach or detach any shadow tiles, change
 * padding(), or change window(). In order to do so, you have to destroy() the shadow first, update
 * relevant properties, and create() the shadow again.
 */
class KWINDOWSYSTEM_EXPORT KWindowShadow : public QObject
{
    Q_OBJECT

public:
    explicit KWindowShadow(QObject *parent = nullptr);
    ~KWindowShadow() override;

    /**
     * Returns the left tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr leftTile() const;

    /**
     * Attaches the left @p tile to the KWindowShadow.
     */
    void setLeftTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the top-left tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr topLeftTile() const;

    /**
     * Attaches the top-left @p tile to the KWindowShadow.
     */
    void setTopLeftTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the top tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr topTile() const;

    /**
     * Attaches the top @p tile to the KWindowShadow.
     */
    void setTopTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the top-right tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr topRightTile() const;

    /**
     * Attaches the top-right @p tile to the KWindowShadow.
     */
    void setTopRightTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the right tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr rightTile() const;

    /**
     * Attaches the right @p tile to the KWindowShadow.
     */
    void setRightTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the bottom-right tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr bottomRightTile() const;

    /**
     * Attaches the bottom-right tile to the KWindowShadow.
     */
    void setBottomRightTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the bottom tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr bottomTile() const;

    /**
     * Attaches the bottom @p tile to the KWindowShadow.
     */
    void setBottomTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the bottom-left tile attached to the KWindowShadow.
     */
    KWindowShadowTile::Ptr bottomLeftTile() const;

    /**
     * Attaches the bottom-left @p tile to the KWindowShadow.
     */
    void setBottomLeftTile(KWindowShadowTile::Ptr tile);

    /**
     * Returns the padding of the KWindowShadow.
     *
     * The padding values specify the visible extents of the shadow. The top left tile is rendered
     * with an offset of -padding().left() and -padding().top().
     */
    QMargins padding() const;

    /**
     * Sets the padding on the KWindowShadow.
     *
     * If the padding values are smaller than the sizes of the shadow tiles, then the shadow will
     * overlap with the window() and will be rendered behind window(). E.g. if all padding values
     * are set to 0, then the shadow will be completely occluded by the window().
     */
    void setPadding(const QMargins &padding);

    /**
     * Returns the window behind which the KWindowShadow will be rendered.
     */
    QWindow *window() const;

    /**
     * Sets the window behind which the KWindowShadow will be rendered.
     *
     * Note that the KWindowShadow does not track the platform surface. If for whatever reason the
     * native platform surface is deleted and then created, you must to destroy() the shadow and
     * create() it again yourself.
     */
    void setWindow(QWindow *window);

    /**
     * Returns @c true if the platform resources associated with the shadow have been allocated.
     */
    bool isCreated() const;

    /**
     * Allocates the platform resources associated with the KWindowShadow.
     *
     * Once the native platform resources have been allocated, you're not allowed to attach or
     * detach shadow tiles, change the padding or the target window. If you want to do so, you
     * must destroy() the shadow, change relevant attributes and call create() again.
     *
     * Returns @c true if the creation succeeded, otherwise returns @c false.
     */
    bool create();

    /**
     * Releases the platform resources associated with the KWindowShadow.
     *
     * Calling destroy() after window() had been destroyed will result in a no-op.
     */
    void destroy();

private:
    QScopedPointer<KWindowShadowPrivate> d;
};

#endif // KWINDOWSHADOW_H
