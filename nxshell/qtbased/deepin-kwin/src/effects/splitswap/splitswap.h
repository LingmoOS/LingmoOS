/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef EFFECTS_SPLITSWAP_H_
#define EFFECTS_SPLITSWAP_H_

#include <kwineffects.h>
#include <kwineffectsex.h>
#include "../utils/common.h"

namespace KWin
{
class SplitSwapEffect : public Effect
{
    Q_OBJECT
public:
    enum SwapClientIndex { First = 1, Second };
    SplitSwapEffect();
    virtual ~SplitSwapEffect();
    void reconfigure(ReconfigureFlags flags) override;
    void prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintScreen(int mask, const QRegion &region, ScreenPaintData &data) override;
    void postPaintScreen() override;
    void prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime) override;
    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) override;

    bool isActive() const override;

private:
    bool isRelevantWithPresentWindows(EffectWindow *w) const;
    void resetWinPos(EffectWindow *w, QuickTileMode mode);
    int paintWinPos(EffectWindow *w, QuickTileMode mode, int calculationMethod);
    void initTextureMask();
public Q_SLOTS:
    void setActive(bool active);
    void toggleActive() {
        setActive(!m_activated);
    }
private Q_SLOTS:
    void onSwapWindow(KWin::EffectWindow *, int index);
private:
    QRect                       m_workarea;
    std::chrono::milliseconds   m_duration;
    TimeLine                    m_animationTime;
    bool                        m_activated = false;
    bool                        m_isSwap = true;
    bool                        m_isFinish = false;
    int                         m_leftMode = int(QuickTileFlag::Left);
    int                         m_rightMode = int(QuickTileFlag::Right);
    EffectWindow                *m_dragEffectWin = nullptr;
    EffectScreen                *m_dragScreen = nullptr;
    QuickTileMode               m_currentMode;
    std::chrono::milliseconds   lastPresentTime;
    std::map<QString, GLTexture*> m_bgTextures;
};
}

#endif