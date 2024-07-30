/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KWINDOWEFFECTS_DUMMY_P_H
#define KWINDOWEFFECTS_DUMMY_P_H
#include "kwindoweffects_p.h"

class KWindowEffectsPrivateDummy : public KWindowEffectsPrivate
{
public:
    KWindowEffectsPrivateDummy();
    ~KWindowEffectsPrivateDummy() override;
    bool isEffectAvailable(KWindowEffects::Effect effect) override;
    void slideWindow(QWindow *window, KWindowEffects::SlideFromLocation location, int offset) override;
    void enableBlurBehind(QWindow *window, bool enable = true, const QRegion &region = QRegion()) override;
    void enableBackgroundContrast(QWindow *window,
                                  bool enable = true,
                                  qreal contrast = 1,
                                  qreal intensity = 1,
                                  qreal saturation = 1,
                                  const QRegion &region = QRegion()) override;
};

#endif
