/*
 * Copyright (C) 2017 ~ 2019 Lingmo Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@lingmo.com>
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
#ifndef CHAMELEONCONFIG_H
#define CHAMELEONCONFIG_H

#include <QObject>

#include <kwineffects.h>
#include <window.h>

// 标记窗口当前是否正在使用变色龙窗口标题栏主题
#define _LINGMO_CHAMELEON "_LINGMO_CHAMELEON_THEME"
// 如果这个属性值为1，则表示此窗口的标题栏高度为0, 会隐藏标题栏
#define _LINGMO_NO_TITLEBAR "_LINGMO_NO_TITLEBAR"
// 强制对窗口开启边框修饰，对kde override类型的窗口，将会去除override标记。对unmanaged类型的窗口不生效
#define _LINGMO_FORCE_DECORATE "_LINGMO_FORCE_DECORATE"
// 设置窗口的裁剪区域，数据内容为QPainterPath导入到QDataStream
#define _LINGMO_SCISSOR_WINDOW "_LINGMO_SCISSOR_WINDOW"
// kwin窗口阴影属性，在没有窗口修饰器的窗口上会使用此属性绘制窗口阴影
#define _KDE_NET_WM_SHADOW "_KDE_NET_WM_SHADOW"
#define _NET_WM_WINDOW_TYPE "_NET_WM_WINDOW_TYPE"

namespace KWin {
class X11Window;
class Unmanaged;
class InternalWindow;
class EffectWindow;
class WaylandWindow;
class Window;
}

class X11Shadow;
class ChameleonConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool activated  READ isActivated NOTIFY activatedChanged)
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

public:
    enum EffectDataRole {
        BaseRole = KWin::DataRole::WindowForceBackgroundContrastRole + 100,
        WindowRadiusRole = BaseRole + 1,
        WindowClipPathRole = BaseRole + 2,
        WindowMaskTextureRole = BaseRole + 3,
        ShadowMaskRole = KWin::DataRole::WindowForceBackgroundContrastRole + 201,
        ShadowOffsetRole
    };

    enum ShadowCacheType {
        ActiveType,
        InactiveType,
        UnmanagedType,
        ShadowCacheTypeCount
    };

    static ChameleonConfig *instance();

    quint32 atomLingmoChameleon() const;
    quint32 atomLingmoNoTitlebar() const;
    quint32 atomLingmoScissorWindow() const;

    bool isActivated() const;

    QString theme() const;

public Q_SLOTS:
    bool setTheme(QString theme);

Q_SIGNALS:
    void activatedChanged(bool activated);
    void themeChanged(QString theme);
    void windowNoTitlebarPropertyChanged(quint32 windowId);
    void windowForceDecoratePropertyChanged(quint32 windowId);
    void windowScissorWindowPropertyChanged(quint32 windowId);
    void windowTypeChanged(QObject *window);
    void titlebarHeightChanged();

protected:
    explicit ChameleonConfig(QObject *parent = nullptr);

private Q_SLOTS:
    void onConfigChanged();
    void onClientAoceand(KWin::Window *client);
    // 针对X11BypassWindowManagerHint类型的窗口需要做一些特殊处理
    void onUnmanagedAoceand(KWin::Unmanaged *client);
    void onInternalWindowAoceand(KWin::InternalWindow *client);
    void onCompositingToggled(bool active);
    void onWindowPropertyChanged(quint32 windowId, quint32 atom);
    void onWindowDataChanged(KWin::EffectWindow *window, int role);

    void updateWindowNoBorderProperty(QObject *window);
    void updateWindowBlurArea(KWin::EffectWindow *window, int role);
    void updateWindowSize();
    void updateClientNoBorder(QObject *client, bool allowReset = true);
    void updateClientClipPath(QObject *client);

    // 用于调试窗口启动速度
    void debugWindowStartupTime(QObject *toplevel);
    void onToplevelDamaged(KWin::Window* toplevel, const QRect& damage);

    void onShellClientAoceand(KWin::WaylandWindow *client);
    void updateWindowRadius();
    void updateTitlebarHeight(const QString& type);

private:
    void init();

    void setActivated(const bool active);
    void clearKWinX11ShadowForWindows();
    void clearX11ShadowCache();
    // 处理所有额外支持的窗口属性，比如_LINGMO_SCISSOR_WINDOW、_LINGMO_FORCE_DECORATE
    // 第二个参数表示这个调用是否由属性变化引起
    void enforceWindowProperties(QObject *client);
    void enforcePropertiesForWindows(bool enable);
    bool setWindowOverrideType(QObject *client, bool enable);
    QString DConfigDecorationReplyPath();
    void updateTitlebarHeightPrivate();

    bool m_activated = false;
    QString m_theme;

    quint32 m_atom_lingmo_chameleon;
    quint32 m_atom_lingmo_no_titlebar;
    quint32 m_atom_lingmo_force_decorate;
    quint32 m_atom_lingmo_scissor_window;
    quint32 m_atom_kde_net_wm_shadow;
    quint32 m_atom_net_wm_window_type;

    QMap<QString, X11Shadow*> m_x11ShadowCache;
    QHash<QObject*, quint32> m_pendingWindows;

public:
    static qreal m_titlebarHeight;
};

#endif // CHAMELEONCONFIG_H
