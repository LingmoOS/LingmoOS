/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "splitprevieweffect.h"
#include "../utils/common.h"
#include "workspace.h"
#include <QWindow>
#include <QKeyEvent>
#include <QtDBus>

#define BRIGHTNESS          0.4
#define FIRST_WIN_SCALE     (float)(720.0 / 1080.0)
#define SPACING_H           (float)(20.0 / 1080.0)
#define SPACING_W           (float)(20.0 / 1920.0)

#define DBUS_IMAGEEFFECT_SERVICE "org.deepin.dde.ImageBlur1"
#define DBUS_BLUR_OBJ "/org/deepin/dde/ImageBlur1"
#define DBUS_BLUR_INTF "org.deepin.dde.ImageBlur1"

namespace KWin
{
SplitPreviewEffect::SplitPreviewEffect()
    : lastPresentTime(std::chrono::milliseconds::zero())
{
    connect(effectsEx, &EffectsHandlerEx::triggerSplitPreview, this, &SplitPreviewEffect::toggle);
    connect(effects, &EffectsHandler::windowFrameGeometryChanged, this, &SplitPreviewEffect::slotWindowGeometryChanged);
}

SplitPreviewEffect::~SplitPreviewEffect()
{
}

void SplitPreviewEffect::prePaintScreen(ScreenPrePaintData &data, std::chrono::milliseconds presentTime)
{
    int time;
    if (lastPresentTime.count()) {
        time = std::max(1, int((presentTime - lastPresentTime).count()));
    } else {
        time = 1;
    }
    lastPresentTime = presentTime;
    if (isActive()) {
        data.mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS;
        for (auto& mm: m_motionManagers) {
            mm.calculate(time / 2.0);
        }
    }

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant(true));
    }

    effects->prePaintScreen(data, presentTime);
}

void SplitPreviewEffect::paintScreen(int mask, const QRegion &region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
}

void SplitPreviewEffect::postPaintScreen()
{
    if (m_activated)
        effects->addRepaintFull();

    for (auto const& w: effects->stackingOrder()) {
        w->setData(WindowForceBlurRole, QVariant());
    }
    effects->postPaintScreen();
}

void SplitPreviewEffect::prePaintWindow(EffectWindow *w, WindowPrePaintData &data, std::chrono::milliseconds presentTime)
{
    data.mask |= PAINT_WINDOW_TRANSFORMED;

    effects->prePaintWindow(w, data, presentTime);
}

void SplitPreviewEffect::paintWindow(EffectWindow *w, int mask, QRegion region, WindowPaintData &data)
{
    if (!isActive()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (w->isWaterMark()) {
        effects->paintWindow(w, mask, region, data);
        return;
    }

    if (m_unPreviewWin.contains(w))
        return;

    int desktop = effects->currentDesktop();
    WindowMotionManager& wmm = m_motionManagers[0];
    if (wmm.isManaging(w) || w->isDesktop()) {
        auto area = effects->clientArea(FullArea, w);
        QRegion reg(area.toRect());
        WindowPaintData d = data;
        if (w->isDesktop()) {
            auto m_bkBlurShader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture);
            const QRectF rect = scaledRect(w->frameGeometry(), effects->renderTargetScale());
            QMatrix4x4 mvp(data.projectionMatrix());
            mvp.translate(rect.x(), rect.y());
            m_bkBlurShader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            QString screenName = w->screen()->name();
            if (!m_bgTextures[screenName]) {
                d.setBrightness(BRIGHTNESS);
                effects->paintWindow(w, mask, reg, d);
                return ;
            }
            m_bgTextures[screenName]->bind();
            m_bgTextures[screenName]->render(infiniteRegion(), w->frameGeometry().toRect(), effects->renderTargetScale());
            m_bgTextures[screenName]->unbind();
            glDisable(GL_BLEND);
            ShaderManager::instance()->popShader();
        } else if (!w->isDesktop()) {
            auto geo = m_motionManagers[0].transformedGeometry(w);
            d += QPoint(qRound(geo.x() - w->x()), qRound(geo.y() - w->y()));
            d.setScale(QVector2D((float)(geo.width() / w->width()), (float)(geo.height() / w->height())));
            effects->paintWindow(w, mask, reg, d);

            if (m_hoverwin == w) {
                m_effectFrame->setGeometry(geo.adjusted(-4, -4, 4, 4).toRect());
                m_effectFrame->setRadius(m_radius);
                QColor color(effectsEx->getActiveColor());
                m_effectFrame->setColor(color);
                m_effectFrame->render(region);
            }
        }
    } else {
        if (w->isDock() || isRelevantWithPresentWindows(w))
            effects->paintWindow(w, mask, region, data);
    }
}

bool SplitPreviewEffect::isActive() const
{
    return m_activated && !effects->isScreenLocked();
}

static QString toRealPath(const QString &path)
{
    // QString res = path;
    QString res = QUrl::fromPercentEncoding(path.toUtf8());
    if (res.startsWith("file:///")) {
        res.remove("file://");
    }

    QFileInfo fi(res);
    if (fi.isSymLink()) {
        res = fi.symLinkTarget();
    }
    return res;
}

void SplitPreviewEffect::toggle(KWin::EffectWindow *w)
{
    if (!effectsEx->getQuickTileMode(w))
        return;
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;
    m_window = w;
    EffectWindowList windows = effects->stackingOrder();
    int currentDesktop = effects->currentDesktop();
    WindowMotionManager wmm;
    for (const auto& w: windows) {
        if (w->isOnDesktop(currentDesktop) && isRelevantWithPresentWindows(w)) {
            if (w == m_window)
                continue;
            if (!effectsEx->isWinAllowSplit(w)) {
                m_unPreviewWin.append(w);
                continue;
            }
            if (w->isMinimized()) {
                w->refVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
            }
            wmm.manage(w);
        }
    }
    m_backgroundRect = getPreviewWindowsGeometry(w);
    m_screenRect = effects->clientArea(ScreenArea, w->screen(), currentDesktop);

    m_radius = effectsEx->getOsRadius() ? 10.0 : 0.0;

    if (wmm.managedWindows().size() != 0) {
        calculateWindowTransformations(wmm.managedWindows(), wmm);
        m_motionManagers.append(wmm);
        setActive(true);
    }
}

void SplitPreviewEffect::initTextureMask()
{
    for (Output *output : workspace()->outputs()) {
        EffectScreen *effectScreen = effectsEx->findScreen(output);
        QString screenName = effectScreen->name();
        QString backgroundUrl;
        QDBusInterface wmInterface("com.deepin.wm", "/com/deepin/wm", "com.deepin.wm");
        QDBusReply<QString> getReply = wmInterface.call("GetCurrentWorkspaceBackgroundForMonitor", screenName);
        if (!getReply.value().isEmpty()) {
            backgroundUrl = getReply.value();
        } else {
            m_bgTextures[screenName] = nullptr;
            continue;
        }
        backgroundUrl = toRealPath(backgroundUrl);

        QDBusInterface imageBlurInterface(DBUS_IMAGEEFFECT_SERVICE, DBUS_BLUR_OBJ, DBUS_BLUR_INTF, QDBusConnection::systemBus());
        imageBlurInterface.setTimeout(100);
        QDBusReply<QString> blurReply = imageBlurInterface.call("Get", backgroundUrl);
        QString imageUrl;
        if (!blurReply.value().isEmpty()) {
            imageUrl = blurReply.value();
        } else {
            m_bgTextures[screenName] = nullptr;
            continue;
        }

        effects->makeOpenGLContextCurrent();
        m_bgTextures[screenName] = new GLTexture(imageUrl);
        m_bgTextures[screenName]->setFilter(GL_LINEAR);
        m_bgTextures[screenName]->setWrapMode(GL_CLAMP_TO_EDGE);
    }
}

void SplitPreviewEffect::setActive(bool active)
{
    if (effects->activeFullScreenEffect() && effects->activeFullScreenEffect() != this)
        return;

    if (m_activated == active)
        return;

    m_activated = active;

    QDBusMessage message = QDBusMessage::createSignal("/KWin", "org.kde.KWin", "SplitScreenStateChanged");
    message << bool(m_activated);
    QDBusConnection::sessionBus().send(message);

    if (active) {
        if (!m_effectFrame) {
            m_effectFrame = effectsEx->effectFrameEx("deepin-kwin/effects/splitscreen/qml/main.qml", false);
        }
        effects->startMouseInterception(this, Qt::PointingHandCursor);
        m_hasKeyboardGrab = effects->grabKeyboard(this);
        effects->setActiveFullScreenEffect(this);
        initTextureMask();
    } else {
        cleanup();
    }

    effects->addRepaintFull();
}

void SplitPreviewEffect::cleanup()
{
    if (m_activated)
        return;

    if (m_hasKeyboardGrab)
        effects->ungrabKeyboard();
    m_hasKeyboardGrab = false;
    effects->stopMouseInterception(this);
    effects->setActiveFullScreenEffect(nullptr);
    lastPresentTime = std::chrono::milliseconds::zero();
    int currentDesktop = effects->currentDesktop();
    while (m_motionManagers.size() > 0) {
        for (const auto& w: m_motionManagers.first().managedWindows()) {
            if (w->isOnDesktop(currentDesktop) && isRelevantWithPresentWindows(w)) {
                if (w->isMinimized()) {
                    w->unrefVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
                }
            }
        }
        m_motionManagers.first().unmanageAll();
        m_motionManagers.removeFirst();
    }
    m_unPreviewWin.clear();

    m_effectFrame = nullptr;
    for (auto itr = m_bgTextures.begin(); itr != m_bgTextures.end(); itr ++) {
        delete itr->second;
    }
    m_bgTextures.clear();
}

QRect SplitPreviewEffect::getPreviewWindowsGeometry(EffectWindow *w)
{
    int mode = effectsEx->getQuickTileMode(w);
    QRectF ret = effectsEx->getQuickTileGeometry(w, mode^0b11, effects->cursorPos());
    m_backgroundMode = mode ^ 0b11;
    return ret.toRect();
}

void SplitPreviewEffect::windowInputMouseEvent(QEvent* e)
{
    if (!m_activated)
        return;

    switch (e->type()) {
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
            break;
        default:
            return;
    }

    auto me = static_cast<QMouseEvent*>(e);

    EffectWindow* target = nullptr;
    WindowMotionManager& wm = m_motionManagers[0];
    for (const auto& w : wm.managedWindows()) {
        auto geo = wm.transformedGeometry(w);
        if (geo.contains(me->pos())) {
            target = w;
            break;
        }
    }

    switch (me->type()) {
        case QEvent::MouseMove:
            if (target) {
                m_hoverwin = target;
            } else {
                m_hoverwin = nullptr;
            }
            break;
        case QEvent::MouseButtonPress:
            if (target) {
                effects->addRepaintFull();
            }
            break;
        case QEvent::MouseButtonRelease:
            if (target) {
                if (target->isMinimized()) {
                    target->unrefVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
                }
                effects->defineCursor(Qt::PointingHandCursor);
                effects->activateWindow(target);
                effectsEx->setQuickTileWindow(target, m_backgroundMode);
            }
            setActive(false);
            break;
        default:
            return;
    }
}

void SplitPreviewEffect::grabbedKeyboardEvent(QKeyEvent *e)
{
    if (e->type() == QEvent::KeyPress) {
        switch (e->key()) {
        case Qt::Key_Escape:
            setActive(false);
            break;
        }
    }
}

void SplitPreviewEffect::slotWindowGeometryChanged(EffectWindow *window, const QRectF &geometry)
{
    if (isActive() && effects->waylandDisplay() && effectsEx->getQuickTileMode(window)) {
        auto area = effects->clientArea(MaximizeArea, window);
        QRegion r = QRegion(area.toRect()).subtracted(QRegion(window->frameGeometry().toRect()));
        m_backgroundRect = r.boundingRect();
        relayout();
    }
}

void SplitPreviewEffect::relayout()
{
    if (m_inhibitCount != 0) {
        return;
    }
    inhibit();
    if (m_motionManagers.size()) {
        WindowMotionManager& wm = m_motionManagers[0];
        calculateWindowTransformations(wm.managedWindows(), wm);
    }
    uninhibit();
}

bool SplitPreviewEffect::isRelevantWithPresentWindows(EffectWindow *w) const
{
    if (w->isSpecialWindow() || w->isUtility()) {
        return false;
    }

    if (w->isDock()) {
        return false;
    }

    if (w->isSkipSwitcher()) {
        return false;
    }

    if (w->isDeleted()) {
        return false;
    }

    if (!w->acceptsFocus()) {
        return false;
    }

    if (!w->isCurrentTab()) {
        return false;
    }

    if (!w->isOnCurrentActivity()) {
        return false;
    }

    return true;
}

void SplitPreviewEffect::calculateWindowTransformations(EffectWindowList windows, WindowMotionManager& wmm)
{
    if (windows.size() == 0)
        return;

    calculateWindowTransformationsClosest(windows, wmm);
}


void SplitPreviewEffect::calculateWindowTransformationsClosest(EffectWindowList windowlist, WindowMotionManager& motionManager)
{
    QHash<EffectWindow*, QRect> targets;
    for (auto &w : windowlist) {
        QRect rect = w->clientGeometry().toRect();
        targets[w] = rect;
    }

    QRect clientRect = m_backgroundRect;
    float scaleHeight = clientRect.height() * FIRST_WIN_SCALE;
    int minSpacingH = SPACING_H * m_screenRect.height();
    int minSpacingW = SPACING_W * m_screenRect.width();
    int totalw = minSpacingW;
    QList<int> centerList;
    int row = 1;
    int index = 1;
    int xpos = 0;
    bool overlap;
    do {
        overlap = false;
        for (int i = windowlist.size() - 1; i >= 0; i--) {
            EffectWindow *w = windowlist[i];
            if (!motionManager.isManaging(w))
                continue;
            QRect *target = &targets[w];
            float width = target->width();
            if (target->height() > scaleHeight) {
                float scale = (float)(scaleHeight / target->height());
                width = target->width() * scale;
            }
            totalw += width;
            totalw += minSpacingW;
            int singlew = width + minSpacingW * 2;
            if (singlew > clientRect.width())
                break;

            if (totalw > clientRect.width()) {
                index ++;
                if (index > row)
                    break;
                xpos = ((clientRect.width() - totalw + width + minSpacingW) / 2) + minSpacingW + clientRect.x();
                centerList.push_back(xpos);
                totalw = minSpacingW;
                totalw += width;
                totalw += minSpacingW;
            }
        }
        xpos = ((clientRect.width() - totalw) / 2) + minSpacingW + clientRect.x();
        centerList.push_back(xpos);

        if (totalw > clientRect.width()) {
            centerList.clear();
            overlap = true;
            scaleHeight -= 15;
            float critical = (float)(clientRect.height() - (row + 2) * minSpacingH) / (float)(row + 1);
            if (scaleHeight <= critical) {
                row++;
            }
            index = 1;
            totalw = minSpacingW;
        }
    } while (overlap);  //calculation layout row

    float winYPos = (clientRect.height() - (index - 1) * minSpacingH - index * scaleHeight) / 2 + clientRect.y();
    row = 1;
    int x = centerList.size() >= row ? centerList[row - 1] : 0;
    totalw = minSpacingW;
    for (int i = windowlist.size() - 1; i >= 0; i--) {
        EffectWindow *w = windowlist[i];
        if (!motionManager.isManaging(w))
            continue;

        QRect *target = &targets[w];
        float width = 0.0, height = 0.0;
        bool isFill = false;
        if (target->height() > scaleHeight) {
            float scale = (float)(scaleHeight / target->height());
            width = target->width() * scale;
            height = scaleHeight;
        } else {
            width = target->width();
            height = target->height();
            isFill = true;
        }
        totalw += width;
        totalw += minSpacingW;
        if (totalw > clientRect.width()) {
            row++;
            totalw = minSpacingW;
            totalw += width;
            totalw += minSpacingW;
            x = centerList.size() >= row ? centerList[row - 1] : 0;
            winYPos += minSpacingH;
            winYPos += scaleHeight;
        }

        target->setRect(x, winYPos + (scaleHeight - height) / 2, width, height);

        x += width;
        x += minSpacingW;

        motionManager.moveWindow(w, targets.value(w));
    }
}

bool SplitPreviewEffect::touchDown(qint32 id, const QPointF &pos, std::chrono::microseconds time)
{
    if (!m_activated)
        return false;
    m_targetTouchWindow = nullptr;
    WindowMotionManager& wm = m_motionManagers[0];
    for (const auto& w : wm.managedWindows()) {
        auto geo = wm.transformedGeometry(w);
        if (geo.contains(pos)) {
            m_targetTouchWindow = w;
            break;
        }
    }
    return true;
}

bool SplitPreviewEffect::touchUp(qint32 id, std::chrono::microseconds time)
{
    if (!m_activated)
        return false;
    if (m_targetTouchWindow) {
        if (m_targetTouchWindow->isMinimized()) {
            m_targetTouchWindow->unrefVisibleEx(EffectWindow::PAINT_DISABLED_BY_MINIMIZE);
        }
        effects->defineCursor(Qt::PointingHandCursor);
        effects->activateWindow(m_targetTouchWindow);
        effectsEx->setQuickTileWindow(m_targetTouchWindow, m_backgroundMode);
    }
    setActive(false);
    return true;
}

void SplitPreviewEffect::inhibit()
{
    m_inhibitCount++;
}

void SplitPreviewEffect::uninhibit()
{
    m_inhibitCount--;
}

}
