/*
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMOUILINGMODESKTOPUNITS_H
#define LINGMOUILINGMODESKTOPUNITS_H

#include <QObject>
#include <QPropertyNotifier>

#include <LingmoUI/Platform/Units>

class AnimationSpeedProvider;

class LingmoDesktopUnits : public LingmoUI::Platform::Units
{
    Q_OBJECT

public:
    explicit LingmoDesktopUnits(QObject *parent = nullptr);

    void updateAnimationSpeed();

private:
    std::unique_ptr<AnimationSpeedProvider> m_animationSpeedProvider;
    QPropertyNotifier m_notifier;
};

#endif
