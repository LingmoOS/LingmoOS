/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef KWINUTILS_H
#define KWINUTILS_H

#include <QObject>
#include <QVariant>

namespace KWin
{
    class Workspace;
}

namespace KWaylandServer
{
    class DDEShellSurfaceInterface;
}

class KWinUtilsPrivate;
class Q_DECL_EXPORT KWinUtils : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ isInitialized)

public:
    void setInitialized();
    enum MaximizeMode {
        MaximizeRestore    = 0, ///< The window is not maximized in any direction.
        MaximizeVertical   = 1, ///< The window is maximized vertically.
        MaximizeHorizontal = 2, ///< The window is maximized horizontally.
        /// Equal to @p MaximizeVertical | @p MaximizeHorizontal
        MaximizeFull = MaximizeVertical | MaximizeHorizontal
    };

    enum class Predicate {
        WindowMatch,
        WrapperIdMatch,
        FrameIdMatch,
        InputIdMatch
    };

    ~KWinUtils();
    static void insertChameleon(QObject *decorationClient, QObject *client);
    static QObject *findObjectByDecorationClient(QObject *decorationClient);
    static KWinUtils *instance();
    static QObject *findObjectByClassName(const QByteArray &name, const QObjectList &list);

    static int kwinBuildVersion();

    static KWin::Workspace *workspace();
    static QObject *compositor();
    static QObject *scripting();
    static void scriptingRegisterObject(const QString& name, QObject* o);
    static QObject *tabBox();
    static QObject *cursor();
    static QObject *virtualDesktop();

    static QObjectList clientList();
    static QObjectList unmanagedList();
    static QObject *findClient(Predicate predicate, quint32 window);
    static QObject *findUnmanaged(quint32 window);
    static KWaylandServer::DDEShellSurfaceInterface *getDDEShellSurface(QObject * shellClient);
    static void clientCheckNoBorder(QObject *client);
    static bool sendPingToWindow(quint32 WId, quint32 timestamp);
    static bool sendPingToWindow(QObject *client, quint32 timestamp);

    static void setDarkTheme(bool isDark);

    static qulonglong getWindowId(const QObject *client, bool *ok = nullptr);
    static int getWindowDepth(const QObject *client);
    static QByteArray readWindowProperty(quint32 WId, quint32 atom, quint32 type);
    static QByteArray readWindowProperty(const QObject *client, quint32 atom, quint32 type);
    static void setWindowProperty(quint32 WId, quint32 atom, quint32 type, int format, const QByteArray &data);
    static void setWindowProperty(const QObject *client, quint32 atom, quint32 type, int format, const QByteArray &data);

    static uint virtualDesktopCount();
    static uint currentVirtualDesktop();

    static bool compositorIsActive();

    struct Window {
        static void setQuikTileMode(QObject* window, int mode, int m, bool isShowReview = false);
        static bool checkSupportFourSplit(QObject *window);
        static void setTitleBarHeight(QObject *window, int titleBarHeight);
    };

    static quint32 internAtom(const QByteArray &name, bool only_if_exists);
    static bool isShowSplitMenu();

    static void showSplitMenu(const QRect &button_rect, uint32_t client_id);
    static void hideSplitMenu(bool delay);
    static void setSplitMenuKeepShowing(bool keep);

    Q_INVOKABLE quint32 getXcbAtom(const QString &name, bool only_if_exists) const;
    Q_INVOKABLE bool isSupportedAtom(quint32 atom) const;
    Q_INVOKABLE QVariant getGtkFrame(const QObject *window) const;
    Q_INVOKABLE bool isDeepinOverride(const QObject *window) const;

    Q_INVOKABLE QVariant getParentWindow(const QObject *window) const;

    // enforce为false时表示只把属性加入到待添加列表，但是不设置_NET_SUPPORTED属性
    Q_INVOKABLE void addSupportedProperty(quint32 atom, bool enforce = true);
    // enforce为false时表示只把属性标记为待删除，但是不设置_NET_SUPPORTED属性
    Q_INVOKABLE void removeSupportedProperty(quint32 atom, bool enforce = true);

    Q_INVOKABLE void addWindowPropertyMonitor(quint32 property_atom);
    Q_INVOKABLE void removeWindowPropertyMonitor(quint32 property_atom);

    Q_INVOKABLE bool isCompositing();

    // Warning: 调用 buildNativeSettings，会导致baseObject的QMetaObject对象被更改
    // 无法使用QMetaObject::cast，不能使用QObject::findChild等接口查找子类，也不能使用qobject_cast转换对象指针类型
    Q_INVOKABLE bool buildNativeSettings(QObject *baseObject, quint32 windowID);

    bool isInitialized() const;

public Q_SLOTS:
    void WalkThroughWindows();
    void WalkBackThroughWindows();
    void WindowMove();
    void WindowMaximize();
    void ShowWorkspacesView();
    void ShowAllWindowsView();
    void ShowWindowsView();

Q_SIGNALS:
    void initialized();
    void windowPropertyChanged(quint32 windowId, quint32 property_atom);
    void windowShapeChanged(quint32 windowId);
    void pingEvent(quint32 windowId, quint32 timestamp);

protected:
    explicit KWinUtils(QObject *parent = nullptr);

    KWinUtilsPrivate *d;

private:

    friend class Mischievous;
    static QHash<QObject *, QObject *> waylandChameleonClients;
    Q_PRIVATE_SLOT(d, void _d_onPropertyChanged(long))
};

#endif // KWINUTILS_H
