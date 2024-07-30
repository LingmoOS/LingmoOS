/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmunsdon@kde.org>
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "plasmadesktopunits.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include "animationspeedprovider.h"

namespace
{
constexpr int defaultLongDuration = 200;
}

PlasmaDesktopUnits::PlasmaDesktopUnits(QObject *parent)
    : Kirigami::Platform::Units(parent)
#if defined(Q_OS_WIN)
    , m_animationSpeedProvider(new WindowsAnimationSpeedProvider)
#elif defined(Q_OS_UNIX)
    , m_animationSpeedProvider(new KConfigAnimationSpeedProvider)
#endif
{
    m_notifier = m_animationSpeedProvider->animationSpeedModifier().addNotifier([this] {
        updateAnimationSpeed();
    });
    updateAnimationSpeed();
}

// Copy from plasma-framework/src/declarativeimports/core/units.cpp, since we don't want to depend on plasma-framework here
void PlasmaDesktopUnits::updateAnimationSpeed()
{
    // Read the old longDuration value for compatibility
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Units"));
    int longDuration = cfg.readEntry("longDuration", defaultLongDuration);

    const qreal animationSpeedModifier = m_animationSpeedProvider->animationSpeedModifier().value();
    longDuration = qRound(longDuration * animationSpeedModifier);

    // Animators with a duration of 0 do not fire reliably
    // see Bug 357532 and QTBUG-39766
    longDuration = qMax(1, longDuration);

    setVeryShortDuration(longDuration / 4);
    setShortDuration(longDuration / 2);
    setLongDuration(longDuration);
    setVeryLongDuration(longDuration * 2);
}

#include "moc_plasmadesktopunits.cpp"
