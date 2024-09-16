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
#include "kwinutils.h"

#include <QLibrary>
#include <QGuiApplication>
#include <QDebug>
#include <QX11Info>
#include <QMargins>
#include <QDateTime>
#include <QQmlEngine>
#include <QQmlContext>
#include <QAbstractNativeEventFilter>

#include "workspace.h"
#include "x11window.h"
#include "unmanaged.h"

#include "composite.h"
#include "scripting/scripting.h"
#include "tabbox/tabbox.h"
// #include "../../../utils/common.h"
#include "utils/xcbutils.h"

#include "xdgshellwindow.h"

// 为了访问 KWinEffects 的保护成员变量
#define protected public
#include <kwineffects.h>
#undef protected

#include <xcb/xcb.h>
#include <xcb/shape.h>

#include <functional>

#include "workspace.h"
#include "core/output.h"

using namespace KWin;

QHash<QObject *, QObject *> KWinUtils::waylandChameleonClients;

static inline bool isPlatformX11()
{
    static bool x11 = QX11Info::isPlatformX11();
    return x11;
}

static xcb_atom_t internAtom(const char *name, bool only_if_exists)
{
    if (!name || *name == 0)
        return XCB_NONE;

    if (!isPlatformX11())
        return XCB_NONE;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(QX11Info::connection(), only_if_exists, strlen(name), name);
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(QX11Info::connection(), cookie, 0);

    if (!reply)
        return XCB_NONE;

    xcb_atom_t atom = reply->atom;
    free(reply);

    return atom;
}

static QByteArray atomName(xcb_atom_t atom)
{
    if (!atom)
        return QByteArray();

    if (!isPlatformX11())
        return QByteArray();

    xcb_generic_error_t *error = 0;
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(QX11Info::connection(), atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(QX11Info::connection(), cookie, &error);
    if (error) {
        // qWarning() << "atomName: bad Atom" << atom;
        free(error);
    }
    if (reply) {
        QByteArray result(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
        free(reply);
        return result;
    }
    return QByteArray();
}

static QByteArray windowProperty(xcb_window_t WId, xcb_atom_t propAtom, xcb_atom_t typeAtom)
{
    if (!isPlatformX11())
        return QByteArray();

    QByteArray data;
    xcb_connection_t* xcb_connection = QX11Info::connection();
    int offset = 0;
    int remaining = 0;

    do {
        xcb_get_property_cookie_t cookie = xcb_get_property(xcb_connection, false, WId,
                                                            propAtom, typeAtom, offset, 1024);
        xcb_get_property_reply_t *reply = xcb_get_property_reply(xcb_connection, cookie, NULL);
        if (!reply)
            break;

        remaining = 0;

        if (reply->type == typeAtom) {
            int len = xcb_get_property_value_length(reply);
            char *datas = (char *)xcb_get_property_value(reply);
            data.append(datas, len);
            remaining = reply->bytes_after;
            offset += len;
        }

        free(reply);
    } while (remaining > 0);

    return data;
}

static void setWindowProperty(xcb_window_t WId, xcb_atom_t propAtom, xcb_atom_t typeAtom, int format, const QByteArray &data)
{
    if (!isPlatformX11())
        return;

    xcb_connection_t* conn = QX11Info::connection();

    if (format == 0 && data.isEmpty()) {
        xcb_delete_property(conn, WId, propAtom);
    } else {
        xcb_change_property(conn, XCB_PROP_MODE_REPLACE, WId, propAtom, typeAtom, format, data.length() * 8 / format, data.constData());
    }
}

static xcb_window_t getParentWindow(xcb_window_t WId)
{
    if (!isPlatformX11())
        return XCB_NONE;

    xcb_connection_t* xcb_connection = QX11Info::connection();
    xcb_query_tree_cookie_t cookie = xcb_query_tree_unchecked(xcb_connection, WId);
    xcb_query_tree_reply_t *reply = xcb_query_tree_reply(xcb_connection, cookie, NULL);

    if (reply) {
        xcb_window_t parent = reply->parent;

        free(reply);

        return parent;
    }

    return XCB_WINDOW_NONE;
}

class KWinInterface
{
    typedef QObject *(*WorkspaceFindShellClient)(void *, quint32);

public:
    KWinInterface()
    {
    }

    WorkspaceFindShellClient findWaylandWindow{nullptr};
};

Q_GLOBAL_STATIC(KWinInterface, interface)

class KWinUtilsPrivate : public QAbstractNativeEventFilter
{
public:
    KWinUtilsPrivate(KWinUtils *utils)
        : q(utils)
    {
        if (isPlatformX11()) {
            _NET_SUPPORTED = internAtom("_NET_SUPPORTED", false);
        }
    }

    static bool isShapeNotifyEvent(int type)
    {
        return Xcb::Extensions::self()->shapeNotifyEvent() == type;
    }

    void updateWMSupported() {
        if (!isPlatformX11())
            return;

        if (wmSupportedList.isEmpty() && removedWMSupportedList.isEmpty()) {
            return;
        }

        QByteArray net_wm_atoms = windowProperty(QX11Info::appRootWindow(), _NET_SUPPORTED, XCB_ATOM_ATOM);
        QVector<xcb_atom_t> atom_list;

        atom_list.resize(net_wm_atoms.size() / sizeof(xcb_atom_t));
        memcpy(atom_list.data(), net_wm_atoms.constData(), net_wm_atoms.size());

        if (!removedWMSupportedList.isEmpty()) {
            bool removed = false;

            for (xcb_atom_t atom : removedWMSupportedList) {
                if (atom_list.removeAll(atom) > 0) {
                    removed = true;
                }
            }

            if (removed) {
                xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_REPLACE, QX11Info::appRootWindow(),
                                    _NET_SUPPORTED, XCB_ATOM_ATOM, 32, atom_list.size(), atom_list.constData());
            }

            removedWMSupportedList.clear();
        }

        QVector<xcb_atom_t> new_atoms;

        for (xcb_atom_t atom : wmSupportedList) {
            if (!atom_list.contains(atom)) {
                new_atoms.append(atom);;
            }
        }

        if (!new_atoms.isEmpty()) {
            xcb_change_property(QX11Info::connection(), XCB_PROP_MODE_APPEND, QX11Info::appRootWindow(),
                                _NET_SUPPORTED, XCB_ATOM_ATOM, 32, new_atoms.size(), new_atoms.constData());
        }
    }

    void _d_onPropertyChanged(long atom) {
        if (atom != _NET_SUPPORTED)
            return;

        quint64 current_time = QDateTime::currentMSecsSinceEpoch();

        // 防止被短时间内多次调用形成死循环
        if (current_time - lastUpdateTime < 500) {
            lastUpdateTime = current_time;
            return;
        } else {
            lastUpdateTime = current_time;
        }

        updateWMSupported();
    }

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override {
        Q_UNUSED(eventType)
        Q_UNUSED(result)

        xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
        uint response_type = event->response_type & ~0x80;

        if (!isPlatformX11())
            return false;

        if (response_type == XCB_PROPERTY_NOTIFY) {
            xcb_property_notify_event_t *ev = reinterpret_cast<xcb_property_notify_event_t*>(event);

            if (Q_UNLIKELY(monitorProperties.contains(ev->atom))) {
                Q_EMIT q->windowPropertyChanged(ev->window, ev->atom);
            }

            if (Q_LIKELY(monitorRootWindowProperty)) {
                static long root = QX11Info::appRootWindow();

                if (Q_UNLIKELY(ev->window == root)) {
                    _d_onPropertyChanged(ev->atom);
                }
            }
        } else if (isShapeNotifyEvent(response_type)) {
            xcb_window_t window = reinterpret_cast<xcb_shape_notify_event_t*>(event)->affected_window;

            Q_EMIT q->windowShapeChanged(window);
        } else if (response_type == XCB_CLIENT_MESSAGE) {
            xcb_client_message_event_t *message = reinterpret_cast<xcb_client_message_event_t*>(event);

            static xcb_atom_t WM_PROTOCOLS = ::internAtom("WM_PROTOCOLS", false);
            static xcb_atom_t _NET_WM_PING = ::internAtom("_NET_WM_PING", false);

            if (message->type == WM_PROTOCOLS && message->data.data32[0] == _NET_WM_PING) {
                xcb_timestamp_t timestamp = message->data.data32[1];
                xcb_window_t window = message->data.data32[2];

                Q_EMIT q->pingEvent(window, timestamp);
            }
        }

        return false;
    }

    void ensureInstallFilter() {
        if (filterInstalled)
            return;

        filterInstalled = true;
        qApp->installNativeEventFilter(this);
    }

    void maybeRemoveFilter() {
        if (!filterInstalled)
            return;

        if (monitorProperties.isEmpty()) {
            filterInstalled = false;
            qApp->removeNativeEventFilter(this);
        }
    }

    KWinUtils *q;

    QList<xcb_atom_t> wmSupportedList;
    QList<xcb_atom_t> removedWMSupportedList;
    QSet<xcb_atom_t> monitorProperties;
    xcb_atom_t _NET_SUPPORTED;
    qint64 lastUpdateTime = 0;
    bool initialized = false;
    bool filterInstalled = false;
    bool monitorRootWindowProperty = false;
};

KWinUtils::KWinUtils(QObject *parent)
    : QObject(parent)
    , d(new KWinUtilsPrivate(this))
{
    if (QObject *ws = workspace()) {
        // 无法从workspace对象获取事件时，则从时间过滤器中取

        /*if (!connect(ws, SIGNAL(propertyNotify(long)), this, SLOT(_d_onPropertyChanged(long)))) {
            d->monitorRootWindowProperty = true;

            // qDebug() << "KWinUtils: Ignore the 'propertyNotify' signal connect warning";
        }*/
    }
}

void KWinUtils::setInitialized()
{
    if (d->initialized)
        return;

    d->initialized = true;

    Q_EMIT initialized();
}

KWinUtils::~KWinUtils()
{

}

KWinUtils *KWinUtils::instance()
{
    static KWinUtils *self = new KWinUtils();

    return self;
}

QObject *KWinUtils::findObjectByClassName(const QByteArray &name, const QObjectList &list)
{
    for (QObject *obj : list) {
        if (obj->metaObject()->className() == name) {
            return obj;
        }
    }

    return nullptr;
}

KWin::Workspace *KWinUtils::workspace()
{
    return KWin::Workspace::self();
}

QObject *KWinUtils::compositor()
{
    return KWin::Compositor::self();
}

QObject *KWinUtils::scripting()
{
    return KWin::Scripting::self();
}

void KWinUtils::scriptingRegisterObject(const QString& name, QObject* o)
{
    if (scripting()) {
        auto engine = scripting()->findChild<QQmlEngine*>();
        if (engine) {
            engine->rootContext()->setContextProperty(name, o);
        }
    }
}

QObject *KWinUtils::tabBox()
{
    return workspace()->tabbox();
}

QObject *KWinUtils::virtualDesktop()
{
    if (!workspace())
        return nullptr;

    return findObjectByClassName("KWin::VirtualDesktopManager", workspace()->children());
}

QObjectList KWinUtils::clientList()
{
    if (!workspace())
        return {};

    QList<KWin::Window*> clients = workspace()->allClientList();
    QObjectList list;
    for (KWin::Window *c : clients) {
        list << c;
    }

    return list;
}

QObjectList KWinUtils::unmanagedList()
{
    QObjectList list;

    // 在查找函数中将所有Unmanaged对象保存起来
    auto get_all = [&list] (const KWin::Unmanaged *unmanaged) {
        list << reinterpret_cast<QObject*>(const_cast<KWin::Unmanaged*>(unmanaged));
        return false;
    };

    Workspace::self()->findUnmanaged(get_all);

    return list;
}

QObject *KWinUtils::findClient(KWinUtils::Predicate predicate, quint32 window)
{
    if (!workspace())
        return nullptr;
    if (!QX11Info::isPlatformX11()) {
        QObject *shellClient = interface->findWaylandWindow(workspace(), window);
        if (shellClient) {
            return shellClient;
        }
    }

    return Workspace::self()->findClient(KWin::Predicate(static_cast<int>(predicate)), window);
}

bool KWinUtils::isShowSplitMenu()
{
    if (!workspace())
        return false;

    return true;
}

void KWinUtils::showSplitMenu(const QRect &rect, uint32_t client_id)
{
    workspace()->showSplitMenu(rect, client_id);
}

void KWinUtils::hideSplitMenu(bool delay)
{
    workspace()->hideSplitMenu(delay);
}

void KWinUtils::setSplitMenuKeepShowing(bool keep)
{
    workspace()->setSplitMenuKeepShowing(keep);
}

QObject *KWinUtils::findUnmanaged(quint32 window)
{
    return Workspace::self()->findUnmanaged(window);
}

void KWinUtils::clientCheckNoBorder(QObject *client)
{
    if (QX11Info::isPlatformX11()) {
        KWin::X11Window* x11Client = dynamic_cast<KWin::X11Window*>(client);
        x11Client->checkNoBorder();
    }
}

bool KWinUtils::sendPingToWindow(quint32 WId, quint32 timestamp)
{
    //Workspace::self()->sendPingToWindow(WId, timestamp);
    return true;
}

bool KWinUtils::sendPingToWindow(QObject *client, quint32 timestamp)
{
    bool ok = false;
    xcb_window_t wid = getWindowId(client, &ok);

    if (!ok) {
        return false;
    }

    return sendPingToWindow(wid, timestamp);
}

qulonglong KWinUtils::getWindowId(const QObject *client, bool *ok)
{
    // kwin class: Toplevel
    return client->property("windowId").toLongLong(ok);
}

int KWinUtils::getWindowDepth(const QObject *client)
{
    bool ok = false;
    xcb_window_t win_id = getWindowId(client, &ok);

    if (!ok)
        return 0;

    if (!isPlatformX11())
        return 0;

    xcb_get_geometry_cookie_t cookit = xcb_get_geometry(QX11Info::connection(), win_id);
    xcb_generic_error_t *error = nullptr;
    xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(QX11Info::connection(), cookit, &error);

    if (error) {
        return 0;
    }

    int depth = reply->depth;
    free(reply);

    return depth;
}

QByteArray KWinUtils::readWindowProperty(quint32 WId, quint32 atom, quint32 type)
{
    return windowProperty(WId, atom, type);
}

QByteArray KWinUtils::readWindowProperty(const QObject *client, quint32 atom, quint32 type)
{
    bool ok = false;
    xcb_window_t wid = getWindowId(client, &ok);

    if (!ok) {
        return QByteArray();
    }

    return windowProperty(wid, atom, type);
}

void KWinUtils::setWindowProperty(quint32 WId, quint32 atom, quint32 type, int format, const QByteArray &data)
{
    ::setWindowProperty(WId, atom, type, format, data);
}

void KWinUtils::setWindowProperty(const QObject *client, quint32 atom, quint32 type, int format, const QByteArray &data)
{
    bool ok = false;
    xcb_window_t wid = getWindowId(client, &ok);

    if (!ok) {
        return;
    }

    ::setWindowProperty(wid, atom, type, format, data);
}

uint KWinUtils::virtualDesktopCount()
{
    if (virtualDesktop()) {
        return virtualDesktop()->property("count").toUInt();
    }

    return 0;
}

uint KWinUtils::currentVirtualDesktop()
{
    if (virtualDesktop()) {
        return virtualDesktop()->property("current").toUInt();
    }

    return 0;
}

bool KWinUtils::compositorIsActive()
{
    QObject *c = compositor();

    if (!c)
        return false;

    QObject *c_dbus = findObjectByClassName("KWin::CompositorDBusInterface", c->children());

    if (!c_dbus) {
        return QX11Info::isCompositingManagerRunning();
    }

    return c_dbus->property("active").toBool();
}

quint32 KWinUtils::internAtom(const QByteArray &name, bool only_if_exists)
{
    return ::internAtom(name.constData(), only_if_exists);
}

quint32 KWinUtils::getXcbAtom(const QString &name, bool only_if_exists) const
{
    return internAtom(name.toLatin1(), only_if_exists);
}

bool KWinUtils::isSupportedAtom(quint32 atom) const
{
    if (atom == XCB_ATOM_NONE) {
        return false;
    }

    static xcb_atom_t _NET_SUPPORTED = internAtom("_NET_SUPPORTED", true);

    if (_NET_SUPPORTED == XCB_ATOM_NONE) {
        return false;
    }

    const QByteArray &datas = windowProperty(QX11Info::appRootWindow(), _NET_SUPPORTED, XCB_ATOM_ATOM);
    const xcb_atom_t *atoms = reinterpret_cast<const xcb_atom_t*>(datas.constData());

    for (int i = 0; i < datas.size() / (int)sizeof(xcb_atom_t); ++i ) {
        if (atoms[i] == atom) {
            return true;
        }
    }

    return false;
}

QVariant KWinUtils::getGtkFrame(const QObject *window) const
{
    if (!window) {
        return QVariant();
    }

    bool ok = false;
    qulonglong wid = getWindowId(window, &ok);

    if (!ok) {
        return QVariant();
    }

    static xcb_atom_t property_atom = internAtom("_GTK_FRAME_EXTENTS", true);

    if (property_atom == XCB_ATOM_NONE) {
        return QVariant();
    }

    const QByteArray &data = windowProperty(wid, property_atom, XCB_ATOM_CARDINAL);

    if (data.size() != 4 * 4)
        return QVariant();

    const int32_t *datas = reinterpret_cast<const int32_t*>(data.constData()); // left right top bottom
    QVariantMap frame_margins {
        {"left", datas[0]},
        {"right", datas[1]},
        {"top", datas[2]},
        {"bottom", datas[3]}
    };

    return frame_margins;
}

bool KWinUtils::isDeepinOverride(const QObject *window) const
{
    bool ok = false;
    qulonglong wid;
    QByteArray data;

    if (!isPlatformX11()) {
        return false;
    }

    static xcb_atom_t property_atom = internAtom("_DEEPIN_OVERRIDE", true);
    if (property_atom == XCB_ATOM_NONE) {
        goto out;
    }

    if (!window) {
        goto out;
    }

    wid = getWindowId(window, &ok);
    if (!ok) {
        goto out;
    }

    data = windowProperty(wid, property_atom, XCB_ATOM_CARDINAL);
    if (data.size() != 4)
        goto out;

    return *reinterpret_cast<const int32_t*>(data.constData()) == 1;

out:
    return false;
}

QVariant KWinUtils::getParentWindow(const QObject *window) const
{
    bool ok = false;
    if (!isPlatformX11()) {
        return QVariant();
    }

    qulonglong wid = getWindowId(window, &ok);

    if (!ok) {
        return QVariant();
    }

    return ::getParentWindow(wid);
}

void KWinUtils::setDarkTheme(bool isDark)
{
    KWin::Workspace *ws = static_cast<KWin::Workspace *>(workspace());
    if (ws) {
        ws->setDarkTheme(isDark);
    }
}

void KWinUtils::addSupportedProperty(quint32 atom, bool enforce)
{
    if (d->wmSupportedList.contains(atom))
        return;

    d->wmSupportedList.append(atom);

    if (enforce) {
        d->updateWMSupported();
    }
}

void KWinUtils::removeSupportedProperty(quint32 atom, bool enforce)
{
    d->wmSupportedList.removeOne(atom);
    d->removedWMSupportedList.append(atom);

    if (enforce) {
        d->updateWMSupported();
    }
}

void KWinUtils::addWindowPropertyMonitor(quint32 property_atom)
{
    d->monitorProperties.insert(property_atom);
    d->ensureInstallFilter();
}

void KWinUtils::removeWindowPropertyMonitor(quint32 property_atom)
{
    d->monitorProperties.remove(property_atom);
    d->maybeRemoveFilter();
}

bool KWinUtils::isCompositing()
{
    return compositorIsActive();
}

bool KWinUtils::buildNativeSettings(QObject *baseObject, quint32 windowID)
{
    static QFunctionPointer build_function = qApp->platformFunction("_d_buildNativeSettings");

    if (!build_function) {
        return false;
    }

    return reinterpret_cast<bool(*)(QObject*, quint32)>(build_function)(baseObject, windowID);
}

bool KWinUtils::isInitialized() const
{
    return d->initialized;
}

void KWinUtils::WalkThroughWindows()
{
    KWin::TabBox::TabBox *tabbox = static_cast<KWin::TabBox::TabBox *>(tabBox());
    if (tabbox) {
        tabbox->slotWalkThroughWindows();
    }
}

void KWinUtils::WalkBackThroughWindows()
{
    KWin::TabBox::TabBox *tabbox = static_cast<KWin::TabBox::TabBox *>(tabBox());
    if (tabbox) {
        tabbox->slotWalkBackThroughWindows();
    }
}

void KWinUtils::WindowMove()
{
    KWin::Workspace *ws = static_cast<KWin::Workspace *>(workspace());
    if (ws) {
        ws->slotWindowMove();
    }
}

void KWinUtils::WindowMaximize()
{
    KWin::Workspace *ws = static_cast<KWin::Workspace *>(workspace());
    if (ws) {
        ws->slotWindowMaximize();
    }
}

void KWinUtils::ShowWorkspacesView()
{

}

void KWinUtils::ShowAllWindowsView()
{

}

void KWinUtils::ShowWindowsView()
{

}

void KWinUtils::Window::setQuikTileMode(QObject *window, int mode, int m, bool isShowReview)
{
    KWin::Window *c = dynamic_cast<KWin::Window*>(window);
}

bool KWinUtils::Window::checkSupportFourSplit(QObject *window)
{
    return false;
}

void KWinUtils::Window::setTitleBarHeight(QObject *window, int titleBarHeight)
{
    KWin::Window *c = dynamic_cast<KWin::Window*>(window);
}

KWaylandServer::DDEShellSurfaceInterface *KWinUtils::getDDEShellSurface(QObject * shellClient)
{
    if (!shellClient) {
        return nullptr;
    }

    KWin::Window* c = dynamic_cast<KWin::Window*>(shellClient);
    return Workspace::self()->getDDEShellSurface(c);
}

void KWinUtils::insertChameleon(QObject *decorationClient, QObject *client)
{
    if (decorationClient) {
        waylandChameleonClients.insert(decorationClient, client);
    }
}

QObject *KWinUtils::findObjectByDecorationClient(QObject *decorationClient)
{
    auto it = waylandChameleonClients.find(decorationClient);
    if (it != waylandChameleonClients.end()) {
        return it.value();
    }
    return nullptr;
}
#include "moc_kwinutils.cpp"
