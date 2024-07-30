/*
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTATEFULBRUSH_H
#define KSTATEFULBRUSH_H

#include "kcolorscheme.h"

#include <memory>

class KStatefulBrushPrivate;

/**
 * @class KStatefulBrush kstatefulbrush.h KStatefulBrush
 * A container for a "state-aware" brush.
 *
 * KStatefulBrush provides an easy and safe way to store a color for use in a
 * user interface. It is "safe" both in that it will make it easy to deal with
 * widget states in a correct manner, and that it insulates you against changes
 * in QPalette::ColorGroup.
 *
 * Basically, a stateful brush is used to cache a particular "color" from the
 * KDE system palette (usually, one which does not live in QPalette). When you
 * are ready to draw using the brush, you use the current state to retrieve the
 * appropriate brush.
 *
 * Stateful brushes can also be used to apply state effects to arbitrary
 * brushes, for example when working with a application specific user-defined
 * color palette.
 *
 * @note As of Qt 4.3, QPalette::ColorGroup is missing a state for disabled
 * widgets in an inactive window. Hopefully Trolltech will fix this bug, at
 * which point KColorScheme and KStatefulBrush will be updated to recognize the
 * new state. Using KStatefulBrush will allow your application to inherit these
 * changes "for free", without even recompiling.
 */
class KCOLORSCHEME_EXPORT KStatefulBrush
{
public:
    /**
     * Construct a "default" stateful brush. For such an instance, all
     * overloads of KStatefulBrush::brush will return a default brush (i.e.
     * <tt>QBrush()</tt>).
     */
    explicit KStatefulBrush();

    /**
     * Construct a stateful brush from given color set and foreground role,
     * using the colors from the given KConfig.
     * If null, the application's color scheme is used (either the system
     * default, or one set by KColorSchemeManager).
     */
    explicit KStatefulBrush(KColorScheme::ColorSet, KColorScheme::ForegroundRole, KSharedConfigPtr = KSharedConfigPtr());

    /**
     * Construct a stateful brush from given color set and background role,
     * using the colors from the given KConfig (if null, the application's
     * colors are used).
     */
    explicit KStatefulBrush(KColorScheme::ColorSet, KColorScheme::BackgroundRole, KSharedConfigPtr = KSharedConfigPtr());

    /**
     * Construct a stateful brush from given color set and decoration role,
     * using the colors from the given KConfig (if null, the application's
     * colors are used).
     */
    explicit KStatefulBrush(KColorScheme::ColorSet, KColorScheme::DecorationRole, KSharedConfigPtr = KSharedConfigPtr());

    /**
     * Construct a stateful background brush from a specified QBrush (or
     * QColor, via QBrush's implicit constructor). The various states are
     * determined from the base QBrush (which fills in the Active state)
     * according to the same rules used to build stateful color schemes from
     * the system color scheme. The state effects from the given KConfig are
     * used (if null, the application's state effects are used).
     */
    explicit KStatefulBrush(const QBrush &, KSharedConfigPtr = KSharedConfigPtr());

    /**
     * Construct a stateful foreground/decoration brush from a specified
     * QBrush (or QColor, via QBrush's implicit constructor). The various
     * states are determined from the base QBrush (which fills in the Active
     * state) according to the same rules used to build stateful color schemes
     * from the system color scheme. The state effects from the given KConfig
     * are used (if null, the application's state effects are used).
     *
     * @param background The background brush (or color) corresponding to the
     * KColorScheme::NormalBackground role and QPalette::Active state for this
     * foreground/decoration color.
     */
    explicit KStatefulBrush(const QBrush &, const QBrush &background, KSharedConfigPtr = KSharedConfigPtr());

    /** Construct a copy of another KStatefulBrush. */
    KStatefulBrush(const KStatefulBrush &);

    /** Destructor */
    ~KStatefulBrush();

    /** Standard assignment operator */
    KStatefulBrush &operator=(const KStatefulBrush &);

    /**
     * Retrieve the brush for the specified widget state. This is used when you
     * know explicitly what state is wanted. Otherwise one of overloads is
     * often more convenient.
     */
    QBrush brush(QPalette::ColorGroup) const;

    /**
     * Retrieve the brush, using a QPalette reference to determine the correct
     * state. Use when your painting code has easy access to the QPalette that
     * it is supposed to be using. The state used in this instance is the
     * currentColorGroup of the palette.
     */
    QBrush brush(const QPalette &) const;

private:
    std::unique_ptr<KStatefulBrushPrivate> d;
};

Q_DECLARE_METATYPE(KStatefulBrush) /* so we can pass it in QVariant's */

#endif
