/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINDOWEFFECTS_H
#define WINDOWEFFECTS_H
#include "kwindoweffects_p.h"
#include <kwindowsystem_version.h>

#include <QHash>
#include <QObject>
#include <QPointer>

class BlurManager;
class Blur;
class ContrastManager;
class Contrast;
class SlideManager;

class WindowEffects : public QObject, public KWindowEffectsPrivate
{
    Q_OBJECT
public:
    WindowEffects();
    ~WindowEffects() override;

    bool eventFilter(QObject *watched, QEvent *event) override;
    void trackWindow(QWindow *window);
    void releaseWindow(QWindow *window);

    bool isEffectAvailable(KWindowEffects::Effect effect) override;
    void slideWindow(QWindow *window, KWindowEffects::SlideFromLocation location, int offset) override;
    void enableBlurBehind(QWindow *window, bool enable = true, const QRegion &region = QRegion()) override;
    void enableBackgroundContrast(QWindow *window,
                                  bool enable = true,
                                  qreal contrast = 1,
                                  qreal intensity = 1,
                                  qreal saturation = 1,
                                  const QRegion &region = QRegion()) override;

private:
    void installContrast(QWindow *window, bool enable = true, qreal contrast = 1, qreal intensity = 1, qreal saturation = 1, const QRegion &region = QRegion());
    void installBlur(QWindow *window, bool enable, const QRegion &region);
    void installSlide(QWindow *window, KWindowEffects::SlideFromLocation location, int offset);

    void resetBlur(QWindow *window, Blur *blur = nullptr);
    void resetContrast(QWindow *window, Contrast *contrast = nullptr);

    QHash<QWindow *, QList<QMetaObject::Connection>> m_windowWatchers;
    QHash<QWindow *, QRegion> m_blurRegions;
    struct BackgroundContrastData {
        qreal contrast = 1;
        qreal intensity = 1;
        qreal saturation = 1;
        QRegion region;
    };
    QHash<QWindow *, BackgroundContrastData> m_backgroundConstrastRegions;
    QHash<QWindow *, QPointer<Blur>> m_blurs;
    QHash<QWindow *, QPointer<Contrast>> m_contrasts;
    struct SlideData {
        KWindowEffects::SlideFromLocation location;
        int offset;
    };
    QHash<QWindow *, SlideData> m_slideMap;
    BlurManager *m_blurManager;
    ContrastManager *m_contrastManager;
    SlideManager *m_slideManager;
};

#endif
