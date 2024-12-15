/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "maximizeeffect.h"
#include "../utils/common.h"

#include <effects.h>

namespace MaxiConsts {
    const QEasingCurve TOGGLE_MODE =  QEasingCurve::OutQuart;// AnimationMode.EASE_OUT_Expo;
    static const int FADE_DURATION = 350;
    static const qreal REBOUND_COEF = 0.85;
}

namespace KWin
{
MaximizeEffect::MaximizeEffect()
{
    connect(effects, &EffectsHandler::windowMaximizedChanged, this, &MaximizeEffect::slotWindowMaxiChanged);

    m_animationTime.setDuration(std::chrono::milliseconds(static_cast<int>(animationTime(MaxiConsts::FADE_DURATION))));
    m_animationTime.setDirection(TimeLine::Forward);
    m_animationTime.setEasingCurve(MaxiConsts::TOGGLE_MODE);
}

MaximizeEffect::~MaximizeEffect()
{
}

void MaximizeEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    if (isActive()) {
        m_animationTime.advance(presentTime);
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
    }

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(true));
    }

    effects->prePaintScreen(data, presentTime);
}

void MaximizeEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
}

void MaximizeEffect::postPaintScreen()
{
    if (m_activated && m_animationTime.done()) {
        setActive(false);
    }
    if (m_activated)
        effects->addRepaintFull();

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant());
    }
    effects->postPaintScreen();
}

void MaximizeEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime)
{
    data.mask |= PAINT_WINDOW_TRANSFORMED;

    effects->prePaintWindow(w, data, presentTime);
}

void MaximizeEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (!isActive()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (w->isWaterMark()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    auto area = effects->clientArea(MaximizeFullArea, w);
    QRegion reg(area.toRect());
    WindowPaintData d = data;
    if (m_maxiWin == w) {
        float t = m_animationTime.value();
        if (t < (effects->waylandDisplay() ? 0.2 : 0.7) && m_mode == 3) {
            GLTexture *texture = effects->getWinPreviousTexture(w);
            if (!texture)
                return;
            texture->setWrapMode(GL_CLAMP_TO_EDGE);

            QPoint p0 = QPoint(w->x(), w->y());
            QPoint p1 = m_oldGeo.toRect().topLeft();
            QPoint p = p1 - (p1-p0) * t;

            float w0 = w->width();
            float w1 = m_oldGeo.width();
            float wt = (w0-w1) * t + w1;

            float h0 = w->height();
            float h1 = m_oldGeo.height();
            float ht = (h0 - h1) * t + h1;

            texture->bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            auto s = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture);
            QMatrix4x4 mvp = data.projectionMatrix();
            mvp.translate(p.x(), p.y());
            s->setUniform(GLShader::ModelViewProjectionMatrix, mvp);
            const qreal scale = 1;
            const QRectF rect = QRect(p, QSize(wt, ht));
            texture->render(rect.toRect(), scale);
            ShaderManager::instance()->popShader();
            texture->unbind();
            glDisable(GL_BLEND);

            return;
        }
        QRect r = m_newGeo.toRect();
        QRect r1 = m_oldGeo.toRect();
        QPoint p0 = r.topLeft();
        QPoint p1 = QPoint(r1.x(), r1.y());

        float w0 = r.width();
        float w1 = r1.width();
        float wt = w0 - (w0 - w1)*(1-t);
        float k = wt/w0;

        float h0 = r.height();
        float h1 = r1.height();
        float ht = h0 - (h0 - h1)*(1-t);
        float k1 = ht/h0;

        QPoint p = (p1-p0) * (1-t) + p0;
        d.setScale(QVector2D(k,k1));
        d += QPoint(qRound(p.x() - w->x()), qRound(p.y() - w->y()));
    }
    effects->paintWindow(w, mask, reg, d);
}

bool MaximizeEffect::isActive() const
{
    return m_activated && !effects->isScreenLocked();
}

void MaximizeEffect::setActive(bool active)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;

    if (m_activated == active)
        return;

    m_activated = active;

    if (active) {
        effects->startMouseInterception(this, Qt::ArrowCursor);
        m_hasKeyboardGrab = effects->grabKeyboard(this);
        effects->setActiveFullScreenEffect(this);
    } else {
        cleanup();
    }

    effects->addRepaintFull();
}

void MaximizeEffect::cleanup()
{
    if (m_activated)
        return;

    if (m_hasKeyboardGrab)
        effects->ungrabKeyboard();
    m_hasKeyboardGrab = false;
    effects->stopMouseInterception(this);
    effects->setActiveFullScreenEffect(nullptr);

}

void MaximizeEffect::slotWindowMaxiChanged(EffectWindow *window, QRectF oldG, QRectF newG, int mode)
{
    Window *w = window ? static_cast<EffectWindowImpl *>(window)->window() : nullptr;
    if (!w || w->isInteractiveMove())
        return;

    m_animationTime.reset();
    m_maxiWin = window;
    m_oldGeo = oldG;
    m_newGeo = newG;
    m_mode = mode;
    setActive(true);
}

}
