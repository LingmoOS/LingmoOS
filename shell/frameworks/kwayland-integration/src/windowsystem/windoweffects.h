/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINDOWEFFECTS_H
#define WINDOWEFFECTS_H
#include <kwindowsystem_version.h>
#include <private/kwindoweffects_p.h>

#include <QHash>
#include <QObject>
#include <QPointer>

namespace KWayland
{
namespace Client
{
class Compositor;
class ConnectionThread;
}
}

class BlurManager;
class Blur;
class ContrastManager;
class Contrast;
class SlideManager;

class WindowEffects : public QObject, public KWindowEffectsPrivateV2
{
    Q_OBJECT
public:
    WindowEffects();
    ~WindowEffects() override;

    static QWindow *windowForId(WId);

    bool eventFilter(QObject *watched, QEvent *event) override;
    void trackWindow(QWindow *window);
    void releaseWindow(QWindow *window);

    bool isEffectAvailable(KWindowEffects::Effect effect) override;
    void slideWindow(WId id, KWindowEffects::SlideFromLocation location, int offset) override;
    QList<QSize> windowSizes(const QList<WId> &ids) override;

    void presentWindows(WId controller, const QList<WId> &ids) override;
    void presentWindows(WId controller, int desktop = NET::OnAllDesktops) override;
    void highlightWindows(WId controller, const QList<WId> &ids) override;
    void enableBlurBehind(WId winId, bool enable = true, const QRegion &region = QRegion()) override;
    void enableBackgroundContrast(WId winId,
                                  bool enable = true,
                                  qreal contrast = 1,
                                  qreal intensity = 1,
                                  qreal saturation = 1,
                                  const QRegion &region = QRegion()) override;
    void markAsDashboard(WId window) override;
    void setBackgroundFrost(QWindow *window, QColor color, const QRegion &region = QRegion()) override;

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
