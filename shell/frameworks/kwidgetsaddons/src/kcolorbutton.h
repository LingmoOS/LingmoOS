/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Martin Jones <mjones@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORBUTTON_H
#define KCOLORBUTTON_H

#include <kwidgetsaddons_export.h>

#include <QPushButton>
#include <memory>

class KColorButtonPrivate;
/**
 * @class KColorButton kcolorbutton.h KColorButton
 *
 * @short A pushbutton to display or allow user selection of a color.
 *
 * This widget can be used to display or allow user selection of a color.
 *
 * \image html kcolorbutton.png "KColorButton Widget"
 *
 * @see QColorDialog
 */
class KWIDGETSADDONS_EXPORT KColorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY changed USER true)
    Q_PROPERTY(QColor defaultColor READ defaultColor WRITE setDefaultColor)
    Q_PROPERTY(bool alphaChannelEnabled READ isAlphaChannelEnabled WRITE setAlphaChannelEnabled)

public:
    /**
     * Creates a color button.
     */
    explicit KColorButton(QWidget *parent = nullptr);

    /**
     * Creates a color button with an initial color @p c.
     */
    explicit KColorButton(const QColor &c, QWidget *parent = nullptr);

    /**
     * Creates a color button with an initial color @p c and default color @p defaultColor.
     */
    KColorButton(const QColor &c, const QColor &defaultColor, QWidget *parent = nullptr);

    ~KColorButton() override;

    /**
     * Returns the currently chosen color.
     */
    QColor color() const;

    /**
     * Sets the current color to @p c.
     */
    void setColor(const QColor &c);

    /**
     * When set to true, allow the user to change the alpha component
     * of the color. The default value is false.
     * @since 4.5
     */
    void setAlphaChannelEnabled(bool alpha);

    /**
     * Returns true if the user is allowed to change the alpha component.
     * @since 4.5
     */
    bool isAlphaChannelEnabled() const;

    /**
     * Returns the default color or an invalid color
     * if no default color is set.
     */
    QColor defaultColor() const;

    /**
     * Sets the default color to @p c.
     */
    void setDefaultColor(const QColor &c);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

Q_SIGNALS:
    /**
     * Emitted when the color of the widget
     * is changed, either with setColor() or via user selection.
     */
    void changed(const QColor &newColor);

protected:
    void paintEvent(QPaintEvent *pe) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    std::unique_ptr<class KColorButtonPrivate> const d;
};

#endif
