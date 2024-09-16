/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2021 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWINEFFECTSEX_H
#define KWINEFFECTSEX_H

#include <kwineffects.h>
#include <kwinconfig.h>
#include <kwineffects_export.h>
#include <kwinglobals.h>

/**
 * Logging category to be used inside the KWin effects.
 * Do not use in this library.
 **/
//Q_DECLARE_LOGGING_CATEGORY(KWINEFFECTS)

namespace KWin
{

class EffectWindow;
class Output;

#define KWIN_EFFECT_API_MAKE_VERSION( major, minor ) (( major ) << 8 | ( minor ))
#define KWIN_EFFECT_API_VERSION_MAJOR 0
#define KWIN_EFFECT_API_VERSION_MINOR 227
#define KWIN_EFFECT_API_VERSION KWIN_EFFECT_API_MAKE_VERSION( \
        KWIN_EFFECT_API_VERSION_MAJOR, KWIN_EFFECT_API_VERSION_MINOR )

class KWINEFFECTS_EXPORT EffectFrameEx : public EffectFrame
{
    // Q_OBJECT
public:
    explicit EffectFrameEx();
    virtual ~EffectFrameEx();

    virtual void renderPixmap(const QRegion &region = infiniteRegion(), double opacity = 1.0) = 0;

    virtual void  setColor(const QColor &color) = 0;
    virtual const QColor &color() const = 0;
    virtual void setRadius(int radius) = 0;
    virtual const int &radius() = 0;
    virtual void setImage(const QUrl &image) = 0;
    virtual void setImage(const QPixmap &image) = 0;
    virtual const QUrl &image() const = 0;
    virtual void setPixmap(const QPixmap &image) = 0;
    virtual const QPixmap &pixmap() const = 0;
    virtual const QPoint &clipOffset() const = 0;
    virtual void setClipOffset(const QPoint &offset) = 0;

// Q_SIGNALS:

};

class KWINEFFECTS_EXPORT EffectsHandlerEx : public EffectsHandler
{
    Q_OBJECT
public:
    explicit EffectsHandlerEx(CompositingType type);
    virtual ~EffectsHandlerEx();

    virtual void setQuickTileWindow(KWin::EffectWindow *w, int mode) = 0;
    virtual int getQuickTileMode(KWin::EffectWindow *w) = 0;
    virtual QRectF getQuickTileGeometry(KWin::EffectWindow *w, int mode, QPointF pos) = 0;
    virtual QString getActiveColor() = 0;
    virtual void setKeepAbove(KWin::EffectWindow *c, bool) = 0;
    virtual void sendPointer(const QPointF &pos, Qt::MouseButton) = 0;
    virtual EffectWindowList getChildWinList(KWin::EffectWindow *w) = 0;
    virtual bool isTransientWin(KWin::EffectWindow *w) = 0;
    virtual bool isShortcuts(QKeyEvent *event) = 0;

    virtual std::unique_ptr<EffectFrameEx> effectFrameEx(QString url, bool staticSize = true, 
                                                     const QPoint &position = QPoint(-1, -1),
                                                     Qt::Alignment alignment = Qt::AlignCenter) const = 0;
    virtual EffectScreen *findScreen(Output *output) const = 0;
    virtual bool isWinAllowSplit(KWin::EffectWindow *) = 0;
    virtual bool isSplitWin(KWin::EffectWindow *) = 0;
    virtual void updateQuickTileMode(KWin::EffectWindow *, int mode) = 0;
    virtual void updateWindowTile(KWin::EffectScreen *) = 0;
    virtual float getOsRadius() = 0;
    virtual float getOsScale() = 0;
    virtual EffectType effectType() const = 0;

    static const QSet<QString> motionEffectList;

    virtual Output *getCurrentPaintingScreen() = 0;


Q_SIGNALS:
    void triggerSplitPreview(KWin::EffectWindow *w);
    void swapSplitWin(KWin::EffectWindow *w, int index);

    void showMultitasking();
    void windowMinimizedAnimation(KWin::EffectWindow *w, QPointF initialPos);
    void unmanagedAddedAnimation(KWin::EffectWindow *w, QPoint point);

};

/**
 * Pointer to the global EffectsHandlerEx object.
 **/
extern KWINEFFECTS_EXPORT EffectsHandlerEx* effectsEx;

} // namespace

#endif // KWINEFFECTSEX_H
