// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DXCBWMSUPPORT_H
#define DXCBWMSUPPORT_H

#include "global.h"

#include <QObject>
#include <QVector>

#include <xcb/xcb.h>

#include <functional>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DXcbWMSupport : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool hasBlurWindow READ hasBlurWindow NOTIFY hasBlurWindowChanged)
    Q_PROPERTY(bool hasComposite READ hasComposite NOTIFY hasCompositeChanged)
    // 属性值只有可能在窗管是否支持混成时发生改变
    Q_PROPERTY(bool hasWindowAlpha READ hasWindowAlpha FINAL)
    // 支持隐藏窗口标题栏
    Q_PROPERTY(bool hasNoTitlebar READ hasNoTitlebar NOTIFY hasNoTitlebarChanged)
    // 支持窗口内容裁剪
    Q_PROPERTY(bool hasScissorWindow READ hasScissorWindow NOTIFY hasScissorWindowChanged)
    // 支持窗口壁纸绘制
    Q_PROPERTY(bool hasWallpaperEffect READ hasWallpaperEffect NOTIFY hasWallpaperEffectChanged)

public:
    enum {
        MWM_HINTS_FUNCTIONS   = (1L << 0),

        MWM_FUNC_ALL      = (1L << 0),
        MWM_FUNC_RESIZE   = (1L << 1),
        MWM_FUNC_MOVE     = (1L << 2),
        MWM_FUNC_MINIMIZE = (1L << 3),
        MWM_FUNC_MAXIMIZE = (1L << 4),
        MWM_FUNC_CLOSE    = (1L << 5),

        MWM_HINTS_DECORATIONS = (1L << 1),

        MWM_DECOR_ALL      = (1L << 0),
        MWM_DECOR_BORDER   = (1L << 1),
        MWM_DECOR_RESIZEH  = (1L << 2),
        MWM_DECOR_TITLE    = (1L << 3),
        MWM_DECOR_MENU     = (1L << 4),
        MWM_DECOR_MINIMIZE = (1L << 5),
        MWM_DECOR_MAXIMIZE = (1L << 6),

        MWM_HINTS_INPUT_MODE = (1L << 2),

        MWM_INPUT_MODELESS                  = 0L,
        MWM_INPUT_PRIMARY_APPLICATION_MODAL = 1L,
        MWM_INPUT_FULL_APPLICATION_MODAL    = 3L
    };

    static DXcbWMSupport *instance();

    struct Global {
        static bool hasBlurWindow();
        static bool hasComposite();
        static bool hasNoTitlebar();
        static bool hasWindowAlpha();
        static bool hasWallpaperEffect();
        static QString windowManagerName();
    };

    static bool connectWindowManagerChangedSignal(QObject *object, std::function<void()> slot);
    static bool connectHasBlurWindowChanged(QObject *object, std::function<void()> slot);
    static bool connectHasCompositeChanged(QObject *object, std::function<void()> slot);
    static bool connectHasNoTitlebarChanged(QObject *object, std::function<void()> slot);
    static bool connectWindowListChanged(QObject *object, std::function<void()> slot);
    static bool connectHasWallpaperEffectChanged(QObject *object, std::function<void()> slot);
    static bool connectWindowMotifWMHintsChanged(QObject *object, std::function<void(quint32 winId)> slot);

    static void setMWMFunctions(quint32 winId, quint32 func);
    static quint32 getMWMFunctions(quint32 winId);
    static void setMWMDecorations(quint32 windId, quint32 decor);
    static quint32 getMWMDecorations(quint32 winId);

    static void popupSystemWindowMenu(quint32 winId);

    bool isDeepinWM() const;
    bool isKwin() const;
    bool isSupportedByWM(xcb_atom_t atom) const;
    bool isContainsForRootWindow(xcb_atom_t atom) const;
    bool hasBlurWindow() const;
    bool hasComposite() const;
    bool hasNoTitlebar() const;
    bool hasScissorWindow() const;
    bool hasWindowAlpha() const;
    bool hasWallpaperEffect() const;

    QString windowManagerName() const;

    QVector<xcb_window_t> allWindow() const;
    xcb_window_t windowFromPoint(const QPoint &p) const;

signals:
    void windowManagerChanged();
    void hasBlurWindowChanged(bool hasBlurWindow);
    void hasCompositeChanged(bool hasComposite);
    void hasNoTitlebarChanged(bool hasNoTitlebar);
    void hasScissorWindowChanged(bool hasScissorWindow);
    void hasWallpaperEffectChanged(bool hasWallpaperEffect);
    void windowListChanged();
    void windowMotifWMHintsChanged(quint32 winId);
    void wallpaperSharedChanged();

protected:
    explicit DXcbWMSupport();

private:
    void updateWMName(bool emitSignal = true);
    void updateNetWMAtoms();
    void updateRootWindowProperties();
    void updateHasBlurWindow();
    void updateHasComposite();
    void updateHasNoTitlebar();
    void updateHasScissorWindow();
    void updateWallpaperEffect();

    qint8 getHasWindowAlpha() const;

    static quint32 getRealWinId(quint32 winId);

    bool m_isDeepinWM = false;
    bool m_isKwin = false;
    bool m_hasBlurWindow = false;
    bool m_hasComposite = false;
    bool m_hasNoTitlebar = false;
    bool m_hasScissorWindow = false;
    bool m_hasWallpaperEffect = false;
    qint8 m_windowHasAlpha = -1;

    QString m_wmName;

    xcb_atom_t _net_wm_deepin_blur_region_rounded_atom = 0;
    xcb_atom_t _kde_net_wm_blur_rehind_region_atom = 0;
    xcb_atom_t _net_wm_deepin_blur_region_mask = 0;
    xcb_atom_t _deepin_wallpaper = 0;
    xcb_atom_t _deepin_wallpaper_shared_key = 0;
    xcb_atom_t _deepin_no_titlebar = 0;
    xcb_atom_t _deepin_scissor_window = 0;

    QVector<xcb_atom_t> net_wm_atoms;
    QVector<xcb_atom_t> root_window_properties;

    friend class XcbNativeEventFilter;
    friend class Utility;
    friend class DBackingStoreProxy;
};

DPP_END_NAMESPACE

#endif // DXCBWMSUPPORT_H
