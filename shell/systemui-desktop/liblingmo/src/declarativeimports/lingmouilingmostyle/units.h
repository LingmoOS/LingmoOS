/*
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LINGMOUILINGMODESKTOPUNITS_H
#define LINGMOUILINGMODESKTOPUNITS_H

#include <QObject>

#include <LingmoUI/Platform/Units>

#include <KConfigWatcher>

class Units : public LingmoUI::Platform::Units
{
    Q_OBJECT

public:
    explicit Units(QObject *parent = nullptr);

    void updateAnimationSpeed();

private:
    KConfigWatcher::Ptr m_animationSpeedWatcher;
};

#endif
