/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once
#include "decorationbutton.h"
#include <functional>
#include <kdecoration2/kdecoration2_export.h>

class QPainter;

namespace KDecoration2
{
class Decoration;
class DecorationButtonGroupPrivate;

/**
 * @brief Helper class to layout DecorationButton.
 *
 * A Decoration normally has two groups of DecorationButtons: one left of the caption and one
 * right of the caption. The DecorationButtonGroup helps in positioning the DecorationButtons in
 * these groups and to update the position of each of the DecorationButtons whenever the state
 * changes in a way that they should be repositioned.
 *
 * A DecorationButtonGroup is a visual layout element not accepting input events. As a visual
 * element it provides a paint method allowing a sub class to provide custom painting for the
 * DecorationButtonGroup.
 **/
class KDECORATIONS2_EXPORT DecorationButtonGroup : public QObject
{
    Q_OBJECT
    /**
     * The spacing to use between the DecorationButtons
     **/
    Q_PROPERTY(qreal spacing READ spacing WRITE setSpacing NOTIFY spacingChanged)
    /**
     * The geometry of the DecorationButtonGroup in Decoration-local coordinates.
     * The size of the DecorationButtonGroup depends on the sizes of the individual
     * DecorationButtons and the spacing.
     **/
    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
    // TODO: pos must consider whether it's left or right
    /**
     * The top left Position of the DecorationButtonGroup. This property needs to be
     * changed to reposition the DecorationButtonGroup. An update should normally be
     * triggered after e.g. a state change like maximization.
     **/
    Q_PROPERTY(QPointF pos READ pos WRITE setPos NOTIFY posChanged)
public:
    enum class Position {
        Left,
        Right,
    };
    explicit DecorationButtonGroup(Position type,
                                   Decoration *parent,
                                   std::function<DecorationButton *(DecorationButtonType, Decoration *, QObject *)> buttonCreator);
    explicit DecorationButtonGroup(Decoration *parent);
    ~DecorationButtonGroup() override;

    /**
     * Paints the DecorationButtonGroup. This method should normally be invoked from the
     * Decoration's paint method. Base implementation just calls the paint method on each
     * of the DecorationButtons. Overwriting sub classes need to either call the base
     * implementation or ensure that the DecorationButtons are painted.
     *
     * @param painter The QPainter which is used to paint this DecorationButtonGroup
     * @param repaintArea The area which is going to be repainted in Decoration coordinates
     **/
    virtual void paint(QPainter *painter, const QRect &repaintArea);

    Decoration *decoration() const;

    qreal spacing() const;
    void setSpacing(qreal spacing);

    QRectF geometry() const;
    QPointF pos() const;
    void setPos(const QPointF &pos);

    /**
     * Adds @p button to the DecorationButtonGroup and triggers a re-layout of all
     * DecorationButtons.
     **/
    void addButton(DecorationButton *button);
    /**
     * Removes @p button from the DecorationButtonGroup and triggers a re-layout of all
     * DecorationButtons.
     **/
    void removeButton(DecorationButton *button);
    /**
     * Removes all DecorationButtons with @p type from the DecorationButtonGroup and
     * triggers a re-layout of all DecorationButtons.
     **/
    void removeButton(DecorationButtonType type);
    /**
     * @returns @c true if the DecorationButtonGroup contains a DecorationButton of @p type
     **/
    bool hasButton(DecorationButtonType type) const;
    /**
     * @returns All DecorationButtons in this DecorationButtonGroup
     **/
    QList<DecorationButton *> buttons() const;

Q_SIGNALS:
    void spacingChanged(qreal);
    void geometryChanged(const QRectF &);
    void posChanged(const QPointF &);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace
