/*
 * Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     zjq <zhaojunqing@uniontech.com>
 *
 * Maintainer: zjq <zhaojunqing@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <kwineffects.h>
#include <map>

#include <QRect>

namespace KWin
{

#define DURATION_FLYING_BACK 300

#define DURATION_FLYING_BACK1 500

#define DURATION_WORKSPACE_SWITCH 300

// Translate and scale a rect along line
// mainly used for switching workspace and dragging window
class MultiTaskEffectLinearMove
{
public:
    MultiTaskEffectLinearMove(const QRect& from, const QRect& to) {
        m_status = -1;

        m_timeLine.setEasingCurve(QEasingCurve::OutQuint);
        m_timeLine.setDuration(std::chrono::milliseconds(DURATION_WORKSPACE_SWITCH));
        m_timeLine.reset();

        m_fromRect = from;
        m_toRect = to;
    }

    ~MultiTaskEffectLinearMove() { reset(); }

    void reset() {
        m_status = -1;
        m_timeLine.reset();
        m_fromRect = m_toRect = QRect(0,0,1,1);
    }

    void setSourceAndDesitation(const QRect& from, const QRect& to) {
        m_fromRect = from;
        m_toRect = to;
    }
    void setSource(const QRect& from) {
        m_fromRect = from;
    }
    void setDesitation(const QRect& to) {
        m_toRect = to;
    }

    void begin() {
        m_status = 0;
        m_timeLine.reset();
    }

    void update(int time) {
        if (m_status == 0) {
            m_timeLine.advance(std::chrono::milliseconds(time));
            m_status = 1;
        } else if (m_status == 1) {
            m_timeLine.advance(std::chrono::milliseconds(time));
            if (m_timeLine.done())
                m_status = 2;
        } else {

        }
    }

    QRect getCurrent() {
        float t = m_timeLine.value();

        float x0 = m_fromRect.x();
        float y0 = m_fromRect.y();
        float x1 = m_toRect.x();
        float y1 = m_toRect.y();

        float w0 = m_fromRect.width();
        float h0 = m_fromRect.height();
        float w1 = m_toRect.width();
        float h1 = m_toRect.height();

        float x = interp(x0, x1, t);
        float y = interp(y0, y1, t);
        float w = interp(w0, w1, t);
        float h = interp(h0, h1, t);

        return QRect(int(x), int(y), int(w), int(h));
    }

    bool animating() {
        return m_status == 1 || m_status == 0;
    }

    bool done() {
        if (m_status == 2 || m_status == -1)
            return true;
        else
            return false;
    }

private:

    float interp(float v0, float v1, float t) {
        return (v1 - v0) * t + v0;
    }

private:

    TimeLine m_timeLine;

    int m_status;  // -1:reset  0:start,  1:doing   2:end

    QRect m_fromRect, m_toRect;
};


class MultiTaskEffectFlyingBack
{

public:
    MultiTaskEffectFlyingBack() {
        m_status = -1;

        m_timeLine.setEasingCurve(QEasingCurve::OutQuint);
        m_timeLine.setDuration(std::chrono::milliseconds(DURATION_FLYING_BACK));
        m_timeLine.reset();

        m_why = -1;
        m_selectedWindow = nullptr;
    }

    ~MultiTaskEffectFlyingBack() {
        reset();
    }

    void reset() {
        m_status = -1;

        m_timeLine.reset();

        m_why = -1;
        m_selectedWindow = nullptr;
        m_winMotionMgr.clear();
    }

    void add(EffectWindow* w, QRect p) {
        m_winMotionMgr[w] = p;
    }

    void because(int why) {
        m_why = why;
    }
    void setSelecedWindow(EffectWindow* selWin) {
        m_selectedWindow = selWin;
    }

    void begin() {
        m_status = 0;
        m_timeLine.reset();
    }

    bool animating() {
        return m_status == 1 || m_status == 0;
    }


    void update(std::chrono::milliseconds presentTime, bool motionEffectEnable) {
        if (m_status == 0) {
            if (motionEffectEnable) {
                m_timeLine.advance(presentTime);
            } else {
                m_timeLine.setElapsed(m_timeLine.duration());
            }
            m_status = 1;
        } else if (m_status == 1) {
            if (m_timeLine.done()) {
                m_status = 2;
            }
            m_timeLine.advance(presentTime);
        }
    }

    bool done() {
        if (m_status == 2) {
            return true;
        } else {
            return false;
        }
    }

    void end() {
        reset();
    }

    int getStatus() {
        return m_status;
    }

    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) {
        if (m_status == 1) {
            if (w->isDesktop() || w->isDock()) {
                effects->paintWindow(w, mask, region, data);
                return;
            } else {
                //QPoint p0 = m_winMotionMgr[w];
                //QPoint p1 = QPoint(w->x(), w->y());

                float t = m_timeLine.value();

                QRect r = m_winMotionMgr[w];
                QPoint p0 = m_winMotionMgr[w].topLeft();
                QPoint p1 = QPoint(w->x(), w->y());

                float w0 = r.width();
                float w1 = w->width();
                float wt = (w1-w0) * t + w0;
                float k = wt/w1;
                QPoint p = (p0-p1) * (1-t);

                data.setScale(QVector2D(k,k));
                data.translate(p.x(), p.y());

                effects->paintWindow(w, mask, region, data);
            }
        }
    }

private:
    std::map<EffectWindow*, QRect> m_winMotionMgr;

    TimeLine m_timeLine;

    int m_status;  // -1:reset  0:start,  1:doing   2:end

    EffectWindow* m_selectedWindow;

    /*
     * what cause this effect
     * 0: Esc pressed, or mouse hit blank area
     * 1: mouse hitted one window
     */
    int m_why;
};

class MultiTaskWindowEffect
{
public:
   MultiTaskWindowEffect() {
        m_status = -1;

        m_timeLine.setEasingCurve(QEasingCurve::OutQuint);
        m_timeLine.setDuration(std::chrono::milliseconds(DURATION_FLYING_BACK1));
        m_timeLine.reset();
    }
    MultiTaskWindowEffect(const QEasingCurve &type, int duration) {
        m_status = -1;

        m_timeLine.setEasingCurve(type);
        m_timeLine.setDuration(std::chrono::milliseconds(duration));
        m_timeLine.reset();
    }
    ~MultiTaskWindowEffect() {}

    void reset() {
        m_status = -1;

        m_timeLine.reset();

        m_winMotionMgr.clear();
    }

    void add(EffectWindow* w, QRect from, QRect to) {
        if (m_winMotionMgr.find(w) == m_winMotionMgr.end())
            m_winMotionMgr[w] = RectInpl{from, to};
        else
            m_winMotionMgr[w].r1 = to;
    }

    void begin() {
        m_status = 0;
        m_timeLine.reset();
    }

    void update(std::chrono::milliseconds presentTime, bool motionEffectEnable) {
        if (m_status == 0) {
            m_timeLine.reset();
            if (motionEffectEnable) {
                m_timeLine.setElapsed(std::chrono::milliseconds(10));
            } else {
                m_timeLine.setElapsed(m_timeLine.duration());
            }
            m_status = 1;
        } else if (m_status == 1) {
            if (m_timeLine.done())
                m_status = 2;
            m_timeLine.advance(presentTime);
        } else {
        }
    }

    void paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data) {
        if (m_status > 0) {
            if (0 == m_winMotionMgr.count(w))
                return;

            float t = m_timeLine.value();
            RectInpl rtpl = m_winMotionMgr[w];
            QRect cur = rtpl.get(t);

            float k = 1.0f * cur.width() / w->width();
            QPoint p = cur.topLeft() - w->pos().toPoint();

            data.setScale(QVector2D(k,k));
            data.translate(p.x(), p.y());
            effects->paintWindow(w, mask, region, data);
        }
    }

    int getStatus() {
        return m_status;
    }

    struct RectInpl {
        QRect r0, r1;
        QRect get(float t) {
            float x0 = r0.x();
            float y0 = r0.y();
            float x1 = r1.x();
            float y1 = r1.y();

            float w0 = r0.width();
            float h0 = r0.height();
            float w1 = r1.width();
            float h1 = r1.height();

            float x = interp(x0, x1, t);
            float y = interp(y0, y1, t);
            float w = interp(w0, w1, t);
            float h = interp(h0, h1, t);

            return QRect(int(x), int(y), int(w), int(h));
        }

        float interp(float v0, float v1, float t) {
            return (v1 - v0) * t + v0;
        }
    };

private:
    std::map<EffectWindow*, RectInpl> m_winMotionMgr;
    TimeLine m_timeLine;
    int m_status;  // -1:reset  0:start,  1:doing   2:end
};

}
