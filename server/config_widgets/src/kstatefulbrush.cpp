/*
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kstatefulbrush.h"

#include "kcolorschemehelpers_p.h"

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 84)
#include <QWidget>
#endif

#include <array>

class KStatefulBrushPrivate
{
public:
    std::array<QBrush, QPalette::NColorGroups> brushes;
};

KStatefulBrush::KStatefulBrush()
    : d(std::make_unique<KStatefulBrushPrivate>())
{
}

KStatefulBrush::~KStatefulBrush() = default;

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::ForegroundRole role, KSharedConfigPtr config)
    : KStatefulBrush()
{
    d->brushes[QPalette::Active] = KColorScheme(QPalette::Active, set, config).foreground(role);
    d->brushes[QPalette::Disabled] = KColorScheme(QPalette::Disabled, set, config).foreground(role);
    d->brushes[QPalette::Inactive] = KColorScheme(QPalette::Inactive, set, config).foreground(role);
}

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::BackgroundRole role, KSharedConfigPtr config)
    : KStatefulBrush()

{
    d->brushes[QPalette::Active] = KColorScheme(QPalette::Active, set, config).background(role);
    d->brushes[QPalette::Disabled] = KColorScheme(QPalette::Disabled, set, config).background(role);
    d->brushes[QPalette::Inactive] = KColorScheme(QPalette::Inactive, set, config).background(role);
}

KStatefulBrush::KStatefulBrush(KColorScheme::ColorSet set, KColorScheme::DecorationRole role, KSharedConfigPtr config)
    : KStatefulBrush()
{
    d->brushes[QPalette::Active] = KColorScheme(QPalette::Active, set, config).decoration(role);
    d->brushes[QPalette::Disabled] = KColorScheme(QPalette::Disabled, set, config).decoration(role);
    d->brushes[QPalette::Inactive] = KColorScheme(QPalette::Inactive, set, config).decoration(role);
}

KStatefulBrush::KStatefulBrush(const QBrush &brush, KSharedConfigPtr config)
    : KStatefulBrush()
{
    if (!config) {
        config = defaultConfig();
    }
    d->brushes[QPalette::Active] = brush;
    d->brushes[QPalette::Disabled] = StateEffects(QPalette::Disabled, config).brush(brush);
    d->brushes[QPalette::Inactive] = StateEffects(QPalette::Inactive, config).brush(brush);
}

KStatefulBrush::KStatefulBrush(const QBrush &brush, const QBrush &background, KSharedConfigPtr config)
    : KStatefulBrush()
{
    if (!config) {
        config = defaultConfig();
    }
    d->brushes[QPalette::Active] = brush;
    d->brushes[QPalette::Disabled] = StateEffects(QPalette::Disabled, config).brush(brush, background);
    d->brushes[QPalette::Inactive] = StateEffects(QPalette::Inactive, config).brush(brush, background);
}

KStatefulBrush::KStatefulBrush(const KStatefulBrush &other)
    : KStatefulBrush()
{
    d->brushes[QPalette::Active] = other.d->brushes[QPalette::Active];
    d->brushes[QPalette::Disabled] = other.d->brushes[QPalette::Disabled];
    d->brushes[QPalette::Inactive] = other.d->brushes[QPalette::Inactive];
}

KStatefulBrush &KStatefulBrush::operator=(const KStatefulBrush &other)
{
    d->brushes[QPalette::Active] = other.d->brushes[QPalette::Active];
    d->brushes[QPalette::Disabled] = other.d->brushes[QPalette::Disabled];
    d->brushes[QPalette::Inactive] = other.d->brushes[QPalette::Inactive];
    return *this;
}

QBrush KStatefulBrush::brush(QPalette::ColorGroup state) const
{
    if (state >= QPalette::Active && state < QPalette::NColorGroups) {
        return d->brushes[state];
    } else {
        return d->brushes[QPalette::Active];
    }
}

QBrush KStatefulBrush::brush(const QPalette &pal) const
{
    return brush(pal.currentColorGroup());
}

#if KCONFIGWIDGETS_BUILD_DEPRECATED_SINCE(5, 84)
QBrush KStatefulBrush::brush(const QWidget *widget) const
{
    if (widget) {
        return brush(widget->palette());
    } else {
        return QBrush();
    }
}
#endif
