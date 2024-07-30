/*

    This file is part of the KDE project, module kdecore.
    SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
    SPDX-FileCopyrightText: 2000 Antonio Larrosa <larrosa@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KICONCOLORS_H
#define KICONCOLORS_H

#include "kiconloader.h"
#include <QPalette>
#include <QSharedDataPointer>

class KIconColorsPrivate;

/**
 * @class KIconColors
 *
 * Sepecifies which colors will be used when recoloring icons as its stylesheet.
 *
 * KIconLoader supports re-coloring svg icons based on a set of colors. This
 * class will define them.
 *
 * @see KIconEngine
 * @see KDE::icon
 */
class KICONTHEMES_EXPORT KIconColors
{
public:
    /**
     * Will fill the colors based on the default QPalette() constructor.
     */
    KIconColors();

    /**
     * Makes all the color property be @p colors
     */
    explicit KIconColors(const QColor &colors);

    /**
     * Uses @palette to define text, highlight, highlightedText, accent and background.
     * The rest being positiveText, negativeText and neutralText are filled from
     * KColorScheme(QPalette::Active, KColorScheme::Window);
     */
    explicit KIconColors(const QPalette &palette);

    KIconColors(const KIconColors &other);
    ~KIconColors();
    KIconColors operator=(const KIconColors &other);

    QColor text() const;
    QColor highlight() const;
    QColor highlightedText() const;
    QColor accent() const;
    QColor background() const;
    QColor neutralText() const;
    QColor positiveText() const;
    QColor negativeText() const;
    QColor activeText() const;

    void setText(const QColor &color);
    void setHighlight(const QColor &color);
    void setHighlightedText(const QColor &color);
    void setAccent(const QColor &color);
    void setBackground(const QColor &color);
    void setNeutralText(const QColor &color);
    void setPositiveText(const QColor &color);
    void setNegativeText(const QColor &color);
    void setActiveText(const QColor& color);

protected:
    /**
     * @returns a CSS stylesheet to be used SVG icon files.
     * @param state defines the state we are rendering the stylesheet for
     *
     * Specifies: .ColorScheme-Text, .ColorScheme-Background, .ColorScheme-Highlight,
     * .ColorScheme-HighlightedText, .ColorScheme-PositiveText, .ColorScheme-NeutralText
     * .ColorScheme-NegativeText, .ColorScheme-ActiveText, .ColorScheme-Complement,
     * .ColorScheme-Contrast, .ColorScheme-Accent,
     */
    QString stylesheet(KIconLoader::States state) const;

private:
    Q_DECLARE_PRIVATE(KIconColors)
    friend class KIconLoaderPrivate;

    QExplicitlySharedDataPointer<KIconColorsPrivate> d_ptr;
};

#endif
