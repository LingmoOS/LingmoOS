/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#include "decorationsettingsprivate.h"
#include <QFontDatabase>

namespace KDecoration2
{
class Q_DECL_HIDDEN DecorationSettingsPrivate::Private
{
public:
    explicit Private(DecorationSettings *settings);
    DecorationSettings *settings;
    int gridUnit = -1;
    int smallSpacing = -1;
    int largeSpacing = -1;
};

DecorationSettingsPrivate::Private::Private(DecorationSettings *settings)
    : settings(settings)
{
}

DecorationSettingsPrivate::DecorationSettingsPrivate(DecorationSettings *parent)
    : d(new Private(parent))
{
}

DecorationSettingsPrivate::~DecorationSettingsPrivate()
{
}

DecorationSettings *DecorationSettingsPrivate::decorationSettings()
{
    return d->settings;
}

const DecorationSettings *DecorationSettingsPrivate::decorationSettings() const
{
    return d->settings;
}

QFont DecorationSettingsPrivate::font() const
{
    return QFontDatabase::systemFont(QFontDatabase::TitleFont);
}

QFontMetricsF DecorationSettingsPrivate::fontMetrics() const
{
    return QFontMetricsF(font());
}

int DecorationSettingsPrivate::gridUnit() const
{
    return d->gridUnit;
}

int DecorationSettingsPrivate::smallSpacing() const
{
    return d->smallSpacing;
}

int DecorationSettingsPrivate::largeSpacing() const
{
    return d->largeSpacing;
}

void DecorationSettingsPrivate::setGridUnit(int unit)
{
    d->gridUnit = unit;
}

void DecorationSettingsPrivate::setLargeSpacing(int spacing)
{
    d->largeSpacing = spacing;
}

void DecorationSettingsPrivate::setSmallSpacing(int spacing)
{
    d->smallSpacing = spacing;
}

}
