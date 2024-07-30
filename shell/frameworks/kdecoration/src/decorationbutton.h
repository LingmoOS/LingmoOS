/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decorationdefines.h"
#include <kdecoration2/kdecoration2_export.h>

#include <QObject>
#include <QRect>

class QHoverEvent;
class QMouseEvent;
class QPainter;
class QWheelEvent;

namespace KDecoration2
{
class DecorationButtonPrivate;
class Decoration;

/**
 * @brief A button to be used in a Decoration.
 *
 * The DecorationButton is a simple Button which can be used (but doesn't have to) in a Decoration.
 * It takes care of the input handling and triggers the correct state change methods on the
 * Decoration.
 *
 * This simplifies the handling of DecorationButtons. A Decoration implementation just needs to
 * subclass DecorationButton and implement the paint method. Everything else is handled by the
 * DecorationButton.
 *
 * For positioning the DecorationButtons it's recommended to use a DecorationButtonGroup.
 *
 * @see Decoration
 * @see DecorationButtonGroup
 **/
class KDECORATIONS2_EXPORT DecorationButton : public QObject
{
    Q_OBJECT
    /**
     * Whether the DecorationButton is visible. By default this is @c true, OnAllDesktops and
     * QuickHelp depend on the DecoratedClient's state.
     **/
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibilityChanged)
    /**
     * Whether the DecorationButton is currently pressed.
     **/
    Q_PROPERTY(bool pressed READ isPressed NOTIFY pressedChanged)
    /**
     * Whether the DecorationButton is currently hovered.
     **/
    Q_PROPERTY(bool hovered READ isHovered NOTIFY hoveredChanged)
    /**
     * Whether the DecorationButton is enabled. Only an enabled button accepts hover and mouse
     * press events.
     **/
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    /**
     * Whether the DecorationButton can be checked. This is used for state aware DecorationButtons
     * like Maximize, Shade, KeepAbove, KeepBelow and OnAllDesktops.
     **/
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable NOTIFY checkableChanged)
    /**
     * Whether the DecorationButton is checked. A DecorationButton can only be checked if the
     * DecorationButton is checkable. Note: the checked state is not changed by clicking the
     * DecorationButton. It gets changed if the DecoratedClient changes it's state, though.
     **/
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked NOTIFY checkedChanged)
    /**
     * The geometry of the DecorationButton in Decoration-local coordinates.
     **/
    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
    /**
     * The mouse buttons the DecorationButton accepts. By default the Qt::LeftButton gets accepted,
     * for some types more buttons are accepted.
     **/
    Q_PROPERTY(Qt::MouseButtons acceptedButtons READ acceptedButtons WRITE setAcceptedButtons NOTIFY acceptedButtonsChanged)
public:
    ~DecorationButton() override;

    QRectF geometry() const;
    QSizeF size() const;
    void setGeometry(const QRectF &geometry);

    bool isVisible() const;
    bool isPressed() const;
    bool isHovered() const;
    bool isEnabled() const;
    bool isChecked() const;
    bool isCheckable() const;
    DecorationButtonType type() const;

    /**
     * Returns @c true if @p pos is inside of the button, otherwise returns @c false.
     **/
    bool contains(const QPointF &pos) const;

    Qt::MouseButtons acceptedButtons() const;
    void setAcceptedButtons(Qt::MouseButtons buttons);

    /**
     * Invoked for painting this DecorationButtons. Implementing sub-classes need to implement
     * this method. The coordinate system of the QPainter is set to Decoration coordinates.
     *
     * This method will be invoked from the rendering thread.
     *
     * @param painter The QPainter to paint this DecorationButton.
     * @param repaintArea The area which is going to be repainted in Decoration coordinates
     **/
    virtual void paint(QPainter *painter, const QRect &repaintArea) = 0;

    Decoration *decoration() const;

    bool event(QEvent *event) override;

public Q_SLOTS:
    void setEnabled(bool enabled);
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    void setVisible(bool visible);

    /**
     * Schedules a repaint of the DecorationButton.
     * Calling update will eventually result in paint being invoked.
     *
     * @param rect The area to repaint in Decoration local coordinates, a null QRect updates the complete geometry
     * @see paint
     **/
    void update(const QRectF &rect);
    /**
     * Schedules a repaint of the DecorationButton.
     *
     * Overloaded method for convenience.
     **/
    void update();

Q_SIGNALS:
    void clicked(Qt::MouseButton);
    void pressed();
    void released();
    void pointerEntered();
    void pointerLeft();
    void doubleClicked();

    void pressedChanged(bool);
    void hoveredChanged(bool);
    void enabledChanged(bool);
    void checkableChanged(bool);
    void checkedChanged(bool);
    void geometryChanged(const QRectF &);
    void acceptedButtonsChanged(Qt::MouseButtons);
    void visibilityChanged(bool);

protected:
    explicit DecorationButton(DecorationButtonType type, Decoration *decoration, QObject *parent = nullptr);

    virtual void hoverEnterEvent(QHoverEvent *event);
    virtual void hoverLeaveEvent(QHoverEvent *event);
    virtual void hoverMoveEvent(QHoverEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace

#ifndef K_DOXYGEN
size_t KDECORATIONS2_EXPORT qHash(const KDecoration2::DecorationButtonType &type, size_t seed = 0);
#endif

Q_DECLARE_METATYPE(KDecoration2::DecorationButtonType)
